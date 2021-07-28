# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- Executable can be compiled as 64bit.

- New `Settings.ini` property `MeasureModuleLoadTime = 0/1` to measure the duration of module loading (archived module extraction included). For benchmarking purpuses.

- `Color` object's RGB values can now be set with index number.  
	```
	Color/TrailColor = Color
		Index = 0-255 // Corresponds with index in palette.bmp
	```

- New `Settings.ini` property `ForceDedicatedFullScreenGfxDriver` to force the game to run in previously removed dedicated fullscreen mode, allowing using lower resolutions (and 1366x768) while still maintaining fullscreen.

- New INI and Lua (R/W) property for Attachables:  
	`ParentBreakWound = AEmitter...`. Use this to define a `BreakWound` that will be applied to the `Attachable`'s parent when the `Attachable` is removed.  
	`BreakWound` is also now R/W accessible to Lua.

- Added Lua (R/W) properties for all hardcoded `Attachables`. You can now set them on the fly to be created objects of the relevant type. Note that trying to set things inappropriately (e.g. setting an `HDFirearm` as something's `Leg`) will probably crash the game; that's your problem to deal with.  
	You can read and write the following properties:  
	**`AHuman`** - `Head`, `Jetpack`, `FGArm`, `BGArm`, `FGLeg`, `BGLeg`, `FGFoot`, `BGFoot`  
	**`ACrab`** - `Turret`, `Jetpack`, `LeftFGLeg`, `LeftBGLeg`, `RightFGLeg`, `RightBGLeg`  
	**`ACDropShip`** - `RightEngine`, `LeftEngine`, `RightThruster`, `LeftThruster`, `RightHatch`, `LeftHatch`  
	**`ACRocket`** - `RightLeg`, `LeftLeg`, `MainEngine`, `LeftEngine`, `RightEngine`, `LeftThruster`, `RightThruster`  
	**`ADoor`** - `Door`  
	**`Turret`** - `MountedDevice`  
	**`Leg`** - `Foot`  
	**`HDFirearm`** - `Magazine`, `Flash`  
	**`AEmitter`** - `Flash`

- Added `Vector:ClampMagnitude(upperLimit, lowerLimit)` Lua function that lets you limit a Vector's upper and lower magnitude.

- Added `MOSRotating` `GibBlastStrength` INI and Lua (R/W) property. This lets you define how much force created `Gibs` and any `Attachables` will get launched when the `MOSRotating` gibs.

- New INI and Lua (R/W) properties for `Attachables`:  
	`ParentBreakWound = AEmitter...` allows you to optionally define different `BreakWounds` for the `Attachable` and its parent. By default it matches `BreakWound` for ease of use.  	
	`InheritsHFlipped = -1/0/1` allows you to define whether the `Attachable` will inherit its parent's HFlipped value or not.  
	-1 means reversed inheritance (i.e. if the parent's HFlipped value is true, this `Attachable`'s HFlipped value will be false), 0 means no inheritance, 1 means normal inheritance. Defaults to 1 to preserve normal behavior.  
	`InheritedRotAngleRadOffset = angle` and `InheritedRotAngleDegOffset = angle` allow you specify an offset to keep an `Attachable`'s rotation at when `InheritsRotAngle` is set to true.  
	In Lua there's only `InheritedRotAngleOffset` which takes/returns radians, to avoid confusion. For example, `InheritedRotAngleDegOffset = 90` would make the `Attachable` always face perpendicular to its parent.  
	Does nothing if the `Attachable`'s `InheritsRotAngle` is set to false or the `Attachable` has no parent.  
	`GibWithParentChance = 0 - 1` allows you to specify whether this `Attachable` should be gibbed when its parent does and what the chance of that happening is. 0 means never, 1 means always.  
	`ParentGibBlastStrengthMultiplier = number` allows you to specify a multiplier for how strongly this `Attachable` will apply its parent's gib blast strength to itself when the parent gibs. Usually this would be a positive number, but it doesn't have to be.

- New INI and Lua (R/W) `Arm` property `GripStrength`. This effectively replaces the `JointStrength` of the held `HeldDevice`, allowing `Arms` to control how tightly equipment is held.

- New INI and Lua (R/W) `HeldDevice` property `GripStrengthMultiplier`. This allows `HeldDevices` to multiply the `GripStrength` of their `Arms` to support them being more or less easy to hold.

- New Lua `MovableObject` function `GetWhichMOToNotHit`. This provides access to the MO that has been set to not be hit by `SetWhichMOToNotHit`.

- Added `HeldDevice` handling to limit which `Actor(s)` can pick it up. Note that pickup limitations are all done by PresetName, so you can not use this to precisely specify individual `Actors`.  
	The INI definition looks like this:  
	```
	PickupableBy = PickupableByEntries
		AddPresetNameEntry = First Actor PresetName Here
		AddPresetNameEntry = Second Actor PresetName Here
	// Alternatively, if you want this not to be pickupable
	PickupableBy = None
	```
	The Lua properties and functions are as follows:  
	```
	heldDevice.HasPickupLimitations; --(R) Whether or not this HeldDevice has any limitations affecting whether it can be picked up.
	heldDevice.UnPickupable --(R/W) Whether this HeldDevice is/should be pickupable.
	heldDevice:IsPickupableBy(actor) -- Whether or not a given Actor can pick up this HeldDevice.
	heldDevice:AddPickupableByPresetName(presetName) -- Allows Actors with the given PresetName to pick up this HeldDevice.
	heldDevice:RemovePickupableByPresetName(presetName) -- Disallows Actors with the given PresetNames from picking up this HeldDevice (as long as there are other pickup limitations).
	```
	
- Added `MOSRotating` Lua (R) property `IndividualMass`. This provides access to the `MOSRotating`'s actual mass value, not including any `Attachables` or inventory items. Note that the normal `Mass` property is still used to set the `MOSRotating`'s mass.

- Added `Actor` Lua (R) property `InventoryMass`. This provides access to the mass of the `Actor`'s inventory separate from the `Actor`'s actual mass.

- Added `LimbPath` INI property `EndSegCount`, which allows you to specify a segment after which the owning `Actor`'s foot will not collide with terrain. This lets you add extra visual-only frames to your `LimbPaths`.

- Added `AHuman` INI property `CrouchLimbPathBG` to allow you to specify a different `LimbPath` for the background leg while crouching.

- Added `AHuman` INI properties `StandRotAngleTarget`, `WalkRotAngleTarget`, `CrouchRotAngleTarget` and `JumpRotAngleTarget` that let you define the rot angle the body should aim towards when in the corresponding `MovementState`.

- Added `AHuman` Lua methods `GetRotAngleTarget(movementState)` and `SetRotAngleTarget(movementState, newRotAngleTarget)` that allow you to get and set rot angle targets for `MovementStates`. Note that only the `MovementStates` mentioned above will actually work.

- `LimbPaths` are now Lua accessible for `ACrabs` and `AHumans`. You can use `GetLimbPath(Layer, MovementState)` for `AHumans` and `GetLimbPath(Side, Layer, MovementState)` for `ACrabs`.  
	`LimbPaths` have the following properties:  
	`limbPath.StartOffset` (R/W) - the start offset for the `LimbPath`. Also defines its position if it has no actual path.  
	`limbPath.SegmentCount` (R) - the number of segments in the `LimbPath`.  
	`limbPath:GetSegment(segmentIndex)` - Gets the segment Vector for the given segment index. You can use this to modify `LimbPaths`.  

- Added `OnStride` special Lua function for `AHumans` that is called whenever they stride (i.e. when their `StrideSound` is played). Like playing `StrideSound`, this does not happen when the AHuman is climbing.

- New `AHuman` and `ACrab` INI and Lua (R/W) property `JetAngleRange` which defines the rate at which the angle of the `Jetpack`'s thrust follows the aim angle of the actor (default being 0.25). 

- New `AHuman` INI property `LookToAimRatio` at which the `Head` turns in the direction of aiming (default being 0.7).

- New `AHuman` INI properties `FGArmFlailScalar` and `BGArmFlailScalar`. Used to change the rate at which each `Arm` follows the rotation angle of the `Actor`, regardless of aim angle. 0 means the `Arm` will always point in aiming direction.

- New `Actor` INI and Lua (R/W) property `CanRevealUnseen` which can be used to disable the ability to reveal unseen areas.

- New `MOPixel` Lua (R/W) property `TrailLength` which returns the trail length of the `Atom` affiliated with this `MOPixel`.

- New `HDFirearm` INI property `ShellEjectAngle` which lets you define the angle at which `Shell` particles are ejected relative to the `HDFirearm`'s rotation.

- New `Gib` INI property `IgnoresTeamHits`.

- New `Atom` INI property `TrailLengthVariation`. Used to randomize `TrailLength` on every frame. 0 means no randomization (default), 1 means anything between full length and zero.

- New `ACraft` INI and Lua (R/W) property `HatchCloseSound`. This is now required separately to `HatchOpenSound`.

- Exposed `MOSRotating` property `OrientToVel` to Lua (R/W).

- Exposed `DataModule` properties `Author`, `Description` and `Version` to Lua (R).

- Exposed `Actor` properties `HolsterOffset` and `ItemInReach` to Lua (R/W).

- Exposed `Arm` property `MaxLength` to Lua (R).

- Exposed broad range of sounds to Lua (R/W) through their relevant SoundContainers. For each class, these include:  
	**Actor**: `BodyHitSound`, `PainSound`, `DeathSound`, `DeviceSwitchSound`, `AlarmSound`  
	**AHuman & ACrab**: `StrideSound`  
	**HDFirearm**: `FireSound`, `FireEchoSound`, `EmptySound`, `ReloadStartSound`, `ReloadEndSound`, `ActiveSound`, `DeactivationSound`, `PreFireSound`  
	**AEmitter**: `EmissionSound`, `BurstSound`, `EndSound`  
	**ACraft**: `HatchOpenSound`, `HatchCloseSound`, `CrashSound`  
	**MOSRotating**: `GibSound`  
	**ADoor**: `DoorMoveStartSound`, `DoorMoveSound`, `DoorDirectionChangeSound`, `DoorMoveEndSound`
  
- Added Lua function `RoundFloatToPrecision`. Utility function to round and format floating point numbers for display in strings.  
`RoundFloatToPrecision(floatValue, digitsPastDecimal, roundingMode) -- Rounding mode 0 for system default, 1 for floored remainder, 2 for ceiled remainder.`

- The Lua console (and all text boxes) now support using `Ctrl` to move the cursor around and select or delete text.

- Added `mosRotating:RemoveAttachable(attachableOrUniqueID, addToMovableMan, addBreakWounds)` method that allows you to remove an `Attachable` and specify whether it should be added to `MovableMan` or not, and whether breakwounds should be added (if defined) to the `Attachable` and parent `MOSRotating`. This method returns the removed `Attachable`, see the `Changed` section for important details on that.

- Added `mosRotating:RemoveEmitter(attachableOrUniqueID, addToMovableMan, addBreakWounds)` method that is identical to the `RemoveAttachable` function mentioned above.  

- Added `attachable:RemoveFromParent()` and `attachable:RemoveFromParent(addToMovableMan, addBreakWounds)` that allow you to remove `Attachables` from their parents without having to use `GetParent` first. Their return value is the same as `RemoveAttachable` above.

- Added `Settings.ini` debug properties to allow modders to turn on some potentially useful information visualizations.  
	`DrawAtomGroupVisualizations` - any `MOSRotating` will draw its `AtomGroup` to the standard view.  
	`DrawHandAndFootGroupVisualizations` - any `Actor` subclasses with  will draw its `AtomGroup` to the standard view.  
	`DrawLimbPathVisualizations` - any  `AHumans` or `ACrabs` will draw some of their `LimbPaths` to the standard view.  
	`DrawRayCastVisualizations` - any rays cast by `SceneMan` will be drawn to the standard view.  
	`DrawPixelCheckVisualizations ` - any pixel checks made by `SceneMan:GetTerrMatter` or `SceneMan:GetMOIDPixel` will be drawn to the standard view.

- Added a fully featured inventory view for managing `AHuman` inventories (to be expanded to other things in future).

- New `Settings.ini` property `CaseSensitiveFilePaths = 0/1` to enable/disable file path case sensitivity in INIs. Enabled by default.  
	It is **STRONGLY** ill-advised to disable this behavior as it makes case sensitivity mismatches immediately obvious and allows fixing them with ease to ensure a path related crash free cross-platform experience.  
	Only disable this if for some reason case sensitivity increases the loading times on your system (which it generally should not). Loading times can be benchmarked using the `Settings.ini` property `MeasureModuleLoadTime`. The result will be printed to the console.

- Added `MovableObject` Lua function `EnableOrDisableAllScripts` that allows you to enable or disable all scripts on a `MovableObject` based on the passed in value.

- Added `Attachable` Lua function and INI property `InheritsFrame` which lets `Attachables` inherit their parent's frame. It is set to false by default.

- Added `MovableObject` Lua (R/W) and INI properties `ApplyWoundDamageOnCollision` and `ApplyWoundBurstDamageOnCollision` which allow `MovableObject`s to apply the `EntryWound` damage/burst damage that would occur when they penetrate another object, without actually creating a wound.

- `Turret`s can now support an unlimited number of mounted `HeldDevice`s. Properties have been added to Lua and INI to support this:  
	`AddMountedDevice = ...` (INI) and `turret:AddMountedDevice` (Lua) - this adds the specified `HeldDevice` or `HDFirearm` as a mounted device on the `Turret`.  
	`turret:GetMountedDevices` (Lua) - this gives you access to all the mounted `HeldDevice`s on the `Turret`. You can loop through them with a for loop, and remove or modify them as needed.  
	Note that `MountedDevice = ...` (INI) and `turret.MountedDevice` (Lua R/W) now deal with the first mounted `HeldDevice`, which is treated as the primary one for things like sharp-aiming.
	
- Added `Turret` Lua (R/W) and INI property `MountedDeviceRotationOffset` that lets you specify a standard rotation offset for all mounted `HeldDevices` on a turret.

- Added option for players to vote to restart multiplayer activities by holding the backslash key, `\`. Requires all players to vote to pass.  
	This is an alternative to the existing ability to vote to end the activity and return to the multiplayer lobby, by holding `Backspace` key.

- New `Settings.ini` properties `MuteMaster = 0/1`, `MuteMusic = 0/1` and `MuteSound = 0/1` to control muting of master/music/sound channels without changing the volume property values.	

- New `Settings.ini` property `TwoPlayerSplitscreenVertSplit = 0/1` to force two player splitscreen into a vertical split mode (horizontal by default).

- Controller hot-plugging is now supported (Windows only).

- Console text can be set to use a monospace font through `Settings.ini` property `ConsoleUseMonospaceFont = 0/1` or through the in-game settings.

- New `ThrownDevice` INI property `StrikerLever`, which is the same as `Shell` for `Round` in `HDFirearm`, but for grenades. Represents the lever/pin coming off when activated.

- New `Arm` INI and Lua (R/W) property `ThrowStrength` which now calculates how far `ThrownDevice`s are thrown, which also takes to account the weight of the device. `ThrownDevice`s can still be defined `MaxThrowVel` and `MinThrowVel` to override this.

### Changed

- `TDExplosive`s will no longer default to a looping animation when activated. Instead, they change to the second frame (i.e 001), similarly to `HDFirearm`. Set `SpriteAnimMode` to `4` if you wish to enable the looping active animation.

- `AHuman` can now manually reload BG devices.

- Jetpack thrust angle is now properly clamped when controlled with an analog stick.

- Aim reticle dots can now be hidden per device by setting `SharpLength` to 0.

- Craft will now automatically scuttle when opening doors at a 90° angle rather than 45°.

- `AHuman` can now aim while walking, however not while reloading.

- Recoil when firing weapons now affects sharp aim.

- The third argument for `distance` to be filled out in `MovableMan:GetClosestActor()` is now a `Vector` rather than `float`.

- File paths in INIs are now case sensitive.

- Hands will now draw in transparent drawing mode, i.e. editing menu.

- `AHuman` background `Leg` will no longer draw in front of the `AHuman`. The real result of this is that the background foot will no longer draw in front of the foreground one.

- Everything draws better when flashing white, including craft which used to be terrible at it.

- Reworked Attachable managment:  
	`DamageMultiplier` on `Attachables` now works as expected, all `Attachables` can now transfer damage to their root parent.  
	This will travel up chains of `Attachables`, as long as every `Attachable` in the chain has a non-zero DamageMultiplier (yes, negative numbers are supported if you wanna have healing instead of damage or weirdness with chaining negative multipliers). Note that the default `DamageMultiplier` for `Attachables` is 0, so you have to set it if you want it. Also note that wounds will default this value to 1 instead of 0.  
	`Attachable` terrain collision has been reworked so that it can be changed simply by setting `CollidesWithTerrainWhileAttached = true/false` in INI or Lua. Also, `Attachables` attached to other `Attachables` will now collide with terrain properly.  
	`BreakWounds` on `Attachables` now gets added to both the `Attachable` and the parent when the `Attachable` is broken off. If `ParentBreakWound` is defined, the parent will use this instead of the regular `BreakWound`.
	
- `Attachable.BreakWound` now has R/W access in Lua.

- `Attachable.DeleteWithParent` is now `Attachable.DeleteWhenRemovedFromParent`, since this more accurately describes what it does.

- `Attachable.OnlyLinearForces` has been renamed to `Attachable.ApplyTransferredForcesAtOffset` and its effect has been reversed, so something that checked `OnlyLinearForces == true` would now check `ApplyTransferredForcesAtOffset == false`, since this makes more sense to use.

- `Arms` and `Legs` on `AHumans` will no longer bleed out indefinitely. If you want this to happen, adjust their `BreakWound` or `ParentBreakWound` accordingly.
		
- Reworked wound management:  
	Wound management is now always done with `MOSRotating` functions, instead of requiring different ones for `Actors`. This means TotalWoundCount and RemoveAnyRandomWounds no longer exist.  
	You can get all wounds with `GetWounds`, get the wound count with `GetWoundCount` (or using the pre-existing WoundCount property), get the gib wound limit with `GetGibWoundLimit` (or using the pre-existing GibWoundLimit property), and remove wounds with `RemoveWounds`.  
	All of these functions have two variants, one lets you just specify any normal arguments (e.g. number of wounds to remove), the other lets you also specify whether you want to include `Attachables` with a positive `DamageMultiplier` (i.e. `Attachables` that damage their parent), `Attachables` with a negative `DamageMultiplier` (i.e. `Attachables` that heal their parent) or `Attachables` with no `DamageMultiplier` (i.e. `Attachables` that don't affect their parent).  
	Without any arguments, `GetWoundCount` and `RemoveWounds` will only include `Attachables` with a positive `DamageMultiplier` in their counting calculations, and `GetGibWoundLimit` will not include any `Attachables` in its counting calculations. The property variants (e.g. `mosr.WoundCount`) behave the same way as the no-argument versions.  
	Note that this process is recursive, so if an `Attachable` that satisfies the conditions has `Attachables` that also satisfy the conditions, their wounds will be included in the results.

- Renamed `Turret` INI property `MountedMO` to `MountedDevice` to better match the new reality that `Turrets` can only mount `HeldDevices` and their child classes.

- Renamed `ACrab` `LFGLeg`, `LBGLeg`, `RFGLeg` and `RBGLeg` Lua properties to `LeftFGLeg`, `LeftBGLeg`, `RightFGLeg`, `RightBGLeg` respectively, to be more consistent with other naming.  
	For the time being, the INI properties (as well as the ones for setting `FootGroups` and `LimbPaths`) support both single letter and written out versions (i.e. `LStandLimbPath` and `LeftStandLimbPath` are both supported). This single letter versions will probably be deprecated over time.

- To better align with the other changes, hardcoded `Attachable` INI definitions for `ACDropShips` and `ACRockets` can now support spelled out words. The following options are all supported in INI:  
	**`ACDropShip`** - `RThruster`/`RightThruster`/`RightEngine`, `LThruster`/`LeftThruster`/`LeftEngine`, `URThruster`/`UpRightThruster`, `ULThruster`/`UpLeftThruster`, `RHatchDoor`/`RightHatchDoor`, `LHatchDoor`/`LeftHatchDoor`  
	**`ACRocket`** - `RLeg`/`RightLeg`, `LLeg`/`LeftLeg`, `RFootGroup`/`RightFootGroup`, `LFootGroup`/`LeftFootGroup`, `MThruster`/`MainThruster`, `RThruster`/`RightThruster`, `LThruster`/`LeftThruster`, `URThruster`/`UpRightThruster`, `ULThruster`/`UpLeftThruster`

- `MovableMan:AddMO` will now add `HeldDevices` (or any child class of `HeldDevice`) to its `Items` collection, making it able to provide the functionality of `AddParticle`, `AddActor` and `AddItem`.

- Changed and cleaned up how gibbing works and how it affects `Attachables`. In particular, limbs will better inherit velocity during gibbing and things are more customizable. See `Attachable` properties for more details.  
	As an added bonus,  `Attachables` on `ACDropShips` and `ACRockets` can now be shot down when the craft gibs; fight back against the baleful dropship engines!
	
- Improved native recoil handling! Guns transfer recoil to arms/turrets, which transfer it to AHumans/ACrabs, all of it properly accounts for joint strengths (or grip strengths) and offsets at every step. ([Issue #7](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/7) and [Issue #8](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/8)).

- `Attachables` now use their `GibImpulseLimit` as well as their `JointStrength` when determining whether they should be detached by strong forces. To maintain backwards compatibility, if the `GibImpulseLimit` is less than the `JointStrength`, the `JointStrength` will be used instead for this purpose.

- The `FacingAngle` function has been moved from `Actor` to `MOSprite` so it can be used more widely.

- Lifetime and ToDelete now work on wounds, giving modders more control over them.

- Some functionality has been moved from `AudioMan` to `SoundContainer` for consistency. As such, the following `AudioMan` Lua bindings have been replaced:  
	`AudioMan:FadeOutSound(fadeOutTime);` has been replaced with `soundContainer:FadeOut(fadeOutTime);`  
	`AudioMan:StopSound(soundContainer);` and `AudioMan:StopSound(soundContainer, player);` have been replaced with `soundContainer:Stop();` and `soundContainer:Stop(player);`
	
- Pressing escape when a buy menu is open now closes it instead of pausing the game.

- `GetParent` will now return an `MOSRotating` instead of a `MovableObject` so it doesn't need to be casted. Additionally, it will always return null for objects with no parents, instead of returning the self object for things that weren't `Attachables`.  
	This makes things more consistent and reasonable throughout and will rarely, if ever, cause Lua problems.

- Previews generated by the `SceneEditor` are now the same as `ScenePreviewDumps`, also both are now saved as PNGs.

- `Attachable` terrain collisions will now propagate to any child `Attachables` on them. This means that `Attachables` will not collide with terrain, even if set to, if they're attached to a parent that doesn't collide with terrain.  
	This means that the `attachable.CollidesWithTerrainWhileAttached` value may not represent the true state of things, you should instead use `attachable.CanCollideWithTerrain` to determine whether a given `Attachable` can collide with terrain.
	
- Actor selection keys can be used to cycle the selected `ObjectPicker` item while it's closed during building phase and in editors.

- The `Actor` property `MaxMass` now no longer includes the `Mass` of the `Actor`, and has been renamed to `MaxInventoryMass` for clarity. In mods, this is most important for `ACraft`, which will now need their total `Mass` subtracted from the old value. 

- `BuyMenu` tooltips now display item info as well as a description. This includes `MaxInventoryMass` and `MaxPassengers` for `ACraft`, `Mass` and `PassengerSlots` required for `Actors`, and `Mass` for other `MoveableObjects`.

- Replaced the above-HUD pie menu inventory view with an animating inventory carousel.

- `AHuman:ReloadFirearm` Lua function has been changed to `AHuman:ReloadFirearms` and will now reload offhand weapons as well, if appropriate.

- `ACrab:ReloadFirearm` Lua function has been changed to `ACrab:ReloadFirearms` and will now reload all of the `ACrab`'s weapons.

- When using the Settings.ini flag `LaunchIntoActivity`, you will start with some default amount of gold; either the `Activity`'s medium difficulty gold amount, or its funds for Team 1, which default to 2000 when not set.

- `AEmitter`s will now obey `SpriteAnimMode` set in Lua while they're emitting. When they stop emitting this will reset to `NOANIM`.

- `Attachable` Lua method `IsDrawnAfterParent` has been changed to the property `DrawnAfterParent`, and is now R/W.

- All `mosRotating:RemoveAttachable`, `mosRotating:RemoveEmitter` and `attachable:RemoveFromParent` functions will return the removed `Attachable` if it hasn't been added to `MovableMan`, or nil if it has. If the `Attachable` is returned, it will belong to Lua like it would if it were newly Created. You could then, for example, add it to MovableMan or to an inventory.

- `Settings.ini` property `MenuTransitionDuration` renamed to `MenuTransitionDurationMultiplier`.

- `Settings.ini` property `DisableLoadingScreen` renamed to `DisableLoadingScreenProgressReport`.

- Scenario scene markers are now color coded to help distinguish them visually:  
	`Base.rte` scenes are yellow as always.  
	`Missions.rte` scenes are now green.  
	`Scenes.rte` or any other mod/user scenes are now cyan.

- Main menu and sub-menus were given a facelift.

- Settings menu was reworked to make it less useless.

- Esc has been disabled in server mode to not disrupt simulation for clients, use Alt+F4 or the window close button to exit.

### Fixed

- `HFlipped` is now properly assigned to emissions, gibs and particles that are shot from a `HDFirearm`'s `Round` when the source object is also flipped.

- `MovableObject:SetWhichMOToNotHit` will now work properly for Attachables. They will also not hit the relevant MO. When they're removed, Attachables will check if they have the same MO for this value and, if so, unset it so they can hit that MO.

- Craft sucking up objects now works properly again.

- Getting the `Mass` of a `MOSRotating` has now been made more efficient. Additionally, `Attachables` of `Attachables` will now be included in Mass, so some things have gotten a lot heavier (e.g. Dummy Dreadnought).

- The moment of inertia of `AtomGroups` now updates when the mass or Atoms change, meaning losing `Attachables` or changing mass will properly affect how rotational forces apply to MOSRotatings.

- `WoundDamageMultipliers` on projectiles will now properly stack with wounds' `DamageMultiplier`. Prior to this, if you set the `DamageMultiplier` of a wound on some object, it'd be overwritten by the hitting projectile's `WoundDamageMultiplier`. Now they multiply together properly.

- `Radius` and `Diameter` now account for `Attachables` on objects that can have them. If you want just the `Radius` or `Diameter` of the object, use `IndividualRadius` and `IndividualDiameter` (only available for `MOSRotating` and subclasses). This means that `Radius` and `Diameter` will now give you a good estimation of an object's total size.

- Fixed various audio bugs that were in Pre3, and fixed clicking noise on sounds that played far away. The game should sound way better now!

- Mobile sounds (i.e. generally things that aren't GUI related) will now pause and resume when you pause and resume your activity.

- The `DeactivationSound` of `HDFirearms` now respects its `SoundOverlapMode` instead of never allowing overlap. If you don't want it overlapping, set it up accordingly.

- Enabled DPI Awareness to fix issues with resolution settings when Windows scaling is enabled.

- Fixed a bug that caused the game to crash when the crab bomb effect was triggered while there were multiple crab bomb eligible Craft in an activity.

- Renamed `Attachable` INI property `CollidesWithTerrainWhenAttached` to more correct, consistent `CollidesWithTerrainWhileAttached`.

- You can now modify `Foot`, `Magazine` and `Flash` (both `HDFirearm` and `AEmitter`) `CopyOfs` in your `Leg`/`HDFirearm``AEmitter` definition without setting a new `PresetName`.

- `OnCollideWithMO` now works for `MOPixels` and `MOSParticles` so you can use it to check if your bullets collide with things.

- `OnCollideWithMO` and `OnCollideWithTerrain` (and other special functions) will run more reliably right after the object is spawned. E.g. `OnCollideWithTerrain` should now work even if your gun is jammed into terrain when you shoot.

- You can now sharpaim through friendlies when playing as any team, instead of just as red team.

- The reload hotkey now works even if there's an object to pick up.

- Improved LZ behaviour on wrapping maps, so your buy cursor will no longer annoyingly wrap around the LZ area.

- Fixed a bug with metagame saves that caused Player numbers to be off by 1.

- Vote counts to end a multiplayer activity now display as intended. 

- Fixed bug where choosing `-Random-` as a player's tech and pressing start game had a 1 in (number of techs + 1) chance to crash the game.

- Console error spam will no longer cripple performance over time.

### Removed

- Removed obsolete graphics drivers and their `Settings.ini` properties `ForceOverlayedWindowGfxDriver` and `ForceNonOverlayedWindowGfxDriver`.

- Removed `Attachable` Lua write capability for `AtomSubGroupID` as changing this can cause all kinds of problems, and `RotTarget` as this didn't actually work.

- Removed `MaxLength` property from `Leg`, since it was a mostly unused leftover caused by Leg being originally copied from Arm, and was actually a fake setting that just set other properties. To replace it, set the following:  
	```
	ContractedOffset = Vector
		X = //Old MaxLength/2
		Y = 0
	ExtendedOffset = Vector
		X = //Old MaxLength
		Y = 0
	```

- Removed `Attachable.RotTarget` from Lua and INI. The property never worked and no longer exists.

- Removed `Attachable:CollectDamage`, `Attachable:TransferJointForces` and `Attachable:TransferJointImpulses` Lua function definitions. These are internal functions that should never have been exposed to Lua.

- Removed `MOSRotating:ApplyForces` and `MOSRotating:ApplyImpulses` Lua functions. These are both internal functions that should never have been exposed to Lua.

- Removed hardcoded INI constraint that forced `Mass` of `MovableObjects` to not be 0. Previously, anytime a `Mass` of 0 was read in from INI, it was changed to 0.0001.

- Removed the ability to set `HDFirearms'` `Magazine` or `Flash`, or `AEmitters'` `Flash` to None in INI. This was a necessary result of some core changes, and may be undone in future if it's possible. If you want no `Magazine` or `Flash` just don't set one, or use a Null one like is done for limbs and other hardcoded `Attachables`.

- Removed the quit-confirmation dialog from the scenarios screen. Now pressing escape will lead back to the main menu.

- Removed `Settings.ini` properties `HSplitScreen` and `VSplitScreen`. Superseded by `TwoPlayerSplitscreenVertSplit`.

***

## [0.1.0 pre-release 3.0][0.1.0-pre3.0] - 2020/12/25

### Added

- Implemented Lua Just-In-Time compilation (MoonJIT 2.2.0).

- Implemented PNG file loading and saving. PNGs still need to be indexed just like BMPs! Transparency (alpha) not supported (yet).

- New `Settings.ini` property `LoadingScreenReportPrecision = intValue` to control how accurately the module loading progress reports what line is currently being read.  
	Only relevant when `DisableLoadingScreen = 0`. Default value is 100, lower values increase loading times (especially if set to 1).  
	This should be used for debugging where you need to pinpoint the exact line that is crashing and the crash message isn't helping or doesn't exist at all.

- New `Settings.ini` property `MenuTransitionDuration = floatValue` to control how fast transitions between different menu screens happen (e.g main menu to activity selection screen and back).  
	This property is a multiplier, the default value is 1 (being the default hardcoded values), lower values decrease transition durations. 0 makes transitions instant.

- New `ADoor` sound properties: ([Issue #106](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/106))  
	```
	// Played when the door starts moving from fully open/closed position towards the opposite end.
	DoorMoveStartSound = SoundContainer
		AddSound = ContentFile
			FilePath = pathToFile

	// Played while the door is moving, between fully open/closed position.
	DoorMoveSound = SoundContainer
		AddSound = ContentFile
			FilePath = pathToFile
		LoopSetting = -1 // Doesn't have to loop indefinitely, but generally should.

	// Played when the door changes direction while moving between fully open/closed position.
	DoorDirectionChangeSound = SoundContainer
		AddSound = ContentFile
			FilePath = pathToFile

	// Played when the door stops moving and is at fully open/closed position.
	DoorMoveEndSound = SoundContainer
		AddSound = ContentFile
			FilePath = pathToFile
	```

- Exposed `Actor.StableVelocityThreshold` to lua. New bindings are: ([Issue #101](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/101))  
	`Actor:GetStableVelocityThreshold()` returns a `Vector` with the currently set stable velocity threshold.  
	`Actor:SetStableVelocityThreshold(xFloatValue, yFloatValue)` sets the stable velocity threshold to the passed in float values.  
	`Actor:SetStableVelocityThreshold(Vector)` sets the stable velocity threshold to the passed in `Vector`.

- New `Attachable` and `AEmitter` property `DeleteWithParent = 0/1`. If enabled the attachable/emitter will be deleted along with the parent if parent is deleted/gibbed/destroyed. ([Issue #97](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/97))

- New `Settings.ini` property `LaunchIntoActivity = 0/1`. With `PlayIntro` functionality changed to actually skip the intro and load into main menu, this flag exists to skip both the intro and main menu and load directly into the set default activity.

- Exposed `AHuman.ThrowPrepTime` to lua and ini: ([Issue #101](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/101))  
	`ThrowPrepTime = valueInMS` will set how long it takes the `AHuman` to fully charge a throw. Default value is 1000.  
	`AHuman.ThrowPrepTime` to get/set values via lua.

- Added new `SpriteAnimMode` modes:  
	```
	SpriteAnimMode = 7 // OVERLIFETIME
	```
	This mode handles exactly like (now removed) `MOSParticle.Framerate = 0` and will complete the sprite's animation cycle over the course of its existence. `SpriteAnimDuration` is inapplicable when using this mode and will do nothing.  
	For example, an object that has a sprite with 10 frames and a lifetime of 10 seconds will animate at a rate of 1 frame per second, finishing its animation cycle just before being deleted from the scene.  
	If this mode is used on an object that has `LifeTime = 0` (infinite) it will be overridden to `SpriteAnimMode = 1` (ALWAYSLOOP) otherwise it will never animate.  
	```
	SpriteAnimMode = 8 // ONCOLLIDE
	```
	This mode will drive the animation forward based on collisions this object has with other MOs or the terrain. `SpriteAnimDuration` is inapplicable when using this mode and will do nothing.  
	This mode is `MOSParticle` specific and used mainly for animating casings and small gibs. Using this mode on anything other than `MOSParticle` will do nothing.

- New `Settings.ini` properties `EnableCrabBombs = 0/1` and `CrabBombThreshold = intValue`.  
	When `EnableCrabBombs` is enabled, releasing a number of crabs equal to `CrabBombThreshold` or more at once will trigger the crab bomb effect.  
	If disabled releasing whatever number of crabs will do nothing except release whatever number of crabs.

- Doors can now be stopped at their exact position using `ADoor:StopDoor()` via lua. When stopped, doors will stop updating their sensors and will not try to reset to a default state.  
	If the door was stopped in a script, it needs to opened/closed by calling either `ADoor:OpenDoor()` or `ADoor:CloseDoor()` otherwise it will remain in the exact position it was stopped forever.  
	If either `DrawMaterialLayerWhenOpen` or `DrawMaterialLayerWhenClosed` properties are set true, a material layer will be drawn when the door is stopped. This is to prevent a situation where the material layer will be drawn only if the door is travelling in one direction, without adding an extra property.

- New value `STOPPED` (4) was to the `ADoor.DoorState` enumeration. `ADoor:GetDoorState` will return this if the door was stopped by the user via `ADoor:StopDoor`.

- New shortcut `ALT + W` to generate a detailed 140x55px mini `WorldDump` to be used for scene previews. No relying on `SceneEditor`, stretches over whole image, no ugly cyan bunkers, no actors or glows, has sky gradient, indexed to palette.

- All text in TextBox (any TextBox) can now be selected using `CTRL + A`.

- Console can now be resized using `CTRL + UP/DOWN` (arrow keys) while open.

- Added new lua function `UInputMan:GetInputDevice(playerNum)` to get a number value representing the input device used by the specified player. Should be useful for making custom key bindings compatible with different input devices.

- Scripts can now be attached to `ACrab.Turret` and `Leg`. Additionally, a binding to get the Foot of a Leg has been added.

- Added H/V flipping capabilities to Bitmap primitives.  New bindings with arguments for flip are:  
	`PrimitiveMan:DrawBitmapPrimitive(pos, entity, rotAngle, frame, bool hFlipped, bool vFlipped)`  
	`PrimitiveMan:DrawBitmapPrimitive(player, pos, entity, rotAngle, frame, bool hFlipped, bool vFlipped)`  
	Original bindings with no flip arguments are untouched and can be called as they were.

- Added new primitive drawing functions to `PrimitiveMan`:  
	```
	-- Arc
	PrimitiveMan:DrawArcPrimitive(Vector pos, startAngle, endAngle, radius, color)
	PrimitiveMan:DrawArcPrimitive(player, Vector pos, startAngle, endAngle, radius, color)

	PrimitiveMan:DrawArcPrimitive(Vector pos, startAngle, endAngle, radius, color, thickness)
	PrimitiveMan:DrawArcPrimitive(player, Vector pos, startAngle, endAngle, radius, color, thickness)

	-- Spline (Bézier Curve)
	PrimitiveMan:DrawSplinePrimitive(Vector start, Vector guideA, Vector guideB, Vector end, color)
	PrimitiveMan:DrawSplinePrimitive(player, Vector start, Vector guideA, Vector guideB, Vector end, color)

	-- Box with rounded corners
	PrimitiveMan:DrawRoundedBoxPrimitive(Vector upperLeftCorner, Vector bottomRightCorner, cornerRadius, color)
	PrimitiveMan:DrawRoundedBoxPrimitive(player, Vector upperLeftCorner, Vector bottomRightCorner, cornerRadius, color)

	PrimitiveMan:DrawRoundedBoxFillPrimitive(Vector upperLeftCorner, Vector bottomRightCorner, cornerRadius, color)
	PrimitiveMan:DrawRoundedBoxFillPrimitive(player, Vector upperLeftCorner, Vector bottomRightCorner, cornerRadius, color)

	-- Triangle
	PrimitiveMan:DrawTrianglePrimitive(Vector pointA, Vector pointB, Vector pointC, color)
	PrimitiveMan:DrawTrianglePrimitive(player, Vector pointA, Vector pointB, Vector pointC, color)

	PrimitiveMan:DrawTriangleFillPrimitive(Vector pointA, Vector pointB, Vector pointC, color)
	PrimitiveMan:DrawTriangleFillPrimitive(player, Vector pointA, Vector pointB, Vector pointC, color)

	-- Ellipse
	PrimitiveMan:DrawEllipsePrimitive(Vector pos, horizRadius, vertRadius, color)
	PrimitiveMan:DrawEllipsePrimitive(player, Vector pos, horizRadius, vertRadius, color)

	PrimitiveMan:DrawEllipseFillPrimitive(Vector pos, short horizRadius, short vertRadius, color)
	PrimitiveMan:DrawEllipseFillPrimitive(player, Vector pos, horizRadius, vertRadius, color)
	```

- Added log for non-fatal loading errors. This log will show image files that have been loaded with incorrect extensions (has no side effects but should be addressed) and audio files that failed loading entirely and will not be audible.  
	If errors are present the console will be forced open to notify the player (only when loading into main menu).  
	Log will be automatically deleted if warnings are no longer present to avoid polluting the root directory.

- Game window resolution can now be changed without restarting the game.

- GUI sliders (like for music volume) can now be adjusted with the mouse scroll wheel.

- Exposed `PEmitter` to lua. Bindings are identical to `AEmitter` bindings, except that damage-related bindings don't exist for `PEmitter`.

- `FLAC` audio files can now be loaded through lua and ini.

- Added new lua `Vector` functions: `GetRadRotated(angle)` and `GetDegRotated(angle)`. They return a rotated copy of the vector without modifying it.

- Added `Enum` binding for `SoundSet.SoundSelectionCycleMode`: `RANDOM = 0, FORWARDS = 1, ALL = 2`.

- Added `Enum` binding for `SoundContainer.SoundOverlapMode`: `OVERLAP = 0, RESTART = 1, IGNORE_PLAY = 2`.

- New `SoundContainer` function `Restart`, which allows you to restart a playing `SoundContainer`. Also another `Play` function, that lets you just specify the player to play the sound for.

- New `HDFirearm` INI property `PreFireSound`, which allows you to specify a sound that will play exactly once before the weapon fires.  
	Note that this was designed primarily for things like flamethrowers, semi-auto weapons may wonky with it, and full-auto weapons may fire multiple shots in the first burst, if you don't also set an `ActivationDelay`.	

- `SoundSets` have been made a bit more fully featured, they can now have sub `SoundSets` and their own `SoundSelectionCycleMode` and they now have a Lua binding so you can create them in Lua with `local mySoundSet = SoundSet()`.  
	They have the following INI and Lua properties:  	
	
	`SoundSelectionCycleMode` (INI and Lua R/W) - Determines how sounds in this `SoundSet` will be selected each time it is played (or when `SelectNextSounds` is called).  
	Note that sub `SoundSets` can have different `SoundSelectionCycleModes`. `SoundSets` with sub `SoundSets` and sounds whose `SoundSelectionCycleMode` is `FORWARDS` will first go through their sounds, then each sub `SoundSet`.  
	
	`soundSet.SubSoundSets` (Lua R) - An iterator over the sub `SoundSets` of this `SoundSet`, allowing you to manipulate them as you would any `SoundSet`.  
	`soundSet:HasAnySounds(includeSubSoundSets)` (Lua) - Whether or not this `SoundSet` has any sounds, optionally including its sub `SoundSets`.  
	`soundSet:SelectNextSounds()` (Lua) - Selects the next sounds for this `SoundSet`. Note that playing a `SoundContainer` will always also do this, so this is only really useful to allow you to skip sounds when `SoundSelectionCycleMode` is set to `FORWARDS`.  
	`soundSet:AddSound("Path/to/sound")` (Lua) - Adds the sound at the given path with no offset, 0 minimum audible distance, and default attenuation start distance.  
	`soundSet:AddSound("Path/to/sound", offset, minimumAudibleDistance, attenuationStartDistance)` (Lua) - Adds the sound at the given path with the given parameters.  
	`soundSet:AddSoundSet(soundSetToAdd)` (Lua) - Adds the given `SoundSet` as a sub `SoundSet` of this `SoundSet`.  
	
	Additionally, `AddSound` and `AddSoundSet` INI properties work for `SoundSets`. They are exactly the same as they are for `SoundContainers`.

- You can get the top level `SoundSet` of a `SoundContainer` with `soundContainer:GetTopLevelSoundSet` and manipulate it as described above. This allows you full interaction with all levels of `SoundSets` in a `SoundContainer`.

### Changed

- Codebase now uses the C++17 standard.

- Updated game framework from Allegro 4.2.3.1 to Allegro 4.4.3.1.

- Major cleanup and reformatting in the `Managers` folder.

- Lua error reporting has been improved so script errors will always show filename and line number.

- Ini error reporting has been improved so asset loading crash messages (image and audio files) will also display the ini file and line they are being referenced from and a better explanation why the crash occured. ([Issue #161](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/161))

- `Settings.ini` will now fully populate with all available settings (now also broken into sections) when being created (first time or after delete) rather than with just a limited set of defaults.

- Temporarily removed `PreciseCollisions` from `Settings.ini` due to bad things happening when disabled by user.

- `Settings.ini` property `PlayIntro` renamed to `SkipIntro` and functionality changed to actually skip the intro and load user directly into main menu, rather than into the set default activity.

- Lua calls for `GetParent` and `GetRootParent` can now be called by any `MovableObject` rather than being limited to `Attachable` only. ([Issue #102](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/102))  
	In some cases a cast to the appropriate type (`ToWhateverType`, e.g `ToMOSRotating`) will be needed when attempting to manipulate the object returned, because it will be returned as `MovableObject` if it is the root parent.  
	In cases where you need to iterate over a parent's attachable list the parent must be cast to the appropriate type that actually has an attachable list to iterate over.  
	For example:  
	```
	for attachable in ToMOSRotating(self:GetParent()).Attachables do
		...
	end
	```
	Or
	```
	local parent = ToMOSRotating(self:GetParent());
	for attachable in parent.Attachables do
		...
	end
	```

- Physics constants handling removed from `FrameMan` and now hardcoded in `Constants`. Lua bindings moved to `RTETools` and are now called without the `FrameMan` prefix like so:  
	`GetPPM()`, `GetMPP()`, `GetPPL()`, `GetLPP()`.

- Removed hardcoded 10 second `LifeTime` restriction for `MOPixel` and `MOSParticle`.

- `MOSParticle` animation can now be set with `SpriteAnimMode` and `SpriteAnimDuration`. If the property isn't defined it will default to `SpriteAnimMode = 7` (OVERLIFETIME).

- Reworked crab bombing behavior. When enabled through `Settings.ini` and triggered will gib all living actors on scene except brains and doors. Devices and non-actor MOs will remain untouched.

- `ADoor` properties `DrawWhenOpen` and `DrawWhenClosed` renamed to `DrawMaterialLayerWhenOpen` and `DrawMaterialLayerWhenClosed` so they are more clear on what they actually do.

- Specially handled Lua function `OnScriptRemoveOrDisable` has been changed to `OnScriptDisable`, and no longer has a parameter saying whether it was removed or disabled, since you can no longer remove scripts.

- When pasting multiple lines of code into the console all of them will be executed instead of the last line being pasted into the textbox and all before it executing.

- Input enums moved from `UInputMan` to `Constants` and are no longer accessed with the `UInputManager` prefix. These enums are now accessed with their own names as the prefix.  
	For example: `UInputManager.DEVICE_KEYB_ONLY` is now `InputDevice.DEVICE_KEYB_ONLY`, `UInputManager.INPUT_L_UP` is now `InputElements.INPUT_L_UP` and so on.

- `CraftsOrbitAtTheEdge` corrected to `CraftOrbitAtTheEdge`. Applies to both ini property and lua binding.

- Game will now Abort with an error message when trying to load a copy of a non-existent `AtomGroup`, `Attachable` or `AEmitter` preset.

- ComboBoxes (dropdown lists) can now also be closed by clicking on their top part.

- `Activity:IsPlayerTeam` renamed to `Activity:IsHumanTeam`.

- Screenshot functionality changed: ([Issue #162](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/162))  
	The `PrintScreen` button will now take a single screenshot on key release and will not take more until the key is pressed and released again.  
	The `Ctrl+S` key combination is unchanged and will take a single screenshot every frame while the keys are held.  
	The `Ctrl+W` and `Alt+W` key combinations will now take a single WorldDump/ScenePreview on `W` key release (while `Ctrl/Alt` are still held) and will not take more until the key is pressed and released again.

	Additionally, all screenshots (excluding abortscreen) will now be saved into a `_Screenshots` folder (`_` so it's on top and not hiding between module folders) to avoid polluting the root directory. ([Issue #163](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/163))  
	This folder will be created automatically after modules are loaded if it is missing.

- `ScreenDumps` and `WorldDumps` are now saved as compressed PNGs.

- Controller deadzone setting ignores more input. Previously setting it to the maximum was just enough to eliminate stick drift.

- `Arm.HandPos` will now get/set the hand position as relative to the arm's joint position, instead of relative to the arm's center of mass.

- Resolution settings in options screen changed:  
	Resolution multiplier button changed to `Fullscreen` button - this will set the game window resolution to match the desktop resolution. When resolution matches the desktop, this button will change to `Windowed` and will allow setting the game window resolution back to default (960x540).  
	Added `Upscaled Fullscreen` button - this will change the resolution to half of the desktop and the multiplier to 2. The `Fullscreen` button will change to `Windowed` in this mode to return to non-upscaled mode (960x540).  
	Selecting any resolution setting from the resolution combobox will immediately change to selected resolution. (Known bug: Clicking off the combobox without making a new selection while in `Upscaled Fullscreen` mode will change resolution to `Fullscreen`. This will be addressed later.)  

	**Note:** Changing the game window resolution while an Activity is active requires ending the Activity. A dialog box will appear asking to confirm the change.

- Moved from C-style random number generation to C++ standard. This includes usage of a `mt19937` random number generator.
	
- Resolution validation changed to support multiple screens. Incompatible/bad resolution settings will be overridden at startup with messages explaining the issue.  
	**Note:** For multi-screen to work properly, the left-most screen MUST be set as primary. Screens having different resolutions does not actually matter but different heights will still be warned about and overridden due to the likeliness of GUI elementes being cropped on the shortest screen.  
	Resolution validation can be disabled for multi-screen setups with `Settings.ini` property `DisableMultiScreenResolutionValidation`. Bad settings are likely to crash, use at own risk.  
	For setups with more than 3 screens `DisableMultiScreenResolutionValidation` must be set true.

- Damage to `Actors` from impulses is now relative to their max health instead of being on a scale from 0 to 100.

- `Scenes` with a `PresetName` containing the strings "Test", "Editor" and "Tutorial" are no longer excluded from the scenarios screen and from the MetaGame.

- `SoundContainer` is now a concrete Lua entity. This means it can now be created with `CreateSoundContainer("PresetName", "DataModule.rte")` and has all the standard functionality like cloning, etc.  
	To support these changes, a bunch of Lua functionality has been added and modified:  

	`soundContainer.Immobile` - Whether or not the `SoundContainer` is immobile. Immobile sounds are generally used for GUI elements and will never be automatically panned, pitched or attenuated.  
	`soundContainer.AttenuationStartDistance` - Formerly INI only, this property is now gettable and settable through Lua. See previous changelog entries for details on it.  
	`soundContainer.Pos` - Rather than updating the `SoundContainer's` position through `AudioMan`, you should now use the `Pos` property.  
	`soundContainer.Volume` - In addition to attenuation based volume changes, it is now possible to set a `SoundContainer's` overall volume. This works together with volume changes caused by attenuation.  
	`soundContainer.Pitch` - Rather than updating the `SoundContainer's` pitch through `AudioMan`, you should now use the `Pitch` property. Also note that this now works properly with the game's global pitch so no complicated handling is necessary.
	
- `AddSound` and `SelectNextSoundSet` Lua bindings have been moved from `SoundContainer` to `SoundSet`. The latter has been renamed and the former have been trimmed down slightly since some complexity is no longer needed. Their speciifcs are mentioned in the `Added` section.

- Pressing escape at the options, mod manager, game editors and credits screens no longer quits the game.

### Fixed

- Fix crash when returning to `MetaGame` scenario screen after activity end.

- Control schemes will no longer get deleted when being configured. Resetting the control scheme will load a preset instead of leaving it blank. ([Issue #121](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/121))

- Fix glow effects being drawn one frame past `EffectStartTime` making objects that exist for a single frame not draw glows. ([Issue #67](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/67))

- Time scale can no longer be lowered to 0 through the performance stats interface.

- Actors now support their held devices identically while facing to either side. ([Issue #31](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/31))

- Fixed issue where clicking a ComboBox's scrollbar would release the mouse, thus causing unexpected behavior like not being able to close the list by clicking outside of it.

- Fixed issue where ComboBoxes did not save the current selection, thus if the ComboBox was deselected without making a selection then the selection would revert to the default value instead of the last selected value.

- Fixed issue with double clicks and missing clicks in menus (anything that uses AllegroInput).

- Fixed issue where OnPieMenu function wasn't working for `AHuman` equipped items, and made it work for `BGArm` equipped items as well as `FGArm` ones.

- The "woosh" sound played when switching actors from a distance will now take scene wrapping into account. Additionally, attempting to switch to previous or next actor with only one actor will play the more correct "error" sound.

- `HDFirearm` INI property `DeactivationSound` now works properly instead of constantly playing.

- Gold mining sound has been set to restart its playback everytime it's played, making it way less annoying. It's still pretty wonky, but it's better.

- Sound panning should now work properly around scene seams. Additionally, sounds should be less stuttery (e.g. distant jetpacks) and generally sound better.

### Removed

- Removed the ability to remove scripts from objects with Lua. This is no longer needed cause of code efficiency increases.

- Removed `Settings.ini` property `PixelsPerMeter`. Now hardcoded and cannot be changed by the user.

- Removed `MOSParticle` property `Framerate` and lua bindings. `MOSParticle` animation is now handled with `SpriteAnimMode` like everything else.

- Removed `ConsoleMan.ForceVisibility` and `ConsoleMan.ScreenSize` lua bindings.

- Removed `ActivityMan.PlayerCount` and `ActivityMan.TeamCount` setters lua bindings (obsolete and did nothing).

- Removed `Activity` properties `TeamCount` and `PlayerCount`. These are handled internally and do nothing when set in ini.

- Removed `Activity` property `FundsOfTeam#`, use `Team#Funds` instead.

- Some functionality has been moved from `AudioMan` to `SoundContainer`. As such, the following `AudioMan` Lua bindings are no longer available:  
	`SetSoundPosition(soundContainer)`, `SetSoundPitch(soundContainer`, `PlaySound(filePath, position, player, loops, priority, pitchOrAffectedByGlobalPitch`, `attenuationStartDistance, immobile)`  
	
	The lengthy `PlaySound` function should be replaced by making a `SoundContainer` in your `Create` function and setting properties appropriately.  
	This can be done by creating one defined INI with `soundContainer = CreateSoundContainer(...)`, or by making an empty one with `soundContainer = SoundContainer()`.

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

- `SoundContainers` have been overhauled to allow for a lot more customization, including per-sound customization. The following INI example shows all currently available capabilities with explanatory comments:
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

- Incompatible/bad resolution settings will be overridden at startup with messages explaining the issue instead of multiple mode switches and eventually a reset to default VGA.  
	Reset to defaults (now 960x540) will happen only on horrible aspect ratio or if you managed to really destroy something.

- You can no longer toggle native fullscreen mode from the settings menu or ini. Instead, either select your desktop resolution at 1X mode or desktop resolution divided by 2 at 2X mode for borderless fullscreen windowed mode.  
	Due to limitations in Allegro 4, changing the actual resolution from within the game still requires a restart.

- If the current game resolution is half the desktop resolution or less, you will be able to instantly switch between 1X and 2X resolution multiplier modes in the settings without screen flicker or delay.  
	If the conditions are not met, the mode switch button will show `Unavailable`.

- `PieMenuActor` and `OrbitedCraft` have now been removed. They are instead replaced with parameters in their respective functions, i.e. `OnPieMenu(pieMenuActor);` and `CraftEnteredOrbit(orbitedCraft);`. Their use is otherwise unchanged.

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
	`FrameMan:ResetSplitScreens`, `FrameMan:PPM` setter, `FrameMan:ResX/Y`, `FrameMan:HSplit/VSplit`, `FrameMan:GetPlayerFrameBufferWidth/Height`, `FrameMan:IsFullscreen`, `FrameMan:ToggleFullScreen`, `FrameMan:ClearBackbuffer8/32`, `FrameMan:ClearPostEffects`, `FrameMan:ResetFrameTimer`, `FrameMan:ShowPerformanceStats`.

- Native fullscreen mode has been removed due to poor performance compared to windowed/borderless mode and various input device issues.  
	The version of Allegro we're running is pretty old now (released in 2007) and probably doesn't properly support/utilize newer features and APIs leading to these issues.  
	The minimal amount of hardware acceleration CC has is still retained through Windows' DWM and that evidently does a better job.

- Removed now obsolete `Settings.ini` properties:  
	**Post-processing:** `TrueColorMode`, `PostProcessing`, `PostPixelGlow`.   
	**Native fullscreen mode:** `Fullscreen`, `NxWindowed`, `NxFullscreen`, `ForceSoftwareGfxDriver`, `ForceSafeGfxDriver`.

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

- Objective arrows and Delivery arrows are now color coordinated to match their teams, instead of being only green or red.

- BuyMenu `Bombs` tab will now show all `ThrownDevices` instead of just `TDExplosives`.

- The list of `MOSRotating` attachables (`mosr.Attachables`) now includes hardcoded attachables like dropship engines, legs, etc.

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
[0.1.0-pre3.0]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/releases/tag/v0.1.0-pre3.0

