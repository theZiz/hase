<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$player_name = mysql_real_escape_string($_POST['player_name']);
$player_pw = rand();
$computer = (int)$_POST['computer'];
$nr = (int)$_POST['nr'];

//counting players
$result = mysql_query("SELECT COUNT(*) AS total FROM " . $mysql_prefix . "player_list WHERE game_id='$game_id'");
$row = mysql_fetch_assoc($result);
$player_count = $row['total'];

//getting max_players
$result = mysql_query("SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id='$game_id'");
$row = mysql_fetch_assoc($result);
$max_player = $row['max_player'];
$status = $row['status'];

if ($player_count >= $max_player || $status != 0)
{
	echo "error: 1";
}
else
{
	$now = time();
	$query = "INSERT INTO " . $mysql_prefix . "player_list (game_id, player_pw, player_name, position_in_game, computer, chat_get_time, status, heartbeat_time, nr) ".
	"VALUES ( '$game_id', '$player_pw', '$player_name', '0', '$computer', '0', '0', '$now', '$nr')";

	mysql_query($query) or die;
	$player_id = mysql_insert_id();
	echo "player_id: $player_id", PHP_EOL;
	echo "player_pw: $player_pw", PHP_EOL;
	echo "error: 0";
}
mysql_close($connection); 
?>
