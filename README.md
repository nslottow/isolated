# Isolated
is just the name of this game project based on the prompt, "Space Isolation."
It's a simple, competitive game in which players build walls in an attempt to
gain territory and ensnare opponents.

The game is currently in a playable state with a single game mode and some
parameters that are configurable via a settings file.

Part of my motivation for writing the game is to take a small networked C++ game
project from start to finish. I'm interested to see what types of design
questions and problems arise throughout the process, whether they be anticipated
or not. I learn mostly from direct experience: trying and failing or succeeding.
I prototyped the game in Unity and thought the concept was neat enough to
continue working on. So for the sake of a full experience (good or bad), here I
am.

## Gameplay
The objective of the currently available versus mode is to smush your opponent
until he runs out of *stock*, or lives. You do so by building walls,
holding down the *build* button to create a stream of them. You can build
through your own walls, meaning streams of walls can pass through walls of the
same color, but not through walls of other players. A player is killed when
more than 50% of his area overlaps with a wall.

If you enclose an empty rectangular region of cells with walls, then that
region will automatically be filled in with walls of your color. Be careful
not to wall yourself in this way. Also note that at this point the regions
must be *empty* (i.e. not contain any other walls) and *rectangular*,
although I'm considering adding support for other alternatives.

## Controls
#### Player 1
* Movement - arrow keys
* Build - period (.)
* Melee - comma (,)

#### Player 2
* Movement - WASD
* Build - 1
* Melee - 2

## Hacking
The game is far from complete, but is simple enough that it is definitely
hackable. Since the APIs for objects and the structure of the program are
in flux right now, I will probably not directly incorporate any pull
requests that affect gameplay in a significant way. I'm open to criticisms
and suggestions though!