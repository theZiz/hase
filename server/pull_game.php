<?php 
header("Content-length: 1536");
//header("Content-type: fun");
include 'config.php';
$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db("sparrowman") or die;

$game_id = (int)$_POST['game_id'];
$player_id = (int)$_POST['player_id'];
$second_of_player = (int)$_POST['second_of_player'];

$query = "SELECT data FROM hase_data_list WHERE ".
"game_id = '$game_id' AND player_id = '$player_id' AND second_of_player = '$second_of_player'";
$result = mysql_query($query) or die;
$row = mysql_fetch_assoc( $result );
$data = $row['data'];

echo $data;

mysql_close($connection);
?>
