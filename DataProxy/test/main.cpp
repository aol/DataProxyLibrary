//
// FILE NAME:		$HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/main.cpp $
//
// REVISION:		$Revision: 305006 $
// 
// COPYRIGHT:		(c) 2008 Advertising.com All Rights Reserved.
// 
// LAST UPDATED:	$Date: 2014-09-25 14:00:37 -0400 (Thu, 25 Sep 2014) $
// UPDATED BY:		$Author: sstrick $
//

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
//#include <cppunit/ui/text/TestRunner.h>
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

int main(int argc, char** argv)
{
    try
    {
		// initialize a Logger since most of our code require this.
		//
		MVLogger::Init( LOGGER_FILENAME, "", "dpl-unit-tests" );
	
		// Create the event manager and test controller
		CppUnit::TestResult controller;
	
		// Add a listener that colllects test result
		CppUnit::TestResultCollector result;
		controller.addListener( &result );  
	
		// Add a listener that print dots as test run.
		CppUnit::BriefTestProgressListener progress;
		controller.addListener( &progress ); 

		bool useDefaultRegistry = true;

		CppUnit::TestRunner runner;

		for(int i = 1; i < argc; ++i)
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
				  
		if (useDefaultRegistry)
		{ 
			CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
			runner.addTest(registry.makeTest());	
		}

		runner.run(controller, "");
	
		CppUnit::TextOutputter textOut(&result, std::cerr);
		textOut.printFailures();
	
		return result.wasSuccessful() ? 0 : 1;
    }
    catch(MVException &mve)
    {
		std::cout << "Caught top-level exception in test main: " << mve << std::endl;
		return 1;
    }

}

