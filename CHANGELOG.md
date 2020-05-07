# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Lua binding for `Box::IntersectsBox(otherBox)`, that returns true if 2 boxes intersect.

- Command line arguments for launching directly into editors using `-editor "EditorName"`.  
Valid editor names are: `ActorEditor`, `GibEditor`, `SceneEditor`, `AreaEditor` and `AssemblyEditor`.

- Added handling for custom number and string values in INI.
```
AddCustomValue = NumberValue
	YourKeyName = YourNumberValue // Integer or floating point number.

AddCustomValue = StringValue
	YourKeyName = YourStringValue
```
`YourKeyName` is a string value and is not limited to just numbers.
	
- New `Settings.ini` property `AdvancedPerformanceStats = 0/1` to disable/enable the performance counter graphs (enabled by default).

- Added `PassengerSlots` INI and Lua property to Actors. This determines how many spaces in the buy menu an actor will take up (1 by default). It must be a whole number but can theoretically be 0 or less.

- Added Lua bindings for `IsInsideX` and `IsInsideY` to `Area`. These act similarly to the pre-existing `IsInside`, but allow you to check for the X and Y axes individually.

### Changed

- Codebase now uses the C++14 standard.

- Major cleanup and reformatting in the `System` folder.

- Upgraded to new, modern FMOD audio library. ([Issue #72](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/72)).  
Sounds now play in 3D space, so they pan to the left and right, and attenuate automatically based on the player's viewpoint.

- `Sounds` have been renamed to `SoundContainers`, and are able to handle multiple sounds playing at once. INI definitions have changed accordingly.  
They must be added using `... = SoundContainer`, and individual sounds for them must be added using `AddSound = ContentFile...`.

- Various lua bindings around audio have been upgraded, changed or fixed, giving modders a lot more control over sounds. See documentation for more details.

- Centered the loading splash screen image when `DisableLoadingScreen` is true.

- `Box:WithinBox` lua bindings have been renamed: 
`Box:WithinBox` is now `Box:IsWithinBox`.  
`Box:WithinBoxX` is now `Box:IsWithinBoxX`.  
`Box:WithinBoxY` is now `Box:IsWithinBoxY`.

- Made `AHuman` show both weapon ammo states when 2 one-handed weapons are equipped.

- Added support for multiple lines in item descriptions ([Issue#58](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/58)). This is done as follows:
```
Description = MultiLineText
	AddLine = First line of text
	AddLine = Second line of text
	...
```

- `FrameMan` broken down to 4 managers. New managers are:  
`PerformanceMan` to handle all performance stats and measurements.  
`PostProcessMan` to handle all post-processing (glows).  
`PrimitiveMan` to handle all lua primitive drawing.

- Post-processing (glow effects) is now enabled at all times with no option to disable.

- All lua primitive draw calls are now called from `PrimitiveMan`.  
For example: `FrameMan:DrawLinePrimitive()` is now `PrimitiveMan:DrawLinePrimitive()`.

- Resolution multiplier properties (`NxWindowed` and `NxFullscreen`) in settings merged into a single property `ResolutionMultiplier`.

- Incompatible/bad resolution settings will be overriden at startup with messages expaining the issue instead of multiple mode switches and eventually a reset to default VGA.  
Reset to defaults (now 960x540) will happen only on horrible aspect ratio or if you managed to really destroy something.

- You can no longer toggle native fullscreen mode from the settings menu or ini. Instead, either select your desktop resolution at 1X mode or desktop resolution divided by 2 at 2X mode for borderless fullscreen windowed mode.  
Due to limitations in Allegro 4, changing the actual resolution from within the game still requires a restart.

- If the current game resolution is half the desktop resolution or less, you will be able to instantly switch between 1X and 2X resolution multiplier modes in the settings without screen flicker or delay.  
If the conditions are not met, the mode switch button will show `Unavailable`.

### Fixed

- Fixed LuaBind being all sorts of messed up. All lua bindings now work properly like they were before updating to the v141 toolset.

- Explosives (and other thrown devices) will no longer reset their explosion triggering timer when they're picked up. ([Issue #71](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/71))

- Sprite Animation Mode `ALWAYSPINGPONG` now works properly. Sprite animation has also been moved to `MOSprite` instead of `MOSRotating`, they they'll be able to properly animate now. ([Issue#77](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/77))

- Fixed `BG Arm` flailing when reloading one-handed weapon, so shields are no longer so useless.

- Fixed crash when clearing an already empty preset list in the buy menu.

- Temporary fix for low mass attachables/emitters being thrown at ridiculous speeds when their parent is gibbed.

### Removed

- Removed all Gorilla Audio and SDL Mixer related code and files.

- Removed all Steam Workshop and Achievement related code.

- Removed a bunch of outdated/unused sources in the repo.

- Removed all OSX/Linux related code and files because we don't care. See [Liberated Cortex](https://github.com/liberated-cortex) for working Linux port.

- Removed a bunch of low-level `FrameMan` lua bindings:  
`FrameMan:ResetSplitScreens`, `FrameMan:PPM` setter, `FrameMan:ResX/Y`, `FrameMan:HSplit/VSplit`, `FrameMan:GetPlayerFrameBufferWidth/Height`, `FrameMan:IsFullscreen`, `FrameMan:ToggleFullScreen`, 
`FrameMan:ClearBackbuffer8/32`, `FrameMan:ClearPostEffects`, `FrameMan:ResetFrameTimer`, `FrameMan:ShowPerformanceStats`.

- Native fullscreen mode has been removed due to poor performance compared to windowed/borderless mode and various input device issues.  
The version of Allegro we're running is pretty old now (released in 2007) and probably doesn't properly support/utilize newer features and APIs leading to these issues.  
The minimal amount of hardware acceleration CC has is still retained through Windows' DWM and that evidently does a better job.

- Removed now obsolete `Settings.ini` properties:  
Post-processing: `TrueColorMode`, `PostProcessing`, `PostPixelGlow`.   
Native fullscreen mode: `Fullscreen`, `NxWindowed`, `NxFullscreen`, `ForceSoftwareGfxDriver`, `ForceSafeGfxDriver`.

***

## [0.0.1.0] - 2020-01-27

### Added

- You can now run the game with command line parameters, including `-h` to see help and `-c` to send ingame console input to cout.

- `MOSprite` now has the `FlipFactor` property that returns -1 if the sprite is flipped and 1 if it's not.  
Using any `nugNum` calculations based on `HFlipped` is now considered criminal activity.

- `TDExplosive` now has the `IsAnimatedManually` property that lets modders set its frames manually through lua.

- You can now add `AEmitters` to `MOSRotating` and have them function similarly to attachables.  
**Addition:** `parent:AddEmitter(emitterToAdd)` or `parent:AddEmitter(emitterToAdd, parentOffsetVector)`  
**Removal:** `parent:RemoveEmitter(emitterToRemove)` or `parent:RemoveEmitter(uniqueIdOfEmitterToRemove)`

- Attachables can now collide with terrain when attached.  
**INI property:** `CollidesWithTerrainWhenAttached = 0/1`  
**Check value:** `attachable.IsCollidingWithTerrainWhileAttached`  
**Manipulate function:** `attachable:EnableTerrainCollisions(trueOrFalse)`  
Collisions can be manipulated only if the attachable was set to `CollidesWithTerrainWhenAttached = 1` in ini.

- `Actor.DeathSound` is now accessible to lua using `Actor.DeathSound = "string pathToNewFile"` or `Actor.DeathSound = nil` for no DeathSound.

- `AHuman` Feet are now accessible to lua using `AHuman.FGFoot` and `AHuman.BGFoot`. Interaction with them may be wonky.

- Streamlined debug process and requirements so old Visual C++ Express edition is no longer needed for debugging.

- Added minimal debug configuration for quicker debug builds without visualization.

### Changed

- `ACrab` aim limits now adjust to crab body rotation.

- `ACrab.AimRange` can now be split into `AimRangeUpperLimit` and `AimRangeLowerLimit`, allowing asymmetric ranges.

- Objective arrows and Delivery arrows are now color co-ordinated to match their teams, instead of being only green or red.

- BuyMenu `Bombs` tab will now show all `ThrownDevices` instead of just `TDExplosives`.

- The list of `MOSRotating` attchables (`mosr.Attachables`) now includes hardcoded attachables like dropship engines, legs, etc.

- Attachable lua manipulation has been significantly revamped. The old method of doing `attachable:Attach(parent)` has been replaced with the following:  
**Addition:** `parent:AddAttachable(attachableToAdd)` or `parent:AddAttachable(attachableToAdd, parentOffsetVector)`  
**Removal:** `parent:RemoveAttachable(attachableToRemove)` or `parent:RemoveAttachable(uniqueIdOfAttachableToRemove)`
  
- Wounds have been separated internally from emitter attachables.  
They can now be added with `parent:AddWound(woundEmitterToAdd)`.  
Removing wounds remains the same as before.

- Built-in Actor angular velocity reduction on death has been lessened.

### Fixed

- SFX slider now works properly.

- BGM now loops properly.

- Sound pitching now respects sounds that are not supposed to be affected by pitch.

- Using `actor:Clone()` now works properly, there are no longer issues with controlling/selecting cloned actors.

- `TDExplosive.ActivatesWhenReleased` now works properly.

- Various bug fixed related to all the Attachable and Emitter changes, so they can now me affected reliably and safely with lua.

- Various minor other things that have gotten lost in the shuffle.


### Removed

- All licensing-related code has been removed since it's no longer needed.

- Wounds can no longer be added via ini, as used to be doable buggily through ini `AddEmitter`.

- All usage of the outdated Slick Profiler has been removed.

- `TDExplosive.ParticleNumberToAdd` property has been removed.

***

Note: For a log of changes made prior to the commencement of the open source community project, look [here.](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Previous-Closed-Source-Changelog)


[unreleased]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/compare/master...cortex-command-community:development
[0.0.1.0]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/releases/tag/v0.0.1
