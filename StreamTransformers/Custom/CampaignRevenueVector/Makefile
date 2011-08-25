# Top level directories
ROOTDIR		 = ../../../../../..

# Directories for common components
INCLUDE_DIRS ?= /usr/local/include
LIB_DIRS ?= /usr/local/lib

# folders that contain files we need to manually compile
# in order to build a self-contained dynamic library
UTILITYDIR		= ${ROOTDIR}/lib/cpp/Utility
SERVICEDIR		= ${ROOTDIR}/lib/cpp/Service
DATAPROXYDIR 	= ${ROOTDIR}/lib/cpp/DataProxy

# for auto-generating hpp files from thpp for GDP
THPP2HPP        = $(ROOTDIR)/lib/cpp/GDP/scripts/thpp2hpp

TRANSFORMERSCOMMONDIR = ../../Common

#Modules
MODULES=\
	lib/cpp/Logger \
	lib/cpp/Utility \
	lib/cpp/DataProxy	\
	lib/cpp/GDP \
	lib/cpp/Database	\
	lib/cpp/Service	\

TESTMODULES=\
	lib/cpp/TestHelpers	\

MODULESPEC		= $(MODULES:%=$(ROOTDIR)/%)
TESTMODULESPEC  = $(TESTMODULES:%=$(ROOTDIR)/%)

# Include directories
INCS=\
	-I. \
	-I$(TRANSFORMERSCOMMONDIR) \
	$(MODULESPEC:%=-I%) \
	$(INCLUDE_DIRS:%=-I%) \

TESTINCS=\
	$(INCS) \
	$(TESTMODULESPEC:%=-I%) \
	-I$(TESTDIR) \

# Library locations
LIBLOC=\
	$(MODULESPEC:%=-L%) \
	${TESTMODULESPEC:%=-L%} \
	${LIB_DIRS:%=-L%} \

# Libraries
LIBS		= -lLogger -lboost_regex -lboost_filesystem -llog4cxx -lboost_thread -lcurl
TESTLIBS	= -lcppunit -lTestHelpers $(LIBS)

# This is required on the 32 bit platform to prevent recursive
# variable definition errors for DEFINE_FLAGS
ifndef DEFINE_FLAGS
	DEFINE_FLAGS=
endif

# Variables for compiler invocation
CXX ?= g++
CXXC = $(CXX) $(TARGETOPTS) -c $(INCS) $(DEFINE_FLAGS)	# compiling object files
CXXT = $(CXX) $(TARGETOPTS) -c $(TESTINCS) $(DEFINE_FLAGS)	# compiling object files
CXXL = $(CXX) $(TARGETOPTS) $(LIBLOC)					# linking executables
CXXD = $(CXX) -MM $(TESTINCS) $(DEFINE_FLAGS)				# generating dependencies
CXXS = $(CXX) -shared -Wl,-soname,$(VERSION_TARGET)

# Source Directories
PRIVATEDIR				= private
TESTDIR					= test

# Target Directories
DEBUGDIR	= debug.obj
OPTIMIZEDIR	= opt.obj
OPTDEBUGDIR	= optd.obj
PROFILEDIR	= prof.obj
ALLTARGETDIRS = $(DEBUGDIR) $(OPTIMIZEDIR) $(OPTDEBUGDIR) $(PROFILEDIR)

PRIVATEFILES=\
	CampaignRevenueVectorStreamTransformer.cpp \

TRANSFORMERSCOMMONFILES=\
	AwkUtilities.cpp \
	TransformerUtilities.cpp \

THPPFILES=\

TESTFILES=\
	main.cpp \
	CampaignRevenueVectorStreamTransformerTest.cpp \

# The following files need to be built in order to create an independent shared object library
UTILITYFILES=\
	MVException.cpp \
	Nullable.cpp \
	ShellExecutor.cpp \
	Stopwatch.cpp \
	StringUtilities.cpp \

# Creating explicit paths to sets of source files
PRIVATEFILESPEC				= $(PRIVATEFILES:%=$(PRIVATEDIR)/%)
TRANSFORMERSCOMMONFILESPEC	= $(TRANSFORMERSCOMMONFILES:%=$(TRANSFORMERSCOMMONDIR)/%)
TESTFILESPEC				= $(TESTFILES:%=$(TESTDIR)/%)

# Specifying object destinations
PRIVATEOBJSPEC			= $(PRIVATEFILES:%.cpp=$(TARGETDIR)/%.o) \
						  $(TRANSFORMERSCOMMONFILES:%.cpp=$(TARGETDIR)/%.o) \
						  $(COREFILES:%.cpp=$(TARGETDIR)/%.o) \
						  $(UTILITYFILES:%.cpp=$(TARGETDIR)/%.o)
TESTOBJSPEC				= $(TESTFILES:%.cpp=$(TARGETDIR)/%.o)
THPP2HPPFILES			= $(THPPFILES:%.thpp=%.hpp)

