TRANSFORMERS=\
	Aggregator \
	Blackout \
	ColumnAppender \
	ColumnFormatter \
	Seeder \
	SelfDescribingStreamHeader \
	Shell \
	Validator \
	Custom/CampaignReferenceGenerator \
	Custom/CampaignRevenueVector \
	EquivalenceClass \

NONTEST_TARGETS=depend localdepend clean localclean debug prof opt opt_debug opt_pic

$(NONTEST_TARGETS): $(TRANSFORMERS:%=%/Makefile)
	$(TRANSFORMERS:%=(cd % && $(MAKE) $@) && ) true

tests: $(TRANSFORMERS:%=%/Makefile)
	$(TRANSFORMERS:%=(cd % && $(MAKE) $@ ) && ) true
	$(TRANSFORMERS:%=(cd % && ./*_$@ ) && ) true

install: opt
	@test "$(LIB_DIR)" || (echo LIB_DIR must be defined for $@ && false)
	@test "$(VERSION)" || (echo VERSION must be defined for $@ && false)
	mkdir -p $(LIB_DIR)
	./install.sh $(VERSION) $(LIB_DIR)
