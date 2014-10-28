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
	lib/cpp/Monitoring \

INCMODULES=\
	lib/cpp/GDP \
	lib/cpp/Service \
	lib/cpp/Utility \
	lib/cpp/Database \

TESTMODULES=\
	lib/cpp/TestHelpers \
	lib/cpp/Service \
	lib/cpp/Utility \
	lib/cpp/Database \
	lib/cpp/Monitoring \

MATLABMODULES=\
	lib/cpp/Matlab \

TRANSFORMERS=\
	Aggregator \

MODULESPEC		= $(MODULES:%=$(ROOTDIR)/%)
INCMODULESPEC		= $(INCMODULES:%=$(ROOTDIR)/%)
TESTMODULESPEC		= $(TESTMODULES:%=$(ROOTDIR)/%)
MATLABMODULESPEC	= $(MATLABMODULES:%=$(ROOTDIR)/%)

# Include directories
INCS=\
	-I. \
	-Iinterface \
	$(MODULESPEC:%=-I%) \
	$(INCMODULESPEC:%=-I%) \
	$(TRANSFORMERS:%=-I%) \
	-I${ROOTDIR}/lib/cpp/Database/private \

TESTINCS=\
	$(INCS) \
	$(TESTMODULESPEC:%=-I%) \
	$(MODULESPEC:%=-I%/mock) \
	$(INCMODULESPEC:%=-I%/mock) \
	$(MATLABMODULESPEC:%=-I%) \
	$(TRANSFORMERS:%=-I%/test) \
	-I$(TESTDIR) \
	-I$(MOCKDIR) \

# Library locations
LIBLOC=\
	$(MODULESPEC:%=-L%) \
	${TESTMODULESPEC:%=-L%} \
	${MATLABMODULESPEC:%=-L%} \

# Libraries
LIBS		=  -lLogger -lMonitoring \
			   -lboost_iostreams -lboost_regex -lboost_filesystem -lboost_program_options -lboost_thread -lboost_system \
			   -lclntsh -lodbc -lxerces-c -lpthread -llog4cxx -lcurl -lssl -luuid -lnnz10 -lcrypto -ldl -lagent++ -lsnmp++
TESTLIBS	= -lcppunit -lTestHelpers -lMockDatabase -lMockService -lMockUtility -lMockMonitoring -lboost_date_time $(LIBS)
MATLABLIBS	= -lMatlab -leng -lmx -lut -lmat

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
CXXS = $(CXX) -shared
MEX ?= mex
MEXC = $(MEX) CXXFLAGS='-std=c++0x -fPIC -fno-omit-frame-pointer -pthread'
SWIG ?= swig
SWIGC = $(SWIG) -c++

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
SWIGGENDIR	= gen-$(lang)
ALLTARGETDIRS = $(DEBUGDIR) $(OPTIMIZEDIR) $(OPTDEBUGDIR) $(PROFILEDIR) $(COVERAGEDIR) $(SWIGGENDIR)

SWIGFILES=\
	SwigDataProxy.i

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
	AggregateStreamTransformer.cpp \
	GroupingAggregateStreamTransformer.cpp \
	AwkUtilities.cpp \
	TransformerUtilities.cpp \
	BlackoutStreamTransformer.cpp \
	ColumnAppenderStreamTransformer.cpp \
	PropertyDomain.cpp \
	EquivalenceClassStreamTransformer.cpp \
	ColumnFormatStreamTransformer.cpp \
	SelfDescribingStreamHeaderTransformer.cpp \
	AtomicsJSONToCSVStreamTransformer.cpp \
	CampaignRevenueVectorStreamTransformer.cpp \
	CampaignReferenceGeneratorStreamTransformer.cpp \
	ValidateStreamTransformer.cpp \
	ShellStreamTransformer.cpp \
	TransformFunctionDomain.cpp \

