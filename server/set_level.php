<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$admin_pw = (int)$_POST['admin_pw'];
$level_string = mysql_real_escape_string($_POST['level_string']);

$query = "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id = '$game_id'";
$result = mysql_query($query) or die;

$row = mysql_fetch_array( $result );
if ($row['admin_pw'] == $admin_pw)
{
	$query = "UPDATE " . $mysql_prefix . "game_list SET level_string='$level_string' WHERE game_id = '$game_id'";
	mysql_query($query) or die;	
}

mysql_close($connection); 
?>
