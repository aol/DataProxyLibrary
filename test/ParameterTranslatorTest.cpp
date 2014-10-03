// FILE NAME:       $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:      $Author$

#include "ShellExecutor.hpp"
#include "ParameterTranslatorTest.hpp"
#include "FileUtilities.hpp"
#include "TempDirectory.hpp"
#include "ProxyUtilities.hpp"
#include "ProxyTestHelpers.hpp"
#include "AssertThrowWithMessage.hpp"
#include "MVUtility.hpp"
#include "UniqueIdGenerator.hpp"
#include "DateTime.hpp"
#include <fstream>
#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

CPPUNIT_TEST_SUITE_REGISTRATION( ParameterTranslatorTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( ParameterTranslatorTest, "ParameterTranslatorTest" );

ParameterTranslatorTest::ParameterTranslatorTest()
:	m_pTempDir(NULL)
{
}

ParameterTranslatorTest::~ParameterTranslatorTest()
{
}

void ParameterTranslatorTest::setUp()
{
	XMLPlatformUtils::Initialize();
	m_pTempDir.reset( new TempDirectory() );
}

void ParameterTranslatorTest::tearDown()
{
	//XMLPlatformUtils::Terminate();
	m_pTempDir.reset( NULL );
}

void ParameterTranslatorTest::testGarbageChildren()
{
	std::stringstream xmlContents;
	xmlContents << "<Whatever >"
				<< " <TranslateParameters>"
				<< "  <garbage />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), XMLUtilitiesException, ".*/XMLUtilities.cpp:\\d+: Found invalid child: garbage in node: TranslateParameters" );

	xmlContents.str("");
	xmlContents << "<Whatever >"
				<< " <TranslateParameters garbage=\"true\">"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), XMLUtilitiesException,
		".*/XMLUtilities.cpp:\\d+: Found invalid attribute: garbage in node: TranslateParameters" );

	xmlContents.str("");
	xmlContents << "<Whatever >"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"name1\" garbage=\"true\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), XMLUtilitiesException,
		".*/XMLUtilities.cpp:\\d+: Found invalid attribute: garbage in node: Parameter" );
}

