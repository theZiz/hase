<?php 
include 'config.php';
include 'utils.php';

$connection = mysql_connect ($mysql_host, $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$admin_pw = (int)$_POST['admin_pw'];
$max_player = (int)$_POST['max_player'];
$seconds_per_turn = (int)$_POST['seconds_per_turn'];
$hares_per_player = (int)$_POST['hares_per_player'];

$query = "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id = '$game_id'";
$result = mysql_query($query) or die;

$row = mysql_fetch_array( $result );
if ($row['admin_pw'] == $admin_pw)
{
	$query = "UPDATE " . $mysql_prefix . "game_list SET max_player='$max_player', seconds_per_turn='$seconds_per_turn', hares_per_player='$hares_per_player' WHERE game_id = '$game_id'";
	mysql_query($query) or die;	
}

echo "error: 0";

mysql_close($connection); 
?>

