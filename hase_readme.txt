Hase 
====

Hase is a game about hares in space. It is a bit like Worms or Artillery, but with gravitation.
The game itself is licsensed under GPLv2+.
The used fonts are Pixel Mania by HiBan (CC-BY-SA) and DejaVu Sans (Public Domain)

Changelog
=========

1.5.4.2:
- Rsotation speed of the cross haire depends on the weapon power
- Targeting line is above cross haire
- Fixed a _very_ rare and uncommon crash because of division by zero
- Set the ingame chat to appear for 1 minute instead of only 20 seconds (testwise)

1.5.4.1:
- Fixed a bug in the HTTP handling code, which made online game unplayable slow

1.5.4:
- Fixed bug on gcw, which made the game unplayable
- (Hopefully) fixed a bug, which made a client crash right after the start if a game with a chance of ~10%
- Automaticly reconnect of IRC loose connection
- Added logging for irc
- Less time between two downloads of a turn to avoid wifi energy saving, which slows down the whole game
- Fixed a crash, when exiting a game via removing the last player
- Creating backups from the OPKs for the GCW, too
- Tomato sprites instead of "Weapon points"
- Line between player and cross haire for easier aiming
- Removed confusing "1 minute timeout" message
- Building tool can build in the ground, but more savety distance to hares
- No sending of empty messages in Chats
- Chat is now [Start] ingame
- Fixed error message if server is not avaible

1.5.3.7:
- Telling when a game happened for replays

1.5.3.6:
- If the game master leaves the game, the game is not deleted from the game list, except (s)he want it.

1.5.3.5:
- Using Menu as fourth button in Linux/Windows

1.5.3.4:
- Line break for ingame chat (Really, you guys talk to much! ;)
- No flickering when entering text while a loading screen is active

1.5.3.3:
- Added spectate modus
- You can even spectate already started, but not yet finished games!

1.5.3.2:
- Recompile with NEON support (forgot it in 1.5.3.1)
- Some little speed optimizations

1.5.3.1:
- Some sparrow3d improvements to draw faster
- Showing the needed server version in the lobby

1.5.3:
- Fixed a bug, that the very close use of the build tool could freeze the game

1.5.2.2:
- Improved handling of disconnects (now called "Zombies" ingame)
- Fixed jump while typing
- No message filter for irc messages in replays

1.5.2.1:
- Right aligned text input if more text than fits in the window
- No movement while typing
- If admin leaves game, the game is not shown in the game list anymore
- rotation stays fix when no gravitation
- Less network spam
- Removed "Aiming 100%" label above AI after it shoot

1.5.2:
- Using NEON for little speed up
- Fixed a bug in the particle system

1.5.1:
- Added mini map

1.5.0.3
- Improved user name checking of IRC nick

1.5.0.2
- Fixed a bug, which fucked the deterministic, which is baaaad
- Replacing spaces with underscore in nicknames

1.5.0.1
- bigger sprites for small screens
- Fixed keyboard-gui-collision
- Backspace button for onscreen keyboard devices (like the GCW)

1.5.0
- More sprites
- Sprites chooseable
- Username saving
- Username guessing with C4A profile information
- server improvements
- better weapon illustration
- Noticement if sprite is used by another player
- Better camera movement
- Username has to be choosen in lobby
- IRC chat
- Chat in lobby
- Sending and receiving chat messages from within the game
- Random button for insult in chat
- Too expensive weapons are greyed out
- Showing level border
- New weapons:
	- A bigger and a smaller building tool
	- Cluster bomb
	- Mine shooter
	- Super jump
	- Weapon point harvesting
	- Little damage shoot without gravity
	- High damage shoot without gravity
	- Instant kill
	- Forced super jump
- Added items:
	- health
	- more weapon points
	- mine
	
1.4.7.2
- Added button arrow images to sparrow3d and use them now in hase in the help

1.4.7.1
- Fixed crash if last player removed in online game (thx @zear)

1.4.7
- Fixed small resolution sprites
- Improved targeting
- Improved button mapping for handhelds without keyboard

1.4.6
- faster showing of own messages in chat
- level color for the bullet traces
- better and new default values for creating a game

1.4.5:
- Added option to not rotate the level, but the bunny
- Added option to invert the controls of the bunny is upside down
- Improved the particle effect (should be faster)

1.4.4:
- Fixed broken network play

1.4.3:
- Arrow stays at minimal size while zooming to see where you are aiming at large zoom-outs
- Circle around active hare to tell which of your characters you are currently controlling
- Options menu for setting like volume, zooming behaviour and hare markation
- Changeable particle size
- Added Sound

1.4.2:
- An option to hide the player names on demand for better aiming when you have a lot of the screen area covered with the names
- Improved colours

1.4.1:
- Fixed little bug after finished turn in online game
- AI is smarter (no friendly fire)
- Faster canceling online game
- Canceled online game will have no replay

1.4:
- 5s extra time after last bullet
- 3 weapon points per turn
- Locking zoom
- Improved Aiming
- New health bar
- Fixed a bug in the bullet calculation
- improved button mapping, using SP_PRACTICE
- Writing down tractory while flying
- New window approach
- Menu
- Multiple hares per player
- New controls
- Global health bar
- Particle Effect
- 6 Weapons


1.3:
- network support
- Start of change logging