void ParameterTranslatorTest::testNoTranslate()
{
	std::stringstream xmlContents;
	xmlContents << "<Whatever />";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator( *nodes[0] );

	std::map< std::string, std::string > inputParameters;
	std::map< std::string, std::string > translatedParameters;

	// to be sure it no longer exists after translation
	translatedParameters["oldParam"] = "value0";
	
	inputParameters["param1"] = "value1";
	inputParameters["param2"] = "value2";
	inputParameters["param3"] = "value3";
	inputParameters["param4"] = "value4";
	inputParameters["param5"] = "value5";

	CPPUNIT_ASSERT_NO_THROW( translator.Translate( inputParameters, translatedParameters ) );
	CPPUNIT_ASSERT_EQUAL( size_t(5), translatedParameters.size() );
	std::map< std::string, std::string >::const_iterator iter = translatedParameters.begin();
	CPPUNIT_ASSERT( iter != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("param1"), iter->first );
	CPPUNIT_ASSERT_EQUAL( std::string("value1"), iter->second );
	++iter;
	CPPUNIT_ASSERT( iter != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("param2"), iter->first );
	CPPUNIT_ASSERT_EQUAL( std::string("value2"), iter->second );
	++iter;
	CPPUNIT_ASSERT( iter != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("param3"), iter->first );
	CPPUNIT_ASSERT_EQUAL( std::string("value3"), iter->second );
	++iter;
	CPPUNIT_ASSERT( iter != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("param4"), iter->first );
	CPPUNIT_ASSERT_EQUAL( std::string("value4"), iter->second );
	++iter;
	CPPUNIT_ASSERT( iter != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( std::string("param5"), iter->first );
	CPPUNIT_ASSERT_EQUAL( std::string("value5"), iter->second );
	++iter;
	CPPUNIT_ASSERT( iter == translatedParameters.end() );
}

void ParameterTranslatorTest::testTranslate()
{
	std::stringstream xmlContents;
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"aaaa\" translatedName=\"param11\" />"	// aaaa will be renamed to param11
				<< "  <Parameter name=\"zzzz\" translatedName=\"param11\" />"	// zzzz will be renamed to param11
				<< "  <Parameter name=\"s1\" translatedName=\"silenced01\" />"	// s1 will be renamed to silenced01, which is silenced
				<< "  <Parameter name=\"param01\" translatedName=\"renamed01\" />"	// param01 will be renamed to renamed01
				<< "  <Parameter name=\"param02\" />"								// param02 will be removed
				<< "  <Parameter name=\"param03\" translatedName=\"renamed03\" />"	// param03 will be renamed to renamed03
				<< "  <Parameter name=\"renamed03\" valueTranslator=\"VALUE03_%v\" valueDefault=\"DEFAULT03\" />"	// renamed03 will be translated to VALUE03_<value> if it exists; otherwise defaulted to DEFAULT03
				<< "  <Parameter name=\"param04\" valueTranslator=\"`echo -n %v | sed 's/alu/ALU/' | sed 's/04/Four/'`\" />" // translated
				<< "  <Parameter name=\"missing01\" valueTranslator=\"`missingVal01`\"/>"	// will not be present so no translation
				<< "  <Parameter name=\"missing02\" valueTranslator=\"missingVal02\"/>"	// will not be present so no translation
				<< "  <Parameter name=\"param05\" valueDefault=\"`echo -n \\`value05\\``\" />" // how to have a default w/ backticks
				<< "  <Parameter name=\"param06\" valueDefault=\"default06\" />" // param06 defaults to default06
				<< "  <Parameter name=\"param07\" valueDefault=\"default07\" />" // param07 defaults to default07
				<< "  <Parameter name=\"param08\" valueDefault=\"`echo -n default08`\" />" // param08 defaults to default08 (through expression)
				<< "  <Parameter name=\"param09\" valueOverride=\"override09\" />" // param09 overrides to override09
				<< "  <Parameter name=\"param10\" valueOverride=\"`echo -n override10`\" />" // param10 overrides to override10
				<< "  <Parameter name=\"silenced01\" />" // silence01 will be removed
				<< " </TranslateParameters>"
				<< "</Whatever>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator( *nodes[0] );

	std::map< std::string, std::string > inputParameters;
	std::map< std::string, std::string > translatedParameters;

	CPPUNIT_ASSERT_NO_THROW( translator.Translate( inputParameters, translatedParameters ) );
	std::map< std::string, std::string > expectedParameters;
	expectedParameters["renamed03"] = "DEFAULT03";
	expectedParameters["param05"] = "`value05`";
	expectedParameters["param06"] = "default06";
	expectedParameters["param07"] = "default07";
	expectedParameters["param08"] = "default08";
	expectedParameters["param09"] = "override09";
	expectedParameters["param10"] = "override10";
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	// to be sure it no longer exists after translation
	translatedParameters["oldParam"] = "value0";
	
	inputParameters["param01"] = "value01";
	inputParameters["param02"] = "value02";
	inputParameters["param03"] = "value03";
	inputParameters["param04"] = "value04";
	inputParameters["param06"] = "value06";
	inputParameters["param09"] = "value09";
	inputParameters["aaaa"] = "aaaaValue";
	inputParameters["zzzz"] = "zzzzValue";
	inputParameters["param11"] = "value11";
	inputParameters["s1"] = "shouldBeMissing01";

	CPPUNIT_ASSERT_NO_THROW( translator.Translate( inputParameters, translatedParameters ) );
	expectedParameters.clear();
	expectedParameters["renamed01"] = "value01";	// param01 renamed to renamed01
	expectedParameters["renamed03"] = "VALUE03_value03";	// param03 renamed to renamed03 and value translated to literal
	expectedParameters["param04"] = "vALUeFour";	// value translated via expression
	expectedParameters["param05"] = "`value05`";	// value defaulted via expression
	expectedParameters["param06"] = "value06";		// value not defaulted since already provided
	expectedParameters["param07"] = "default07";	// value defaulted to literal
	expectedParameters["param08"] = "default08";	// value defaulted via expression
	expectedParameters["param09"] = "override09";	// value overriden to literal
	expectedParameters["param10"] = "override10";	// value overriden via expression
	expectedParameters["param11"] = "value11";		// value taken from the translate-to target, not from the source(s)
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );
}

