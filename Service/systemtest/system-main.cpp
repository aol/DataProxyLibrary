#include <iostream>
#include <fstream>
#include <string.h>

#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TextOutputter.h>

#include "MVLogger.hpp"
#include "MVException.hpp"


const char * LOGGER_FILENAME = "cppunit_Logger_log.xml";
const char*  CPPUNIT_OUTPUT_FILENAME = "data_proxy_service_SystemTests.xml";
const char*  TRANSFORM_FILENAME = "cppUnitToJUnit.xsl";

void CleanUp( void )
{
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

                CppUnit::TestRunner runner;
                CppUnit::TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
                runner.addTest(registry.makeTest());

                runner.run(controller, "");

                if(argc > 1 && strcmp(argv[1], "-xml") == 0)
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

                std::ofstream outFile("data_proxy_service_system_test_results");

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
