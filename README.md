# SA MixSets v4.3.8 Release

This release includes improvements and fixes for the gunflash system in addition to the features found in the regular MixSets version. 
It curently changes:
* Muzzle flashes when onfoot
* Muzzle flashes when driveby-ing as driver 
* Muzzle flashes when driveby-ing as passenger

## For SA:MP Users

Also, the version has been adapted to **SA-MP**. There is no need to reload Mixsets for the gunflashes to appear.

## **Warning:**
**Always backup your files before replacing anything!**

## Installation Instructions:

1. Replace your existing `Mixsets.asi` with the one provided in this release.

2. In your `Mixsets.ini` file, navigate to the section titled `# Special effects.` Locate the line starting with `Fix2DGunflash`.

3. Add the following line below the mentioned section:
   ```ini
   ExperimentalGunFlash = 1     # (0|1) Adds m0b's gunflash fixes, to be used along with No3DGunflash Fix2DGunflash
![image](https://github.com/m0b-x/SA-MixSets/assets/72597190/d42a5049-2022-4830-9d7c-2c55e16f5cd6)
