//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/private/ParameterTranslator.cpp $
//
// REVISION:        $Revision: 281815 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-24 18:07:41 -0400 (Mon, 24 Jun 2013) $
// UPDATED BY:      $Author: sstrick $

#include "ParameterTranslator.hpp"
#include "DPLCommon.hpp"
#include "XMLUtilities.hpp"
#include "ProxyUtilities.hpp"
#include "MVLogger.hpp"
#include "Environment.hpp"
#include "MVUtility.hpp"
#include "ShellExecutor.hpp"
#include "ContainerToString.hpp"
#include "DateTime.hpp"
#include "UniqueIdGenerator.hpp"
#include "LargeStringStream.hpp"
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/replace.hpp>

namespace
{
	const std::string TRANSLATED_NAME_ATTRIBUTE( "translatedName" );
	const std::string VALUE_TRANSLATOR_ATTRIBUTE( "valueTranslator" );
	const std::string VALUE_DEFAULT_ATTRIBUTE( "valueDefault" );
	const std::string VALUE_OVERRIDE_ATTRIBUTE( "valueOverride" );
	const std::string VALUE_SOURCE_ATTRIBUTE( "valueSource" );
	const std::string EVAL_TIMEOUT_ATTRIBUTE( "evalTimeout" );
	const std::string VALUE_SOURCE_MULTIPLE( "*" );
	const int EVAL_TIMEOUT_DEFAULT( 10 );
	boost::regex VARIABLE_NAME("\\$\\{.*?\\}");

	const std::string HOSTNAME = "hostname";
	const std::string INSTANCE = "instance";
	const std::string LOGGER_INSTANCE_NAME = "app.instance.name";	// what logger's Environment calls the instance
	const std::string DATE_TIME = "datetime";
	const std::string GUID = "guid";
	const std::string PID = "pid";
	const std::string UNKNOWN = "unknown";
	const std::string s_Hostname = MVUtility::GetHostName();

	bool IsExpression( const std::string& i_rValue )
	{
		return ( i_rValue[0] == '`' && i_rValue[i_rValue.size()-1] == '`' );
	}

	std::string GetExpression( const std::string& i_rValue )
	{
		return i_rValue.substr( 1, i_rValue.size() - 2 );
	}

	std::string EvalExpression( const std::string& i_rCommand, int i_Timeout )
	{
		std::large_stringstream standardOut;
		std::large_stringstream standardErr;
		ShellExecutor exe( i_rCommand );
		int status = exe.Run( i_Timeout, standardOut, standardErr );

		if( status != 0 )
		{
			MV_THROW( ParameterTranslatorException, "Evaluation command returned non-zero return code: " << status
				<< ". Command was: " << i_rCommand << ". Standard Error: " << standardErr.str() );
		}

		if( standardErr.str().size() > 0 )
		{
			MVLOGGER( "root.lib.DataProxy.ParameterTranslator.Eval.StandardError",
				"Command: " << i_rCommand << " generated output to standard error: " << standardErr.str() );
		}
		return standardOut.str();
	}

	bool IsBuiltIn( const std::string& i_rValue )
	{
		return ( i_rValue.size() > 1 && i_rValue[0] == '[' && i_rValue[i_rValue.size()-1] == ']' );
	}

	std::string GetBuiltIn( const std::string& i_rValue )
	{
		return i_rValue.substr( 1, i_rValue.size() - 2 );
	}

	std::string EvalBuiltIn( const std::string& i_rValue )
	{
		if( i_rValue == HOSTNAME )
		{
			return s_Hostname;
		}
		else if( i_rValue == INSTANCE )
		{
			if( Environment::DoesExist( LOGGER_INSTANCE_NAME ) )
			{
				return Environment::Get( LOGGER_INSTANCE_NAME );
			}
			else
			{
				MVLOGGER( "root.lib.DataProxy.ParameterTranslator.BuiltIn.InstanceUnknown",
					"Instance id requested, but is unknown to the environment" );
				return i_rValue + "-" + UNKNOWN;
			}
		}
		else if( i_rValue == GUID )
		{
			return UniqueIdGenerator().GetUniqueId();
		}
		else if( i_rValue == PID )
		{
			return boost::lexical_cast< std::string >( ::getpid() );
		}
		else if( i_rValue.substr( 0, DATE_TIME.length() ) == DATE_TIME )
		{
			std::string format = "%Y%m%dT%H%M%S";
			if( i_rValue.length() > DATE_TIME.length() + 1 )
			{
				format = i_rValue.substr( DATE_TIME.length() + 1 );
			}
			return DateTime().GetFormattedString( format );
		}
		else if( i_rValue == MD5 )
		{
			return std::string(""); 
		}

		// we should have returned by now; if not, return unknown
		MVLOGGER( "root.lib.DataProxy.ParameterTranslator.BuiltIn.Unknown",
			"Unknown built-in requested: " << i_rValue << "; using unknown" );
		return i_rValue + "-" + UNKNOWN;
	}

