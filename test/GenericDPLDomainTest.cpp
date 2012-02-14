//
// FILE NAME:      $HeadURL$
//
// REVISION:        $Revision$
//
// COPYRIGHT:      (c) 2007 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date$
// UPDATED BY:    $Author$

#include "GenericDPLDomainTest.hpp"
#include "TempDirectory.hpp"
#include "MockDataProxyClient.hpp"
#include "AssertUnorderedContents.hpp"
#include "GenericDPLDomainTestTypes.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( GenericDPLDomainTest );
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION( GenericDPLDomainTest, "GenericDPLDomainTest" );

namespace
{
	std::string PrettyPrintTestContainer(TestContainer& i_rTestContainer)
	{
		std::stringstream containerStream;
		TestContainer::iterator iter = i_rTestContainer.begin();
		for (; iter != i_rTestContainer.end(); ++iter)
		{
			containerStream << iter->second.GetValue<TestHelpersGDPTypes::Key1Entry>() << ", " ;
			containerStream	<< iter->second.GetValue<TestHelpersGDPTypes::Key2Entry>() << ", ";
			containerStream	<< iter->second.GetValue<TestHelpersGDPTypes::Col1Entry>() << ", ";
			containerStream	<< iter->second.GetValue<TestHelpersGDPTypes::Col2Entry>() << std::endl;
		}
		return containerStream.str();
	}
}

GenericDPLDomainTest::GenericDPLDomainTest()
	: m_pTempDirectory(NULL)
{
}

GenericDPLDomainTest::~GenericDPLDomainTest()
{
}

void GenericDPLDomainTest::setUp()
{
	m_pTempDirectory.reset(new TempDirectory());
}

void GenericDPLDomainTest::tearDown()
{
	m_pTempDirectory.reset();
}

void GenericDPLDomainTest::testNormal()
{
	std::stringstream dplConfiguration;
	dplConfiguration << "<DPLConfig>" << std::endl;
	dplConfiguration << "<DataNode name=\"TestDataNode\" type=\"local\" location=\"" << m_pTempDirectory->GetDirectoryName() << "\" />";
	dplConfiguration << "</DPLConfig>" << std::endl;

	std::string dplConfigFileSpec = m_pTempDirectory->GetDirectoryName() + "/dplConfig.xml";
	std::ofstream dplConfigFile;
	dplConfigFile.open( dplConfigFileSpec.c_str());
	dplConfigFile << dplConfiguration.str();
	dplConfigFile.close();

	std::ofstream dataFile;
	std::string dataFileSpec = m_pTempDirectory->GetDirectoryName() + "/Foo~Bar";
	dataFile.open(dataFileSpec.c_str());
	dataFile << "Key1Column,Key2Column,Col1Column,Col2Column" << std::endl;
	dataFile << "11, 21, 31, 41" << std::endl;  
	dataFile << "12, 22, 32, 42" << std::endl;  
	dataFile << "12, 23, 33, 43" << std::endl;  
	dataFile << "14, 23, 34, 44" << std::endl;  
	dataFile << "14, 23, 35, 45" << std::endl;  //duplicate, will not show up in container
	dataFile << "16, 26, 36, 46" << std::endl;  
	dataFile.close();

	GenericDPLDomain< TestDatum, TestAggregator, TestBinder > testDomain;

	std::map<std::string, std::string> params;
	params["Foo"] = "Bar";
	
	testDomain.Load(dplConfigFileSpec, "TestDataNode", params);

	std::string actual(PrettyPrintTestContainer(testDomain));
	std::stringstream expected;

	expected << "11, 21, 31, 41" << std::endl;  
	expected << "12, 22, 32, 42" << std::endl;  
	expected << "12, 23, 33, 43" << std::endl;  
	expected << "14, 23, 34, 44" << std::endl;  
	expected << "16, 26, 36, 46" << std::endl;  

	CPPUNIT_ASSERT_UNORDERED_CONTENTS(expected.str(), actual, false);

}

