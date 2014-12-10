CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( DataProxy )

#################################################
#################  BOILERPLATE  #################
#################################################

# incude common cmake directives
INCLUDE( "${CMAKE_CURRENT_SOURCE_DIR}/../lib/adl-cmake/Common.cmake" )

# set c++ standard
SET_PROPERTY( DIRECTORY PROPERTY COMPILE_OPTIONS "-std=c++0x" )

# define LIBDPL_BUILD for all builds so only the right classes will be exposed
SET_DIRECTORY_PROPERTIES( PROPERTIES COMPILE_DEFINITIONS LIBDPL_BUILD )

#################################################
###################  VERSION  ###################
#################################################

FILE( STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/Version.txt VERSION_STRING )
STRING( REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\1" VERSION_MAJOR "${VERSION_STRING}" )
STRING( REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\2" VERSION_MINOR "${VERSION_STRING}" )
STRING( REGEX REPLACE "^([0-9]+)\\.([0-9]+)\\.([0-9]+)" "\\3" VERSION_BUILD "${VERSION_STRING}" )

#################################################
##################### MAIN  #####################
#################################################

# define the libs needed for this build
ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../lib/adl-liblogger Logger )
ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../lib/adl-libcore Utility )
ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../lib/adl-libcore GDP )
ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../lib/adl-libcore Service )
ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../lib/adl-libcore Database )
ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../lib/adl-libmon Monitoring )

# external package dependencies
FIND_PACKAGE( Boost 1.44 REQUIRED system filesystem program_options date_time regex thread iostreams )
FIND_PACKAGE( CURL REQUIRED )
FIND_PACKAGE( AgentPP REQUIRED )
FIND_PACKAGE( log4cxx REQUIRED )
FIND_PACKAGE( ODBC REQUIRED )
FIND_PACKAGE( OpenSSL REQUIRED )
FIND_PACKAGE( Oracle REQUIRED )
FIND_PACKAGE( SnmpPP REQUIRED )
FIND_PACKAGE( Threads REQUIRED )
FIND_PACKAGE( UUID REQUIRED )
FIND_PACKAGE( XercesC REQUIRED )

# set libs based on external package dependencies above
SET( DataProxy_Libs
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
	${AGENT_PP_LIBRARIES}
	${LOG4CXX_LIBRARIES}
	${ODBC_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${ORACLE_LIBRARIES}
	${SNMP_PP_LIBRARIES}
	${UUID_LIBRARIES}
	${XERCESC_LIBRARY}
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)

# main source files
SET( DataProxy_Src
	src/AbstractNode.cpp
	src/AggregateStreamTransformer.cpp
	src/AtomicsJSONToCSVStreamTransformer.cpp
	src/AwkUtilities.cpp
	src/BlackoutStreamTransformer.cpp
	src/CampaignReferenceGeneratorStreamTransformer.cpp
	src/CampaignRevenueVectorStreamTransformer.cpp
	src/ColumnAppenderStreamTransformer.cpp
	src/ColumnFormatStreamTransformer.cpp
	src/CustomEntityResolver.cpp
	src/DatabaseConnectionManager.cpp
	src/DatabaseProxy.cpp
	src/DataProxyClient.cpp
	src/EquivalenceClassStreamTransformer.cpp
	src/ExecutionProxy.cpp
	src/GroupingAggregateStreamTransformer.cpp
	src/JoinNode.cpp
	src/LocalFileProxy.cpp
	src/NodeFactory.cpp
	src/ParameterTranslator.cpp
	src/PartitionNode.cpp
	src/PropertyDomain.cpp
	src/ProxyUtilities.cpp
	src/RestDataProxy.cpp
	src/RestRequestBuilder.cpp
	src/RouterNode.cpp
	src/SelfDescribingStreamHeaderTransformer.cpp
	src/ShellStreamTransformer.cpp
	src/StreamTransformer.cpp
	src/TransformerManager.cpp
	src/TransformerUtilities.cpp
	src/TransformFunctionDomain.cpp
	src/ValidateStreamTransformer.cpp
)

SET( DataProxy_ThppSrc
	gdp/DatabaseConnectionBinder.thpp
	gdp/GenericDPLDomainTestTypes.thpp
	gdp/PreCampaignReferenceSerialization.thpp
)

# establish a directory to use for auto-generated stubs
SET( AutoGenDir ${CMAKE_CURRENT_BINARY_DIR}/gen )

# generate *.hpp files from thpp files
ADD_THPP_DEPENDENCIES( DataProxyThppGeneratedHpps ${AutoGenDir} "${DataProxy_ThppSrc}" )

# define includes for this build, and export them into the cache
SET( DataProxy_Incs
	${CMAKE_CURRENT_SOURCE_DIR}/include
	CACHE INTERNAL "${PROJECT_NAME} include directories" FORCE
)

# include the directories for this build
INCLUDE_DIRECTORIES( ${DataProxy_Incs} )
INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include/detail )
INCLUDE_DIRECTORIES( ${AutoGenDir} )
# include all the dependent libs' include folders
INCLUDE_DIRECTORIES( ${Logger_Incs} )
INCLUDE_DIRECTORIES( ${Utility_Incs} )
INCLUDE_DIRECTORIES( ${GDP_Incs} )
INCLUDE_DIRECTORIES( ${Service_Incs} )
INCLUDE_DIRECTORIES( ${Database_Incs} )
INCLUDE_DIRECTORIES( ${Monitoring_Incs} )
# include the external dependencies
INCLUDE_DIRECTORIES( SYSTEM
	${Boost_INCLUDE_DIRS}
	${AGENT_PP_INCLUDE_DIR}
	${LOG4CXX_INCLUDE_DIR}
	${ODBC_INCLUDE_DIRECTORIES}
	${OPENSSL_INCLUDE_DIR}
	${ORACLE_INCLUDE_DIR}
	${SNMP_PP_INCLUDE_DIR}
	${UUID_INCLUDE_DIR}
	${XERCESC_INCLUDE_DIRS}
)

