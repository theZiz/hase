<?php
include 'config.php';

$connection = mysqli_connect ($mysql_host, $mysql_username, $mysql_password, $mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$admin_pw = (int)$_POST['admin_pw'];

$query = "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id = '$game_id'";
$result = mysqli_query( $connection, $query) or die;

$row = mysqli_fetch_array( $result );
if ($row['admin_pw'] == $admin_pw)
{
	if ($row['status'] == 1) //closing a running game
	{
		$query = "UPDATE " . $mysql_prefix . "game_list SET status='-1' WHERE game_id = '$game_id'";
		mysqli_query( $connection, $query) or die;
	}
	else
	{
		$query = "UPDATE " . $mysql_prefix . "game_list SET status='-2' WHERE game_id = '$game_id'";
		mysqli_query( $connection, $query) or die;
	}
}
echo "error: 0";
mysqli_close($connection);
?>
