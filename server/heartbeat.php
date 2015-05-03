<?php 
include 'config.php';

$connection = mysql_connect ($mysql_host, $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$now = time();

//player heartbeat
$query = "UPDATE " . $mysql_prefix . "player_list SET heartbeat_time='$now' WHERE game_id = '$game_id' AND player_id = '$player_id'";
mysql_query($query) or die;

//game heartbeat
$query = "UPDATE " . $mysql_prefix . "game_list SET create_date='$now' WHERE game_id = '$game_id'";
mysql_query($query) or die;

mysql_close($connection); 
?>
