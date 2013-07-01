#
#   FILE NAME:		$HeadURL$
#
#   DESCRIPTION:
#
#   REVISION:		$Revision$
#
#   COPYRIGHT:		(c) 2007 Advertising.com All Rights Reserved.
#
#   LAST UPDATED:	$Date$
#   UPDATED BY:		$Author$

# Top level directories
ROOTDIR		= ../../../..

# Directories for common components
INCLUDE_DIRS ?= /usr/local/include
LIB_DIRS ?= /usr/local/lib
THPP2HPP = $(ROOTDIR)/lib/cpp/GDP/scripts/thpp2hpp

# Ad.com Modules used by the project, directory path should be fully specified from
# Optimization/ directory -- does not include CPPUNIT
MODULES=\
	lib/cpp/DataProxy \
	lib/cpp/Logger \
	lib/cpp/Monitoring \
	lib/cpp/Utility \
	lib/cpp/Service \

INCMODULES=\
	lib/cpp/GDP \
	lib/cpp/Database \

TESTMODULES=\
	lib/cpp/Service \
	lib/cpp/TestHelpers \

MOCKMODULES=\
	lib/cpp/Service \

MODULESPEC = $(MODULES:%=$(ROOTDIR)/%)
INCMODULESPEC		= $(INCMODULES:%=$(ROOTDIR)/%)
TESTMODULESPEC = $(TESTMODULES:%=$(ROOTDIR)/%)
MOCKMODULESPEC = $(MOCKMODULES:%=$(ROOTDIR)/%)

# Include directories
INCS=\
	-I. \
	$(MODULESPEC:%=-I%) \
	$(INCMODULESPEC:%=-I%) \
	$(INCLUDE_DIRS:%=-I%) \

TESTINCS=\
	$(INCS) \
	-I$(MOCKDIR) \
	-I$(TESTDIR) \
	$(MOCKMODULESPEC:%=-I%/mock) \
	$(TESTMODULESPEC:%=-I%) \

# Library locations
LIBLOC=\
	-L${ORACLE_HOME}/lib \
	$(MODULES:%=-L$(ROOTDIR)/%) \
	$(TESTMODULES:%=-L$(ROOTDIR)/%) \
	$(LIB_DIRS:%=-L%) \

# Libraries
LIBS =	-lDataProxy -lLogger -lMonitoring -lService -lUtility -llog4cxx -lcurl \
		-lagent++ -lsnmp++ \
		-lboost_system -lboost_filesystem -lboost_program_options -lboost_regex \
		-lclntsh -lodbc -lxerces-c -lboost_thread -lboost_iostreams -lcrypto -lpthread -ldl
TESTLIBS = -lTestHelpers -lMockService -lcppunit

ifndef DEFINE_FLAGS
	DEFINE_FLAGS=
endif

# Variables for compiler invocation
CXX ?= g++
CXXC = $(CXX) -std=c++0x $(TARGETOPTS) -c $(INCS) $(DEFINE_FLAGS)		# compiling object files
CXXT = $(CXX) -std=c++0x $(TARGETOPTS) -c $(TESTINCS) $(DEFINE_FLAGS)	# compiling object files for test
CXXL = $(CXX) -std=c++0x $(TARGETOPTS) $(LIBLOC)						# linking executables
CXXD = $(CXX) -std=c++0x -MM $(TESTINCS) $(DEFINE_FLAGS)				# generating dependencies

# Source Directories
PRIVATEDIR				= private
TESTDIR					= test
MOCKDIR					= mock
SYSTESTDIR				= systemtest

# Target Directories
DEBUGDIR	= debug.obj
OPTIMIZEDIR	= opt.obj
OPTDEBUGDIR	= optd.obj
PROFILEDIR	= prof.obj
COVERAGEDIR	= cov.obj
ALLTARGETDIRS = $(DEBUGDIR) $(OPTIMIZEDIR) $(OPTDEBUGDIR) $(PROFILEDIR) $(COVERAGEDIR)

MAINFILE = DataProxyService.cpp

PRIVFILES=\
	DataProxyServiceConfig.cpp \
	AbstractHandler.cpp \
	PingHandler.cpp \
	LoadHandler.cpp \
	StoreHandler.cpp \
	DeleteHandler.cpp \

