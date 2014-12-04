DROP TABLE IF EXISTS shard_tables;
DROP TABLE IF EXISTS stg_shard_tables;
DROP TABLE IF EXISTS shard_nodes;
DROP TABLE IF EXISTS stg_shard_nodes;

CREATE TABLE shard_nodes
(
	node_id INTEGER NOT NULL AUTO_INCREMENT,
	type VARCHAR(64) NOT NULL,
	server VARCHAR(64) NOT NULL,
	db_name VARCHAR(64) NOT NULL,
	db_user VARCHAR(64) NOT NULL,
	db_password VARCHAR(64) NOT NULL,
	db_schema VARCHAR(64),
	disable_cache TINYINT(1),
	CONSTRAINT cpk_shard_nodes PRIMARY KEY (node_id),
	CONSTRAINT cunq_shard_nodes UNIQUE (type, server, db_name, db_user, db_schema)
) ENGINE=INNODB;


CREATE TABLE stg_shard_nodes
(
	type VARCHAR(64) NOT NULL,
	server VARCHAR(64) NOT NULL,
	db_name VARCHAR(64) NOT NULL,
	db_user VARCHAR(64) NOT NULL,
	db_password VARCHAR(64) NOT NULL,
	db_schema VARCHAR(64),
	disable_cache TINYINT(1),
	CONSTRAINT cunq_shard_nodes PRIMARY KEY (type, server, db_name, db_user, db_schema)
) ENGINE=INNODB;

CREATE TABLE shard_tables
(
	table_id VARCHAR(64),
	node_id INTEGER,
	CONSTRAINT cpk_shard_nodes PRIMARY KEY (table_id),
	CONSTRAINT cfk_shard_nodes FOREIGN KEY (node_id) REFERENCES shard_nodes (node_id)
) ENGINE=INNODB;

CREATE TABLE stg_shard_tables
(
	table_id VARCHAR(64),
	node_id INTEGER,
	CONSTRAINT cpk_shard_nodes PRIMARY KEY (table_id)
) ENGINE=INNODB;
