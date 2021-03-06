//
// FILE NAME:       $HeadURL: svn+ssh://svn.cm.aol.com/advertising/adlearn/gen1/trunk/lib/cpp/DataProxy/test/ColumnAppenderStreamTransformerTest.hpp $
//
// REVISION:        $Revision: 281517 $
//
// COPYRIGHT:       (c) 2006 Advertising.com All Rights Reserved.
//
// LAST UPDATED:    $Date: 2013-06-19 17:36:40 -0400 (Wed, 19 Jun 2013) $
// UPDATED BY:      $Author: esaxe $
#ifndef _COLUMN_APPEDNER_STREAM_TRANSFORMER_TEST_HPP__
#define _COLUMN_APPEDNER_STREAM_TRANSFORMER_TEST_HPP__

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include <boost/scoped_ptr.hpp>

class TempDirectory;

class ColumnAppenderStreamTransformerTest : public CppUnit::TestFixture
{
private:
        CPPUNIT_TEST_SUITE( ColumnAppenderStreamTransformerTest );
        CPPUNIT_TEST( testColumnAppender );
        CPPUNIT_TEST_SUITE_END();
		void PrepareDatafile();
public:
        ColumnAppenderStreamTransformerTest();
        virtual ~ColumnAppenderStreamTransformerTest();

        void setUp();
        void tearDown();

        void testColumnAppender();
private:
        boost::scoped_ptr< TempDirectory > m_pTempDir;
};

#endif //_COLUMN_APPENDER_STREAM_TRANSFORMER_TEST_HPP_
