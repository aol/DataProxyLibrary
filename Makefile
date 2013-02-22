# FILE NAME:           $HeadURL$
#
# REVISION:            $Revision$
#
# COPYRIGHT:           (c) 2008 Advertising.com All Rights Reserved.
#
# LAST UPDATED:        $Date$
#
# UPDATED BY:          $Author$

# Top level directories
ROOTDIR		 = ../../..

# Path to this project from Optimization
PROJECT_PATH = lib/DataProxy

# Directories for common components
ARCH=$(shell uname -m)
MATLABDIR	?= /data/app/matlab
MYSQLHOME	?= /usr/local/mysql
ORACLE_HOME	?= /usr/oracle

# Directories for including & linking
INCLUDE_DIRS 	?= /usr/local/include
LIB_DIRS 	?= /usr/local/lib

# folders that contain files we need to manually compile
# in order to build a self-contained dynamic library
UTILITYDIR		= ${ROOTDIR}/lib/cpp/Utility
SERVICEDIR		= ${ROOTDIR}/lib/cpp/Service
DATABASEDIR		= ${ROOTDIR}/lib/cpp/Database
TESTHELPERDIR		= ${ROOTDIR}/lib/cpp/TestHelpers

# for auto-generating hpp files from thpp for GDP
THPP2HPP        = $(ROOTDIR)/lib/cpp/GDP/scripts/thpp2hpp

#Modules
MODULES=\
	lib/cpp/Logger \
	lib/cpp/Utility \
	lib/cpp/Service \
	lib/cpp/GDP \
	lib/cpp/Database \

TESTMODULES=\
	lib/cpp/TestHelpers \
	lib/cpp/Service \
	lib/cpp/Utility \
	lib/cpp/Database \

MATLABMODULES=\
	lib/cpp/Matlab \

MODULESPEC		= $(MODULES:%=$(ROOTDIR)/%)
TESTMODULESPEC		= $(TESTMODULES:%=$(ROOTDIR)/%)
MATLABMODULESPEC	= $(MATLABMODULES:%=$(ROOTDIR)/%)

# Include directories
INCS=\
	-I. \
	$(MODULESPEC:%=-I%) \
	-I${ROOTDIR}/lib/cpp/Database/private \
	-I${MYSQLHOME}/include \
	-I${ORACLE_HOME}/rdbms/demo \
	-I${ORACLE_HOME}/network/public \
	-I${ORACLE_HOME}/rdbms/public \
	$(INCLUDE_DIRS:%=-I%) \
	-I$(MATLABDIR)/extern/include \

TESTINCS=\
	$(INCS) \
	$(TESTMODULESPEC:%=-I%) \
	$(MODULESPEC:%=-I%/mock) \
	$(MATLABMODULESPEC:%=-I%) \
	-I$(MATLABDIR)/extern/include \
	-I$(TESTDIR) \
	-I$(MOCKDIR) \

# Library locations
LIBLOC=\
	-L${ORACLE_HOME}/lib \
	$(MODULESPEC:%=-L%) \
	${TESTMODULESPEC:%=-L%} \
	${MATLABMODULESPEC:%=-L%} \
	$(LIB_DIRS:%=-L%) \

# Libraries
LIBS		= -lLogger -lclntsh -lmyodbc3 -lxerces-c -lpthread -lboost_regex -lboost_filesystem -llog4cxx -lboost_thread -lcurl -lssl -luuid -lnnz10 -lcrypto -ldl -lboost_system
TESTLIBS	= -lcppunit -lTestHelpers -lMockDatabase -lMockService -lMockUtility $(LIBS)
MATLABLIBS	= -lMatlab -leng -lmx -lut -lmat

ifeq ($(ARCH), x86_64)
	LIBLOC += -L${MATLABDIR}/bin/glnxa64
else
	LIBLOC += -L${MATLABDIR}/bin/glnx86
endif

# This is required on the 32 bit platform to prevent recursive
# variable definition errors for DEFINE_FLAGS
ifndef DEFINE_FLAGS
	DEFINE_FLAGS=
endif

# This is an XML-requiring build
export BUILDXML=ON
DEFINE_FLAGS += -DBUILDXML

