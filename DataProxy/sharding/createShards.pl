#!/usr/bin/perl -w

use strict;
use Getopt::Long;
use DBI;

######################################
# CONFIG VARIABLES
my $help;
my $g_workingDir;			# temporary directory for filewriting
my $g_ddlFile;				# file containing shard table DDL
my $g_shardAxis;			# axis on which to perform sharding
my $g_shardWeight;			# weight to add to node if it is selected for a single new shard
my $g_baseTableName;		# base tablename; full tablename = 
my $g_dplConfig;			# file spec of dpl config
my $g_tablesNodeName;		# name of node containing table assignments
my $g_nodesNodeName;		# name of node containing available shard nodes
my $g_distNodeName;			# name of node containing current node:table distribution
my $g_dplShell = "dplShell";
my $g_noCleanUp = 0;
# GLOBAL VARIABLES
my @g_shards;				# array of shard-ids that we need shard tables for
my %g_nodeDistribution;		# mapping of node id->count of tables on this node
my %g_nodeConnections;		# mapping of node id->connection info & handles to db
my %g_nodeSqlStatements;	# mapping of node id->sql create-table statements
my %g_tableDistribution;	# mapping of table id->node id (if the table exists)
my $g_tableUpdateFile;		# file spec of table index update file (to be stored in shard_tables table)
my $g_uniqueId = `date -u +%Y.%m.%d-%H.%M.%S | tr -d [:space:]`;
######################################

sub usage( )
{
	print STDERR "Usage: perl $0 <OPTIONS>\n\n" .
		  "OPTIONS:\n" .
		  "=====================+=============================================================================\n" .
		  "Parameter            |    Description\n" .
		  "---------------------+-----------------------------------------------------------------------------\n" .
		  " --workingDir        | working directory to place temporary files\n" .
		  " --shardAxis         | axis on which to shard (e.g. campaign_id)\n" .
		  " --shardDDLFile      | file spec of shard table DDL (\${table_name} replaced with chosen table name)\n" .
		  " --shardWeight       | weight to add to a single node's distribution when selected for a new shard\n" .
		  " --baseTableName     | base tablename for shard tables (full tablename = <g_baseTableName>_<shard>\n" .
		  " --dplConfig         | config file to initialize dpl with (see below for required nodes)\n" .
		  " --tablesNodeName    | name of the node in the dpl config that contains table assignments\n" .
		  " --nodesNodeName     | name of the node in the dpl config that contains shard nodes\n" .
		  " --distNodeName      | name of the node in the dpl config that contains node distribution\n" .
		  " --dplShellPath      | (Optional) full path to dplShell application (default: $g_dplShell)\n" .
		  " --noCleanUp         | (Optional) if toggled, do not remove the temporary working subdir\n\n" .
		  "A CSV should be fed into this application including a column with the <shardAxis>\n" .
		  "Required nodes for dpl LOAD operations: tables, dist, nodes\n" .
		  "Required nodes for dpl STORE operations: tables\n\n";
}

# Process input parameters
GetOptions( 'help',	\$help,
			'shardAxis:s', \$g_shardAxis,
			'workingDir:s', \$g_workingDir,
			'baseTableName:s', \$g_baseTableName,
			'shardDDLFile:s', \$g_ddlFile,
			'shardWeight:i', \$g_shardWeight,
			'tablesNodeName:s', \$g_tablesNodeName,
			'nodesNodeName:s', \$g_nodesNodeName,
			'distNodeName:s', \$g_distNodeName,
			'dplConfig:s', \$g_dplConfig,
			'dplShellPath:s', \$g_dplShell,
			'noCleanUp', \$g_noCleanUp );

if (defined($help))
{
	&usage();
	exit 0;
}

if( !defined($g_shardWeight) || !defined($g_shardAxis) || !defined($g_workingDir)
 || !defined($g_baseTableName) || !defined($g_ddlFile) || !defined($g_dplConfig) 
 || !defined($g_tablesNodeName) || !defined($g_nodesNodeName) || !defined($g_distNodeName) )
{
	&usage();
	exit 1;
}

if( !-d $g_workingDir )
{
	die "Working Directory: $g_workingDir does not exist";
}

$g_workingDir = "$g_workingDir/$g_uniqueId";
if( -d $g_workingDir )
{
	die "Temporary Working directory: $g_workingDir already exists";
}
mkdir $g_workingDir;

&GetShards;
&GetTableDistribution;
&GetNodeDistribution;
&GetNodeConnectionInfo;

