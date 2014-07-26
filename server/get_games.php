<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db("sparrowman") or die;

$now = time();

$query = "SELECT * FROM hase_game_list";
$result = mysql_query($query) or die;
$i = 0;
while ($row = mysql_fetch_array( $result ))
{
	$game_id = $row['game_id'];
	$create_date = $row['create_date'];
	$status = $row['status'];
	if ($create_date < $now-3600*24*7) //7 days
	{
		$query = "DELETE FROM hase_game_list WHERE game_id = '$game_id'";
		mysql_query($query) or die;
		$query = "DELETE FROM hase_player_list WHERE game_id = '$game_id'";
		mysql_query($query) or die;
		$query = "DELETE FROM hase_data_list WHERE game_id = '$game_id'";
		mysql_query($query) or die;
		$query = "DELETE FROM hase_chat_list WHERE game_id = '$game_id'";
		mysql_query($query) or die;
		continue;
	}
	else
	if ($create_date < $now-60 && $status != -1 && statu != -2) //1 minute ingame without reaction...
	{
		$query = "UPDATE hase_game_list SET status='-2' WHERE game_id = '$game_id'";
		mysql_query($query) or die;
		continue;
	}
	if ($status == -2)
		continue;
	$game_name = $row['game_name'];
	$max_player = $row['max_player'];
	$seconds_per_turn = $row['seconds_per_turn'];
	//count player
	$subresult = mysql_query("SELECT COUNT(*) AS total FROM hase_player_list WHERE game_id='$game_id'");
	$subrow = mysql_fetch_assoc($subresult);
	$player_count = $subrow['total'];
	echo "player_count: $player_count", PHP_EOL;
	echo "create_date: $create_date", PHP_EOL;
	echo "status: $status", PHP_EOL;
	echo "seconds_per_turn: $seconds_per_turn", PHP_EOL;
	echo "max_player: $max_player", PHP_EOL;
	echo "game_name: $game_name", PHP_EOL;
	echo "game_id: $game_id", PHP_EOL;
	$i = $i + 1;
}
echo "game_count: $i";
mysql_close($connection); 
?>
