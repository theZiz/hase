<?php 
//header("Content-type: fun");
include 'config.php';
$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$second_of_player = (int)$_POST['second_of_player'];

$query = "SELECT data FROM " . $mysql_prefix . "data_list WHERE ".
"game_id = '$game_id' AND player_id = '$player_id' AND second_of_player = '$second_of_player'";
$result = mysql_query($query) or die;
if (mysql_num_rows($result) > 0)
{
	$row = mysql_fetch_assoc( $result );
	header("Content-length: 1539");
	echo 'ACK';
	$data = $row['data'];
	echo $data;
}
else
{
	$query = "SELECT * FROM " . $mysql_prefix . "player_list WHERE game_id = '$game_id' AND  player_id = '$player_id'";
	$result = mysql_query($query) or die;
	$row = mysql_fetch_assoc( $result );
	if ($row['status'] < 0)
	{
		header("Content-length: 3");
		echo 'NUL';
	}
	else
	{
		//Okay, not found. Maybe is the player dead?
		$now = time();
		$query = "SELECT * FROM " . $mysql_prefix . "player_list WHERE game_id = '$game_id' AND  player_id = '$player_id'";
		$result = mysql_query($query) or die;
		$row = mysql_fetch_assoc( $result );
		if ($row['heartbeat_time'] < $now-60) //one minute no reaction
		{
			$query = "UPDATE " . $mysql_prefix . "player_list SET status='-2' WHERE game_id = '$game_id' AND player_id = '$player_id'";
			mysql_query($query) or die;		
			//this will work on next pull
		}
		header("Content-length: 3");
		echo 'ERR';
	}
}

mysql_close($connection);
?>
