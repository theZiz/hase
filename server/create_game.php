<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db("sparrowman") or die;

$game_name = mysql_real_escape_string($_POST['game_name']);
$level_string = mysql_real_escape_string($_POST['level_string']);
$max_player = (int)$_POST['max_player'];
$seconds_per_turn = (int)$_POST['seconds_per_turn'];
try {
	$hares_per_player = (int)$_POST['hares_per_player'];
} catch (Exception $e) {
	$hares_per_player = 0;
}
$create_date = time();
$admin_pw = 0;
while ($admin_pw == 0)
	$admin_pw = rand();

$query = "INSERT INTO " . $mysql_prefix . "game_list (game_name, max_player, seconds_per_turn, admin_pw, create_date, status, level_string, hares_per_player) ".
"VALUES ( '$game_name', '$max_player', '$seconds_per_turn', '$admin_pw', '$create_date', '0', '$level_string', '$hares_per_player')";

mysql_query($query) or die;
$game_id = mysql_insert_id();
echo "game_id: $game_id", PHP_EOL;
echo "admin_pw: $admin_pw", PHP_EOL;
echo "create_date: $create_date";
mysql_close($connection); 
?>
