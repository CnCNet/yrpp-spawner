Downloads
---------
- [Latest releases](https://github.com/CnCNet/yrpp-spawner/releases)

Building manually
-----------------

1. Install **Visual Studio** (2022 is recommended, 2019 is minimum) with the dependencies listed in `.vsconfig` (it will prompt you to install missing dependencies when you open the project, or you can run VS installer and import the config). If you prefer to use **Visual Studio Code** you may install **VS Build Tools** with the dependencies from `.vsconfig` instead. Not using a code editor or IDE and building via **command line scripts** included with the project is also an option.
2. Clone this repo recursively via your favorite git client (that will also clone YRpp and private submodule, if you have access).
3. To build the extension:
   - in Visual Studio: open the solution file in VS and build it (`Debug` build config is recommended);
   - in VSCode: open the project folder, hit `Run Build Task...` (`Ctrl + Shift + B`) and select the needed config;
   - barebones: run the corresponding `scripts/build_*.bat`.
4. Upon build completion the resulting `CnCNet-Spawner.dll` and `CnCNet-Spawner.pdb` would be placed in the subfolder identical to the name of the build config executed.

Please note that you can build the hardened version only if you have access to it's source code. It is though automatically built and made available to download on pull requests, releases and nightly builds via GitHub Actions.

Credits
-------
- **[Belonit](https://github.com/Belonit)** - Porting and adapting
- **[Kerbiter (Metadorius)](https://github.com/Metadorius)** - Further maintenance
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
