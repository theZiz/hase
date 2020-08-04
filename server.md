# How to run a hase server yourself

The Server of hase are just some php scripts and a mysql database. To Install a hase server in your own machine you need

* php7
* mysql (or MariaDB)

If you only have php5 or php6 you can probably still use an older version of the hase server as long as I don't change the API. However I recently updated the server to php7 and this change is not downwards compatible and only this version will get feature updates.

To "install" the server, just copy all files in the folder `server` to your server with php and mysql installed.
Next create a user, a password and database for hase. Edit the `config.php` accordingly:

```php
<?php
$mysql_username = "user";
$mysql_password = "password";
$mysql_dbname = "database_name";
$mysql_prefix = "hase10_";
$mysql_host = "localhost";
?>
```

The `mysql_prefix` is an optional prefix set before every database table. May be handy if you want to have different hase versions in your database at the same time. For now insert whatever prefix you like or let it this way.

If the user, the passowrd and the database is setup, open `INIT_SERVER.php` in your browser. If everything was working the script will tell you and you successfully set up the database! If the page stays white an error occured and you need to check your php error log...

# How to tell hase about the server

To tell hase about the server create a "hase root file" somewhere on your server, which looks like this:
```php
<?php
echo "url: yourserver.com/server", PHP_EOL;
echo "version: 18", PHP_EOL;

echo "irc_server: irc.freenode.net", PHP_EOL;
echo "irc_channel: #hase", PHP_EOL;
echo "irc_port: 6665,6666,6667", PHP_EOL;

if (function_exists("gzdecode"))
        echo "gzip: yes";
?>
```

You can also choose a different irc channel here and steer whether your server is able to use zip compressing or not. Save this file e.g. as `hase.php` and till hase where to find this file in the internet by editing the `config.ini` of your local hase installation (probably in `~/.config/hase` or the same folder as the game):

```ini
circle: 1
music_volume: 0
sample_volume: 4096
particles: 1
rotation: 1
direction_flip: 0
server: yourserver.com/hase.php
username: You
show_names: 1
show_map: 1
global_chat: 1
game_options: 125903138
game_seconds: 60
game_hares: 3
first_game: 0
sprite: 6
update_server: 1
```

That's all! All your hare belong to us.
