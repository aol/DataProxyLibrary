CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

PROJECT( DataProxyShell )
FILE( STRINGS ${CMAKE_CURRENT_SOURCE_DIR}/../../Version.txt VERSION )

#################################################
#################  BOILERPLATE  #################
#################################################

# incude common cmake directives
INCLUDE( "${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/adl-cmake/Common.cmake" )

#################################################
##################### MAIN  #####################
#################################################

# define the libs needed for this build
ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/adl-liblogger Logger )
ADD_LIB_ONCE( ${CMAKE_CURRENT_SOURCE_DIR}/../../../lib/adl-libcore Utility )

# set c++ standard
SET_PROPERTY( DIRECTORY PROPERTY COMPILE_OPTIONS "-std=c++0x" )

# external package dependencies
FIND_PACKAGE( Boost 1.44 REQUIRED system filesystem program_options regex thread iostreams )
FIND_PACKAGE( log4cxx REQUIRED )
FIND_PACKAGE( OpenSSL REQUIRED )
FIND_PACKAGE( SnmpPP REQUIRED )
FIND_PACKAGE( AgentPP REQUIRED )
FIND_PACKAGE( Threads REQUIRED )
FIND_PACKAGE( XercesC REQUIRED )

# set libs based on external package dependencies above
SET( DataProxyShell_Libs
	${Boost_LIBRARIES}
	${LOG4CXX_LIBRARIES}
	${OPENSSL_LIBRARIES}
	${SNMP_PP_LIBRARIES}
	${AGENT_PP_LIBRARIES}
	${XERCESC_LIBRARY}
	${CMAKE_DL_LIBS}
	${CMAKE_THREAD_LIBS_INIT}
)

# main source files
SET( DataProxyShell_Src
	src/DataProxyShellConfig.cpp
)

SET( DataProxyShell_Main src/DataProxyShell.cpp )

# define includes for this build
SET( DataProxyShell_Incs ${CMAKE_CURRENT_SOURCE_DIR}/include )

# include the directories for this build
INCLUDE_DIRECTORIES( ${DataProxyShell_Incs} )
# include all the dependent libs' include folders
INCLUDE_DIRECTORIES( ${Logger_Incs} )
INCLUDE_DIRECTORIES( ${Utility_Incs} )
INCLUDE_DIRECTORIES( ${DataProxy_Incs} )
# include the external dependencies
INCLUDE_DIRECTORIES( SYSTEM
	${Boost_INCLUDE_DIRS}
	${LOG4CXX_INCLUDE_DIR}
	${OPENSSL_INCLUDE_DIR}
	${SNMP_PP_INCLUDE_DIR}
	${AGENT_PP_INCLUDE_DIR}
	${XERCESC_INCLUDE_DIRS}
)

# define link directories
LINK_DIRECTORIES( ${Boost_LIBRARY_DIRS} )

# define object files which will be used for tests AND src
ADD_LIBRARY( DataProxyShellObjects OBJECT ${DataProxyShell_Src} )

# define primary executable
ADD_EXECUTABLE( DataProxyShell ${DataProxyShell_Main} $<TARGET_OBJECTS:DataProxyShellObjects> )

# add dependencies for this target so they will be built first
ADD_DEPENDENCIES( DataProxyShell Logger Utility DataProxy )

# link target against external libs
TARGET_LINK_LIBRARIES( DataProxyShell
	${DataProxyShell_Libs}
	$<TARGET_SONAME_FILE:DataProxy>
	$<TARGET_FILE:Utility>
	$<TARGET_SONAME_FILE:Logger>
)

# set position-independent-code so this static lib can be linked to by dynamic libs
SET_PROPERTY( TARGET DataProxyShell PROPERTY POSITION_INDEPENDENT_CODE ON )

# set the executable name
SET( LINK_NAME dplShell )
SET( EXE_NAME ${LINK_NAME}_v${VERSION} )
SET_PROPERTY( TARGET DataProxyShell PROPERTY OUTPUT_NAME ${EXE_NAME} )

# add a target to link to the full dplShell version
ADD_CUSTOM_COMMAND( TARGET DataProxyShell POST_BUILD COMMAND ln -sfn ${EXE_NAME} ${LINK_NAME} )

#################################################
###################  INSTALL  ###################
#################################################

