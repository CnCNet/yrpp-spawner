**CnCNet Spawner** is a project that allows CnCNet client application (like [XNA CnCNet Client](https://github.com/CnCNet/xna-cncnet-client)) to "spawn" (launch) the game right into a battle, skipping the menus and configuring game's own and CnCNet-specific things using a config file (`spawn.ini`).

This particular spawner is written using a framework set up by [Ares](https://github.com/Ares-developers/Ares), [Phobos](https://github.com/Phobos-developers/Phobos) projects, uses [YRpp](https://github.com/Phobos-developers/YRpp) to interface with the game and is a rewrite of the original [YR patches](https://github.com/CnCNet/yr-patches) made by CnCNet team. It is built as a DLL loadable by [Syringe](https://github.com/Phobos-developers/Syringe).

Downloads
---------

- [Latest releases](https://github.com/CnCNet/yrpp-spawner/releases)

Building manually
-----------------

1. Install **Visual Studio** (2022 is required) with the dependencies listed in `.vsconfig` (it will prompt you to install missing dependencies when you open the project, or you can run VS installer and import the config). If you prefer to use **Visual Studio Code** you may install **VS Build Tools** with the dependencies from `.vsconfig` instead. Not using a code editor or IDE and building via **command line scripts** included with the project is also an option.
2. Clone this repo either recursively, if you are a member of CnCNet org, or if you are not (you don't have access to private submodule) - clone non-recursively, then init-and-update the YRpp submodule only.
3. To build the extension:
   - in Visual Studio: open the solution file in VS and build it (`Debug` build config is recommended);
   - in VSCode: open the project folder, hit `Run Build Task...` (`Ctrl + Shift + B`) and select the needed config;
   - barebones: run the corresponding `scripts/build_*.bat`.
4. Upon build completion the resulting `CnCNet-Spawner.dll` and `CnCNet-Spawner.pdb` would be placed in the subfolder identical to the name of the build config executed.

Please note that you can build the hardened version only if you have access to it's source code. It is though automatically built and made available to download on pull requests, releases and nightly builds via GitHub Actions.

Credits
-------

- **[Belonit](https://github.com/Belonit)**
  - Porting and adapting of the original spawner
  - Further maintenance
  - Fix for drawing maps smaller than the screen
- **[Kerbiter (Metadorius)](https://github.com/Metadorius)**
  - Further maintenance
  - Event verification checks
  - Save button for multiplayer pause menu
  - Beacon crash fix for multiplayer save/load
  - Game speed slider toggle
  - Fake multiplayer flag
- **[ZivDero](https://github.com/ZivDero)**
  - Handicaps (difficulty & credits) support
- **[Starkku](https://github.com/Starkku)**
  - Allow customizing whether or not special house is ally to all players via spawn.ini option (#51)
- **[RAZER](https://github.com/CnCRAZER)**
  - Game speed slider toggle
- **[TaranDahl](https://github.com/TaranDahl)**
  - Porting of multiplayer save/load
  - Porting of autosaves
- **[Rampastring](https://github.com/Rampastring)**
  - Original event verification checks
- **[Vinifera](https://github.com/Vinifera-Developers/Vinifera) Contributors and [TS Patches](https://github.com/CnCNet/ts-patches) Contributors**
  - Original TS implementation of multiplayer save/load
  - Original TS implementation of autosaves
- **[CnCNet](https://github.com/CnCNet) Contributors** - the [original spawner](https://github.com/CnCNet/yr-patches)
- **[Ares](https://github.com/Ares-Developers/Ares) and [Phobos](https://github.com/Phobos-developers/Phobos) Contributors** - [YRpp](https://github.com/Phobos-developers/yrpp) and [Syringe](https://github.com/Ares-Developers/Syringe) which are used and some code snippets

Legal and License
-----

[![GPL v3](https://www.gnu.org/graphics/gplv3-127x51.png)](https://opensource.org/licenses/GPL-3.0)

This project is an unofficial open-source community collaboration project to patch the Red Alert 2 Yuri's Revenge engine for CnCNet support & compatibility purposes.

This project has no direct affiliation with Electronic Arts Inc. Command & Conquer, Command & Conquer Red Alert 2, Command & Conquer Yuri's Revenge are registered trademarks of Electronic Arts Inc. All Rights Reserved.

Sponsored by
------------

<a href="https://www.digitalocean.com/?refcode=337544e2ec7b&utm_campaign=Referral_Invite&utm_medium=opensource&utm_source=CnCNet" title="Powered by Digital Ocean" target="_blank">
    <img src="https://opensource.nyc3.cdn.digitaloceanspaces.com/attribution/assets/PoweredByDO/DO_Powered_by_Badge_blue.svg" width="201px" alt="Powered By Digital Ocean" />
</a>
