<?php 
include 'config.php';

$connection = mysql_connect ($mysql_host, $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];

//Loading game data
$query = "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id='$game_id'";
$result = mysql_query($query) or die;
$game_row = mysql_fetch_assoc( $result );

//List of player names and IDs and count player
$query = "SELECT * FROM " . $mysql_prefix . "player_list WHERE game_id='$game_id'";
$result = mysql_query($query) or die;
$i = 0;
while ($row = mysql_fetch_array( $result ))
{
	$now = time();
	$heartbeat_diff = $now - $row['heartbeat_time'];
	if ($game_row['status'] == 0 && $heartbeat_diff > 90) //one and a half minute no reaction
	{
		$player_id = $row['player_id'];
		$query = "DELETE FROM " . $mysql_prefix . "player_list WHERE game_id = '$game_id' AND player_id = '$player_id'";
		mysql_query($query) or die;
	}
	else
	{
		$nr = $row['nr'];
		echo "nr: $nr", PHP_EOL;
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
}
echo "player_count: $i", PHP_EOL;

$create_date = $game_row['create_date'];
echo "create_date: $create_date", PHP_EOL;
$game_name = $game_row['game_name'];
echo "game_name: $game_name", PHP_EOL;
$level_string = $game_row['level_string'];
echo "level_string: $level_string", PHP_EOL;
$max_player = $game_row['max_player'];
echo "max_player: $max_player", PHP_EOL;
$seconds_per_turn = $game_row['seconds_per_turn'];
echo "seconds_per_turn: $seconds_per_turn", PHP_EOL;
$hares_per_player = $game_row['hares_per_player'];
echo "hares_per_player: $hares_per_player", PHP_EOL;
$status = $game_row['status'];
echo "status: $status";

mysql_close($connection);
?>