	void GetReferencedParameters( const std::string& i_rInput, std::set< std::string >& o_rParameters )
	{
		boost::sregex_iterator iter( i_rInput.begin(), i_rInput.end(), VARIABLE_NAME );
		boost::sregex_iterator end;
		for (; iter != end; ++iter)
		{
			//strip off the dollar-braces
			std::string parameter = iter->str();
			std::string strippedParameter = parameter.substr(2, parameter.size() - 3);
			o_rParameters.insert( strippedParameter );
		}
	}
}

ParameterTranslator::ParameterTranslator( const xercesc::DOMNode& i_rNode )
:	m_Parameters(),
	m_PrimaryDefaults(),
	m_SecondaryDefaults(),
	m_DerivedValues(),
	m_ShellTimeout( EVAL_TIMEOUT_DEFAULT ),
	m_DelayedEvaluationParameters()
{
	xercesc::DOMNode* pTranslateNode = XMLUtilities::TryGetSingletonChildByName( &i_rNode, TRANSLATE_PARAMETERS_NODE );
	if( pTranslateNode == NULL )
	{
		return;
	}

	std::set< std::string > allowedChildren;
	allowedChildren.insert( PARAMETER_NODE );
	XMLUtilities::ValidateNode( pTranslateNode, allowedChildren );
	std::set< std::string > allowedAttributes;
	allowedAttributes.insert( EVAL_TIMEOUT_ATTRIBUTE );
	XMLUtilities::ValidateAttributes( pTranslateNode, allowedAttributes );

	allowedAttributes.clear();
	allowedAttributes.insert( NAME_ATTRIBUTE );
	allowedAttributes.insert( TRANSLATED_NAME_ATTRIBUTE );
	allowedAttributes.insert( VALUE_TRANSLATOR_ATTRIBUTE );
	allowedAttributes.insert( VALUE_DEFAULT_ATTRIBUTE );
	allowedAttributes.insert( VALUE_OVERRIDE_ATTRIBUTE );
	allowedAttributes.insert( VALUE_SOURCE_ATTRIBUTE );

	xercesc::DOMAttr* pAttribute = XMLUtilities::GetAttribute( pTranslateNode, EVAL_TIMEOUT_ATTRIBUTE );
	if( pAttribute != NULL )
	{
		m_ShellTimeout = boost::lexical_cast<int>( XMLUtilities::XMLChToString(pAttribute->getValue()) );
	}

	std::vector<xercesc::DOMNode*> parameters;
	XMLUtilities::GetChildrenByName( parameters, pTranslateNode, PARAMETER_NODE );
	std::vector<xercesc::DOMNode*>::const_iterator paramIter = parameters.begin();
	for( ; paramIter != parameters.end(); ++paramIter )
	{
		TranslatorDatum datum;

		XMLUtilities::ValidateNode( *paramIter, std::set< std::string >() );
		XMLUtilities::ValidateAttributes( *paramIter, allowedAttributes );
		std::string name = XMLUtilities::GetAttributeValue( *paramIter, NAME_ATTRIBUTE );
		datum.SetValue< ParameterName >( name );

		// check to be sure valueSource isn't the only attribute
		if( XMLUtilities::GetAttribute( *paramIter, VALUE_SOURCE_ATTRIBUTE ) != NULL
		 && XMLUtilities::GetAttribute( *paramIter, TRANSLATED_NAME_ATTRIBUTE ) == NULL
		 && XMLUtilities::GetAttribute( *paramIter, VALUE_TRANSLATOR_ATTRIBUTE ) == NULL
		 && XMLUtilities::GetAttribute( *paramIter, VALUE_DEFAULT_ATTRIBUTE ) == NULL
		 && XMLUtilities::GetAttribute( *paramIter, VALUE_OVERRIDE_ATTRIBUTE ) == NULL )
		{
			MV_THROW( ParameterTranslatorException, "Parameters with a value source must have an override or translator supplied. Violating parameter: " << name );
		}
		
		Nullable< std::string > translateToName;
		pAttribute = XMLUtilities::GetAttribute( *paramIter, TRANSLATED_NAME_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			std::string translatedName = XMLUtilities::XMLChToString(pAttribute->getValue());
			// ensure an override or a default is not provided
			if( XMLUtilities::GetAttribute( *paramIter, VALUE_TRANSLATOR_ATTRIBUTE ) != NULL
			 || XMLUtilities::GetAttribute( *paramIter, VALUE_DEFAULT_ATTRIBUTE ) != NULL
			 || XMLUtilities::GetAttribute( *paramIter, VALUE_OVERRIDE_ATTRIBUTE ) != NULL )
			{
				MV_THROW( ParameterTranslatorException, "If translating the name of a parameter, cannot also provide a value manipulator. "
					<< "Violating parameter: " << name << ". "
					<< "Provide the value manipulation as part of the translatedName Parameter configuration: " << translatedName << "." );
			}
			if( XMLUtilities::GetAttribute( *paramIter, VALUE_SOURCE_ATTRIBUTE ) != NULL )
			{
				MV_THROW( ParameterTranslatorException, "Cannot provide a value source when translating a parameter name. "
					<< "Violating parameter: " << name );
			}
			datum.SetValue< TranslatedName >( translatedName );
		}
		
		pAttribute = XMLUtilities::GetAttribute( *paramIter, VALUE_TRANSLATOR_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			// ensure an override is not provided in addition
			if( XMLUtilities::GetAttribute( *paramIter, VALUE_OVERRIDE_ATTRIBUTE ) != NULL )
			{
				MV_THROW( ParameterTranslatorException, "Cannot provide a value override and a value translator. Violating parameter: " << name )
			}
			std::string valueTranslator = XMLUtilities::XMLChToString(pAttribute->getValue());

			// if it's a derived parameter, insert it into derived parameters & continue
			pAttribute = XMLUtilities::GetAttribute( *paramIter, VALUE_SOURCE_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				std::string valueSource = XMLUtilities::XMLChToString(pAttribute->getValue());
				DerivedValueDatum derivedDatum;
				derivedDatum.SetValue< ParameterName >( name );
				derivedDatum.SetValue< ValueSource >( valueSource );
				derivedDatum.SetValue< ValueDerivation >( valueTranslator );
				derivedDatum.SetValue< IsOverride >( false );
				m_DerivedValues.InsertUpdate( derivedDatum );

				// at this point, there still could be a default..
				pAttribute = XMLUtilities::GetAttribute( *paramIter, VALUE_DEFAULT_ATTRIBUTE );
				if( pAttribute != NULL )
				{
					// insert this as a secondary default, since it must be evaluated LAST
					m_SecondaryDefaults[ name ] = XMLUtilities::XMLChToString(pAttribute->getValue());
				}
				continue;
			}

			// set the translator
			datum.SetValue< ValueTranslator >( valueTranslator );
		}
		
		pAttribute = XMLUtilities::GetAttribute( *paramIter, VALUE_DEFAULT_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			// ensure an override is not provided in addition
			if( XMLUtilities::GetAttribute( *paramIter, VALUE_OVERRIDE_ATTRIBUTE ) != NULL )
			{
				MV_THROW( ParameterTranslatorException, "Cannot provide a value override and a value default. Violating parameter: " << name )
			}
			if( XMLUtilities::GetAttribute( *paramIter, VALUE_SOURCE_ATTRIBUTE ) != NULL )
			{
				MV_THROW( ParameterTranslatorException, "Cannot provide a value source and a value default. Violating parameter: " << name );
			}
			std::string valueDefault = XMLUtilities::XMLChToString(pAttribute->getValue());
			// set the default
			m_PrimaryDefaults[ name ] = valueDefault;
		}
		
		pAttribute = XMLUtilities::GetAttribute( *paramIter, VALUE_OVERRIDE_ATTRIBUTE );
		if( pAttribute != NULL )
		{
			std::string valueOverride = XMLUtilities::XMLChToString(pAttribute->getValue());

			// if it's a derived parameter, insert it into derived parameters & continue
			pAttribute = XMLUtilities::GetAttribute( *paramIter, VALUE_SOURCE_ATTRIBUTE );
			if( pAttribute != NULL )
			{
				std::string valueSource = XMLUtilities::XMLChToString(pAttribute->getValue());
				DerivedValueDatum derivedDatum;
				derivedDatum.SetValue< ParameterName >( name );
				derivedDatum.SetValue< ValueSource >( valueSource );
				derivedDatum.SetValue< ValueDerivation >( valueOverride );
				derivedDatum.SetValue< IsOverride >( true );
				m_DerivedValues.InsertUpdate( derivedDatum );
				continue;
			}

			// set this value as both the translator AND the default
			datum.SetValue< ValueTranslator >( valueOverride );
			m_PrimaryDefaults[ name ] = valueOverride;
		}

		if( m_Parameters.find( datum ) != m_Parameters.end() )
		{
			MV_THROW( ParameterTranslatorException, "Translation of parameter '" << name << "' is ambiguous" );
		}
		m_Parameters.InsertUpdate( datum );
	}

	// now iterate over our parameters and ensure there are no name-translation chains > 1 jump
	TranslatorContainer::const_iterator iter = m_Parameters.begin();
	for( ; iter != m_Parameters.end(); ++iter )
	{
		if( !iter->second.GetValue< TranslatedName >().IsNull() )
		{
			TranslatorDatum datum;
			datum.SetValue< ParameterName >( iter->second.GetValue< TranslatedName >() );
			TranslatorContainer::const_iterator findIter = m_Parameters.find( datum );
			if( findIter != m_Parameters.end() && !findIter->second.GetValue< TranslatedName >().IsNull() )
			{
				MV_THROW( ParameterTranslatorException, "Parameter name translations must not be chained. Violating chain: "
					<< iter->second.GetValue< ParameterName >() << "->" 
					<< iter->second.GetValue< TranslatedName >() << "->" 
					<< findIter->second.GetValue< TranslatedName >() );
			}
		}
	}

	// now iterate over our derived values and ensure none of them depend on each other
	DerivedValueContainer::const_iterator derivedIter = m_DerivedValues.begin();
	for( ; derivedIter != m_DerivedValues.end(); ++derivedIter )
	{
		DerivedValueDatum datum;
		std::set< std::string > sources;
		if( derivedIter->second.GetValue< ValueSource >() == VALUE_SOURCE_MULTIPLE )
		{
			GetReferencedParameters( derivedIter->second.GetValue< ValueDerivation >(), sources );
		}
		else
		{
			sources.insert( derivedIter->second.GetValue< ValueSource >() );
		}
		std::set< std::string >::const_iterator sourceIter = sources.begin();
		for( ; sourceIter != sources.end(); ++sourceIter )
		{
			datum.SetValue< ParameterName >( *sourceIter );
			DerivedValueContainer::const_iterator findIter = m_DerivedValues.find( datum );
			if( findIter != m_DerivedValues.end() )
			{
				std::string source = findIter->second.GetValue< ValueSource >();
				if( source == VALUE_SOURCE_MULTIPLE )
				{
					std::set< std::string > multiSource;
					GetReferencedParameters( findIter->second.GetValue< ValueDerivation >(), multiSource );
					source = OrderedContainerToString( multiSource );
				}
				MV_THROW( ParameterTranslatorException, "Cannot make values derive from other derived values. Violating parameters: "
					<< derivedIter->second.GetValue< ParameterName >() << " derives from " << findIter->second.GetValue< ParameterName >()
					<< ", which derives from " << source );
			}
		}
	}
}