THPPFILES=\
	DatabaseConnectionBinder.thpp \
	GenericDPLDomainTestTypes.thpp \
	PreCampaignReferenceSerialization.thpp \

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
	AggregateStreamTransformerTest.cpp \
	GroupingAggregateStreamTransformerTest.cpp \
	AwkUtilitiesTest.cpp \
	TransformerUtilitiesTest.cpp \
	BlackoutStreamTransformerTest.cpp \
	ColumnAppenderStreamTransformerTest.cpp \
	PropertyDomainTest.cpp \
	EquivalenceClassStreamTransformerTest.cpp \
	ColumnFormatStreamTransformerTest.cpp \
	SelfDescribingStreamHeaderTransformerTest.cpp \
	AtomicsJSONToCSVStreamTransformerTest.cpp \
	CampaignRevenueVectorStreamTransformerTest.cpp \
	CampaignReferenceGeneratorStreamTransformerTest.cpp \
	ValidateStreamTransformerTest.cpp \
	ShellStreamTransformerTest.cpp \
	TransformFunctionDomainTest.cpp \

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
	MockTransformFunctions.cpp \
	MockTransformFunctionDomain.cpp \

# The following files need to be built in order to create an independent shared object library
SERVICEFILES=\
	HttpException.cpp \
	HTTPRequest.cpp \
	mongoose.cpp \
	MongooseHTTPRequest.cpp \
	MongooseHTTPResponse.cpp \
	RESTParameters.cpp \
	RESTClient.cpp \
	WebServerCommon.cpp \
	WebServerConfig.cpp \
	WebServer.cpp \

UTILITYFILES=\
	CliOptions.cpp \
	CSVReader.cpp \
	DateTime.cpp \
	FileUtilities.cpp \
	MutexFileLock.cpp \
	MVException.cpp \
	MVUtility.cpp \
	NetworkUtilities.cpp \
	Nullable.cpp \
	ShellExecutor.cpp \
	Stopwatch.cpp \
	StringUtilities.cpp \
	TimeTracker.cpp \
	UniqueIdGenerator.cpp \
	UTCTimeProvider.cpp \
	XercesString.cpp \
	XMLUtilities.cpp \
	FilesystemWatcher.cpp \

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

# swig-specific files & objects
GENCPPSWIGFILES		= $(SWIGFILES:%.i=%_wrap.$(lang).cpp)
DIRGENSWIGFILES		= $(GENCPPSWIGFILES:%=$(PRIVATEDIR)/%)
SWIGOBJECTFILES		= $(GENCPPSWIGFILES:%.cpp=$(TARGETDIR)/%.o)

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
						  $(DATABASEFILES:%.cpp=$(TARGETDIR)/%.o) \

HELPEROBJSPEC			= $(TESTHELPERFILES:%.cpp=$(TARGETDIR)/%.o)
TESTOBJSPEC			= $(TESTFILES:%.cpp=$(TARGETDIR)/%.o) 
THREADTESTOBJSPEC		= $(THREADTESTFILES:%.cpp=$(TARGETDIR)/%.o) 
MATLABTESTOBJSPEC		= $(MATLABTESTFILES:%.cpp=$(TARGETDIR)/%.o)
MOCKOBJSPEC			= $(MOCKFILES:%.cpp=$(TARGETDIR)/%.o)
THPP2HPPFILES			= $(THPPFILES:%.thpp=%.hpp)

# Per-target specification
DEBUGOPTS		= -ggdb3 -Wall -Werror -fno-strict-aliasing -fPIC -fvisibility=hidden -D LIBDPL_BUILD -D DPL_TEST -std=c++0x
OPTIMIZEOPTS	= -ggdb3 -O3 -D LIBDPL_BUILD -D MV_OPTIMIZE -Wall -Werror -fno-strict-aliasing -fPIC -fvisibility=hidden -std=c++0x
PROFILEOPTS		= -ggdb3 -O3 -D LIBDPL_BUILD -D MV_OPTIMIZE -pg -Wall -Werror -fno-strict-aliasing -fPIC -fvisibility=hidden -std=c++0x
COVERAGEOPTS		= -D LIBDPL_BUILD -fprofile-arcs -ftest-coverage -Wall -Werror -fno-strict-aliasing -fPIC -fvisibility=hidden -std=c++0x