# Variables for compiler invocation
CXX ?= g++
CXXC = $(CXX) $(TARGETOPTS) -c $(INCS) $(DEFINE_FLAGS)		# compiling object files
CXXT = $(CXX) $(TARGETOPTS) -c $(TESTINCS) $(DEFINE_FLAGS)	# compiling object files
CXXL = $(CXX) $(TARGETOPTS) $(LIBLOC)						# linking executables
CXXD = $(CXX) -MM $(TESTINCS) $(DEFINE_FLAGS)				# generating dependencies
CXXS = $(CXX) -shared -Wl,-soname,$(MINOR_VERSION_TARGET)
MEX ?= mex
MEXC = $(MEX) CXXFLAGS='-std=c++0x -fPIC -fno-omit-frame-pointer -pthread'

# Source Directories
PRIVATEDIR		= private
TESTDIR			= test
MOCKDIR			= mock

# Target Directories
DEBUGDIR	= debug.obj
OPTIMIZEDIR	= opt.obj
OPTDEBUGDIR	= optd.obj
PROFILEDIR	= prof.obj
COVERAGEDIR	= cov.obj
ALLTARGETDIRS = $(DEBUGDIR) $(OPTIMIZEDIR) $(OPTDEBUGDIR) $(PROFILEDIR) $(COVERAGEDIR)

MATLABWRAPPERFILE=\
	DataProxyWrapper.cpp \

PRIVATEFILES=\
	CustomEntityResolver.cpp \
	AbstractNode.cpp \
	NodeFactory.cpp \
	RouterNode.cpp \
	JoinNode.cpp \
	PartitionNode.cpp \
	ExecutionProxy.cpp \
	ProxyUtilities.cpp \
	LocalFileProxy.cpp \
	RestRequestBuilder.cpp \
	RestDataProxy.cpp \
	ParameterTranslator.cpp \
	TransformerManager.cpp \
	StreamTransformer.cpp \
	DataProxyClient.cpp \
	DatabaseProxy.cpp \
	DatabaseConnectionManager.cpp \

THPPFILES=\
	DatabaseConnectionBinder.thpp \
	GenericDPLDomainTestTypes.thpp \

TESTFILES=\
	main.cpp \
	ProxyTestHelpers.cpp \
	TransformerTestHelpers.cpp \
	NodeFactoryTest.cpp \
	ProxyUtilitiesTest.cpp \
	AbstractNodeTest.cpp \
	RouterNodeTest.cpp \
	JoinNodeTest.cpp \
	PartitionNodeTest.cpp \
	ExecutionProxyTest.cpp \
	LocalFileProxyTest.cpp \
	RestRequestBuilderTest.cpp \
	RestDataProxyTest.cpp \
	DatabaseConnectionManagerTest.cpp \
	DatabaseProxyTest.cpp \
	ParameterTranslatorTest.cpp \
	StreamTransformerTest.cpp \
	TransformerManagerTest.cpp \
	DataProxyClientTest.cpp \
	GenericDPLDomainTest.cpp \

THREADTESTFILES=\
	main.cpp \
	MultithreadDataProxyClientTest.cpp \

MATLABTESTFILES=\
	main.cpp \
	DataProxyWrapperTest.cpp \

MOCKFILES=\
	TestableNode.cpp \
	TestableDataProxyClient.cpp \
	MockNode.cpp \
	MockNodeFactory.cpp \
	MockDatabaseConnectionManager.cpp \
	MockDataProxyClient.cpp \

# The following files need to be built in order to create an independent shared object library
SERVICEFILES=\
	RESTParameters.cpp \
	RESTClient.cpp \
	HttpException.cpp \

UTILITYFILES=\
	DateTime.cpp \
	MVException.cpp \
	Nullable.cpp \
	FileUtilities.cpp \
	StringUtilities.cpp \
	XMLUtilities.cpp \
	ShellExecutor.cpp \
	MVUtility.cpp \
	CSVReader.cpp \
	Stopwatch.cpp \
	UniqueIdGenerator.cpp \
	MutexFileLock.cpp \
	XercesString.cpp \

DATABASEFILES=\
	Database.cpp \
	Bind.cpp \
	OCILOBBind.cpp \
	OCIQueryHandler.cpp \
	OCIdb.cpp \
	ODBCdb.cpp \
	QueryHandler.cpp \
	ODBCQueryHandler.cpp \
	SQLLoader.cpp \
	UniqueKeyInsertOrUpdateDatumStatement.cpp \
	BLOB.cpp \
	CLOB.cpp \
	DatabaseElementBind.cpp \
	DatabaseElementTranslator.cpp \
	LargeScaleSelectStatement.cpp \
	Statement.cpp \