# Per-target specification
DEBUGOPTS		= -ggdb3 -Wall -Werror -fno-strict-aliasing -fPIC -D DPL_TEST
OPTIMIZEOPTS	= -O3 -D MV_OPTIMIZE -Wall -Werror -fno-strict-aliasing -fPIC
PROFILEOPTS		= -O3 -D MV_OPTIMIZE -pg -Wall -Werror -fno-strict-aliasing -fPIC

# Executable targets
PRIMARY_TARGET					= libCampaignRevenueVectorStreamTransformer.so
TEST_TARGET						= campaign_revenue_vector_tests
ALL_TARGETS = $(PRIMARY_TARGET) $(TEST_TARGET)

# Defaults for target dir and options
TARGETDIR	?= $(DEBUGDIR)
TARGETOPTS	?= $(DEBUGOPTS)

# Command for linking
LN ?= ln -sf

# Default build
default: $(TARGETDIR)
	$(MAKE) $(PRIMARY_TARGET)

.PHONY: extlibs default debug prof opt opt_debug tests \
	depend localdepend clean localclean nodepend $(ALL_TARGETS)

cvs-tag:
	@test "$(VERSION)" || (echo VERSION must be defined for $@ && false)
	(mkdir $(SYSTEM_TEMP_DIR)/$(PRIMARY_TARGET)-tag && \
	cd $(SYSTEM_TEMP_DIR)/$(PRIMARY_TARGET)-tag && \
	cvs co $(MODULES:%=Optimization/%) $(TESTMODULES:%=Optimization/%) Optimization/$(PROJECT_PATH) Optimization/CppUnit && \
	cd Optimization/CppUnit && \
	sh configure && \
	make -j && \
	cd ../$(PROJECT_PATH) && \
	make depend && \
	make -j tests && \
	./$(TEST_TARGET) && \
	make -j opt && \
	cd $(ROOTDIR) && cd .. && \
	cvs tag $(CVS_TAG_STRING) Optimization && \
	rm -rf $(SYSTEM_TEMP_DIR)/$(PRIMARY_TARGET)-tag)

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

tests: $(TARGETDIR) extlibs
	$(MAKE) SUBTARGET=debug $(TEST_TARGET)

opt_pic: $(OPTIMIZEDIR)
	$(MAKE) TARGETDIR='$(OPTIMIZEDIR)' TARGETOPTS='$(OPTIMIZEOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

# Create links to the target dir
$(ALL_TARGETS): % : $(TARGETDIR)/% $(TARGETDIR)
	$(LN) $< $@

# Building tests
$(TEST_TARGET:%=$(TARGETDIR)/%): $(PRIVATEOBJSPEC) $(TESTOBJSPEC)
	$(CXXL) -o $@ $(PRIVATEOBJSPEC) $(TESTOBJSPEC) $(TESTLIBS) $(LIBS) $(CPPUNITLIBS)

# Building libraries & primary targets
$(TARGETDIR)/$(PRIMARY_TARGET): $(PRIVATEOBJSPEC)
	$(CXXS) -o $@ $^

# if necessary, build external libraries.
extlibs: $(MODULESPEC) $(TESTMODULESPEC)
	$(MODULESPEC:%=(cd % && ${MAKE} $(SUBTARGET)) && ) true
	${TESTMODULESPEC:%=(cd % && ${MAKE} ${SUBTARGET}) &&} true

# Default Builder rules
$(ALLTARGETDIRS):
	mkdir $@

$(TARGETDIR)/%.o: %.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(PRIVATEDIR)/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(TRANSFORMERSCOMMONDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(COREDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(UTILITYDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(TESTDIR)/%.cpp
	$(CXXT) $< -o $@

%.hpp:%.thpp
	$(THPP2HPP) -o $@ $<

$(TESTDIR)/%.hpp:$(TESTDIR)/%.thpp
	$(THPP2HPP) -o $@ $<


#############################################################################

-include Makefile.depend

DEPENDFILES = $(PRIVATEFILESPEC) \
			  $(TESTFILESPEC) \
			  $(THPPFILES)

## Make depend will make sure all target directories exist and all .hpp
## files are generated, and will then have g++ generate all dependencies.
## Then, for every line that starts with a non-whitespace character, it
## will prepend "$(TARGETDIR)/" to that line.

depend: localdepend $(MODULESPEC) $(TESTMODULESPEC)
	$(MODULESPEC:%=(cd % && ${MAKE} localdepend) && ) true
	${TESTMODULESPEC:%=(cd % && ${MAKE} localdepend) &&} true

localdepend: $(THPP2HPPFILES)
	$(CXXD) $(DEPENDFILES) | sed '/^\(\S\)/ s/^/$$(TARGETDIR)\//' > Makefile.depend

nodepend:
	rm -f Makefile.depend

clean: localclean
	$(MODULESPEC:%=(cd % && ${MAKE} localclean) && ) true
	${TESTMODULESPEC:%=(cd % && ${MAKE} localclean) && } true

localclean:
	- rm -f $(ALL_TARGETS) \
			$(THPP2HPPFILES) \
			cppunit_Logger_log.txt \
			aggregate_stream_transformer_tests_result
	- rm -rf $(ALLTARGETDIRS)