# iterate over all eligible shards & establish presence of their necessary tables
foreach my $shard (@g_shards)
{
	if (exists($g_tableDistribution{&GetTableNameForShard($shard)}))
	{
		next;
	}
	if( !defined($g_tableUpdateFile) )
	{
		$g_tableUpdateFile = "$g_workingDir/tableIndex.dat";
		open( TABLE_UPDATE, ">$g_tableUpdateFile" );
		print TABLE_UPDATE "table_id,node_id\n";
		close TABLE_UPDATE;
	}
	my $node = &GetNodeForNewTable;
	print STDERR "Assigning shard: '$shard' to node: $node\n";
	&SetupTableForShard($shard,$node);
}

# if we haven't established a table-update file, we're done
if( !defined($g_tableUpdateFile) )
{
	print STDERR "No updates necessary; all shards have tables\n";
	if( !$g_noCleanUp )
	{
		`rm -rf $g_workingDir`;
	}
	exit 0;
}

# iterate over the node-sql statements & execute them
foreach my $node ( keys %g_nodeSqlStatements )
{
	my $handle = &GetDbHandle( $node );
	my @sqlStatements = ( split/;/, $g_nodeSqlStatements{$node} );
	foreach my $sql ( @sqlStatements )
	{
		chomp $sql;
		if( $sql eq '' ) { next; }
		my $stmt = $handle->prepare( $sql );
		$stmt->execute() || die "Error executing stmt on node $node: $DBI::errstr";
	}
}

# update the index table
`$g_dplShell --i $g_dplConfig --n $g_tablesNodeName --d \@$g_tableUpdateFile`;
if( $? != 0 )
{
	die("Unable to execute dplShell command");
}

# disconnect & remove working directory
&DisconnectHandles;
if( !$g_noCleanUp )
{
	`rm -rf $g_workingDir`;
}

exit 0;



###########################################################################
# END MAIN
###########################################################################

sub SetupTableForShard
{
	my ($shard, $node) = @_;

	# generate the DDL for the new table on the appropriate node
	$g_nodeSqlStatements{$node} .= &GetTableDDLForShard($shard) . ";\n";
	
	# write the line in the table update file
	open( TABLE_UPDATE, ">>$g_tableUpdateFile" );
	print TABLE_UPDATE &GetTableNameForShard($shard) . ",$node\n";
	close TABLE_UPDATE;

	$g_nodeDistribution{$node} += $g_shardWeight;
}

sub GetDbHandle
{
	my ($node) = @_;
	if (!exists($g_nodeConnections{$node}{'handle'}))
	{
		print STDERR "Connecting to node $node\n";
		if( $g_nodeConnections{$node}{'type'} eq 'mysql' )
		{
			$g_nodeConnections{$node}{'handle'} = DBI->connect( "dbi:mysql:$g_nodeConnections{$node}{'database'}:$g_nodeConnections{$node}{'server'}",
															  $g_nodeConnections{$node}{'user'},
															  $g_nodeConnections{$node}{'password'} )
				|| die "Database connection not made: $DBI::errstr";
		}
		elsif( $g_nodeConnections{$node}{'type'} eq 'oracle' )
		{
			$g_nodeConnections{$node}{'handle'} = DBI->connect( "dbi:Oracle:$g_nodeConnections{$node}{'database'}",
															  $g_nodeConnections{$node}{'user'},
															  $g_nodeConnections{$node}{'password'} )
				|| die "Database connection not made: $DBI::errstr";
			if ($g_nodeConnections{$node}{'schema'} ne '')
			{
				my $stmt = $g_nodeConnections{$node}{'handle'}->prepare( "ALTER SESSION SET CURRENT_SCHEMA = $g_nodeConnections{$node}{'schema'}" );
				$stmt->execute() || die "Error switching schema: $DBI::errstr";
			}
		}
		else
		{
			die "Unrecognized database type: $g_nodeConnections{$node}{'type'}";
		}
	}
	return $g_nodeConnections{$node}{'handle'};
}

sub DisconnectHandles
{
	foreach my $node (keys %g_nodeConnections)
	{
		if (exists($g_nodeConnections{$node}{'handle'}))
		{
			print STDERR "Disconnecting from node $node\n";
			$g_nodeConnections{$node}{'handle'}->disconnect;
		}
	}
}

sub GetTableNameForShard
{
	my ($shard) = @_;
	return $g_baseTableName . "_$shard";
}

sub GetTableDDLForShard
{
	my ($shard) = @_;
	my $result;
	open( DDL_FILE, $g_ddlFile ) or die "Couldn't open base DDL file: $g_ddlFile";
	while( <DDL_FILE> )
	{
		$result .= $_;
	}
	close DDL_FILE;

	my $tableName = &GetTableNameForShard( $shard );
	$result =~ s/\${table_name}/$tableName/g;
	return $result;
}