# define link directories
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

# define primary output
ADD_LIBRARY( DataProxy SHARED ${DataProxy_Src} )

# add dependencies for this target so they will be built first
ADD_DEPENDENCIES( DataProxy DataProxyThppGeneratedHpps Logger Utility Service Database Monitoring )

# link target against external libs
TARGET_LINK_LIBRARIES( DataProxy
	${DataProxy_Libs}
	$<TARGET_SONAME_FILE:Monitoring>
	$<TARGET_FILE:Database>
	$<TARGET_FILE:Service>
	$<TARGET_FILE:Utility>
	$<TARGET_SONAME_FILE:Logger>
)

# set the SONAME version of this dynamic lib
SET_TARGET_PROPERTIES( DataProxy PROPERTIES VERSION ${VERSION_STRING} SOVERSION "${VERSION_MAJOR}.${VERSION_MINOR}" )

IF( CMAKE_COMPILER_IS_GNUCXX )
	SET_TARGET_PROPERTIES( DataProxy PROPERTIES COMPILE_FLAGS "-fvisibility=hidden -fvisibility-inlines-hidden" )
	SET_TARGET_PROPERTIES( DataProxy PROPERTIES LINK_FLAGS "-Wl,--exclude-libs=ALL" )
ENDIF()

#################################################
##################### SWIG  #####################
#################################################

# swig boilerplate
FIND_PACKAGE( SWIG REQUIRED )
INCLUDE( ${SWIG_USE_FILE} )

