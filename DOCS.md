## Command Line Arguments

### -SPAWN
Launch the game according to the parameters specified in the `SPAWN.INI` file.

### -CD:
Allows the game to run without the `Yuri’s Revenge CD`. Requires the content of the `Yuri’s Revenge CD` to be copied to the `Red Alert` 2 directory first. `The First Decade` and `The Ultimate Collection` users already have all the required files installed, so no further action has to be taken.

### -DumpTypes
lore

### -RA2ModeSaveID=N
lore

## New Parameters of `RA2MD.INI` file
```ini
[Options]
DisableEdgeScrolling=no ; <boolean>

QuickExit=no            ; <boolean>, If 'yes', then turns on the ALT+F4 handler,
                        ; which allows you to safely close games quickly

DDrawHandlesClose=no    ; <boolean>, If 'yes', then turns on the ALT+F4 handler provided by cnc-ddraw
                        ; It's not secure, but it's fast
                        ; Requires the cnc-ddraw

SkipScoreScreen=no      ; <boolean>, If 'yes', then the score screen at the end of the match will be skipped

MPDEBUG=no              ; <boolean>, If 'yes', then the game launches with multiplayer debugging mode enabled

SingleProcAffinity=yes  ; <boolean>, If 'no', If 'no', then disables the patch that forces the game to use only one processor core
                        ; Please note that a similar patch is also present in Ares and some ddraw.dll
                        ; More details: https://ares-developers.github.io/Ares-docs/ui-features/commandlinearguments.html?highlight=-AFFINITY:N:

[Video]
Video.Windowed=no       ; <boolean>, If 'yes', then the game launches in windowed mode

NoWindowFrame=no        ; <boolean>, If 'yes', then in windowed mode will have its window frame disabled

DDrawTargetFPS=-1       ; <integer>, Sets the target FPS
                        ; Requires the ts-ddraw
```

## Fixed Vanilla Game Bugs

- Extend IsoMapPack5 decoding size limit. Author: E1 Elite

## Parameters of `SPAWN.INI` file

### Extended Options

```ini
[Settings]
Ra2Mode=no                  ; <boolean>, If 'yes', then special patches will be applied
                            ; to make the engine look like vanilla RA2

QuickMatch=no               ; <boolean>, If 'yes', then the player names will be forcibly hidden

SkipScoreScreen=no          ; <boolean>, If 'yes', then the score screen will be skipped

WriteStatistics=no          ; <boolean>, If 'yes', then the match statistics will be saved in the stats.dmp file

AINamesByDifficulty=no      ; <boolean>, then AI players will be named according to their difficulty, instead of TXT_COMPUTER
                            ; The names are taken from the following csf table entries: GUI_AIEasy, GUI_AINormal, GUI_AIHard

ContinueWithoutHumans=no    ; <boolean>, If 'yes', then the match continue if all human players are lost, but AI continue to fight

DefeatedBecomesObserver=no  ; <boolean>, If 'yes', then when losing the player will automatically switch to observer mode

Observer.ShowAIOnSidebar=no ; <boolean>, If 'yes', then AI players will be displayed on the observer panel.
```

### Scenario Options
```ini
[Settings]
Seed=0                      ; <integer>, Sets the random seed number

IsSinglePlayer=no           ; <boolean>, If 'yes', then the game launches in campaign mode

ScenarioName=spawnmap.ini   ; <string>, Sets scenario file name, maximum 60 characters
                            ; In addition, special prefixes are supported:
                            ; Prefix 'RA2->' is the same as [Settings]->Ra2Mode=yes
                            ; Prefix 'PlayIntro->' it will show the movie EA_WWLOGO, followed by the specified movie

UIMapName=                  ; <UTF-8 string>, Sets map name, which will be displayed in the diplomacy menu

Tournament=0                ; <integer>

MapHash=                    ; <string>

WOLGameID=-559038737        ; <integer>
```

