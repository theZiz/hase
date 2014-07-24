<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db("sparrowman") or die;

$game_id = (int)$_POST['game_id'];

//List of player names and IDs and count player
$query = "SELECT * FROM hase_player_list WHERE game_id='$game_id'";
$result = mysql_query($query) or die;
$i = 0;
while ($row = mysql_fetch_array( $result ))
{
	$computer = $row['computer'];
	echo "computer: $computer", PHP_EOL;
	$position_in_game = $row['position_in_game'];
	echo "position_in_game: $position_in_game", PHP_EOL;
	$player_name = $row['player_name'];
	echo "player_name: $player_name", PHP_EOL;
	$player_id = $row['player_id'];
	echo "player_id: $player_id", PHP_EOL;
	$i = $i + 1;
}
echo "player_count: $i", PHP_EOL;

$query = "SELECT * FROM hase_game_list WHERE game_id='$game_id'";
$result = mysql_query($query) or die;
$row = mysql_fetch_assoc( $result );
$create_date = $row['create_date'];
echo "create_date: $create_date", PHP_EOL;
$game_name = $row['game_name'];
echo "game_name: $game_name", PHP_EOL;
$level_string = $row['level_string'];
echo "level_string: $level_string", PHP_EOL;
$max_player = $row['max_player'];
echo "max_player: $max_player", PHP_EOL;
$seconds_per_turn = $row['seconds_per_turn'];
echo "seconds_per_turn: $seconds_per_turn", PHP_EOL;
$seconds_per_turn = $row['mom_player'];
echo "mom_player: $mom_player", PHP_EOL;
$status = $row['status'];
echo "status: $status";

mysql_close($connection);
?>