# Creating explicit paths to sets of source files
PRIVATEFILESPEC			= $(PRIVATEFILES:%=$(PRIVATEDIR)/%)
UTILITYFILESPEC			= $(UTILITYFILES:%=$(UTILITYDIR)/$(PRIVATEDIR)/%)
SERVICEFILESPEC			= $(SERVICEFILES:%=$(SERVICEDIR)/$(PRIVATEDIR)/%)
DATABASEFILESPEC		= $(DATABASEFILES:%=$(DATABASEDIR)/$(PRIVATEDIR)/%)
TESTFILESPEC			= $(TESTFILES:%=$(TESTDIR)/%)
THREADTESTFILESPEC		= $(TESTFILES:%=$(TESTDIR)/%)
MATLABTESTFILESPEC		= $(MATLABTESTFILES:%=$(TESTDIR)/%)
MOCKFILESPEC			= $(MOCKFILES:%=$(MOCKDIR)/%)
TESTHELPERFILESPEC		= $(TESTHELPERFILES:%=$(TESTHELPERDIR)/private/%)

# Specifying object destinations
MATLABOBJSPEC			= $(MATLABWRAPPERFILE:%.cpp=%.mexa64)
PRIVATEOBJSPEC			= $(PRIVATEFILES:%.cpp=$(TARGETDIR)/%.o) \
						  $(SERVICEFILES:%.cpp=$(TARGETDIR)/%.o) \
						  $(UTILITYFILES:%.cpp=$(TARGETDIR)/%.o) \
						  $(DATABASEFILES:%.cpp=$(TARGETDIR)/%.o)
HELPEROBJSPEC			= $(TESTHELPERFILES:%.cpp=$(TARGETDIR)/%.o)
TESTOBJSPEC			= $(TESTFILES:%.cpp=$(TARGETDIR)/%.o) 
THREADTESTOBJSPEC		= $(THREADTESTFILES:%.cpp=$(TARGETDIR)/%.o) 
MATLABTESTOBJSPEC		= $(MATLABTESTFILES:%.cpp=$(TARGETDIR)/%.o)
MOCKOBJSPEC			= $(MOCKFILES:%.cpp=$(TARGETDIR)/%.o)
THPP2HPPFILES			= $(THPPFILES:%.thpp=%.hpp)

# Per-target specification
DEBUGOPTS		= -ggdb3 -Wall -Werror -fno-strict-aliasing -fPIC -D DPL_TEST -std=c++0x
OPTIMIZEOPTS	= -ggdb3 -O3 -D MV_OPTIMIZE -Wall -Werror -fno-strict-aliasing -fPIC -std=c++0x
PROFILEOPTS		= -ggdb3 -O3 -D MV_OPTIMIZE -pg -Wall -Werror -fno-strict-aliasing -fPIC -std=c++0x
COVERAGEOPTS		= -fprofile-arcs -ftest-coverage -Wall -Werror -fno-strict-aliasing -fPIC -std=c++0x

# Executable targets
BASE_NAME				= DataProxy
FRIENDLY_TARGET				= lib$(BASE_NAME).so
MAJOR_VERSION_TARGET			= $(FRIENDLY_TARGET).3
MINOR_VERSION_TARGET			= $(MAJOR_VERSION_TARGET).1
FULL_VERSION_TARGET			= $(MINOR_VERSION_TARGET).9
PRIMARY_TARGET				= $(FULL_VERSION_TARGET)
MOCK_TARGET				= libMockDataProxy.a
MATLAB_TARGET				= DataProxy.mexa64
TEST_TARGET				= data_proxy_tests
THREADTEST_TARGET			= multithread_data_proxy_test
MATLAB_TEST_TARGET			= matlab_wrapper_tests
OTHER_TARGETS = $(MATLAB_TARGET) $(TEST_TARGET) $(THREADTEST_TARGET) $(MATLAB_TEST_TARGET) $(MOCK_TARGET)
ALL_TARGETS = $(PRIMARY_TARGET) $(MAJOR_VERSION_TARGET) $(FRIENDLY_TARGET) $(OTHER_TARGETS)

# Defaults for target dir and options
TARGETDIR	?= $(DEBUGDIR)
TARGETOPTS	?= $(DEBUGOPTS)

# Command for linking
LN = ln -sf

# Default build
default: $(TARGETDIR)
	$(MAKE) $(PRIMARY_TARGET)

.PHONY: extlibs matlablibs default check-syntax debug prof opt opt_debug coverage tests test_coverage mock \
	depend localdepend clean localclean nodepend $(ALL_TARGETS)

debug: $(DEBUGDIR)
	$(MAKE) TARGETDIR='$(DEBUGDIR)' TARGETOPTS='$(DEBUGOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

prof: $(PROFILEDIR)
	$(MAKE) TARGETDIR='$(PROFILEDIR)' TARGETOPTS='$(PROFILEOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

