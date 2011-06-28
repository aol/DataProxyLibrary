#
#   FILE NAME:		$RCSfile: Makefile,v $
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
THPP2HPP = $(ROOTDIR)/lib/perl/Utility/thpp2hpp

# Ad.com Modules used by the project, directory path should be fully specified from
# Optimization/ directory -- does not include CPPUNIT
MODULES=\
	lib/cpp/Utility \
	lib/cpp/Logger \
	lib/cpp/GDP \
	lib/cpp/Service \
	lib/cpp/Database \
	lib/cpp/DataProxy \

TESTMODULES=\
	lib/cpp/Service \
	lib/cpp/TestHelpers \

MODULESPEC = $(MODULES:%=$(ROOTDIR)/%)
TESTMODULESPEC = $(TESTMODULES:%=$(ROOTDIR)/%)

# Include directories
INCS=\
	-I. \
	$(MODULESPEC:%=-I%) \
	$(INCLUDE_DIRS:%=-I%) \

TESTINCS=\
	$(INCS) \
	-I$(MOCKDIR) \
	-I$(TESTDIR) \
	$(MODULESPEC:%=-I%/mock) \
	$(TESTMODULESPEC:%=-I%) \

# Library locations
LIBLOC=\
	-L${ORACLE_HOME}/lib \
	$(MODULES:%=-L$(ROOTDIR)/%) \
	$(TESTMODULES:%=-L$(ROOTDIR)/%) \
	$(LIB_DIRS:%=-L%) \

# Libraries
LIBS =	-lService -lUtility -lLogger -llog4cxx -lcurl -lDataProxy \
		-lboost_filesystem -lboost_program_options -lboost_regex -lclntsh -lmyodbc3 -lxerces-c -lboost_thread -lboost_iostreams
TESTLIBS = -lTestHelpers -lMockService -lcppunit

ifndef DEFINE_FLAGS
	DEFINE_FLAGS=
endif

# Variables for compiler invocation
CXX ?= g++
CXXC = $(CXX) $(TARGETOPTS) -c $(INCS) $(DEFINE_FLAGS)		# compiling object files
CXXT = $(CXX) $(TARGETOPTS) -c $(TESTINCS) $(DEFINE_FLAGS)	# compiling object files for test
CXXL = $(CXX) $(TARGETOPTS) $(LIBLOC)						# linking executables
CXXD = $(CXX) -MM $(TESTINCS) $(DEFINE_FLAGS)				# generating dependencies

# Source Directories
PRIVATEDIR				= private
TESTDIR					= test
MOCKDIR					= test/common
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
	LoadHandler.cpp \
	StoreHandler.cpp \

TESTFILES=\
	test-main.cpp \
	DataProxyServiceConfigTest.cpp \
	LoadHandlerTest.cpp \
	StoreHandlerTest.cpp \

MOCKFILES=\

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
DEBUGOPTS				= -ggdb3 -Wall -Werror
OPTIMIZEOPTS			= -O3 -D MV_OPTIMIZE -Wall -Werror
PROFILEOPTS				= -O3 -D MV_OPTIMIZE -pg -Wall -Werror
COVERAGEOPTS			= -fprofile-arcs -ftest-coverage -Wall -Werror

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

test_coverage: $(COVERAGEDIR)
	$(MAKE) TARGETDIR='$(COVERAGEDIR)' TARGETOPTS='$(COVERAGEOPTS)' \
	SUBTARGET=coverage $(TEST_TARGET)

# if necessary, build libraries.
extlibs:
	$(MODULES:%=cd $(ROOTDIR)/%;make ${SUBTARGET};cd -;)
	$(TESTMODULES:%=cd $(ROOTDIR)/%;make ${SUBTARGET};cd -;)

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

$(TEST_TARGET:%=$(TARGETDIR)/%): extlibs $(PRIVOBJSPEC)
$(TEST_TARGET:%=$(TARGETDIR)/%): $(TESTOBJSPEC)
	$(CXXL) ${LIBLOC} -o $@ $(PRIVOBJSPEC) $(TESTOBJSPEC) \
	${TESTLIBS} ${LIBS}

$(SYSTEST_TARGET:%=$(TARGETDIR)/%): $(PRIMARY_TARGET)
$(SYSTEST_TARGET:%=$(TARGETDIR)/%): extlibs $(PRIVOBJSPEC)
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

depend: localdepend
	$(MODULES:%=(cd $(ROOTDIR)/% && ${MAKE} localdepend) && ) true
	$(TESTMODULES:%=(cd $(ROOTDIR)/% && ${MAKE} localdepend) && ) true

localdepend: $(THPP2HPPFILES)
	$(CXXD) $(DEPENDFILES) | sed '/^\(\S\)/ s/^/$$(TARGETDIR)\//' > Makefile.depend

nodepend:
	rm -f Makefile.depend

clean: localclean
	- $(MODULES:%=(cd $(ROOTDIR)/% && make clean)&&) true
	- rm -rf $(ALLTARGETDIRS)

localclean:
	- rm -f $(PRIMARY_TARGET) \
		$(TEST_TARGET) \
		$(SYSTEST_TARGET) \
		$(THPP2HPPFILES) \
		cppunit_Logger_log.xml \
		data_proxy_service_system_test_results \
		data_proxy_service_test_results
	- rm -rf $(ALLTARGETDIRS)

LOG_CONFIG_FILE=$(shell pwd)/logger.config
FIND_LOG_IDS=grep -r 'MVLOGGER[^(]*([^"]*"' ./ | grep -v Binary | sed 's/.*MVLOGGER[^(]*([^"]*"\([^"]*\)"[^,]*,.*/\#log4j.logger.\1=OFF/' | sort -u

logger-config:
	@${FIND_LOG_IDS} > ${LOG_CONFIG_FILE}
	@$(MODULES:%=(cd $(ROOTDIR)/% && ${FIND_LOG_IDS} >> ${LOG_CONFIG_FILE}) &&) true
