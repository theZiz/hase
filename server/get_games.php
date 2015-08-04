<?php 
include 'config.php';

$connection = mysql_connect ($mysql_host, $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$now = time();

$query = "SELECT * FROM " . $mysql_prefix . "game_list";
$result = mysql_query($query) or die;
$i = 0;
while ($row = mysql_fetch_array( $result ))
{
	$game_id = $row['game_id'];
	$create_date = $row['create_date'];
	$status = $row['status'];
	if ($create_date < $now-3600*24*7) //7 days
	{
		$query = "DELETE FROM " . $mysql_prefix . "game_list WHERE game_id = '$game_id'";
		mysql_query($query) or die;
		$query = "DELETE FROM " . $mysql_prefix . "player_list WHERE game_id = '$game_id'";
		mysql_query($query) or die;
		$query = "DELETE FROM " . $mysql_prefix . "data_list WHERE game_id = '$game_id'";
		mysql_query($query) or die;
		continue;
	}
	else
	if ($create_date < $now-90 && $status >= 0) //2 minutes ingame without reaction...
	{
		if ($status == 0)
			$query = "UPDATE " . $mysql_prefix . "game_list SET status='-2' WHERE game_id = '$game_id'";
		else
			$query = "UPDATE " . $mysql_prefix . "game_list SET status='-1' WHERE game_id = '$game_id'";
		mysql_query($query) or die;
		continue;
	}
	if ($status == -2)
		continue;
	$game_name = $row['game_name'];
	$options = $row['options'];
	$seconds_per_turn = $row['seconds_per_turn'];
	$hares_per_player = $row['hares_per_player'];
	//count player
	$subresult = mysql_query("SELECT COUNT(*) AS total FROM " . $mysql_prefix . "player_list WHERE game_id='$game_id'");
	$subrow = mysql_fetch_assoc($subresult);
	$player_count = $subrow['total'];
	echo "player_count: $player_count", PHP_EOL;
	echo "create_date: $create_date", PHP_EOL;
	echo "status: $status", PHP_EOL;
	echo "seconds_per_turn: $seconds_per_turn", PHP_EOL;
	echo "hares_per_player: $hares_per_player", PHP_EOL;
	echo "options: $options", PHP_EOL;
	echo "game_name: $game_name", PHP_EOL;
	echo "game_id: $game_id", PHP_EOL;
	$i = $i + 1;
}
echo "game_count: $i";
mysql_close($connection); 
?>