opt: $(OPTIMIZEDIR)
	$(MAKE) TARGETDIR='$(OPTIMIZEDIR)' TARGETOPTS='$(OPTIMIZEOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

opt_debug: $(OPTDEBUGDIR)
	$(MAKE) TARGETDIR='$(OPTDEBUGDIR)' TARGETOPTS='$(OPTIMIZEOPTS) $(DEBUGOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

coverage: $(COVERAGEDIR)
	$(MAKE) TARGETDIR='$(COVERAGEDIR)' TARGETOPTS='$(COVERAGEOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

thread_test: $(TARGETDIR) extlibs
	$(MAKE) SUBTARGET=debug $(THREADTEST_TARGET)
	cd $(ROOTDIR)/lib/cpp/TestHelpers && $(MAKE) java-server

tests: $(TARGETDIR) extlibs
	$(MAKE) SUBTARGET=debug $(TEST_TARGET)
	cd $(ROOTDIR)/lib/cpp/TestHelpers && $(MAKE) java-server

matlab_tests: $(TARGETDIR) extlibs matlablibs
	$(MAKE) SUBTARGET=debug $(MATLAB_TEST_TARGET)
	cd $(ROOTDIR)/lib/Logger && $(MAKE) matlab_log_wrapper
	$(MAKE) matlab_wrapper

test_coverage: $(COVERAGEDIR) extlibs
	$(MAKE) TARGETDIR='$(COVERAGEDIR)' TARGETOPTS='$(COVERAGEOPTS)' \
	SUBTARGET=coverage $(TEST_TARGET) \

mock: $(TARGETDIR) extlibs
	$(MAKE) SUBTARGET=debug $(MOCK_TARGET) TARGETOPTS='${DEBUGOPTS}'\

opt_pic: $(OPTIMIZEDIR)
	$(MAKE) TARGETDIR='$(OPTIMIZEDIR)' TARGETOPTS='$(OPTIMIZEOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

matlab_wrapper: $(OPTIMIZEDIR)
	$(MAKE) TARGETDIR='$(OPTIMIZEDIR)' TARGETOPTS='$(OPTIMIZEOPTS)' \
	SUBTARGET=opt_pic $(MATLAB_TARGET)

# Create links to the target dir
$(PRIMARY_TARGET): % : $(TARGETDIR)/% $(TARGETDIR)
	$(LN) $(FULL_VERSION_TARGET) $(TARGETDIR)/$(MINOR_VERSION_TARGET)
	$(LN) $(MINOR_VERSION_TARGET) $(TARGETDIR)/$(MAJOR_VERSION_TARGET)
	$(LN) $(MAJOR_VERSION_TARGET) $(TARGETDIR)/$(FRIENDLY_TARGET)
	$(LN) $(TARGETDIR)/$(FRIENDLY_TARGET) $(FRIENDLY_TARGET)

$(OTHER_TARGETS): % : $(TARGETDIR)/% $(TARGETDIR)
	$(LN) $< $@

$(TARGETDIR)/$(MOCK_TARGET): ${MOCKOBJSPEC}
	ar -crs $@ $^
	ranlib $@

# Building tests
$(TEST_TARGET:%=$(TARGETDIR)/%): $(PRIVATEOBJSPEC) $(TESTOBJSPEC) $(MOCKOBJSPEC)
	$(CXXL) -o $@ $(PRIVATEOBJSPEC) $(TESTOBJSPEC) $(MOCKOBJSPEC) $(TESTLIBS)

$(THREADTEST_TARGET:%=$(TARGETDIR)/%): $(PRIVATEOBJSPEC) $(THREADTESTOBJSPEC) $(MOCKOBJSPEC)
	$(CXXL) -o $@ $(PRIVATEOBJSPEC) $(THREADTESTOBJSPEC) $(MOCKOBJSPEC) $(TESTLIBS)

$(MATLAB_TEST_TARGET:%=$(TARGETDIR)/%): $(MATLAB_TARGET) $(MATLABTESTOBJSPEC)
	$(CXXL) -o $@ $(PRIVATEOBJSPEC) $(HELPEROBJSPEC) $(MATLABTESTOBJSPEC) $(MATLABLIBS) $(TESTLIBS)

# Building libraries & primary targets
$(TARGETDIR)/$(PRIMARY_TARGET): $(PRIVATEOBJSPEC)
	$(CXXS) -o $@ $^ $(LIBLOC) $(LIBS)