void ParameterTranslatorTest::testTranslateDerived()
{
	// case 1: derivedParam will only be derived if it is not provided
	std::stringstream xmlContents;
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"sourceParam\" valueOverride=\"sourceParamValue\" />"
				<< "  <Parameter name=\"derivedParam\" valueSource=\"sourceParam\" valueTranslator=\"derived from: %v\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator1( *nodes[0] );

	std::map< std::string, std::string > inputParameters;
	std::map< std::string, std::string > translatedParameters;

	CPPUNIT_ASSERT_NO_THROW( translator1.Translate( inputParameters, translatedParameters ) );
	std::map< std::string, std::string > expectedParameters;
	expectedParameters["sourceParam"] = "sourceParamValue";
	expectedParameters["derivedParam"] = "derived from: sourceParamValue";
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	translatedParameters.clear();
	inputParameters["derivedParam"] = "originalValue";
	expectedParameters["derivedParam"] = "originalValue";
	CPPUNIT_ASSERT_NO_THROW( translator1.Translate( inputParameters, translatedParameters ) );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	// case 1a: derivedParam will only be derived if it is not provided
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"derivedParam\" valueDefault=\"not present\" valueSource=\"sourceParam\" valueTranslator=\"derived from: %v\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator1a( *nodes[0] );

	inputParameters.clear();

	CPPUNIT_ASSERT_NO_THROW( translator1a.Translate( inputParameters, translatedParameters ) );
	expectedParameters.clear();
	expectedParameters["derivedParam"] = "not present";
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	translatedParameters.clear();
	inputParameters["sourceParam"] = "my source";
	expectedParameters["sourceParam"] = "my source";
	expectedParameters["derivedParam"] = "derived from: my source";
	CPPUNIT_ASSERT_NO_THROW( translator1a.Translate( inputParameters, translatedParameters ) );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	// case 2: derivedParam will always use the derived value
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"sourceParam\" valueOverride=\"sourceParamValue\" />"
				<< "  <Parameter name=\"derivedParam\" valueSource=\"sourceParam\" valueOverride=\"`echo -n 'derived from: %v'`\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator2( *nodes[0] );

	inputParameters.clear();
	translatedParameters.clear();
	expectedParameters.clear();
	expectedParameters["sourceParam"] = "sourceParamValue";
	expectedParameters["derivedParam"] = "derived from: sourceParamValue";
	CPPUNIT_ASSERT_NO_THROW( translator2.Translate( inputParameters, translatedParameters ) );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	translatedParameters.clear();
	inputParameters["derivedParam"] = "willNotAppear";
	CPPUNIT_ASSERT_NO_THROW( translator2.Translate( inputParameters, translatedParameters ) );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	// case 3: derivedParam will not appear if its source doesn't appear
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"derivedParam\" valueSource=\"sourceParam\" valueOverride=\"derived from: %v\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator3( *nodes[0] );

	inputParameters.clear();
	translatedParameters.clear();
	expectedParameters.clear();
	CPPUNIT_ASSERT_NO_THROW( translator3.Translate( inputParameters, translatedParameters ) );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"derivedParam\" valueSource=\"sourceParam\" valueTranslator=\"derived from: %v\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator4( *nodes[0] );

	inputParameters.clear();
	translatedParameters.clear();
	expectedParameters.clear();
	CPPUNIT_ASSERT_NO_THROW( translator4.Translate( inputParameters, translatedParameters ) );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	// case 4: valueSource is several parameters (using dollar-substitution)
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"sourceParam1\" valueOverride=\"value1\" />"
				<< "  <Parameter name=\"sourceParam2\" valueOverride=\"value2\" />"
				<< "  <Parameter name=\"sourceParam3\" valueOverride=\"value3\" />"
				<< "  <Parameter name=\"derivedParam\" valueSource=\"*\" valueOverride=\"1: ${sourceParam1} 2: ${sourceParam2} 3: ${sourceParam3}\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator5( *nodes[0] );

	inputParameters.clear();
	translatedParameters.clear();
	expectedParameters.clear();
	expectedParameters["sourceParam1"] = "value1";
	expectedParameters["sourceParam2"] = "value2";
	expectedParameters["sourceParam3"] = "value3";
	expectedParameters["derivedParam"] = "1: value1 2: value2 3: value3";
	CPPUNIT_ASSERT_NO_THROW( translator5.Translate( inputParameters, translatedParameters ) );
	CPPUNIT_ASSERT_EQUAL( ProxyUtilities::ToString( expectedParameters ), ProxyUtilities::ToString( translatedParameters ) );

	// case 5: a parameter is referenced but not specified (with the multiple-sources flag only!)
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"sourceParam1\" valueOverride=\"value1\" />"
				//MISSING: << "  <Parameter name=\"sourceParam2\" valueOverride=\"value2\" />"
				<< "  <Parameter name=\"sourceParam3\" valueOverride=\"value3\" />"
				<< "  <Parameter name=\"derivedParam\" valueSource=\"*\" valueOverride=\"1: ${sourceParam1} 2: ${sourceParam2} 3: ${sourceParam3}\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator6( *nodes[0] );

	inputParameters.clear();
	translatedParameters.clear();
	expectedParameters.clear();
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( translator6.Translate( inputParameters, translatedParameters ), ProxyUtilitiesException,
		".*:\\d+: The following parameters are referenced, but are not specified in the parameters: sourceParam2" );
}

