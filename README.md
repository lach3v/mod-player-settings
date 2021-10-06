# ![logo](https://raw.githubusercontent.com/azerothcore/azerothcore.github.io/master/images/logo-github.png) AzerothCore

## `mod-player-settings`

### Description
`mod-player-settings` automatically scales dungeons and raids using the [Player Settings](https://diablo2.diablowiki.net/Player_Settings) formula from Diablo II. It is tuned for a solo experience using the notion of offensive and defensive units. In a 5 man for example you'd have 3 DPS, a tank, and a healer which would be 3.5 offensive units (3 DPS and a tank) and 1.5 defensive units (a healer and a tank). Standard compositions have been used for raids:

* 10 mans have 2 tanks, 6 DPS, and 2 healers for 7.0 offensive units and 3.0 defensive units.
* 25 mans have 3 tanks, 17 DPS, and 5 healers for 18.5 offensive units and 6.5 defensive units.
* 40 mans have 4 tanks, 28 DPS, and 8 healers for 30.0 offensive units and 10.0 defensive units.

To find the multiplier for health and damage of the mobs find the reciprocal of the offensive and defensive units respectively.

The module is Blizzlike in that 5 players in a dungeon is a 1.0 multiplier to health and damage. Think of the solo scaling as a base with additional players trending the damage and health multipliers towards 1.0.

*Note: This module hasn't been extensively tested for 10, 25, or 40 mans.*

### How to use ingame
If enabled Player Settings will announce this on login to the player. In an instance players have access to the `.players` command which can be used to change the number of players when given an integer argument or to check the number of players the instance is set to. In an instance, Game Masters also have access to the `.playersettings` command which shows the experience multiplier, and if a creature is selected their health and damage multipliers.

### Installation
Simply `git clone` the module under the `modules` directory of your AzerothCore source or copy paste it manually then re-run cmake and launch a clean build of AzerothCore.

### Edit the module's configuration (optional)
If you need to change the module configuration, go to your server configuration directory (where your `worldserver` or `worldserver.exe` is), copy `PlayerSettings.conf.dist` to `PlayerSettings.conf` and edit that new file.

### Credits
* Wobbling Goblin: [repository](https://github.com/wobgob) - [website](https://wobgob.com)
* AzerothCore: [repository](https://github.com/azerothcore) - [website](http://azerothcore.org/) - [discord chat community](https://discord.gg/PaqQRkd)
