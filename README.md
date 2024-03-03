# SA MixSets v4.3.8 Release

This release includes improvements and fixes for the gunflash system in addition to the features found in the regular MixSets version. The latest update adds option to configure internal variables like offsets, time multiplicators and surfing speed detection. Those are all possible by reloading Mixsets (i.e.: typing `sets` in game)

It curently changes:
* Muzzle flashes when onfoot
* Muzzle flashes when drivebying as driver 
* Muzzle flashes when drivebying as passenger

## Features Examples

[Introduction Video - long](https://www.youtube.com/watch?v=cP7IlqUQ1bU)

[Changing Weapon Moving Offsets - short](https://www.youtube.com/watch?v=wXu1R0vs46I)

[Changing Weapon Particle - short](https://www.youtube.com/watch?v=f3LBxqJXteI)

[Changing Weapon Particle Speed Multiplier - short](https://www.youtube.com/watch?v=3FuMX88LCz8)



## For SA-MP Users

Also, the version has been adapted to **SA-MP**. There is no need to reload Mixsets for the gunflashes to appear.

## **Warning:**
**Always backup your files before replacing anything! In this case, make a zip out of your Mixsets.asi and Mixsets.ini.**

## Installation Instructions:

0. If you are using IMFX, open `imfx.dat`, then set `ENABLE_GUNFLASHES` to `NO`.

1. Replace your existing `Mixsets.asi` with the one provided in this release.

3. In your `Mixsets.ini` file, navigate to the section titled `# Special effects.`. Set `No3DGunflash` and `Fix2DGunflash` options to 1.

3. In the same `Mixsets.ini`, navigate to the section titled `# Special effects.` and go to it's last variable (`DisplayDialogAnyAR`).

4. Add the following line below the mentioned section:
```ini
#m0b's gunflashes system
ExperimentalGunFlash = 1		# (0|1) Add's m0b's gunflash fixes, to be used along with No3DGunflash Fix2DGunflash
#Offsets for m0b's gunflashes system
OnFootOffset 		 = 2.15		# (2.0) Moving offset distance multiplier for firing weapon while on foot (needs ExperimentalGunFlash = 1)
CarDriverOffset		 = 2.0		# (2.0) Moving offset distance multiplier for firing weapon while driving a car (needs ExperimentalGunFlash = 1)
CarPassengerOffset	 = 1.15		# (1.15 Moving offset distance multiplier for firing weapon while being in a car passenger seat (needs ExperimentalGunFlash = 1)
BikeDriverOffset	 = 2.0		# (2.0) Moving offset distance multiplier for firing weapon while driving a bike  (needs ExperimentalGunFlash = 1)		
BikePassengerOfffset = 1.15		# (1.15) Moving offset distance multiplier for firing weapon while being in a bike passenger seat  (needs ExperimentalGunFlash = 1)
#Time multipliers for m0b's gunflashes system
InVehicleTimeMult	 = 1.35     # (1.35) Speed multiplier for gunflash particle when shooting from a vehicle (the higher, the less the particle will last)
DualWeildingTimeMult = 1.35     # (1.25) Speed multiplier for gunflash particle when moving while shooting with dual weilding weapons (the higher, the less the particle will last)
SingleWeaponTimeMult = 1.15     # (1.15) Speed multiplier for gunflash particle when moving while shooting with a single weapon that can be dual weilded(the higher, the less the particle will last)
SurfingSpeed 		 = 0.1      # (0.1) Surfing speed detection (sitting on a moving vehicle), when surfing, the mod will not draw the gunflash partticle

[IMFX_Gunflashes]
22 = gunflash 1 1 # (gunflash 1 1) particle for firing a pistol, then rotation and smoke
23 = gunflash 1 1 # (gunflash 1 1) particle for firing a silenced pistol, then rotation and smoke
24 = gunflash 1 1 # (gunflash 1 1) particle for firing a desert eagle, then rotation and smoke
25 = gunflash 1 1 # (gunflash 1 1) particle for firing a shotgun, then rotation and smoke
26 = gunflash 1 1 # (gunflash 1 1) particle for firing a sawn-off shotgun, then rotation and smoke
27 = gunflash 1 1 # (gunflash 1 1) particle for firing a spas-12, then rotation and smoke
28 = gunflash 1 1 # (gunflash 1 1) particle for firing a micro-uzi, then rotation and smoke
29 = gunflash 1 1 # (gunflash 1 1) particle for firing a mp5, then rotation and smoke
30 = gunflash 1 1 # (gunflash 1 1) particle for firing a ak47, then rotation and smoke
31 = gunflash 1 1 # (gunflash 1 1) particle for firing a m4 , then rotation and smoke
32 = gunflash 1 1 # (gunflash 1 1) particle for firing a tec9, then rotation and smoke
33 = gunflash 1 1 # (gunflash 1 1) particle for firing a country rifle, then rotation and smoke
34 = gunflash 1 1 # (gunflash 1 1) particle for firing a sniper rifle, then rotation and smoke
38 = gunflash 1 1 # (gunflash 1 1) particle for firing a minigun, then rotation and smoke
   ```

After the addition, your `Mixsets.ini` should look like this:

![image](https://github.com/m0b-x/SA-MixSets/assets/72597190/3c057922-683a-4e73-9678-6e1f7a79cc9a)
