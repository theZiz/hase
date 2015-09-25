<!DOCTYPE html>
<html>
	<head>
	<META HTTP-EQUIV="refresh" CONTENT="10">
	</head>
	<body>
		<h1>Hase notify tool</h1>
		<p>This page will refresh every 10 seconds and will tell you with a notification if a new game is created. Keep sure to allow notifications, otherwise the whole page doesn't make much sense. ;) The notification will be resend until the game starts.</p>
		<!--<p>Next update in:</p>
		<img src="./countdown.gif" alt="countdown"/> !-->
		<script type="text/javascript">
			if (!("Notification" in window))
				alert("This browser does not support desktop notification");
			else
			{
				if (Notification.permission !== 'denied')
					Notification.requestPermission();
			}
		</script>
		<p>Games on server:</p>
		<ul>
		<?php 
			include 'config.php';
			$connection = mysql_connect ($mysql_host, $mysql_username, $mysql_password) or die;
			mysql_select_db($mysql_dbname) or die;
			$now = time();
			$query = "SELECT * FROM " . $mysql_prefix . "game_list ORDER BY create_date DESC";
			$result = mysql_query($query) or die;
			while ($row = mysql_fetch_array( $result ))
			{
				$status = $row['status'];
				if ($status == -2)
					continue;
				$create_date = $row['create_date'];
				$game_name = $row['game_name'];
				switch ($status)
				{
					case 0: 
						echo "<li><b>$game_name</b> Open</li>", PHP_EOL;				
						break;
					case 1: 
						echo "<li><b>$game_name</b> Running</li>", PHP_EOL;				
						break;
					default:
						$diff = $now - $create_date;
						$seconds = $diff % 60;
						$diff /= 60;
						$minutes = $diff % 60;
						$diff /= 60;
						$hours = $diff % 24;
						$diff /= 24;
						$days = (int)$diff;
						echo "<li><b>$game_name</b> Done since $days:$hours:$minutes:$seconds</li>", PHP_EOL;
				}
				if ($create_date > $now-15) //younger than 10 seconds?
				{
					echo '<script type="text/javascript">', PHP_EOL;
					echo 'if (Notification.permission === "granted")', PHP_EOL;
					echo "var notification = new Notification('Game $game_name created! ')", PHP_EOL;
					echo '</script>', PHP_EOL;
				}
			}
			mysql_close($connection); 
		?>
		</ul>
	</body>
</html>
