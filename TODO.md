# Gameplay
* Pushing blocks
* Wall materials as resources
* Strafing while building walls
* Optional: Gradually falling blocks post-construction to free up the board
* Game modes
  * Stock
  * Kills
  * Fill
* Respawn cells that can't be built on. Have the player spawn randomly or
  choose where to respawn.
* Death cells (black holes)
* Safe cells that can't be build on
* Change fill method via config. Any non-instantaneous method will create
  different strategies for saving yourself when blocked in.
  * Instantaneous (default)
  * Fill with concentric rings
  * Spiral inward from filling block
  * Sweep across
* Change fill rules via config
  * Empty rectangular regions (default)
  * Rectangular regions
  * Empty regions of any shape, team
  * Empty regions of a given team
  * Single cells surrounded on 3 or more sides
* Powerups embedded in walls that killed players
  * Wall extend speed up
  * Wall build speed up
  * Player movement speed up
  * Longer fall time for gradually falling blocks
* Bombs
* PvP melee knock back attack
* Teleporter cells
* Ice cells: pushed blocks slide until they hit a wall or non-ice
* AI

# Usability
* ~~Debug font~~
* ~~Load input mappings from config~~
* Joystick support
* Game configuration menu
* Saving configuration options
* Video options configuration menu
* Map editor
* Input configuration menu
* Steam API integration, mostly to see how that works
* Maybe make player collision shape into a circle so it's easier to move around walls

# Frills
* Pixel art tileset
* Character sprites
* Sounds

# Code 
* ~~Assign unique Entity IDs to prepare for networking~~
* ~~Allow comments at the end of lines in config files~~
* Explicit registration/creation of config variables (get rid of having to provide default, type checking)
  Maybe throw an exception if you try to get a variable that doesn't exist
* Logger based on ostream
* Proper rendering separate from game state classes
* (De)serialization of game objects (Entities, Timers, Game)