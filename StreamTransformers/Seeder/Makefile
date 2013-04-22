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
ROOTDIR		 = ../../../../../

# Directories for common components
ARCH=$(shell uname -m)
MYSQLHOME	?= /usr/local/mysql
ORACLE_HOME	?= /usr/oracle
INCLUDE_DIRS ?= /usr/local/include
LIB_DIRS ?= /usr/local/lib

# folders that contain files we need to manually compile
# in order to build a self-contained dynamic library
UTILITYDIR	= ${ROOTDIR}/lib/cpp/Utility
SERVICEDIR	= ${ROOTDIR}/lib/cpp/Service
DATAPROXYDIR	= ${ROOTDIR}/lib/cpp/DataProxy
DATABASEDIR	= ${ROOTDIR}/lib/cpp/Database

# for auto-generating hpp files from thpp for GDP
GDPDIR		= ${ROOTDIR}/lib/GDP
THPP2HPP        = $(GDPDIR)/scripts/thpp2hpp

TRANSFORMERSCOMMONDIR = ../Common

#Modules
MODULES=\
	lib/cpp/Logger \
	lib/cpp/Utility \
	lib/cpp/Service \
	lib/cpp/GDP \
	lib/cpp/Database \
	lib/cpp/DataProxy \

TESTMODULES=\
	lib/cpp/TestHelpers \
	lib/cpp/Database \

MOCKMODULES=\
	lib/cpp/DataProxy \

MODULESPEC	= $(MODULES:%=$(ROOTDIR)/%)
TESTMODULESPEC  = $(TESTMODULES:%=$(ROOTDIR)/%)
MOCKMODULESPEC  = $(MOCKMODULES:%=$(ROOTDIR)/%)

# Include directories
INCS=\
	-I. \
	-I$(TRANSFORMERSCOMMONDIR) \
	$(MODULESPEC:%=-I%) \
	$(INCLUDE_DIRS:%=-I%) \
	-I${MYSQLHOME}/include \
	-I${ORACLE_HOME}/rdbms/demo \
	-I${ORACLE_HOME}/network/public \
	-I${ORACLE_HOME}/rdbms/public \

TESTINCS=\
	$(INCS) \
	$(TESTMODULESPEC:%=-I%) \
	$(MOCKMODULESPEC:%=-I%/mock) \
	-I$(TESTDIR) \

# Library locations
LIBLOC=\
	$(MODULESPEC:%=-L%) \
	${TESTMODULESPEC:%=-L%} \
	${LIB_DIRS:%=-L%} \
	-L${ORACLE_HOME}/lib \
	-L/data/lib \

# Libraries
LIBS	    = -lDataProxy -lLogger -lmyodbc3 -lxerces-c -lpthread -lboost_regex -lboost_filesystem -llog4cxx -lboost_thread -lcurl -lclntsh -lmyodbc3
TESTLIBS    = -lcppunit -lTestHelpers -lDatabase -lclntsh -lmyodbc3 -lMockDataProxy

# This is required on the 32 bit platform to prevent recursive
# variable definition errors for DEFINE_FLAGS
ifndef DEFINE_FLAGS
	DEFINE_FLAGS=
endif

# Variables for compiler invocation
CXX ?= g++
CXXC = $(CXX) $(TARGETOPTS) -c $(INCS) $(DEFINE_FLAGS) -std=c++0x	# compiling object files
CXXT = $(CXX) $(TARGETOPTS) -c $(TESTINCS) $(DEFINE_FLAGS) -std=c++0x	# compiling object files
CXXL = $(CXX) $(TARGETOPTS) $(LIBLOC) -std=c++0x			# linking executables
CXXD = $(CXX) -MM $(TESTINCS) $(DEFINE_FLAGS) -std=c++0x		# generating dependencies
CXXS = $(CXX) $(LIBLOC) -shared $(LIBLOC) $(LIBS) -std=c++0x

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
	SeederEquivalenceClassDomain.cpp \
	SeederStreamTransformer.cpp \

TRANSFORMERSCOMMONFILES=\
	AwkUtilities.cpp \
	TransformerUtilities.cpp \

THPPFILES=\

TESTFILES=\
	main.cpp \
	SeederEquivalenceClassDomainTest.cpp \
	SeederStreamTransformerTest.cpp \


# The following files need to be built in order to create an independent shared object library
UTILITYFILES=\
	FileUtilities.cpp \
	XMLUtilities.cpp \
	MVUtility.cpp \
	CSVReader.cpp \
	StringUtilities.cpp \
	MVException.cpp \
	Nullable.cpp \
	ShellExecutor.cpp \
	Stopwatch.cpp \