ParameterTranslator::~ParameterTranslator()
{
}

void ParameterTranslator::Translate( const std::map<std::string,std::string>& i_rInputParameters,
									 std::map<std::string,std::string>& o_rTranslatedParameters )
{
	m_DelayedEvaluationParameters.clear();
	o_rTranslatedParameters.clear();

	// first, iterate over the incoming parameters & perform whatever translations are necessary
	std::map<std::string,std::string>::const_iterator inputIter = i_rInputParameters.begin();
	for( ; inputIter != i_rInputParameters.end(); ++inputIter )
	{
		TranslatorDatum datum;
		std::string name = inputIter->first;
		std::string value = inputIter->second;
		datum.SetValue< ParameterName >( name );
		TranslatorContainer::const_iterator findIter = m_Parameters.find( datum );

		// if we aren't configured to do anything with this parameter, write it to the output as is
		if( findIter == m_Parameters.end() )
		{
			o_rTranslatedParameters[ name ] = value;
			continue;
		}

		// get the translated name if it exists
		if( !findIter->second.GetValue< TranslatedName >().IsNull() )
		{
			name = findIter->second.GetValue< TranslatedName >();
			// first check to see if the incoming parameters actually HAS a parameter for the target name; if so log & skip it
			if( i_rInputParameters.find( name ) != i_rInputParameters.end() )
			{
				MVLOGGER( "root.lib.DataProxy.ParameterTranslator.IgnoringDuplicate",
					"Incoming parameter map has a value for both source parameter name: '" << findIter->second.GetValue< ParameterName >()
					<< "' and target translated parameter name: '" << name << "'. Ignoring the former since it is most likely obsolete." );
				continue;
			}
			// we have to reset the findIter to get any value translation that may occur on the target
			datum.SetValue< ParameterName >( name );
			findIter = m_Parameters.find( datum );
			// if there is none, we can simply insert it as is
			if( findIter == m_Parameters.end() )
			{
				o_rTranslatedParameters[ name ] = value;
				continue;
			}
		}
		// get the translated value if it exists
		if( !findIter->second.GetValue< ValueTranslator >().IsNull() )
		{
			std::string valueTranslator = findIter->second.GetValue< ValueTranslator >();
			// if it's an expression, evaluate it
			if( IsExpression( valueTranslator ) )
			{
				std::string cmd( GetExpression( valueTranslator ) );
				boost::replace_all( cmd, VALUE_FORMATTER, value );
				value = EvalExpression( cmd, m_ShellTimeout );
			}
			else if( IsBuiltIn( valueTranslator ) )
			{
				std::string builtIn( GetBuiltIn( valueTranslator ) );
				boost::replace_all( builtIn, VALUE_FORMATTER, value );
				value = EvalBuiltIn( builtIn );
				AddDelayedParameter( builtIn, name );
			}
			// otherwise use it as a literal
			else
			{
				std::string valueSubbedTranslator = boost::replace_all_copy( valueTranslator, VALUE_FORMATTER, value );
				value = valueSubbedTranslator;
			}
		}
		
		// as long as the name has not been silenced, we can insert this name/value pair, else remove the name if it has been added into m_DelayedEvaluationParameters
		if( !IsSilenced( name ) )
		{
			o_rTranslatedParameters[ name ] = value;
		}
		else
		{
			m_DelayedEvaluationParameters.erase( name );
		}
	}

	// now iterate over the primary defaults and fill them in where necessary
	for( inputIter = m_PrimaryDefaults.begin(); inputIter != m_PrimaryDefaults.end(); ++inputIter )
	{
		if( o_rTranslatedParameters.find(inputIter->first) == o_rTranslatedParameters.end() )
		{
			std::string valueDefault = inputIter->second;
			if( IsExpression( valueDefault ) )
			{
				valueDefault = EvalExpression( GetExpression( valueDefault ), m_ShellTimeout );
			}
			else if( IsBuiltIn( valueDefault ) )
			{
				std::string builtIn( GetBuiltIn( valueDefault ) );
				valueDefault = EvalBuiltIn( builtIn );
				AddDelayedParameter( builtIn, inputIter->first );
			}
			o_rTranslatedParameters[ inputIter->first ] = valueDefault;
		}
	}

	// now we can fill out our derived values
	DerivedValueContainer::const_iterator derivedIter = m_DerivedValues.begin();
	for( ; derivedIter != m_DerivedValues.end(); ++derivedIter )
	{
		std::string nameSource = derivedIter->second.GetValue< ValueSource >();
		std::string derivedValue;
		// if valueSource comes from multiple entities
		if( nameSource == VALUE_SOURCE_MULTIPLE )
		{
			derivedValue = ProxyUtilities::GetVariableSubstitutedString( derivedIter->second.GetValue< ValueDerivation >(), o_rTranslatedParameters );
		}
		else // valueSource is from a single source
		{
			std::map< std::string, std::string >::const_iterator findIter = o_rTranslatedParameters.find( nameSource );
			// if we can't pull the source, then skip it
			if( findIter == o_rTranslatedParameters.end() )
			{
				continue;
			}
			std::string sourceValue = findIter->second;
			derivedValue = derivedIter->second.GetValue< ValueDerivation >();
			boost::replace_all( derivedValue, VALUE_FORMATTER, sourceValue );
		}

		std::string builtIn("");

		// calculate the derived value from the source value
		if( IsExpression( derivedValue ) )
		{
			derivedValue = EvalExpression( GetExpression( derivedValue ), m_ShellTimeout );
		}
		else if( IsBuiltIn( derivedValue ) )
		{
			builtIn = GetBuiltIn( derivedValue );
			derivedValue = EvalBuiltIn( builtIn );
		}

		// if it's an override or it's currently missing from the output parameters, add it
		std::string paramName = derivedIter->second.GetValue< ParameterName >();
		if( derivedIter->second.GetValue< IsOverride >() || o_rTranslatedParameters.find( paramName ) == o_rTranslatedParameters.end() )
		{
			o_rTranslatedParameters[ paramName ] = derivedValue;
			AddDelayedParameter( builtIn, paramName );
		}
	}

	// finally, iterate over the secondary defaults and fill them in where necessary
	for( inputIter = m_SecondaryDefaults.begin(); inputIter != m_SecondaryDefaults.end(); ++inputIter )
	{
		if( o_rTranslatedParameters.find(inputIter->first) == o_rTranslatedParameters.end() )
		{
			std::string valueDefault = inputIter->second;
			if( IsExpression( valueDefault ) )
			{
				valueDefault = EvalExpression( GetExpression( valueDefault ), m_ShellTimeout );
			}
			else if( IsBuiltIn( valueDefault ) )
			{
				std::string builtIn = GetBuiltIn( valueDefault );
				valueDefault = EvalBuiltIn( builtIn );
				AddDelayedParameter( builtIn, inputIter->first );
			}
			o_rTranslatedParameters[ inputIter->first ] = valueDefault;
		}
	}
}