# Executable targets
BASE_NAME				= DataProxy
FRIENDLY_TARGET			= lib$(BASE_NAME).so
MAJOR_VERSION_TARGET	= $(FRIENDLY_TARGET).3
MINOR_VERSION_TARGET	= $(MAJOR_VERSION_TARGET).2
FULL_VERSION_TARGET		= $(MINOR_VERSION_TARGET).4
PRIMARY_TARGET			= $(FULL_VERSION_TARGET)
MOCK_TARGET				= libMockDataProxy.a
MATLAB_TARGET			= DataProxy.mexa64
TEST_TARGET				= data_proxy_tests
THREADTEST_TARGET		= multithread_data_proxy_test
MATLAB_TEST_TARGET		= matlab_wrapper_tests
OTHER_TARGETS = $(MATLAB_TARGET) $(TEST_TARGET) $(THREADTEST_TARGET) $(MATLAB_TEST_TARGET) $(MOCK_TARGET)
ALL_TARGETS = $(PRIMARY_TARGET) $(MAJOR_VERSION_TARGET) $(FRIENDLY_TARGET) $(OTHER_TARGETS)

# each swig target language has a different target (damnit)
SWIG_TARGET_csharp	= libSwigDataProxy.so
SWIG_TARGET_java	= libSwigDataProxy.so
SWIG_TARGET_octave	= SwigDataProxy.c
SWIG_TARGET_perl	= SwigDataProxy.so
SWIG_TARGET_php		= SwigDataProxy.so
SWIG_TARGET_python	= _SwigDataProxy.so
SWIG_TARGET_ruby	= SwigDataProxy.so
SWIG_TARGET_r		= SwigDataProxy.so

# some swig targets require special parameters when compiling wrappers to object files
SWIGOPTS_perl	= -Dbool=char
SWIGOPTS_php	= `php-config --includes`
SWIGOPTS_python	= -Wno-write-strings

# Defaults for target dir and options
TARGETDIR	?= $(DEBUGDIR)
TARGETOPTS	?= $(DEBUGOPTS)

# Command for linking
LN = ln -sf

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

thread_test: $(TARGETDIR)
	$(MAKE) SUBTARGET=debug $(THREADTEST_TARGET)
	cd $(ROOTDIR)/lib/cpp/TestHelpers && $(MAKE) java-server

tests: $(TARGETDIR)
	$(MAKE) SUBTARGET=debug $(TEST_TARGET)
	cd $(ROOTDIR)/lib/cpp/TestHelpers && $(MAKE) java-server

matlab_tests: $(TARGETDIR)
	$(MAKE) SUBTARGET=debug $(MATLAB_TEST_TARGET)
	cd $(ROOTDIR)/lib/Logger && $(MAKE) matlab_log_wrapper
	$(MAKE) matlab_wrapper

test_coverage: $(COVERAGEDIR)
	$(MAKE) TARGETDIR='$(COVERAGEDIR)' TARGETOPTS='$(COVERAGEOPTS)' \
	SUBTARGET=coverage $(TEST_TARGET) \

mock: $(TARGETDIR)
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
$(TEST_TARGET:%=$(TARGETDIR)/%): $(PRIVATEOBJSPEC) $(TESTOBJSPEC) $(MOCKOBJSPEC) testextlibs
	$(CXXL) -o $@ $(PRIVATEOBJSPEC) $(TESTOBJSPEC) $(MOCKOBJSPEC) $(TESTLIBS)

$(THREADTEST_TARGET:%=$(TARGETDIR)/%): $(PRIVATEOBJSPEC) $(THREADTESTOBJSPEC) $(MOCKOBJSPEC) testextlibs
	$(CXXL) -o $@ $(PRIVATEOBJSPEC) $(THREADTESTOBJSPEC) $(MOCKOBJSPEC) $(TESTLIBS)

$(MATLAB_TEST_TARGET:%=$(TARGETDIR)/%): $(MATLAB_TARGET) $(MATLABTESTOBJSPEC) testextlibs matlablibs
	$(CXXL) -o $@ $(PRIVATEOBJSPEC) $(HELPEROBJSPEC) $(MATLABTESTOBJSPEC) $(MATLABLIBS) $(TESTLIBS)

# Building libraries & primary targets
$(TARGETDIR)/$(PRIMARY_TARGET): $(PRIVATEOBJSPEC) extlibs
	$(CXXS) -Wl,-soname,$(MINOR_VERSION_TARGET) -o $@ $(PRIVATEOBJSPEC) $(LIBLOC) $(LIBS)

$(TARGETDIR)/$(MATLAB_TARGET): $(MATLABWRAPPERFILE) $(PRIMARY_TARGET) extlibs matlablibs
	$(MEXC) $(MATLABWRAPPERFILE) -DMV_OPTIMIZE $(INCS) $(LIBLOC) -L. -lDataProxy $(LIBS) -o $(TARGETDIR)/$(MATLAB_TARGET)

