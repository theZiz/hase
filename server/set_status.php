<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db("sparrowman") or die;

$game_id = (int)$_POST['game_id'];
$admin_pw = (int)$_POST['admin_pw'];
$status = (int)$_POST['status'];

$query = "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id = '$game_id'";
$result = mysql_query($query) or die;

$row = mysql_fetch_array( $result );
if ($row['admin_pw'] == $admin_pw && $row['status'] != $status)
{
	$query = "UPDATE " . $mysql_prefix . "game_list SET status='$status' WHERE game_id = '$game_id'";
	mysql_query($query) or die;	
	if ($status == 1) //game started, setting positions ingame
	{
		//count player
		$result = mysql_query("SELECT COUNT(*) AS total FROM " . $mysql_prefix . "player_list WHERE game_id='$game_id'");
		$row = mysql_fetch_assoc($result);
		$player_count = $row['total'];
		$positions = new SplFixedArray($player_count);
		for ($i=0;$i<$player_count;$i++)
			$positions[$i] = $i;
		//Shuffle
		for ($i=0;$i<$player_count*10;$i++)
		{
			$a = rand(0,$player_count-1);
			$b = rand(0,$player_count-1);
			if ($a == $b)
				continue;
			$temp = $positions[$a];
			$positions[$a] = $positions[$b];
			$positions[$b] = $temp;
		}
		//Setting in db
		$query = "SELECT * FROM " . $mysql_prefix . "player_list WHERE game_id='$game_id'";
		$result = mysql_query($query) or die;
		$i = 0;
		while ($row = mysql_fetch_array( $result ))
		{
			$player_id = $row['player_id'];
			$position = $positions[$i];
			$query = "UPDATE " . $mysql_prefix . "player_list SET position_in_game='$position' WHERE game_id = '$game_id' AND player_id='$player_id'";
			mysql_query($query) or die;
			$i++;
		}
	}
}
mysql_close($connection); 
?>
