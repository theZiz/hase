<?php 
include 'config.php';
$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db("sparrowman") or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$player_pw = (int)$_POST['player_pw'];
$second_of_player = (int)$_POST['second_of_player'];
$data = mysql_real_escape_string($_POST['data']);

$query = "SELECT * FROM hase_player_list WHERE game_id = '$game_id' AND  player_id = '$player_id'";
$result = mysql_query($query) or die;

$row = mysql_fetch_assoc( $result );
if ($row['player_pw'] == $player_pw)
{
	$query = "INSERT INTO hase_data_list (game_id, player_id, second_of_player, data) ".
	"VALUES ( '$game_id', '$player_id', '$second_of_player', '$data' )";
	mysql_query($query) or die;
}
if ($row['status'] == -2) //killed...
	echo "Error: 1";
else
	echo "Error: 0";
mysql_close($connection);
?>
