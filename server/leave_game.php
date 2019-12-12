<?php
include 'config.php';

$connection = mysqli_connect ($mysql_host, $mysql_username, $mysql_password, $mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$player_pw = (int)$_POST['player_pw'];

$query = "SELECT * FROM " . $mysql_prefix . "player_list WHERE game_id = '$game_id' AND  player_id = '$player_id'";
$result = mysqli_query( $connection, $query) or die;

$row = mysqli_fetch_assoc( $result );
if ($row['player_pw'] == $player_pw)
{
	$query = "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id = '$game_id'";
	$result = mysqli_query( $connection, $query) or die;
	$row = mysqli_fetch_array( $result );
	if ($row['status'] == 0) //Never started
	{
		$query = "DELETE FROM " . $mysql_prefix . "player_list WHERE game_id = '$game_id' AND player_id = '$player_id'";
		mysqli_query( $connection, $query) or die;
	}
	else
	{
		$query = "UPDATE " . $mysql_prefix . "player_list SET status='-1' WHERE game_id = '$game_id' AND player_id = '$player_id'";
		mysqli_query( $connection, $query) or die;
	}
}

echo "error: 0";

mysqli_close($connection);
?>