$(TARGETDIR)/$(MATLAB_TARGET): $(MATLABWRAPPERFILE) $(PRIMARY_TARGET)
	$(MEXC) $(MATLABWRAPPERFILE) -DMV_OPTIMIZE $(INCS) $(LIBLOC) -L. -lDataProxy $(LIBS) -o $(TARGETDIR)/$(MATLAB_TARGET)

# if necessary, build external libraries.
extlibs: $(MODULESPEC) $(TESTMODULESPEC)
	$(MODULESPEC:%=(cd % && ${MAKE} $(SUBTARGET)) && ) true
	${TESTMODULESPEC:%=(cd % && ${MAKE} ${SUBTARGET}) &&} true
	${TESTMODULESPEC:%=(cd % && ${MAKE} mock) &&} true

matlablibs: $(MATLABMODULESPEC)
	${MATLABMODULESPEC:%=(cd % && ${MAKE} ${SUBTARGET}) &&} true

# Default Builder rules
$(ALLTARGETDIRS):
	mkdir $@

$(TARGETDIR)/%.o: %.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(PRIVATEDIR)/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(SERVICEDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(UTILITYDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(DATABASEDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(TESTHELPERDIR)/private/%.cpp
	$(CXXT) $< -o $@

$(TARGETDIR)/%.o:$(TESTDIR)/%.cpp
	$(CXXT) $< -o $@

$(TARGETDIR)/%.o:$(MOCKDIR)/%.cpp
	$(CXXT) $< -o $@

%.hpp:%.thpp
	$(THPP2HPP) -o $@ $<

$(TESTDIR)/%.hpp:$(TESTDIR)/%.thpp
	$(THPP2HPP) -o $@ $<


#############################################################################

-include Makefile.depend

DEPENDFILES = $(PRIVATEFILESPEC) \
			  $(SERVICEFILESPEC) \
			  $(UTILITYFILESPEC) \
			  $(DATABASEFILESPEC) \
			  $(MATLABWRAPPERFILE) \
			  $(TESTFILESPEC) \
			  $(THREADTESTFILESPEC) \
			  $(MOCKFILESPEC) \
			  $(TESTHELPERFILESPEC) \
			  $(THPP2HPPFILES)

## Make depend will make sure all target directories exist and all .hpp
## files are generated, and will then have g++ generate all dependencies.
## Then, for every line that starts with a non-whitespace character, it
## will prepend "$(TARGETDIR)/" to that line.

depend: localdepend $(MODULESPEC) $(TESTMODULESPEC)
	$(MODULESPEC:%=(cd % && ${MAKE} localdepend) && ) true
	${TESTMODULESPEC:%=(cd % && ${MAKE} localdepend) &&} true

localdepend: $(THPP2HPPFILES)
	$(CXXD) -std=c++0x $(DEPENDFILES) | sed '/^\(\S\)/ s/^/$$(TARGETDIR)\//' > Makefile.depend

nodepend:
	rm -f Makefile.depend

clean: localclean
	$(MODULESPEC:%=(cd % && ${MAKE} localclean) && ) true
	${TESTMODULESPEC:%=(cd % && ${MAKE} localclean) && } true

localclean:
	- rm -f $(ALL_TARGETS) \
			$(THPP2HPPFILES) \
			cppunit_Logger_log.txt \
			data_proxy_tests_result
	- rm -rf $(ALLTARGETDIRS)

LOG_CONFIG_FILE=$(shell pwd)/logger.config
FIND_LOG_IDS=grep -r 'MVLOGGER[^(]*([^"]*"' ./ | grep -v Binary | grep -v '/test/' | sed 's/.*MVLOGGER[^(]*([^"]*"\([^"]*\)"[^,]*,.*/\#log4j.logger.\1=OFF/' | sort -u

logger-config:
	@${FIND_LOG_IDS} > ${LOG_CONFIG_FILE}
	@$(MODULESPEC:%=(cd % && ${FIND_LOG_IDS} >> ${LOG_CONFIG_FILE}) &&) true

install: opt
	@test "$(LIB_DIR)" || (echo LIB_DIR must be defined for $@ && false)
	cp $(OPTIMIZEDIR)/$(PRIMARY_TARGET) $(LIB_DIR)
	$(LN) $(FULL_VERSION_TARGET) $(LIB_DIR)/$(MINOR_VERSION_TARGET)
	$(LN) $(MINOR_VERSION_TARGET) $(LIB_DIR)/$(MAJOR_VERSION_TARGET)
	$(LN) $(MAJOR_VERSION_TARGET) $(LIB_DIR)/$(FRIENDLY_TARGET)
