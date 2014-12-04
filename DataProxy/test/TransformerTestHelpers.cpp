//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/TransformerTestHelpers.cpp $
//
// REVISION:        $Revision: 281219 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-17 17:17:43 -0400 (Mon, 17 Jun 2013) $
// UPDATED BY:      $Author: esaxe $


#include "TransformerTestHelpers.hpp"
#include <fstream>
#include <stdlib.h>

void TransformerTestHelpers::SetupLibraryFile( const std::string& i_rTempDirectory, std::string& o_rLibrarySpec )
{
	//write cpp file
	std::string srcFileSpec = i_rTempDirectory + "/testTransform.cpp";
	std::ofstream srcfile( srcFileSpec.c_str() );
	srcfile << "#include <boost/shared_ptr.hpp>" << std::endl
			<< "#include <map>" << std::endl
			<< "#include <sstream>" << std::endl
			<< "#include <iostream>" << std::endl
			<< "#include <exception>" << std::endl
			<< "#include <stdexcept>" << std::endl
			<< "extern \"C\"" << std::endl
			<< "{" << std::endl
			<< "		// const char* DLL_VERSION = \"1.0.0\";" << std::endl
			<< "		boost::shared_ptr<std::stringstream> TransformFunction(std::istream& i_rData, const std::map<std::string, std::string>& i_rParameters)" << std::endl
			<< "		{" << std::endl
			<< "		boost::shared_ptr<std::stringstream> pStream;" << std::endl
			<< "		pStream.reset( new std::stringstream() );" << std::endl
			<< "" << std::endl
			<< "		std::map<std::string, std::string>::const_iterator paramIter = i_rParameters.begin();" << std::endl
			<< "		for( ; paramIter!= i_rParameters.end(); ++paramIter )" << std::endl
			<< "		{" << std::endl
			<< "			*pStream << paramIter->first << \" : \" << paramIter->second << std::endl;" << std::endl
			<< "		}" << std::endl
			<< "		*pStream << i_rData.rdbuf();" << std::endl
			<< "" << std::endl
			<< "		return pStream;" << std::endl
			<< "		}" << std::endl
			<< "" << std::endl
			<< "    boost::shared_ptr<std::stringstream> TransformFunction_null(std::istream& i_rData, const std::map<std::string, std::string>& i_rParameters)" << std::endl
			<< "        {" << std::endl
			<< "                boost::shared_ptr<std::stringstream> pStream;" << std::endl
			<< "                return pStream;" << std::endl
			<< "" << std::endl
			<< "		}" << std::endl
			<< "" << std::endl
			<< "    boost::shared_ptr<std::stringstream> TransformFunction_exception(std::istream& i_rData, const std::map<std::string, std::string>& i_rParameters)" << std::endl
			<< "        {" << std::endl
			<< "                boost::shared_ptr<std::stringstream> pStream;" << std::endl
			<< "     		    throw std::runtime_error(\"an exception\") ;" << std::endl
			<< "                return pStream;" << std::endl
			<< "" << std::endl
			<< "        }" << std::endl
			<< "" << std::endl
			<< "" << std::endl
			<< "}" << std::endl;
			

	srcfile.close();
	
	//build .so
	o_rLibrarySpec = i_rTempDirectory + "/testTransformer.so";
	std::string cmd = std::string("g++ -fPIC -shared -o ") + o_rLibrarySpec + " " + srcFileSpec;
	system( cmd.c_str() );
}
