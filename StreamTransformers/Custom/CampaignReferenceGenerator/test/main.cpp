//
// FILE NAME:		$HeadURL$
//
// REVISION:		$Revision$
// 
// COPYRIGHT:		(c) 2008 Advertising.com All Rights Reserved.
// 
// LAST UPDATED:	$Date$
// UPDATED BY:		$Author$
//

#include <iostream>
#include <fstream>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TextOutputter.h>

#include "MVLogger.hpp"
#include "MVException.hpp"

const char * LOGGER_FILENAME = "cppunit_Logger_log.txt";
const char*  CPPUNIT_OUTPUT_FILENAME = "DataProxyTests.xml";
const char*  TRANSFORM_FILENAME = "cppUnitToJUnit.xsl";

void CleanUp( void )
{
	system("rm -f /tmp/GenWeightsTestLPFilename* 2>>/dev/null");
	system("rm -f /tmp/LPWrapper* 2>>/dev/null");
	system("rm -rf /tmp/AlphaLPTest* 2>>/dev/null");
	system("rm -rf /tmp/ReapportionmentLPTest* 2>>/dev/null");
	system("rm -rf /tmp/future 2>>/dev/null");
}

int main(int argc, char** argv)
{
    try
    {
		// initialize a Logger since most of our code require this.
		//
		MVLogger::Init( LOGGER_FILENAME );
	
		// Create the event manager and test controller
		CppUnit::TestResult controller;
	
		// Add a listener that colllects test result
		CppUnit::TestResultCollector result;
		controller.addListener( &result );  
	
		// Add a listener that print dots as test run.
		CppUnit::BriefTestProgressListener progress;
		controller.addListener( &progress ); 

		bool xmlOutput = false;
		bool useDefaultRegistry = true;

		CppUnit::TestRunner runner;

		for(int i = 1; i < argc; ++i)
		{
			if (strcmp(argv[i], "-xml") == 0)
			{
				xmlOutput = true;
			}
			else
			{
				useDefaultRegistry = false;
																																	  
				CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry(argv[i]);
				CppUnit::Test* pTests = registry.makeTest();
																																																	    
				if (pTests->getChildTestCount() == 0)
				{
					std::cerr << "Can't find registry " << argv[i] << std::endl;
				}
				else
				{
					runner.addTest(pTests);
				}
			}
		}
				  
		if (useDefaultRegistry)
		{ 
			CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
			runner.addTest(registry.makeTest());	
		}

		runner.run(controller, "");
	
		if(xmlOutput)
		{	
			std::ofstream file( CPPUNIT_OUTPUT_FILENAME );
			CppUnit::XmlOutputter xml( &result, file );
			xml.write();
			file.close();
		}
		else
		{
			CppUnit::TextOutputter textOut(&result, std::cerr);
			textOut.printFailures();
		}
	
		std::ofstream outFile("campaign_reference_generator_tests_results");
	
		if(outFile)
		{	
			if(result.wasSuccessful())
			{
				outFile << "0";
			}
			else
			{
				outFile << "1";
			}
		}
	
		CleanUp();
		return result.wasSuccessful() ? 0 : 1;
    }
    catch(MVException &mve)
    {
		std::cout << "Caught top-level exception in test main: " << mve << std::endl;
		return 1;
    }

}