SET( APPLICATION_DIRECTORY "app/dpl/shell" )
INSTALL( DIRECTORY DESTINATION ${APPLICATION_DIRECTORY}
	DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
INSTALL( DIRECTORY DESTINATION ${APPLICATION_DIRECTORY}/bin
	DIRECTORY_PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
INSTALL( TARGETS DataProxyShell DESTINATION ${APPLICATION_DIRECTORY}/bin
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )
INSTALL( FILES ${CMAKE_CURRENT_BINARY_DIR}/${LINK_NAME} DESTINATION ${APPLICATION_DIRECTORY}/bin
	PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE )

#################################################
####################  TESTS  ####################
#################################################

IF( BUILD_TESTS )

	# external package dependencies for tests
	FIND_PACKAGE( Cppunit REQUIRED )
	
	# set test libs based on external package dependencies above
	SET( DataProxyShellTest_Libs
		${DataProxyShell_Libs}
		${CPPUNIT_LIBRARIES}
	)
	
	# test source files
	SET( DataProxyShellTest_Src
		test/DataProxyShellConfigTest.cpp
	)
	
	# define includes for test exe
	SET( DataProxyShellTest_Incs
		${CMAKE_CURRENT_SOURCE_DIR}/test
	)
	
	# include test directories for this build
	INCLUDE_DIRECTORIES( ${DataProxyShellTest_Incs} )
	# include all the test dependent libs' include folders
	INCLUDE_DIRECTORIES( ${TestHelpers_Incs} )
	# include all the external libs' include folders
	INCLUDE_DIRECTORIES( SYSTEM
		${Boost_INCLUDE_DIRS}
		${LOG4CXX_INCLUDE_DIR}
		${OPENSSL_INCLUDE_DIR}
		${SNMP_PP_INCLUDE_DIR}
		${AGENT_PP_INCLUDE_DIR}
		${CPPUNIT_INCLUDE_DIR}
	)
	
	# add test executable target, but exclude it from "all" target
	ADD_EXECUTABLE( DataProxyShellTest EXCLUDE_FROM_ALL test/test-main.cpp ${DataProxyShellTest_Src} $<TARGET_OBJECTS:DataProxyShellObjects> )
	
	# add dependencies for test target so they will be built first
	ADD_DEPENDENCIES( DataProxyShellTest Logger Utility TestHelpers DataProxy )
	
	# link test library to dependent libs
	TARGET_LINK_LIBRARIES( DataProxyShellTest 
		${DataProxyShellTest_Libs}
		$<TARGET_SONAME_FILE:DataProxy>
		$<TARGET_FILE:TestHelpers>
		$<TARGET_FILE:Utility>
		$<TARGET_SONAME_FILE:Logger>
	)

ENDIF()

#################################################
###################  SYSTEST  ###################
#################################################

IF( BUILD_TESTS )

	# external package dependencies for tests
	FIND_PACKAGE( Cppunit REQUIRED )
	FIND_PACKAGE( CURL REQUIRED )
	
	# set test libs based on external package dependencies above
	SET( DataProxyShellSystest_Libs
		${DataProxyShell_Libs}
		${CPPUNIT_LIBRARIES}
		${CURL_LIBRARIES}
	)
	
	# test source files
	SET( DataProxyShellSystest_Src
		systemtest/DataProxyShellSystest.cpp
	)
	
	# define includes for test exe
	SET( DataProxyShellSystest_Incs
		${CMAKE_CURRENT_SOURCE_DIR}/systemtest
	)
	
	# include test directories for this build
	INCLUDE_DIRECTORIES( ${DataProxyShellSystest_Incs} )
	# include all the test dependent libs' include folders
	INCLUDE_DIRECTORIES( ${TestHelpers_Incs} )
	# include all the external libs' include folders
	INCLUDE_DIRECTORIES( SYSTEM
		${Boost_INCLUDE_DIRS}
		${LOG4CXX_INCLUDE_DIR}
		${OPENSSL_INCLUDE_DIR}
		${SNMP_PP_INCLUDE_DIR}
		${AGENT_PP_INCLUDE_DIR}
		${CPPUNIT_INCLUDE_DIR}
	)

	# add test executable target, but exclude it from "all" target
	ADD_EXECUTABLE( DataProxyShellSystest EXCLUDE_FROM_ALL systemtest/system-main.cpp ${DataProxyShellSystest_Src} )
	
	# add dependencies for test target so they will be built first
	ADD_DEPENDENCIES( DataProxyShellSystest Logger Utility TestHelpers DataProxyShell )
	
	# link test library to dependent libs
	TARGET_LINK_LIBRARIES( DataProxyShellSystest 
		${DataProxyShellSystest_Libs}
		$<TARGET_SONAME_FILE:DataProxy>
		$<TARGET_FILE:TestHelpers>
		$<TARGET_FILE:Utility>
		$<TARGET_SONAME_FILE:Logger>
	)

ENDIF()
