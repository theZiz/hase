<?php 
include 'config.php';
include 'utils.php';

$connection = mysql_connect ($mysql_host, $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_name = escape_input($_POST['game_name']);
$level_string = escape_input($_POST['level_string']);
$options = (int)$_POST['options'];
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

$query = "INSERT INTO " . $mysql_prefix . "game_list (game_name, options, seconds_per_turn, admin_pw, create_date, status, level_string, hares_per_player) ".
"VALUES ( '$game_name', '$options', '$seconds_per_turn', '$admin_pw', '$create_date', '0', '$level_string', '$hares_per_player')";

mysql_query($query) or die;
$game_id = mysql_insert_id();
echo "game_id: $game_id", PHP_EOL;
echo "admin_pw: $admin_pw", PHP_EOL;
echo "create_date: $create_date";
mysql_close($connection); 
?>