# set swig properties (applicable to all)
SET( BASE_SWIG_FLAGS "-nodefaultctor" )
SET( CMAKE_SWIG_OUTDIR "${CMAKE_CURRENT_BINARY_DIR}/gen-swig" )
SET_DIRECTORY_PROPERTIES( PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${CMAKE_SWIG_OUTDIR};${AutoGenDir}" )

# set swig source(s)
SET( DataProxy_SwigSrc
	swig/SwigDataProxy.i
)

# set source properties
SET_SOURCE_FILES_PROPERTIES( ${DataProxy_SwigSrc} PROPERTIES CPLUSPLUS ON )
SET_SOURCE_FILES_PROPERTIES( ${DataProxy_SwigSrc} PROPERTIES SWIG_FLAGS "-includeall" )

# support for python swig libraries, if enabled
IF( BUILD_SWIG_PYTHON )
	FIND_PACKAGE( PythonLibs REQUIRED )
	INCLUDE_DIRECTORIES( ${PYTHON_INCLUDE_PATH} )
	SET( CMAKE_SWIG_FLAGS ${BASE_SWIG_FLAGS} -module SwigPythonDataProxy )
	SET_SOURCE_FILES_PROPERTIES( ${DataProxy_SwigSrc} PROPERTIES SWIG_MODULE_NAME SwigPythonDataProxy )
	SET( SWIG_MODULE_SwigPythonDataProxy_EXTRA_DEPS DataProxy )
	SWIG_ADD_MODULE( SwigPythonDataProxy python ${DataProxy_SwigSrc} )
	SWIG_LINK_LIBRARIES( SwigPythonDataProxy ${PYTHON_LIBRARIES} $<TARGET_SONAME_FILE:DataProxy> )
ENDIF()

# support for perl swig libraries, if enabled
IF( BUILD_SWIG_PERL )
	SET( CMAKE_SWIG_FLAGS ${BASE_SWIG_FLAGS} -module SwigPerlDataProxy )
	SET_SOURCE_FILES_PROPERTIES( ${DataProxy_SwigSrc} PROPERTIES SWIG_MODULE_NAME SwigPerlDataProxy )
	SET( SWIG_MODULE_SwigPerlDataProxy_EXTRA_DEPS DataProxy )
	SET_SOURCE_FILES_PROPERTIES( ${CMAKE_SWIG_OUTDIR}/SwigDataProxyPERL_wrap.cxx PROPERTIES COMPILE_FLAGS "-Wno-unused-variable -Wno-maybe-uninitialized" )
	SWIG_ADD_MODULE( SwigPerlDataProxy perl ${DataProxy_SwigSrc} )
	SWIG_LINK_LIBRARIES( SwigPerlDataProxy $<TARGET_SONAME_FILE:DataProxy> )
ENDIF()

#################################################
####################  MOCK  #####################
#################################################

# define the libs needed for the mock build
ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../lib/adl-libcore TestHelpers )

# mock source files
# NOTE: these are only the mocks of the PUBLIC classes!
#   there _are_ other mocks listed in the tests below,
#   but they are internal and should not be in the mock lib
SET( MockDataProxy_Src
	mock/MockDataProxyClient.cpp
)

# define includes for mock lib, and export them into the cache
SET( MockDataProxy_Incs
	${CMAKE_CURRENT_SOURCE_DIR}/mock
	CACHE INTERNAL "${PROJECT_NAME} mock include directories" FORCE
)

# include mock directories for this build
INCLUDE_DIRECTORIES( ${MockDataProxy_Incs} )
# include all the mock dependent libs' include folders
INCLUDE_DIRECTORIES( ${TestHelpers_Incs} )

# add mock library target, but exclude it from "all" target
ADD_LIBRARY( MockDataProxy EXCLUDE_FROM_ALL ${MockDataProxy_Src} )

# add dependencies for mock target so they will be built first
ADD_DEPENDENCIES( MockDataProxy Logger Utility Service TestHelpers )

# link mock library to dependent libs
TARGET_LINK_LIBRARIES( MockDataProxy
	${DataProxy_Libs}
	$<TARGET_FILE:TestHelpers>
	$<TARGET_FILE:Service>
	$<TARGET_FILE:Utility>
	$<TARGET_SONAME_FILE:Logger>
)

#################################################
####################  TESTS  ####################
#################################################

