<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$now = time();

$query = "UPDATE " . $mysql_prefix . "player_list SET heartbeat_time='$now' WHERE game_id = '$game_id' AND player_id = '$player_id'";
mysql_query($query) or die;

//I forgot, why I put this in... Let's see, how well hase works without!
//$query = "UPDATE " . $mysql_prefix . "game_list SET create_date='$now' WHERE game_id = '$game_id'";
//mysql_query($query) or die;

mysql_close($connection); 
?>