sub IndexOfHeader
{
	my ($search, $line) = @_;
	my @lineArray = split(/,/, $line);
	my %index;
	@index{@lineArray} = ( 0..$#lineArray );
	my $result= $index{$search};
	if( !defined( $result ) )
	{
		die "Unable to find column: $search in header: $line\n";
	}
	return $result;
}

sub GetShards
{
	my %uniqueShards;
	my $shardIndex;
	while( <STDIN> )
	{
		chomp;
		if ($. == 1)
		{
			$shardIndex = &IndexOfHeader( $g_shardAxis, $_ );
			next;
		}
		my @field=split( ',', $_ );
		my $value=$field[$shardIndex];
		if( !defined($value) )
		{
			$value="";
		}
		$uniqueShards{$value}++;
	}
	@g_shards = keys %uniqueShards;
}

sub GetTableDistribution
{
	my $tableIndex;
	my $nodeIndex;
	open( PIPE, "$g_dplShell --i $g_dplConfig --n $g_tablesNodeName |" ) or die("Unable to execute dplShell command");
	if( $? != 0 ){ close PIPE; die "Error executing dpl load command\n"; }
	while( <PIPE> )
	{
		chomp;
		if ($. == 1)
		{
			$tableIndex = &IndexOfHeader( 'table_id', $_ );
			$nodeIndex = &IndexOfHeader( 'node_id', $_ );
			next;
		}
		my @field=split( ',', $_ );
		$g_tableDistribution{$field[$tableIndex]}  = $field[$nodeIndex];
	}
	close PIPE;
}

sub GetNodeDistribution
{
	my $nodeIndex;
	my $countIndex;
	open( PIPE, "$g_dplShell --i $g_dplConfig --n $g_distNodeName |" ) or die("Unable to execute dplShell command");
	if( $? != 0 ){ close PIPE; die "Error executing dpl load command\n"; }
	while( <PIPE> )
	{
		chomp;
		if( $. == 1 )
		{
			$nodeIndex = &IndexOfHeader( 'node_id', $_ );
			$countIndex = &IndexOfHeader( 'size', $_ );
			next;
		}
		my @field=split( ',', $_ );
		$g_nodeDistribution{$field[$nodeIndex]}  = $field[$countIndex];
	}
	close PIPE;
}

sub GetNodeConnectionInfo
{
	my $nodeIndex;
	my $typeIndex;
	my $serverIndex;
	my $dbIndex;
	my $userIndex;
	my $passwordIndex;
	my $schemaIndex;
	open( PIPE, "$g_dplShell --i $g_dplConfig --n $g_nodesNodeName |" ) or die("Unable to execute dplShell command");
	if( $? != 0 ){ close PIPE; die "Error executing dpl load command\n"; }
	while( <PIPE> )
	{
		chomp;
		if( $. == 1 )
		{
			$nodeIndex = &IndexOfHeader( 'node_id', $_ );
			$typeIndex = &IndexOfHeader( 'type', $_ );
			$serverIndex = &IndexOfHeader( 'server', $_ );
			$dbIndex = &IndexOfHeader( 'database', $_ );
			$userIndex = &IndexOfHeader( 'username', $_ );
			$passwordIndex = &IndexOfHeader( 'password', $_ );
			$schemaIndex = &IndexOfHeader( 'schema', $_ );
			next;
		}
		my @field=split( ',', $_ );
		$g_nodeConnections{$field[$nodeIndex]}{'type'} = $field[$typeIndex];
		$g_nodeConnections{$field[$nodeIndex]}{'server'} = $field[$serverIndex];
		$g_nodeConnections{$field[$nodeIndex]}{'database'} = $field[$dbIndex];
		$g_nodeConnections{$field[$nodeIndex]}{'user'} = $field[$userIndex];
		$g_nodeConnections{$field[$nodeIndex]}{'password'} = $field[$passwordIndex];
		$g_nodeConnections{$field[$nodeIndex]}{'schema'} = $field[$schemaIndex];
	}
	close PIPE;
}

sub GetNodeForNewTable
{
	my $minKey;
	foreach my $key (keys %g_nodeDistribution)
	{
		if( !defined($minKey) )
		{
			$minKey=$key;
		}
		elsif ($g_nodeDistribution{$key} < $g_nodeDistribution{$minKey})
		{
			$minKey = $key;
		}
	}
	return $minKey;
}