TESTFILES=\
	test-main.cpp \
	DataProxyServiceConfigTest.cpp \
	PingHandlerTest.cpp \
	LoadHandlerTest.cpp \
	StoreHandlerTest.cpp \
	DeleteHandlerTest.cpp \
	AbstractHandlerTest.cpp \

MOCKFILES=\
	TestableHandler.cpp \

SYSTESTFILES=\
	system-main.cpp \
	DataProxyServiceSystest.cpp \

THPPFILES=\

# Creating explicit paths to sets of source files
MAINFILESPEC					= $(MAINFILE:%=%)
PRIVFILESPEC					= $(PRIVFILES:%=$(PRIVATEDIR)/%)
TESTFILESPEC					= $(TESTFILES:%=$(TESTDIR)/%)
MOCKFILESPEC					= $(MOCKFILES:%=$(MOCKDIR)/%)
SYSTESTFILESPEC					= $(SYSTESTFILES:%=$(SYSTESTDIR)/%)

# Specifying object destinations
MAINOBJSPEC				= $(MAINFILE:%.cpp=$(TARGETDIR)/%.o)
PRIVOBJSPEC				= $(PRIVFILES:%.cpp=$(TARGETDIR)/%.o)
TESTOBJSPEC				= $(TESTFILESPEC:$(TESTDIR)/%.cpp=$(TARGETDIR)/%.o) \
					  	$(MOCKFILES:%.cpp=$(TARGETDIR)/%.o)
SYSTESTOBJSPEC			= $(SYSTESTFILESPEC:$(SYSTESTDIR)/%.cpp=$(TARGETDIR)/%.o)
THPP2HPPFILES			= $(THPPFILES:%.thpp=%.hpp)

# Per-target specification
DEBUGOPTS		= -ggdb3 -Wall -Werror -fno-strict-aliasing
OPTIMIZEOPTS	= -ggdb3 -O3 -D MV_OPTIMIZE -Wall -Werror -fno-strict-aliasing
PROFILEOPTS		= -ggdb3 -O3 -D MV_OPTIMIZE -pg -Wall -Werror -fno-strict-aliasing
COVERAGEOPTS	= -fprofile-arcs -ftest-coverage -Wall -Werror -fno-strict-aliasing

# Executable targets
PRIMARY_TARGET			= dplService
TEST_TARGET				= ${PRIMARY_TARGET}_tests
SYSTEST_TARGET			= ${PRIMARY_TARGET}_systest

# Defaults for target dir and options
TARGETDIR	?= $(DEBUGDIR)
TARGETOPTS	?= $(DEBUGOPTS)

# Command for linking
LN ?= ln -sf

# Default build
default: $(TARGETDIR)
	$(MAKE) $(PRIMARY_TARGET)

.PHONY: $(PRIMARY_TARGET)

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

tests: $(TARGETDIR)
	$(MAKE) SUBTARGET=debug $(TEST_TARGET)

systest: $(TARGETDIR)
	$(MAKE) SUBTARGET=debug $(SYSTEST_TARGET)
	$(MAKE) opt

test_coverage: $(COVERAGEDIR)
	$(MAKE) TARGETDIR='$(COVERAGEDIR)' TARGETOPTS='$(COVERAGEOPTS)' \
	SUBTARGET=coverage $(TEST_TARGET)

# if necessary, build libraries.
extlibs:
	$(MODULES:%=cd $(ROOTDIR)/%;make ${SUBTARGET};cd -;)

testextlibs: extlibs
	$(TESTMODULES:%=cd $(ROOTDIR)/%;make ${SUBTARGET};cd -;)
	$(MOCKMODULES:%=cd $(ROOTDIR)/%;make mock;cd -;)

# Create links to the target dir
$(PRIMARY_TARGET): % : $(TARGETDIR)/% $(TARGETDIR)
	$(LN) $< $@

$(TEST_TARGET): % : $(TARGETDIR)/% $(TARGETDIR)
	$(LN) $< $@