# swig targets
swig:
	@test "$(lang)" || (echo "lang must be defined for $@ (currently supported: csharp, java, octave, perl, php, python, ruby, r)" && false)
	$(MAKE) TARGETDIR='$(OPTIMIZEDIR)' SUBTARGET='opt' swig-build 

swig-build: opt $(SWIGGENDIR)
	$(MAKE) TARGETDIR='$(OPTIMIZEDIR)' SUBTARGET='opt' OPTS='$(SWIGOPTS_$(lang))' SWIG_TARGET_$(lang)

SWIG_TARGET_csharp: $(SWIGOBJECTFILES)
	$(CXXS) -o $(SWIGGENDIR)/$($@) $^ -L. -lDataProxy

SWIG_TARGET_java: $(SWIGOBJECTFILES)
	$(CXXS) -o $(SWIGGENDIR)/$($@) $^ -L. -lDataProxy

SWIG_TARGET_octave: $(DIRGENSWIGFILES)
	mkoctfile $^ $(SWIGGENDIR)/$($@)

SWIG_TARGET_perl: $(SWIGOBJECTFILES)
	$(CXXS) -o $(SWIGGENDIR)/$($@) $^ -L. -lDataProxy

SWIG_TARGET_php: $(SWIGOBJECTFILES)
	$(CXXS) -o $(SWIGGENDIR)/$($@) $^ -L. -lDataProxy

SWIG_TARGET_python: $(SWIGOBJECTFILES)
	$(CXXS) -o $(SWIGGENDIR)/$($@) $^ -L. -lDataProxy

SWIG_TARGET_ruby: $(SWIGOBJECTFILES)
	$(CXXS) -o $(SWIGGENDIR)/$($@) $^ -L. -lDataProxy

SWIG_TARGET_r: $(DIRGENSWIGFILES)
	PKG_LIBS="$($@:%.so=%.cxx)" R CMD SHLIB $@

$(SWIGOBJECTFILES): $(DIRGENSWIGFILES)
	$(CXXC) $(OPTS) $< -Wno-unused-variable -o $@

$(DIRGENSWIGFILES): $(SWIGFILES)
	$(SWIGC) -o $@ -$(lang) -outdir $(SWIGGENDIR) $<

# if necessary, build external libraries.
extlibs: $(MODULESPEC)
	$(MODULESPEC:%=(cd % && ${MAKE} $(SUBTARGET)) && ) true

testextlibs: $(TESTMODULESPEC) extlibs
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
			$(SWIGFILES:%.i=$(PRIVATEDIR)/%_wrap.*.cpp) \
			gen-* \
			Makefile.depend \
			cppunit_Logger_log.txt \
	- rm -rf $(ALLTARGETDIRS)

LOG_CONFIG_FILE=$(shell pwd)/logger.config
FIND_LOG_IDS=grep -r 'MVLOGGER[^(]*([^"]*"' ./ | grep -v Binary | grep -v '/test/' | sed 's/.*MVLOGGER[^(]*([^"]*"\([^"]*\)"[^,]*,.*/\#log4j.logger.\1=OFF/' | sort -u

logger-config:
	@${FIND_LOG_IDS} > ${LOG_CONFIG_FILE}
	@$(MODULESPEC:%=(cd % && ${FIND_LOG_IDS} >> ${LOG_CONFIG_FILE}) &&) true
	@$(INCMODULESPEC:%=(cd % && ${FIND_LOG_IDS} >> ${LOG_CONFIG_FILE}) &&) true

install: opt
	@test "$(LIB_DIR)" || (echo LIB_DIR must be defined for $@ && false)
	cp $(OPTIMIZEDIR)/$(PRIMARY_TARGET) $(LIB_DIR)
	$(LN) $(FULL_VERSION_TARGET) $(LIB_DIR)/$(MINOR_VERSION_TARGET)
	$(LN) $(MINOR_VERSION_TARGET) $(LIB_DIR)/$(MAJOR_VERSION_TARGET)
	$(LN) $(MAJOR_VERSION_TARGET) $(LIB_DIR)/$(FRIENDLY_TARGET)