# Creating explicit paths to sets of source files
PRIVATEFILESPEC			= $(PRIVATEFILES:%=$(PRIVATEDIR)/%)
TRANSFORMERSCOMMONFILESPEC	= $(TRANSFORMERSCOMMONFILES:%=$(TRANSFORMERSCOMMONDIR)/%)
TESTFILESPEC			= $(TESTFILES:%=$(TESTDIR)/%)
TESTCOMMONFILESPEC		= $(TESTCOMMONFILES:%=$(TESTCOMMONDIR)/%)

# Specifying object destinations
PRIVATEOBJSPEC			= $(PRIVATEFILES:%.cpp=$(TARGETDIR)/%.o) \
						  $(TRANSFORMERSCOMMONFILES:%.cpp=$(TARGETDIR)/%.o) \
						  $(UTILITYFILES:%.cpp=$(TARGETDIR)/%.o)
TESTOBJSPEC				= $(TESTFILES:%.cpp=$(TARGETDIR)/%.o) 
TESTCOMMONOBJSPEC		= $(TESTCOMMONFILES:%.cpp=$(TARGETDIR)/%.o)
THPP2HPPFILES			= $(THPPFILES:%.thpp=%.hpp)

# Per-target specification
DEBUGOPTS		= -ggdb3 -Wall -Werror -fno-strict-aliasing -fPIC -D DPL_TEST -std=c++0x
OPTIMIZEOPTS	= -ggdb3 -O3 -D MV_OPTIMIZE -Wall -Werror -fno-strict-aliasing -fPIC -std=c++0x
PROFILEOPTS		= -ggdb3 -O3 -D MV_OPTIMIZE -pg -Wall -Werror -fno-strict-aliasing -fPIC -std=c++0x

# Executable targets
PRIMARY_TARGET					= libSeederStreamTransformer.so
TEST_TARGET						= seeder_stream_transformer_tests
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

debug: $(DEBUGDIR) extlibs
	$(MAKE) TARGETDIR='$(DEBUGDIR)' TARGETOPTS='$(DEBUGOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

prof: $(PROFILEDIR) extlibs
	$(MAKE) TARGETDIR='$(PROFILEDIR)' TARGETOPTS='$(PROFILEOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

opt: $(OPTIMIZEDIR) extlibs
	$(MAKE) TARGETDIR='$(OPTIMIZEDIR)' TARGETOPTS='$(OPTIMIZEOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

opt_debug: $(OPTDEBUGDIR) extlibs
	$(MAKE) TARGETDIR='$(OPTDEBUGDIR)' TARGETOPTS='$(OPTIMIZEOPTS) $(DEBUGOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

tests: $(TARGETDIR) testextlibs
	$(MAKE) SUBTARGET=debug $(TEST_TARGET)

opt_pic: $(OPTIMIZEDIR) extlibs
	$(MAKE) TARGETDIR='$(OPTIMIZEDIR)' TARGETOPTS='$(OPTIMIZEOPTS)' \
	SUBTARGET=$@ $(PRIMARY_TARGET)

# Create links to the target dir
$(ALL_TARGETS): % : $(TARGETDIR)/% $(TARGETDIR)
	$(LN) $< $@

# Building tests
$(TEST_TARGET:%=$(TARGETDIR)/%): $(PRIVATEOBJSPEC) $(TESTOBJSPEC) $(TESTCOMMONOBJSPEC)
	$(CXXL) -o $@ $(PRIVATEOBJSPEC) $(TESTOBJSPEC) $(TESTCOMMONOBJSPEC) $(TESTLIBS) $(LIBS) $(CPPUNITLIBS)

# Building libraries & primary targets
$(TARGETDIR)/$(PRIMARY_TARGET): $(PRIVATEOBJSPEC)
	$(CXXS) -o $@ $^

# if necessary, build external libraries.
extlibs: $(MODULESPEC)
	$(MODULESPEC:%=(cd % && ${MAKE} $(SUBTARGET)) && ) true

testextlibs: $(TESTMODULESPEC) $(MOCKMODULESPEC)
	${TESTMODULESPEC:%=(cd % && ${MAKE} ${SUBTARGET}) &&} true
	${MOCKMODULESPEC:%=(cd % && ${MAKE} mock) &&} true

# Default Builder rules
$(ALLTARGETDIRS):
	mkdir $@

$(TARGETDIR)/%.o: %.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(PRIVATEDIR)/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(UTILITYDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(SERVICEDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(DATABASEDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(TRANSFORMERSCOMMONDIR)/private/%.cpp
	$(CXXC) $< -o $@

$(TARGETDIR)/%.o:$(TESTCOMMONDIR)/%.cpp
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
			  $(TESTCOMMONFILESPEC) \
			  $(TESTHELPERFILESPEC) \
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
			seeder_stream_transformer_tests_result
	- rm -rf $(ALLTARGETDIRS)