IF( BUILD_TESTS )

	# define the libs needed for the test build
	ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../lib/adl-libcore TestHelpers )
	
	# external package dependencies for tests
	FIND_PACKAGE( Cppunit REQUIRED )
	
	# set test libs based on external package dependencies above
	SET( DataProxyTest_Libs
		${DataProxy_Libs}
		${CPPUNIT_LIBRARIES}
	)
	
	# test source files
	# SEE NOT ABOVE IN THE MOCK SECTION
	# regarding why there are mock files listed here
	SET( DataProxyTest_Src
		test/AbstractNodeTest.cpp
		test/AggregateStreamTransformerTest.cpp
		test/AtomicsJSONToCSVStreamTransformerTest.cpp
		test/AwkUtilitiesTest.cpp
		test/BlackoutStreamTransformerTest.cpp
		test/CampaignReferenceGeneratorStreamTransformerTest.cpp
		test/CampaignRevenueVectorStreamTransformerTest.cpp
		test/ColumnAppenderStreamTransformerTest.cpp
		test/ColumnFormatStreamTransformerTest.cpp
		test/DatabaseConnectionManagerTest.cpp
		test/DatabaseProxyTest.cpp
		test/DataProxyClientTest.cpp
		test/EquivalenceClassStreamTransformerTest.cpp
		test/ExecutionProxyTest.cpp
		test/GenericDPLDomainTest.cpp
		test/GroupingAggregateStreamTransformerTest.cpp
		test/JoinNodeTest.cpp
		test/LocalFileProxyTest.cpp
		test/main.cpp
		#test/MultithreadDataProxyClientTest.cpp
		test/NodeFactoryTest.cpp
		test/ParameterTranslatorTest.cpp
		test/PartitionNodeTest.cpp
		test/PropertyDomainTest.cpp
		test/ProxyTestHelpers.cpp
		test/ProxyUtilitiesTest.cpp
		test/RestDataProxyTest.cpp
		test/RestRequestBuilderTest.cpp
		test/RouterNodeTest.cpp
		test/SelfDescribingStreamHeaderTransformerTest.cpp
		test/ShellStreamTransformerTest.cpp
		test/StreamTransformerTest.cpp
		test/TransformerManagerTest.cpp
		test/TransformerTestHelpers.cpp
		test/TransformerUtilitiesTest.cpp
		test/TransformFunctionDomainTest.cpp
		test/ValidateStreamTransformerTest.cpp
		mock/MockDatabaseConnectionManager.cpp
		mock/MockNode.cpp
		mock/MockNodeFactory.cpp
		mock/MockTransformFunctionDomain.cpp
		mock/MockTransformFunctions.cpp
		mock/TestableDataProxyClient.cpp
		mock/TestableNode.cpp
	)
	
	# define includes for test exe
	SET( DataProxyTest_Incs
		${CMAKE_CURRENT_SOURCE_DIR}/test
	)
	
	# include test directories for this build
	INCLUDE_DIRECTORIES( ${DataProxyTest_Incs} )
	# include mock directories for this build
	INCLUDE_DIRECTORIES( ${MockDataProxy_Incs} )
	# include all the test dependent libs' include folders
	INCLUDE_DIRECTORIES( ${TestHelpers_Incs} )
	# include all the test dependent mock libs' include folders
	INCLUDE_DIRECTORIES( ${MockUtility_Incs} )
	INCLUDE_DIRECTORIES( ${MockService_Incs} )
	INCLUDE_DIRECTORIES( ${MockDatabase_Incs} )
	INCLUDE_DIRECTORIES( ${MockMonitoring_Incs} )
	# include all the external libs' include folders
	INCLUDE_DIRECTORIES( SYSTEM
		${CPPUNIT_INCLUDE_DIR}
	)
	
	# add test executable target, but exclude it from "all" target
	# NOTE: this executable is built off the main sources as well, because we have
	# to use them directly as most of their symbols will NOT be exported
	ADD_EXECUTABLE( DataProxyTest EXCLUDE_FROM_ALL test/main.cpp ${DataProxy_Src} ${DataProxyTest_Src} )
	
	# add dependencies for test target so they will be built first
	ADD_DEPENDENCIES( DataProxyTest Logger Utility MockUtility Service MockService Database MockDatabase Monitoring MockMonitoring TestHelpers MockDataProxy )
	
	# link test library to dependent libs
	TARGET_LINK_LIBRARIES( DataProxyTest 
		${DataProxyTest_Libs}
		$<TARGET_FILE:MockDataProxy>
		$<TARGET_FILE:TestHelpers>
		$<TARGET_FILE:MockMonitoring>
		$<TARGET_SONAME_FILE:Monitoring>
		$<TARGET_FILE:MockDatabase>
		$<TARGET_FILE:Database>
		$<TARGET_FILE:MockService>
		$<TARGET_FILE:Service>
		$<TARGET_FILE:MockUtility>
		$<TARGET_FILE:Utility>
		$<TARGET_SONAME_FILE:Logger>
	)
	
	SET_TARGET_PROPERTIES( DataProxyTest PROPERTIES COMPILE_DEFINITIONS DPL_TEST )

ENDIF()
