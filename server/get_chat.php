<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db("sparrowman") or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$now = time();

//Getting last get time from player
$query = "SELECT * FROM hase_player_list WHERE game_id = '$game_id' AND player_id = '$player_id'";
$result = mysql_query($query) or die;

$row = mysql_fetch_assoc( $result );
$chat_get_time = $row['chat_get_time'];

$query = "SELECT * FROM hase_chat_list WHERE game_id = '$game_id' AND chat_time > '$chat_get_time'";
$result = mysql_query($query) or die;
while ($row = mysql_fetch_array( $result ))
{
	$chat_name = $row['chat_name'];
	$chat_message = $row['chat_message'];
	$chat_time = $row['chat_time'];
	echo "chat_message: $chat_message", PHP_EOL;
	echo "chat_time: $chat_time", PHP_EOL;
	echo "chat_name: $chat_name", PHP_EOL;
	if ($chat_get_time < $row['chat_time'])
		$chat_get_time = $row['chat_time'];
}

$query = "UPDATE hase_player_list SET chat_get_time='$chat_get_time', heartbeat_time='$now' WHERE game_id = '$game_id' AND player_id = '$player_id'";
mysql_query($query) or die;

$now = time();
$query = "UPDATE hase_game_list SET create_date='$now' WHERE game_id = '$game_id'";
mysql_query($query) or die;

mysql_close($connection); 
?>
