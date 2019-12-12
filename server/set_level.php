<?php
include 'config.php';
include 'utils.php';

$connection = mysqli_connect ($mysql_host, $mysql_username, $mysql_password, $mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$admin_pw = (int)$_POST['admin_pw'];
$level_string = escape_input( $connection, $_POST['level_string']);

$query = "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id = '$game_id'";
$result = mysqli_query( $connection, $query) or die;

$row = mysqli_fetch_array( $result );
if ($row['admin_pw'] == $admin_pw)
{
	$query = "UPDATE " . $mysql_prefix . "game_list SET level_string='$level_string' WHERE game_id = '$game_id'";
	mysqli_query( $connection, $query) or die;
}

echo "error: 0";

mysqli_close($connection);
?>
