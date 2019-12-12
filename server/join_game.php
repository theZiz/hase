<?php
include 'config.php';
include 'utils.php';

$connection = mysqli_connect ($mysql_host, $mysql_username, $mysql_password, $mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$player_name = escape_input( $connection, $_POST['player_name']);
$player_pw = rand();
$computer = (int)$_POST['computer'];
$nr = (int)$_POST['nr'];

//counting players
$result = mysqli_query( $connection, "SELECT COUNT(*) AS total FROM " . $mysql_prefix . "player_list WHERE game_id='$game_id'");
$row = mysqli_fetch_assoc($result);
$player_count = $row['total'];

//getting optionss
$result = mysqli_query( $connection, "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id='$game_id'");
$row = mysqli_fetch_assoc($result);
$options = $row['options'];
$status = $row['status'];

if ($player_count >= $options || $status != 0)
{
	echo "error: 1";
}
else
{
	$now = time();
	$query = "INSERT INTO " . $mysql_prefix . "player_list (game_id, player_pw, player_name, position_in_game, computer, chat_get_time, status, heartbeat_time, nr) ".
	"VALUES ( '$game_id', '$player_pw', '$player_name', '0', '$computer', '0', '0', '$now', '$nr')";

	mysqli_query( $connection, $query) or die;
	$player_id = mysqli_insert_id( $connection );
	echo "player_id: $player_id", PHP_EOL;
	echo "player_pw: $player_pw", PHP_EOL;
	echo "error: 0";
}
mysqli_close($connection);
?>
