# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- New `Settings.ini` property `LoadingScreenReportPrecision = intValue` to control how accurately the module loading progress reports what line is currently being read.  
	Only relevant when `DisableLoadingScreen = 0`. Default value is 100, lower values increase loading times (especially if set to 1).  
	This should be used for debugging where you need to pinpoint the exact line that is crashing and the crash message isn't helping or doesn't exist at all.

- New `Settings.ini` property `MenuTransitionDuration = floatValue` to control how fast transitions between different menu screens happen (e.g main menu to activity selection screen and back).  
	This property is a multiplier, the default value is 1 (being the default hardcoded values), lower values decrease transition durations. 0 makes transitions instant.

### Changed

- `Settings.ini` will now fully populate with all available settings (now also broken into sections) when being created (first time or after delete) rather than with just a limited set of defaults.

- Temporarily removed `PreciseCollisions` from `Settings.ini` due to bad things happening when disabled by user.

- Lua error reporting has been improved so script errors will always show filename and line number.

- Specially handled Lua function `OnScriptRemoveOrDisable` has been changed to `OnScriptDisable`, and no longer has a parameter saying whether it was removed or disabled, since you can no longer remove scripts.

- Game will now Abort with error message when trying to load a non-existent AtomGroup Presetname.

### Fixed

- Control schemes will no longer get deleted when being configured.
Resetting the control scheme will load a preset instead of leaving it blank. (Issue #121)

- Time scale can no longer be lowered to 0 through the performance stats interface.

### Removed

- Removed the ability to remove scripts from objects with Lua. This is no longer needed cause of code efficiency increases.

***

## [0.1.0 pre-release 2][0.1.0-pre2] - 2020/05/08

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

- Added the concept of `SoundSets`, which are collections of sounds inside a `SoundContainer`. This allows you to, for example, put multiple sounds for a given gunshot inside a `SoundSet` so they're played together.

- `SoundContainers` have been overhauled to allow for a lot more customization, including per-sound customization. The following INI example shows all currently availble capabilities with explanatory comments:
	```
	AddSoundContainer = SoundContainer // Note that SoundContainers replace Sounds, so this can be used for things like FireSound = SoundContainer
		PresetName = Preset Name Here

		CycleMode = MODE_RANDOM (default) | MODE_FORWARDS // How the SoundContainer will cycle through its `SoundSets` whenever it's told to select a new one. The former is prior behaviour, the latter cycles through SoundSets in the order they were added.

		LoopSetting = -1 | 0 (default) | 1+ // How the SoundContainer loops its sounds. -1 means it loops forever, 0 means it plays once, any number > 0 means it plays once and loops that many times.

		Immobile = 0 (default) | 1 // Whether or not the SoundContainer's sounds should be treated as immobile. Immobile sounds are generally used for UI and system sounds; they will always play at full volume and will not be panned or affected by global pitch during game slowdown.

		AttenuationStartDistance = Number (default -1) // The distance at which the SoundContainer's sounds will start to attenuate out, any number < 0 set it to the game's default. Attenuation calculations follows FMOD's Inverse Rolloff model, which you can find linked below.

		Priority = 0 - 256 (default 128) // The priority at which the SoundContainer's sounds will be played, between 0 (highest priority) and 256 (lowest priority). Lower priority sounds are less likely to be played are a lot of sounds playing.

		AffectedByGlobalPitch = 0 | 1 (default) // Whether or not the SoundContainer's sounds will be affected by global pitch, or only change pitch when manually made to do so via Lua (note that pitch setting is done via AudioMan).

		AddSoundSet = SoundSet // This adds a SoundSet containing one or more sounds to the SoundContainer.

			AddSound = ContentFile // This adds a sound to the SoundSet, allowing it to be customized as shown.
				Filepath = "SomeRte.rte/Path/To/Sound.wav"

				Offset = Vector // This specifies where the sound plays with respect to its SoundContainer. This allows, for example, different sounds in a gun's reload to come from slightly different locations.
					X = Number
					Y = Number

				AttenuationStartDistance = Number // This functions identically to SoundContainer AttenuationStartDistance, allowing you to override it for specific sounds in the SoundContainer.

				MinimumAudibleDistance = Number (default 0) // This allows you to make a sound not play while the listener is within a certain distance, e.g. for gunshot echoes. It is automatically accounted for in sound attenuation.

			AddSound = "SomeRte.rte/Path/To/AnotherSound.wav" // This adds a sound to the SoundSet in oneline, allowing it to be compactly added (without customisation).

		AddSound = "SomeRte.rte/Path/To/YetAnotherSound.wav" // This adds a sound to the SoundContainer, creating a new SoundSet for it with just this sound.
	```
	NOTE: Here is a link to [FMOD's Inverse Rolloff Model.](https://fmod.com/resources/documentation-api?version=2.0&page=white-papers-3d-sounds.html#inverse)

- `SoundContainer` Lua controls have been overhauled, allowing for more control in playing and replaying them. The following Lua bindings are available:
	```
	soundContainer:HasAnySounds() - Returns whether or not the SoundContainer has any sounds in it. Returns True or false.
	```
	```
	soundContainer:IsBeingPlayed() - Returns whether or not any sounds in the SoundContainer are currently being played. Returns True or False.
	```
	```
	soundContainer:Play(optionalPosition, optionalPlayer) - Plays the sounds belonging to the SoundContainer's currently selected SoundSet. The sound will play at the position and for the player specified, or at (0, 0) for all players if parameters aren't specified.
	```
	```
	soundContainer:Stop(optionalPlayer) - Stops any playing sounds belonging to the SoundContainer, optionally only stopping them for a specified player.
	```
	```
	soundContainer:AddSound(filePath, optional soundSetToAddSoundTo, optionalSoundOffset, optionalAttenuationStartDistance, optionalAbortGameIfSoundIsInvalid) - Adds the sound at the given filepath to the SoundContainer. If a SoundSet index is specified it'll add it to that SoundSet. If an offset or attenuation start distance are specified they'll be set, as mentioned in the INI section above. If set to abort for invalid sounds, the game will error out if it can't load the sound, otherwise it'll show a console error.
	```
	```
	soundContainer:SetPosition(position) - Sets the position at which the SoundContainer's sounds will play.
	```
	```
	soundContainer:SelectNextSoundSet() - Selects the next SoundSet to play when soundContainer:Play(...) is called, according to the INI defined CycleMode.
	```
	```
	soundContainer.Loops - Set or get the number of loops for the SoundContainer, as mentioned in the INI section above.
	```
	```
	soundContainer.Priority - Set or get the priority of the SoundContainer, as mentioned in the INI section above.
	```
	```
	soundContainer.AffectedByGlobalPitch - Set or get whether the SoundContainer is affected by global pitch, as mentioned in the INI section above.
	```
- `MovableObjects` can now run multiple scripts by putting multiple `AddScript = FilePath.lua` lines in the INI definition. ([Issue #109](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/pull/109))  
	Scripts will have their appropriate functions run in the order they were added. Note that all scripts share the same `self`, so care must be taken when naming self variables.  
	Scripts can be checked for with `movableObject:HasScript(filePath);` and added and removed with `movableObject:AddScript(filePath);` and `movableObject:RemoveScript(filePath);`. They can also be enabled and disabled in Lua (preserving their ordering) with `movableObject:EnableScript(filePath);` and `movableObject:DisableScript(filePath);`.

- Scripts on `MovableObjects` and anything that extends them (i.e. most things) now support the following new functions (in addition to `Create`, `Update`, `Destroy` and `OnPieMenu`). They are added in the same way as the aforementioned scripts:  
	```
	OnScriptRemoveOrDisable(self, scriptWasRemoved) - This is run when the script is removed or disabled. The scriptWasRemoved parameter will be True if the script was removed and False if it was disabled.
	```
	```
	OnScriptEnable(self) - This is run when the script was disabled and has been enabled.
	```
	```
	OnCollideWithTerrain(self, terrainMaterial) - This is run when the MovableObject this script on is in contact with terrain. The terrainMaterial parameter gives you the material ID for the terrain collided with. It is suggested to disable this script when not needed to save on overhead, as it will be run a lot!
	```
	```
	OnCollideWithMO(self, collidedMO, collidedRootMO) - This is run when the MovableObject this script is on is in contact with another MovableObject. The collidedMO parameter gives you the MovableObject that was collided with, and the collidedRootMO parameter gives you the root MovableObject of that MovableObject (note that they may be the same). Collisions with MovableObjects that share the same root MovableObject will not call this function.
	```

- Scripts on `Attachables` now support the following new functions:  
	```
	OnAttach(self, newParent) - This is run when the Attachable this script is on is attached to a new parent object. The newParent parameter gives you the object the Attachable is now attached to.
	```
	```
	OnDetach(self, exParent) - This is run when the Attachable this script is on is detached from an object. The exParent gives you the object the Attachable was attached to.
	```

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

- `PieMenuActor` and `OrbittedCraft` have now been removed. They are instead replaced with parameters in their respective functions, i.e. `OnPieMenu(pieMenuActor);` and `CraftEnteredOrbit(orbittedCraft);`. Their use is otherwise unchanged.

### Fixed

- Fixed LuaBind being all sorts of messed up. All lua bindings now work properly like they were before updating to the v141 toolset.

- Explosives (and other thrown devices) will no longer reset their explosion triggering timer when they're picked up. ([Issue #71](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/71))

- Sprite Animation Mode `ALWAYSPINGPONG` now works properly. Sprite animation has also been moved to `MOSprite` instead of `MOSRotating`, they they'll be able to properly animate now. ([Issue#77](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/77))

- Fixed `BG Arm` flailing when reloading one-handed weapon, so shields are no longer so useless.

- Fixed crash when clearing an already empty preset list in the buy menu.

- Temporary fix for low mass attachables/emitters being thrown at ridiculous speeds when their parent is gibbed.

- The audio system now better supports splitscreen games, turning off sound panning for them and attenuating according to the nearest player.

- The audio system now better supports wrapping maps so sounds handle the seam better. Additionally, the game should be able to function if the audio system fails to start up.

- Scripts on attached attachables will only run if their parent exists in MovableMan. ([Issue #83](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/83))

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

## [0.1.0 pre-release 1][0.1.0-pre1] - 2020/01/27

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
[0.1.0-pre1]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data/releases/tag/v0.1.0-pre1
[0.1.0-pre2]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data/releases/tag/v0.1.0-pre2