$(SYSTEST_TARGET): % : $(TARGETDIR)/% $(TARGETDIR)
	$(LN) $< $@

# Build the executables
$(TARGETDIR)/$(PRIMARY_TARGET): extlibs ${MAINOBJSPEC} ${PRIVOBJSPEC}
	${CXXL} ${LIBLOC} -o $@ ${MAINOBJSPEC} ${PRIVOBJSPEC} ${LIBS}

$(TEST_TARGET:%=$(TARGETDIR)/%): testextlibs $(PRIVOBJSPEC)
$(TEST_TARGET:%=$(TARGETDIR)/%): $(TESTOBJSPEC)
	$(CXXL) ${LIBLOC} -o $@ $(PRIVOBJSPEC) $(TESTOBJSPEC) \
	${TESTLIBS} ${LIBS}

$(SYSTEST_TARGET:%=$(TARGETDIR)/%): $(PRIMARY_TARGET)
$(SYSTEST_TARGET:%=$(TARGETDIR)/%): testextlibs $(PRIVOBJSPEC)
$(SYSTEST_TARGET:%=$(TARGETDIR)/%): $(SYSTESTOBJSPEC)
	$(CXXL) ${LIBLOC} -o $@ $(PRIVOBJSPEC) $(SYSTESTOBJSPEC) \
	${TESTLIBS} ${LIBS}

# Default Builder rules
$(ALLTARGETDIRS):
	mkdir $@

$(TARGETDIR)/%.o: %.cpp
	${CXXC} $< -o $@

$(TARGETDIR)/%.o: $(PRIVATEDIR)/%.cpp
	${CXXC} $< -o $@

$(TARGETDIR)/%.o: $(TESTDIR)/%.cpp
	${CXXT} $< -o $@

$(TARGETDIR)/%.o: $(MOCKDIR)/%.cpp
	${CXXT} $< -o $@

$(TARGETDIR)/%.o: $(SYSTESTDIR)/%.cpp
	${CXXT} $< -o $@

%.hpp:%.thpp
	$(THPP2HPP) $<

%.hpp:%.fw
	perl $(FW2HPP) $<


#############################################################################
# this target generates a dependency and command list that
# can be cut and pasted into the Makefile below.


-include Makefile.depend

DEPENDFILES=\
	$(MAINFILESPEC) \
	$(PRIVFILESPEC) \
	$(TESTFILESPEC) \
	$(MOCKFILESPEC) \
	$(SYSTESTFILESPEC)

depend: localdepend $(MODULESPEC) $(TESTMODULESPEC)
	$(MODULES:%=(cd $(ROOTDIR)/% && ${MAKE} localdepend) && ) true
	$(TESTMODULES:%=(cd $(ROOTDIR)/% && ${MAKE} localdepend) && ) true

localdepend: $(THPP2HPPFILES)
	$(CXXD) $(DEPENDFILES) | sed '/^\(\S\)/ s/^/$$(TARGETDIR)\//' > Makefile.depend

nodepend:
	rm -f Makefile.depend

clean: localclean
	- $(MODULES:%=(cd $(ROOTDIR)/% && make clean)&&) true
	${TESTMODULESPEC:%=(cd % && ${MAKE} localclean) && } true
	- rm -rf $(ALLTARGETDIRS)

localclean:
	- rm -f $(PRIMARY_TARGET) \
		$(TEST_TARGET) \
		$(SYSTEST_TARGET) \
		$(THPP2HPPFILES) \
		Makefile.depend \
		cppunit_Logger_log.xml \
	- rm -rf $(ALLTARGETDIRS)

LOG_CONFIG_FILE=$(shell pwd)/logger.config
FIND_LOG_IDS=grep -r 'MVLOGGER[^(]*([^"]*"' ./ | grep -v Binary | sed 's/.*MVLOGGER[^(]*([^"]*"\([^"]*\)"[^,]*,.*/\#log4j.logger.\1=OFF/' | sort -u

logger-config:
	@${FIND_LOG_IDS} > ${LOG_CONFIG_FILE}
	@$(MODULES:%=(cd $(ROOTDIR)/% && ${FIND_LOG_IDS} >> ${LOG_CONFIG_FILE}) &&) true
