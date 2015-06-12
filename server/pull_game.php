<?php 
//header("Content-type: fun");
include 'config.php';
$connection = mysql_connect ($mysql_host, $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$second_of_player = (int)$_POST['second_of_player'];

$query = "SELECT data FROM " . $mysql_prefix . "data_list WHERE ".
"game_id = '$game_id' AND player_id = '$player_id' AND second_of_player = '$second_of_player'";
$result = mysql_query($query) or die;

$heartbeat_diff = 0;

if (mysql_num_rows($result) > 0)
{
	$row = mysql_fetch_assoc( $result );
	header("Content-length: 1543");
	echo 'ACK';
	$data = $row['data'];
	echo $data;

	$query = "SELECT * FROM " . $mysql_prefix . "player_list WHERE game_id = '$game_id' AND  player_id = '$player_id'";
	$result = mysql_query($query) or die;
	$row = mysql_fetch_assoc( $result );
	$now = time();
	$heartbeat_diff = $now - $row['heartbeat_time'];
}
else
{
	$query = "SELECT * FROM " . $mysql_prefix . "player_list WHERE game_id = '$game_id' AND  player_id = '$player_id'";
	$result = mysql_query($query) or die;
	$row = mysql_fetch_assoc( $result );

	$now = time();
	$heartbeat_diff = $now - $row['heartbeat_time'];

	if ($row['status'] < 0)
	{
		header("Content-length: 7");
		echo 'NUL';
	}
	else
	{
		//Okay, not found. Maybe is the player dead?
		if ($heartbeat_diff > 90) //two minutes no reaction
		{
			$query = "UPDATE " . $mysql_prefix . "player_list SET status='-2' WHERE game_id = '$game_id' AND player_id = '$player_id'";
			mysql_query($query) or die;		
			//this will work on next pull
		}
		header("Content-length: 7");
		echo 'ERR';
	}
}
echo pack("l", $heartbeat_diff);
mysql_close($connection);
?>