void ParameterTranslatorTest::testBadEvalCommand()
{
	// case 1: command doesn't exist
	std::stringstream xmlContents;
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param01\" valueOverride=\"`this_command_better_not_exist`\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator( *nodes[0] );

	std::map< std::string, std::string > inputParameters;
	std::map< std::string, std::string > translatedParameters;

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( translator.Translate( inputParameters, translatedParameters ), ParameterTranslatorException,
		".*:\\d+: Evaluation command returned non-zero return code: 127\\. Command was: this_command_better_not_exist. Standard Error: .*" );
	
	// case 2: timeout
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters evalTimeout=\"-1\" >"	// essentially disallow eval commands
				<< "  <Parameter name=\"param01\" valueOverride=\"`sleep 2 &amp;&amp; echo -n 10`\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator2( *nodes[0] );

	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( translator2.Translate( inputParameters, translatedParameters ), TimeoutException,
		".*\\.cpp:\\d+: The command 'sleep 2 && echo -n 10' failed to finish after -1 seconds\\. Wrote 0 bytes to standard input\\. Read 0 bytes from standard output\\. Read 0 bytes from standard error\\." );
}

void ParameterTranslatorTest::testIllegalConfig()
{
	std::stringstream xmlContents;
	std::vector<xercesc::DOMNode*> nodes;

	// case 1: Cannot supply name translation & value translation on the same node
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" translatedName=\"newParam1\" valueTranslator=\"newValue1\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: If translating the name of a parameter, cannot also provide a value manipulator.*" );

	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" translatedName=\"newParam1\" valueDefault=\"defaultValue1\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: If translating the name of a parameter, cannot also provide a value manipulator.*" );

	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" translatedName=\"newParam1\" valueOverride=\"overrideValue1\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: If translating the name of a parameter, cannot also provide a value manipulator.*" );

	// case 2: Cannot provide value translator & value override on the same node
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" valueTranslator=\"translatedParam1\" valueOverride=\"overrideValue1\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Cannot provide a value override and a value translator\\. Violating parameter: param1" );

	// case 3: Cannot provide value default & value override on the same node
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" valueDefault=\"defaultParam1\" valueOverride=\"overrideValue1\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Cannot provide a value override and a value default\\. Violating parameter: param1" );

	// case 4: Cannot provide name translation with more than one jump
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param2\" translatedName=\"param3\" />"
				<< "  <Parameter name=\"param1\" translatedName=\"param2\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Parameter name translations must not be chained. Violating chain: param1->param2->param3" );

	// case 5: Cannot provide same Parameter name on two lines
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" translatedName=\"param3\" />"
				<< "  <Parameter name=\"param1\" translatedName=\"param2\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Translation of parameter 'param1' is ambiguous" );

	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" translatedName=\"PARAM1\" />"
				<< "  <Parameter name=\"param1\" valueOverride=\"override1\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Translation of parameter 'param1' is ambiguous" );

	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" valueDefault=\"default1\" />"
				<< "  <Parameter name=\"param1\" valueOverride=\"override1\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Translation of parameter 'param1' is ambiguous" );

	// case 6: Cannot provide valueSource alone
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" valueSource=\"param3\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Parameters with a value source must have an override or translator supplied\\. Violating parameter: param1" );

	// case 7: Cannot provide valueSource with a name translator
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" valueSource=\"param3\" translatedName=\"newParam\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Cannot provide a value source when translating a parameter name\\. Violating parameter: param1" );

	// case 8: Cannot provide valueSource with a value default
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" valueSource=\"param3\" valueDefault=\"newParam\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Cannot provide a value source and a value default\\. Violating parameter: param1" );

	// case 9: Cannot make derived values derive from other derived values
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" valueSource=\"param3\" valueTranslator=\"%v\" />"
				<< "  <Parameter name=\"param2\" valueSource=\"param1\" valueTranslator=\"%v\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Cannot make values derive from other derived values. Violating parameters: param2 derives from param1, which derives from param3" );

	// case 9b: same, but with multiple-sources flag
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" valueSource=\"param2\" valueTranslator=\"%v\" />"
				<< "  <Parameter name=\"param2\" valueSource=\"*\" valueTranslator=\"p1: ${param1}, p3: ${param3}, p4: ${param4}\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Cannot make values derive from other derived values. Violating parameters: param1 derives from param2, which derives from param1, param3, param4" );

	// case 9c: same, but both are derived from multiple sources
	xmlContents.str("");
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"param1\" valueSource=\"*\" valueTranslator=\"p2: ${param2}\" />"
				<< "  <Parameter name=\"param2\" valueSource=\"*\" valueTranslator=\"p1: ${param1} p3: ${param3}\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	CPPUNIT_ASSERT_THROW_WITH_MESSAGE( ParameterTranslator translator( *nodes[0] ), ParameterTranslatorException,
		".*:\\d+: Cannot make values derive from other derived values. Violating parameters: param1 derives from param2, which derives from param1, param3" );
}

