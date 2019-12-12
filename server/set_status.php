<?php
include 'config.php';

$connection = mysqli_connect ($mysql_host, $mysql_username, $mysql_password, $mysql_dbname) or die;

$game_id = (int)$_POST['game_id'];
$admin_pw = (int)$_POST['admin_pw'];
$status = (int)$_POST['status'];

$query = "SELECT * FROM " . $mysql_prefix . "game_list WHERE game_id = '$game_id'";
$result = mysqli_query( $connection, $query) or die;

$row = mysqli_fetch_array( $result );
if ($row['admin_pw'] == $admin_pw && $row['status'] != $status)
{
	if ($status == 1) //game started, setting positions ingame
	{
		//count player
		$result = mysqli_query( $connection, "SELECT COUNT(*) AS total FROM " . $mysql_prefix . "player_list WHERE game_id='$game_id'");
		$row = mysqli_fetch_assoc($result);
		$player_count = $row['total'];
		$positions = array_fill(0,$player_count,0);
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
		$result = mysqli_query( $connection, $query) or die;
		$i = 0;
		while ($row = mysqli_fetch_array( $result ))
		{
			$player_id = $row['player_id'];
			$position = $positions[$i];
			$query = "UPDATE " . $mysql_prefix . "player_list SET position_in_game='$position' WHERE game_id = '$game_id' AND player_id='$player_id'";
			mysqli_query( $connection, $query) or die;
			$i++;
		}
	}
	$query = "UPDATE " . $mysql_prefix . "game_list SET status='$status' WHERE game_id = '$game_id'";
	mysqli_query( $connection, $query) or die;
}
mysqli_close($connection);
echo "Everything: ok";
?>
