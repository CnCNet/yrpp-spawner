# Observer overlay for streamers

This overlay is for people who observe and stream CnCNet Yuri's Revenge games. It shows each player's credits, power,
current production and build queues, and how many key units and buildings they own (Rhinos, Grizzlies, War Factories,
Battle Labs and so on). You add it to OBS as a Browser Source.

It has two parts:

1. **The spawner** writes `observer_overlay.json` to the game folder about once a second, but only while you are
   **observing** (`HouseClass::IsCurrentPlayerObserver()`). Players can't turn this on to see their opponents.
2. **`server.py`** serves that file and the overlay pages on `http://127.0.0.1:8765/`. It needs Python 3.9 or newer and
   nothing else.

## Setup

1. Turn the export on in `RA2MD.INI`:

   ```ini
   [Options]
   ObserverOverlay=yes
   ; Optional. Game frames between updates. The default of 15 is about once a second at game speed 6.
   ObserverOverlay.Interval=15
   ```

2. Start the server and point it at your game folder:

   ```
   python server.py --game-dir "C:\Games\CnCNet\Yuri's Revenge"
   ```

3. In OBS, add a **Browser Source**:
   - `http://127.0.0.1:8765/` shows all players side by side.
   - `http://127.0.0.1:8765/player.html?slot=0` shows a single player (`slot=0` to `slot=7`, in house order). Add one
     source per player to place them separately, for example in the corners of the screen.

   The pages have a transparent background, so they sit on top of the game capture.

## Choosing which units are counted

`units.json` lists the units and buildings shown as counters, in display order, by their `rulesmd.ini` ID. Add a
`label` to override the in-game name. Edit it for mods, or pass a different file with `--units my_units.json`.

The spawner exports the counts of **every** unit and building a player owns, so any ID you list will work.

## Snapshot format (`observer_overlay.json`, version 1)

```json
{
  "version": 1,
  "frame": 1800,
  "players": [
    {
      "index": 0, "name": "Kane", "country": "Russians", "color": "#E01008",
      "credits": 4250, "power": { "output": 300, "drain": 350 }, "defeated": false,
      "production": [
        { "category": "Vehicle", "current": { "id": "HTNK", "name": "Rhino Tank" }, "progress": 50,
          "onHold": false, "done": false, "queue": [ { "id": "HTK", "name": "Flak Track" } ] }
      ],
      "counts": [ { "category": "Vehicle", "id": "HTNK", "name": "Rhino Tank", "count": 6 } ]
    }
  ]
}
```

The spawner writes a temporary file and then swaps it in, so readers never see a half-written snapshot.

## Tests

From the repository root:

```
g++ -std=c++20 -Wall -Wextra -I src/Misc/ObserverOverlay src/Misc/ObserverOverlay/Snapshot.cpp tools/ObserverOverlay/tests/snapshot_json_test.cpp -o snapshot_json_test && ./snapshot_json_test
python -m unittest discover -s tools/ObserverOverlay/tests
node --test tools/ObserverOverlay/tests/overlay.test.js
```
