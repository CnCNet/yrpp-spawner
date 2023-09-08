### Custom mixes

Mixes can be preloaded and postloaded. Preload mixes will overwrite all content of original mix files and postloaded mix files.
Postloaded mixes will be overwritten by any other mix files content (include original files).

> Overwite priority it's loading order:\
Preload mixes -> Original mixes -> Postload mixes

Configuration file (spawn.ini) allow to configure mix loading via two sections: *mixes_preload* for preloading, *mixes_postload* for postloading.
This section it is just a sorted list of filenames. 

Example:

> [mixes_preload] \
1=HTNK.mix \
0=HTK.mix

The first will be HTK.mix and then HTNK.mix - this list will sorted in ascending order.