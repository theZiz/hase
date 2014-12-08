<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db("sparrowman") or die;

$game_id = (int)$_POST['game_id'];
$chat_name = mysql_real_escape_string($_POST['chat_name']);
$chat_message = mysql_real_escape_string($_POST['chat_message']);
$chat_time = time();

$query = "INSERT INTO " . $mysql_prefix . "chat_list (game_id, chat_name, chat_message, chat_time) ".
"VALUES ( '$game_id', '$chat_name', '$chat_message', '$chat_time')";

mysql_query($query) or die;
mysql_close($connection); 
echo "Error: 0";
?>