### Game Mode Options
```ini
[Settings]
GameMode=1          ; <integer>, Sets the game mode index
TechLevel=10        ; <integer>, Sets the maximum TechLevel achievable in the game
Bases=yes           ; <integer>
Credits=10000       ; <integer>, Sets the initial amount of credits
BridgeDestroy=yes   ; <boolean>
Crates=no           ; <boolean>
ShortGame=no        ; <boolean>
Superweapons=yes    ; <boolean>
BuildOffAlly=no     ; <boolean>
GameSpeed=0         ; <integer>, Sets game speed (0 - fastest, 6 - slowest)
MultiEngineer=no    ; <boolean>
UnitCount=0         ; <integer>
AlliesAllowed=no    ; <boolean>
HarvesterTruce=no   ; <boolean>
FogOfWar=no         ; <boolean>
MCVRedeploy=yes     ; <boolean>
AIPlayers=0         ; <integer>, Sets number of AI players
AIDifficulty=1      ; <integer>, Sets the difficulty level, relevant only for campaign mode
UIGameMode=         ; <UTF-8 string>, Sets game mode name, which will be displayed in the diplomacy menu
```

### Save Game Options
```ini
[Settings]
LoadSaveGame=no ; <boolean>, If 'yes', then the game will load the save game file
SaveGameName=   ; <string>, Sets the save game file name
```

### Network Options
```ini
[Settings]
Protocol=2              ; <integer>
                        ; '2' - Default protocol
                        ; '0' - Dynamic protocol that adjusts LatencyLevel on the fly

; for Dynamic protocol
PreCalcMaxAhead=0       ; <integer>
MaxLatencyLevel=255     ; <integer>

; for Default protocol
FrameSendRate=4         ; <integer>
MaxAhead=-1             ; <integer>

ReconnectTimeout=2400   ; <integer>
ConnTimeout=3600        ; <integer>
```

### Tunnel Options
```ini
[Settings]
Port=   ;

[Tunnel]
Ip=     ;
Port=   ;
```

### Human Player Options
<!-- Note that the character `SomePlayerSection` in parameter and section names means house index starting from 1 -->
```ini
[SomePlayerSection] ; Settings, Other1, Other2, Other3, Other4, Other5, Other6, Other7
Name=           ; <UTF-8 string>, Sets the player's display name
Color=-1        ; <integer>     , Sets the player's display color index
Country=-1      ; <integer>     , Sets the player's country index
IsObserver=no   ; <boolean>     , Sets observer loading screen for this player

Ip=0.0.0.0      ; <string>
Port=-1         ; <integer>
```

### House Options
- Note that the character `*` in parameter and section names means house index starting from 1.
- Please note that player indexes do not correspond to house indexes.
- To project player indexes to house indexes, you need to sort the players by their in-game color index.
- AI players do not need to be sorted and are located immediately behind human players
```ini
[SpawnLocations]
Multi*=-2           ; <integer>
                    ; From '0' to '7' to select spawn locations
                    ; '-2' to select random
                    ; '-1' or '90' to disable spawn locations reservations and make observer

[IsSpectator]
Multi*=no           ; <boolean>, If 'yes', disable spawn locations reservations and make observer

[Multi*_Alliances]
HouseAllyOne=-1     ; <integer> - House index
HouseAllyTwo=-1     ; <integer> - House index
HouseAllyThree=-1   ; <integer> - House index
HouseAllyFour=-1    ; <integer> - House index
HouseAllyFive=-1    ; <integer> - House index
HouseAllySix=-1     ; <integer> - House index
HouseAllySeven=-1   ; <integer> - House index
HouseAllyEight=-1   ; <integer> - House index
```
Note that in `[Multi*_Alliances]` the house index value starts from 0.

While `-1` means no ally in the slot

### AI House Options
```ini
[HouseColors]
Multi*=-1        ; <integer>, Sets the display color index of the AI ​​house

[HouseCountries]
Multi*=-1        ; <integer>, Sets the country index of the AI ​​house

[HouseHandicaps]
Multi*=-1        ; <integer>, Sets the difficulty level of the AI ​​house
```