<?php 
include 'config.php';

$connection = mysql_connect ($mysql_host, $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$admin_pw = (int)$_POST['admin_pw'];

$query = "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id = '$game_id'";
$result = mysql_query($query) or die;

$row = mysql_fetch_array( $result );
if ($row['admin_pw'] == $admin_pw)
{
	if ($row['status'] == 0) //Never started
	{
		$query = "DELETE FROM " . $mysql_prefix . "player_list WHERE game_id = '$game_id' AND player_id = '$player_id'";
		mysql_query($query) or die;
	}
	else
	{
		$query = "UPDATE " . $mysql_prefix . "player_list SET status='-1' WHERE game_id = '$game_id' AND player_id = '$player_id'";
		mysql_query($query) or die;
	}	
}
echo "error: 0";
mysql_close($connection); 
?>
