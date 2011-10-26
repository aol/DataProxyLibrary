#!/bin/bash
set -e
set -u

if [ $# -ne 4 ]; then
	echo "usage: $0 <master_host> <server_list_file> <dbname_list_file> <dpl_config_file>"
	echo "	NOTE: <dpl_config_file> must contain a node called 'shard_nodes'"
	exit 1
fi

master_host=$1
server_list_file=$2
dbname_list_file=$3
dpl_config_file=$4

# REGION: MASTER SETUP
master_db=shard_master
echo "`date`: Setting up master on $master_host (dbname=$master_db)"
echo "CREATE DATABASE IF NOT EXISTS $master_db" | mysql -u adlearn -pAdv.commv -h $master_host
mysql -uadlearn -pAdv.commv -h$master_host $master_db < masterTables.sql


# REGION: NODES SETUP
nodesFile="nodes.txt"
echo "type,db_name,server,db_user,db_password,db_schema,disable_cache" > $nodesFile

servers=`cat $server_list_file`
dbNames=`cat $dbname_list_file`

for server in $servers; do
	echo "`date`: Setting up node: $server"
	command='';
	for db in $dbNames; do
		command="drop database if exists $db; create database $db; "$command
		echo "mysql,$db,$server,adlearn,Adv.commv,,0" >> $nodesFile
	done
	mysql -uadlearn -pAdv.commv -h$server -e "$command"
done

echo "`date`: Updating master with node information..."
dplShell --i $dpl_config_file --n shard_nodes --d @$nodesFile

rm $nodesFile
