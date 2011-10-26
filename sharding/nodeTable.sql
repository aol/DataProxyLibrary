DROP TABLE IF EXISTS ${table_name};

CREATE TABLE ${table_name}
(
	media_id INT NOT NULL,
	website_id INT NOT NULL,
	slot_id INT NOT NULL,
	segment_id INT NOT NULL,
	sourced_hourperiod INT NOT NULL,
	campaign_id INT NOT NULL,
	impressions BIGINT NOT NULL,
	clicks BIGINT NOT NULL,
	i2c_conversions BIGINT NOT NULL,
	c2c_conversions BIGINT NOT NULL,
	prematched_conversions BIGINT NOT NULL,
	PRIMARY KEY PK_${table_name) ( media_id, website_id, slot_id, segment_id, sourced_hourperiod ),
	INDEX idx_${table_name} USING BTREE ( sourced_hourperiod )
) ENGINE=MyISAM;