bool ParameterTranslator::IsSilenced( const std::string& i_rName ) const
{
	TranslatorDatum datum;
	datum.SetValue< ParameterName >( i_rName );
	TranslatorContainer::const_iterator iter = m_Parameters.find( datum );
	return ( iter != m_Parameters.end()								// it's in the parameters
		  && iter->second.GetValue< TranslatedName >().IsNull()	// there is not translated name
		  && iter->second.GetValue< ValueTranslator >().IsNull()	// there is no value translator
		  && m_PrimaryDefaults.find( i_rName ) == m_PrimaryDefaults.end() );		// there is no value default
}

void ParameterTranslator::AddDelayedParameter(const std::string& i_rBuiltIn, const std::string& i_rParameter )
{
	if( i_rBuiltIn == MD5 )
	{
		m_DelayedEvaluationParameters[i_rParameter] = MD5;
	}
	else if( i_rBuiltIn == BYTE_COUNT )
	{
		m_DelayedEvaluationParameters[i_rParameter] = BYTE_COUNT;
	}
}

void ParameterTranslator::TranslateDelayedParameters( std::map< std::string, std::string >& o_rTranslatedParameters, std::istream& i_rData, std::streampos i_StrPos ) const
{
	bool hasCalculateMD5 = false;
	bool hasCalculateByte = false;
	std::string md5Value("");
	std::streampos byteCount = 0;

	std::map< std::string, std::string >::const_iterator iter = m_DelayedEvaluationParameters.begin();
	for( ; iter != m_DelayedEvaluationParameters.end(); ++iter )
	{
		if( iter->second == MD5 )
		{
			if( !hasCalculateMD5 )
			{
				i_rData.clear();
				i_rData.seekg( i_StrPos );
				std::string strData( std::istreambuf_iterator<char>( i_rData ), {} );
				md5Value = MVUtility::GetMD5( strData );
				hasCalculateMD5 = true;
			}
			o_rTranslatedParameters[iter->first] = md5Value;
		}
		else if( iter->second == BYTE_COUNT )
		{
			if( !hasCalculateByte )
			{
				i_rData.clear();
				i_rData.seekg( 0, i_rData.end );
				byteCount = i_rData.tellg() - i_StrPos;
				hasCalculateByte = true;
			}
			o_rTranslatedParameters[iter->first] = boost::lexical_cast< std::string >( byteCount );
		}
	}
}
