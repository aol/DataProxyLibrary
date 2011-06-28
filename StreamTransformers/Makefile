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

NONTEST_TARGETS=depend localdepend clean localclean debug prof opt opt_debug opt_pic

$(NONTEST_TARGETS): $(TRANSFORMERS:%=%/Makefile)
	$(TRANSFORMERS:%=(cd % && $(MAKE) $@) && ) true

tests: $(TRANSFORMERS:%=%/Makefile)
	$(TRANSFORMERS:%=(cd % && $(MAKE) $@ ) && ) true
	$(TRANSFORMERS:%=(cd % && ./*_$@ ) && ) true
