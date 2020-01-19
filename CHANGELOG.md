# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.0.1] - 2020-01-19
### Added
- You can now run the game with command line parameters, including `-h` to see help and `-c` to send ingame console input to cout
- `MOSprite` now has the `FlipFactor` property that returns -1 if the sprite is flipped and 1 if it's not
- `TDExplosive` now has the `IsAnimatedManually` property that lets modders set its frames manually through lua
- You can now add `AEmitters` to `MOSRotatings` and have them function similarly to attachables.  
  Addition: `parent:AddEmitter(emitterToAdd)` or `parent:AddEmitter(emitterToAdd, parentOffsetVector)`  
  Removal: `parent:RemoveEmitter(emitterToRemove)` or `parent:RemoveEmitter(uniqueIdOfEmitterToRemove)`
- Attachables can now collide with terrain when attached.  
  Check value: `attachable.IsCollidingWithTerrainWhileAttached`  
  Manipulate value: `attachable:EnableTerrainCollisions(trueOrFalse)`

### Changed
- `AimRange` is now split into `UpperLimit` and `LowerLimit`, allowing asymmetric ranges, and allowing crabs' aiming to work with their rotation.
- Objective arrows and Delivery arrows are now colour co-ordinated to match their teams, instead of being only green or red.
- `BuyMenu` Bombs tab will now show all `ThrownDevices` instead of just `TDExplosives`
- The list of an `MOSRotating's` attchables (`mosr.Attachables`) now includes hardcoded attachables like dropship engines, legs, etc.
- Attachable lua manipulation has been significantly revamped. The old method of doing `attachable:Attach(parent)` has been replaced with the following:  
  Addition: `parent:AddAttachable(attachableToAdd)` or `parent:AddAttachable(attachableToAdd, parentOffsetVector)`  
  Removal: `parent:RemoveAttachable(attachableToRemove)` or `parent:RemoveAttachable(uniqueIdOfAttachableToRemove)`
- Wounds have been separated internally from emitter attachables. They can now be added with `parent:AddWound(woundEmitterToAdd)`. Removing wounds remains the same as before.

### Removed
- All licensing-related code has been removed since it's no longer needed
- Wounds can no longer be added via ini, as used to be doable buggily through ini `AddEmitter`

### Fixed
- SFX slider now works properly
- BGM now loops properly
- Sound pitching now respects sounds that are not supposed to be affected by pitch
- Using `actor:Clone()` now works properly, there are no longer issues with controlling/selecting cloned actors
- Various bug fixed related to all the Attachable and Emitter changes, so they can now me affected reliably and safely with lua
- Various minor other things that have gotten lost in the shuffle


[unreleased]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/compare/master...cortex-command-community:development
[0.0.1]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/releases/tag/v0.0.1
