<?php 
include 'config.php';

$connection = mysql_connect ("localhost", $mysql_username, $mysql_password) or die;
mysql_select_db($mysql_dbname) or die;

$query = "CREATE TABLE IF NOT EXISTS `".$mysql_prefix."chat_list` (".
"`chat_id` int(11) NOT NULL,".
"  `game_id` int(11) NOT NULL,".
"  `chat_time` int(11) NOT NULL,".
"  `chat_name` varchar(256) NOT NULL,".
"  `chat_message` varchar(256) NOT NULL".
") ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=0;";
mysql_query($query) or die;
$query = "CREATE TABLE IF NOT EXISTS `".$mysql_prefix."data_list` (".
"  `game_id` int(11) NOT NULL,".
"  `player_id` int(11) NOT NULL,".
"  `second_of_player` int(11) NOT NULL,".
"  `data` mediumblob NOT NULL".
") ENGINE=InnoDB DEFAULT CHARSET=latin1;";
mysql_query($query) or die;
$query = "CREATE TABLE IF NOT EXISTS `".$mysql_prefix."game_list` (".
"`game_id` int(11) NOT NULL,".
"  `game_name` varchar(32) NOT NULL,".
"  `max_player` int(11) NOT NULL,".
"  `seconds_per_turn` int(11) NOT NULL,".
"  `admin_pw` int(11) NOT NULL,".
"  `create_date` int(11) NOT NULL,".
"  `status` int(11) NOT NULL,".
"  `level_string` varchar(512) NOT NULL,".
"  `hares_per_player` int(11) NOT NULL".
") ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=0;";
mysql_query($query) or die;
$query = "CREATE TABLE IF NOT EXISTS `".$mysql_prefix."player_list` (".
"`player_id` int(11) NOT NULL,".
"  `game_id` int(11) NOT NULL,".
"  `player_pw` int(11) NOT NULL,".
"  `player_name` varchar(32) NOT NULL,".
"  `position_in_game` int(11) NOT NULL,".
"  `computer` int(11) NOT NULL,".
"  `chat_get_time` int(11) NOT NULL,".
"  `status` int(11) NOT NULL,".
"  `heartbeat_time` int(11) NOT NULL".
") ENGINE=InnoDB  DEFAULT CHARSET=latin1 AUTO_INCREMENT=0;";
mysql_query($query) or die;
$query = "ALTER TABLE `".$mysql_prefix."chat_list`".
" ADD PRIMARY KEY (`chat_id`);";
mysql_query($query) or die;
$query = "ALTER TABLE `".$mysql_prefix."data_list`".
" ADD PRIMARY KEY (`game_id`,`player_id`,`second_of_player`);";
mysql_query($query) or die;
$query = "ALTER TABLE `".$mysql_prefix."game_list`".
" ADD PRIMARY KEY (`game_id`), ADD UNIQUE KEY `game_id` (`game_id`);";
mysql_query($query) or die;
$query = "ALTER TABLE `".$mysql_prefix."player_list`".
" ADD PRIMARY KEY (`player_id`,`game_id`);";
mysql_query($query) or die;
$query = "ALTER TABLE `".$mysql_prefix."chat_list`".
" MODIFY `chat_id` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=0;";
mysql_query($query) or die;
$query = "ALTER TABLE `".$mysql_prefix."game_list`".
" MODIFY `game_id` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=0;";
mysql_query($query) or die;
$query = "ALTER TABLE `".$mysql_prefix."player_list`".
" MODIFY `player_id` int(11) NOT NULL AUTO_INCREMENT,AUTO_INCREMENT=0;";
mysql_query($query) or die;

mysql_close($connection); 

echo "Success!";
echo "Delete this file now, you will not need it again!";
?>