void ParameterTranslatorTest::testTranslateBuiltIn()
{
	std::stringstream xmlContents;
	xmlContents << "<Whatever>"
				<< " <TranslateParameters>"
				<< "  <Parameter name=\"host\" valueDefault=\"[hostname]\" />"
				<< "  <Parameter name=\"instance\" valueDefault=\"[instance]\" />"
				<< "  <Parameter name=\"unixtime1\" valueDefault=\"[datetime:%s]\" />"
				<< "  <Parameter name=\"unixtime2\" valueDefault=\"[datetime %s]\" />"
				<< "  <Parameter name=\"dm-datetime1\" valueDefault=\"[datetime]\" />"
				<< "  <Parameter name=\"dm-datetime2\" valueDefault=\"[datetime:%Y%m%dT%H%M%S]\" />"
				<< "  <Parameter name=\"unknown-datetime\" valueDefault=\"[datetime blah]\" />"
				<< "  <Parameter name=\"guid\" valueDefault=\"[guid]\" />"
				<< "  <Parameter name=\"pid\" valueDefault=\"[pid]\" />"
				<< "  <Parameter name=\"unkown1\" valueDefault=\"[cpu]\" />"
				<< "  <Parameter name=\"unkown2\" valueDefault=\"[mem]\" />"
				<< " </TranslateParameters>"
				<< "</Whatever>";
	std::vector<xercesc::DOMNode*> nodes;
	ProxyTestHelpers::GetDataNodes( m_pTempDir->GetDirectoryName(), xmlContents.str(), "Whatever", nodes );
	CPPUNIT_ASSERT_EQUAL( size_t(1), nodes.size() );
	ParameterTranslator translator( *nodes[0] );

	std::map< std::string, std::string > inputParameters;
	std::map< std::string, std::string > translatedParameters;

	DateTime before;
	CPPUNIT_ASSERT_NO_THROW( translator.Translate( inputParameters, translatedParameters ) );
	DateTime after;
	CPPUNIT_ASSERT_EQUAL( size_t( 11 ), translatedParameters.size() );
	std::map< std::string, std::string >::const_iterator findIter;
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "host" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( MVUtility::GetHostName(), findIter->second );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "instance" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "unixtime1" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT( before.GetFormattedString( "%s" ) <= findIter->second );
	CPPUNIT_ASSERT( after.GetFormattedString( "%s" ) >= findIter->second );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "unixtime2" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT( before.GetFormattedString( "%s" ) <= findIter->second );
	CPPUNIT_ASSERT( after.GetFormattedString( "%s" ) >= findIter->second );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "dm-datetime1" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT( before.GetFormattedString( "%Y%m%dT%H%M%S" ) <= findIter->second );
	CPPUNIT_ASSERT( after.GetFormattedString( "%Y%m%dT%H%M%S" ) >= findIter->second );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "dm-datetime2" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT( before.GetFormattedString( "%Y%m%dT%H%M%S" ) <= findIter->second );
	CPPUNIT_ASSERT( after.GetFormattedString( "%Y%m%dT%H%M%S" ) >= findIter->second );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "unknown-datetime" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( std::string( "blah" ), findIter->second );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "guid" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT_MESSAGE( findIter->second, UniqueIdGenerator::IsValidUniqueIdString( findIter->second ) );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "pid" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( boost::lexical_cast< std::string >( ::getpid() ), findIter->second );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "unkown1" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( std::string( "cpu-unknown" ), findIter->second );
	CPPUNIT_ASSERT( ( findIter = translatedParameters.find( "unkown2" ) ) != translatedParameters.end() );
	CPPUNIT_ASSERT_EQUAL( std::string( "mem-unknown" ), findIter->second );
}
