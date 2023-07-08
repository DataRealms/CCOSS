# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

<details><summary><b>Added</b></summary>

- New `Settings.ini` property `EnableMultithreadedAI`, which can be used to enable experimental support for multithreaded AI. Please note that this is in a testing phase and is likely to cause bugs, especially with mods.

- Multithreaded asynchronous pathfinding, which dramatically improves performance on large maps and improves AI responsiveness.
	New `Actor` Lua property (R) `IsWaitingOnNewMovePath`, which returns true while the actor is currently calculating a new path.  
	New Lua `SceneMan` function `CalculatePathAsync` for asynchronous pathfinding:
	```lua
	-- No return value
	SceneMan.Scene:CalculatePathAsync(
		function(pathRequest) -- Callback that is triggered when the path has finished calculating, passing the pathRequest object
			pathRequest.Path; -- The calculated path, list of Vector
			pathRequest.Status; -- The status of the path, int 
								--	SOLVED			== 0,
								--	NO_SOLUTION		== 1,
								--	START_END_SAME	== 2
			pathRequest.TotalCost; -- The total cost of path, float
		end,
		-- All other arguments are the same as Scene:CalculatePath():
		startPos, -- Start position, Vector
		endPos,  -- End position, Vector
		movePathToGround,  -- Whether or not to move the path points to the ground, bool
		GetPathFindingDefaultDigStrength(), -- The dig strength we're using, float
		team -- Team that is doing the pathfinding. Their doors will be ignored, Team, optional parameter that defaults to Team.NOTEAM (no doors are ignored)
	);
	```

</details>

<details><summary><b>Changed</b></summary>

- Lua `Scene.ScenePath` property has been changed to a function `Scene:GetScenePath()`. This was done for thread-safety with multithreading, but can be used in the same way.

</details>

<details><summary><b>Fixed</b></summary>

- Fixed frame spiking on Decision Day activity.

</details>

<details><summary><b>Removed</b></summary>
</details>

***

## [0.1.0 pre-release 5.0][0.1.0-pre5.0] - 2023/06/17

<details><summary><b>Added</b></summary>

- New INI `HDFirearm` property `LegacyCompatibilityRoundsAlwaysFireUnflipped`. This is used to make guns fire their projectiles unflipped, like they used to in old game versions, and should only be turned on for `HDFirearms` in old mods that need it.

- New INI and Lua (R) `HDFirearm` property `InheritsFirerVelocity`, which determines whether or not the particles in a `Round` should inherit their firer's velocity. Defaults to true to preserve normal behavior.
	
- New INI `BuyableMode` setting `BuyableMode = 3` for script only items.  
	This makes the item unable to be bought by any player at all as if it were `Buyable = 0`, but without removing the item from the `CreateRandom` Lua functions that the AI and activities use.  
	That way players can still find these weapons used by activities and AI enemies, and modders can have big arsenals, without actually having big arsenals.

- New `DataModule` INI property `SupportedGameVersion` to define what version of the game a mod supports. This must be specified, and must match the current game version, in order for the mod to load successfully.

- New Lua event functions for `HDFirearm` - `OnFire(self)` that triggers when the gun fires, and `OnReload(self, hadMagazineBeforeReload)` that triggers when the gun is reloaded.

- New `MOSRotating` INI and Lua (R/W) `MOSRotating` property `GibAtEndOfLifetime` that, when set to true, will make the `MOSRotating` gib if its age exceeds its lifetime, rather than deleting as it normally would.

- New `Actor` INI properties `Organic = 0/1` and `Mechanical = 0/1` and supporting Lua functions `Actor:IsOrganic()` and `Actor:IsMechanical()`.  
	These have no direct gameplay effect (and default to false), but will be very useful for inter-mod compatibility, as they allow scripts to know if an `Actor` is organic or mechanical, and treat them accordingly.
  
- New INI and Lua (R/W) `ACDropShip` property `HoverHeightModifier`. This allows for modification of the height at which an `ACDropShip` will hover when unloading cargo, or staying at a location.

- New `Scene` Lua property `BackgroundLayers` (R/O) to access an iterator of the `Scene`'s `SLBackground` layers.

- `SLBackground` layers can now be animated using the same INI and Lua animation controls as everything else (`FrameCount`, `SpriteAnimMode`, etc). ([Issue #66](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/66))  
	The collection of the `Scene`'s background layers can be accessed via `scene.BackgroundLayers`.

- Added `SLBackground` auto-scrolling INI and Lua controls. ([Issue #66](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/66))  
	The INI definition looks like this:  
	```
	CanAutoScrollX = 0/1 // Whether this can auto-scroll on the X axis. Scrolling will take effect only when combined with WrapX = 1, otherwise ignored.
	CanAutoScrollY = 0/1 // Whether this can auto-scroll on the Y axis. Scrolling will take effect only when combined with WrapY = 1, otherwise ignored.

	AutoScrollStepInterval = intValue // The duration between auto-scrolling steps on both axis, in milliseconds.

	AutoScrollStep = Vector
		X = intValue // The number of pixels to progress the scrolling on the X axis each step. Float values are supported but may end up choppy and inconsistent due to internal rounding and lack of sub-pixel precision.
		Y = intValue // The number of pixels to progress the scrolling on the Y axis each step. Float values are supported but may end up choppy and inconsistent due to internal rounding and lack of sub-pixel precision.
	```
	You can read and write the following Lua properties:  
	```
	slBackground.CanAutoScrollX = bool -- this may be true even if X axis scrolling is not in effect due to WrapX = 0.
	slBackground.CanAutoScrollY = bool -- this may be true even if Y axis scrolling is not in effect due to WrapY = 0.
	slBackground.AutoScrollInterval = intValue
	slBackground.AutoScrollStep = vector
	slBackground.AutoScrollStepX = intValue
	slBackground.AutoScrollStepY = intValue
	```
	`slBackground:IsAutoScrolling()` - (R/O) returns whether auto-scrolling is actually in effect on either axis (meaning either `WrapX` and `CanAutoScrollX` or `WrapY` and `CanAutoScrollY` are true).

	The collection of the `Scene`'s background layers can be accessed via `scene.BackgroundLayers`.

- New `Settings.ini` property `SceneBackgroundAutoScaleMode = 0/1/2`. ([Issue #243](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/243))  
	Auto-scaling modes are: 0 = Off, 1 = Fit Screen, 2 = Always 2x.  
	This will auto-scale all `Scene` background layers to cover the whole screen vertically in cases where the layer's sprite is too short and creates a gap at the bottom.  
	In cases where a layer is short by design, auto-scaling behavior can be disabled on a per-layer basis using the `SLBackground` INI property `IgnoreAutoScaling`, otherwise it may be excessively scaled up to 2x (maximum).  
	Note that any `ScaleFactor` definitions in layers that aren't set to ignore auto-scaling will be overridden.  

	Only relevant when playing on un-scaled vertical resolutions of 640p and above as most background layer sprites are currently around 580px tall, and comes with a minor performance impact.  
	Note that in fit screen mode each layer is scaled individually so some layers may be scaled more than others, or not scaled at all.  
	In always 2x mode all layers will be equally scaled to 2x.

- New `SLBackground` INI property `IgnoreAutoScaling = 0/1` to ignore the global background layer auto-scaling setting and use the `ScaleFactor` defined in the preset, if any.

- New `SLBackground` INI property `OriginPointOffset` to offset the layer from the top left corner of the screen.

- Added new `TerrainDebris` scattering functionality. ([Issue #152](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/152))  
	New INI properties are:  
	```
	MinRotation/MaxRotation = angleDegrees // Rotates each debris piece to a random angle within the specified values. Values in degrees, negative values are counter-clockwise.  

	FlipChance = floatValue // Chance for a debris piece to be flipped when either `CanHFlip` or `CanYFlip` are set true. 0-1, defaults to 0.5.  
	
	CanHFlip/CanVFlip = 0/1 // Flips each debris piece on the X, Y or both axis.
	```

- Added support for `Material` background textures with INI property `BGTextureFile`.

- New `MovableObject` INI and Lua (R/W) property `SimUpdatesBetweenScriptedUpdates`, that lets `MovableObject`s run their Lua update function less frequently, for performance benefits.

- Added faction themes.  
	Faction themes apply to the `BuyMenu` when the faction is set as the native module (i.e. playing as the faction) in both Conquest and Scenario battle.

	The theme properties are defined in the `DataModule`'s `Index.ini` (before any `IncludeFile` lines) like so:
	```
	FactionBuyMenuTheme = BuyMenuTheme
		SkinFile = pathToSkinFile // GUI element visuals (NOT actual layout).
		BackgroundColorIndex = paletteIndex // Color of the parent box that holds all the elements. Palette colors only, no support for images.
		BannerFile = pathToBannerImage
		LogoFile = pathToLogoImage
	```
	All properties are optional, any combination works.  
	The skin and background color are also applied to the `ObjectPicker` (scene object placer) for visual consistency.

- New `Settings.ini` property `DisableFactionBuyMenuThemes = 0/1` which will cause custom faction theme definitions in all modules to be ignored and the default theme to be used instead.

- New `Settings.ini` property `DisableFactionBuyMenuThemeCursors = 0/1` which will cause custom faction theme cursor definitions in all modules to be ignored and the default cursors to be used instead.

- New `Settings.ini` and `SceneMan` Lua (R/W) property `ScrapCompactingHeight` which determines the maximum height of a column of scrap terrain to collapse when the bottom pixel is knocked loose. 0 means no columns of terrain are ever collapsed, much like in old builds of CC.

- New `DataModule` INI and Lua (R/O) property `IsMerchant` which determines whether a module is an independent merchant. Defaults to false (0). ([Issue #401](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/401))  
	A module defined as a merchant will stop being playable (in Conquest, etc.) but will have its buyable content available for purchase/placement when playing as any other faction (like how base content is).  
	Only has a noticeable effect when the "Allow purchases from other factions" (`Settings.ini` `ShowForeignItems`) gameplay setting is disabled.

	Note that this property takes priority over the `IsFaction` property. A module that is set as both `IsFaction = 1` and `IsMerchant = 1` will be treated as `IsFaction = 0`.

- Made `Vector` properties `AbsRadAngle` and `AbsDegAngle` writable, so you can set a `Vector`'s direction directly, instead of rotating it by an angle via `RadRotate` and `DegRotate`.

- New `HDFirearm` Lua function `GetNextMagazineName()`, that gets the name of the next `Magazine` to be loaded into the `HDFirearm`.

- New INI and Lua (R/W) `Actor` property `PieMenu`. This allows you to set an `Actor`'s `PieMenu` via INI and Lua. If no value is set for this, the default `PieMenu` for the type of `Actor` will be used, these can be found in `Base.rte/GUIs/PieMenus/PieMenus.ini`.

- `PieMenu`s have been completely redone, and are now fully defined in INI and customizable in Lua.	Additionally, `PieMenu`s can have sub-`PieMenu`s to allow for better organization and more controls.

	You can define `PieMenu`s in INI using standard INI concepts like `CopyOf` and `PresetName`, and modify their visuals as you please. They have the following INI properties:  
	```
	IconSeparatorMode // The visuals style of the PieMenu's separators. The options are "Line", "Circle" and "Square". Defaults to "Line".  
	FullInnerRadius // The inner radius of the PieMenu when it's fully expanded, in pixels. Defaults to 58.  
	BackgroundThickness // The thickness of the background ring of the PieMenu. Defaults to 16.  
	BackgroundSeparatorSize // The size of the background separators (lines, circles, squares). Defaults to 2.  
	DrawBackgroundTransparent // Whether or not the PieMenu's background should be drawn transparent. Defaults to true.  
	BackgroundColor // The color used to draw the background ring of the PieMenu. Color values are palette indexes.  
	BackgroundBorderColor // The color used to draw the border around the background ring of the PieMenu. Color values are palette indexes.  
	SelectedItemBackgroundColor // The color used to highlight selected PieSlices in the PieMenu. Color values are palette indexes.  
	AddPieSlice = PieSlice // Add a PieSlice to the PieMenu. Standard INI concepts like CopyOf, etc. apply.
	```	

	Additionally, `PieMenu`s have the following Lua properties and functions:  

	**`Owner`** (R) - Gets the owning `Actor` for the `PieMenu`, if there is one.  
	**`Controller`** (R) - Gets the `Controller` controlling the `PieMenu`. If there's an `Actor` owner, it'll be that `Actor`'s `Controller`, otherwise it's probably a general `Controller` used for handling in-game menu inputs.  
	**`AffectedObject`** (R) - Gets the affected `MovableObject` for the `PieMenu` if there is one. Support isn't fully here for it yet, but `PieMenu`s can theoretically be made to affect objects that aren't `Actors`.  
	**`Pos`** (R) - Gets the position of the center of the `PieMenu`. Generally updated to move with its `Owner` or `AffectedObject`.  
	**`RotAngle`** (R/W) - Gets/sets the rotation of the `PieMenu`. Note that changing this may cause oddities and issues, especially if the **`PieMenu`** is currently visible.  
	**`FullInnerRadius`** (R/W) - Gets/sets the inner radius of the `PieMenu`, i.e. the radius distance before the inside of the ring.  
	**`PieSlices`** - Gets all of the `PieSlice`s in the `PieMenu` for iterating over in a for loop.  

	**`IsEnabled()`** - Gets whether or not the `PieMenu` is enabled or enabling.  
	**`IsEnabling()`** - Gets whether or not the `PieMenu` is currently enabling.  
	**`IsDisabling()`** - Gets whether or not the `PieMenu` is currently disabling.  
	**`IsEnablingOrDisabling()`** - Gets whether or not the `PieMenu` is currently enabling or disabling.  
	**`IsVisible()`** - Gets whether or not the `PieMenu` is currently visible (i.e. not disabled).  
	**`HasSubPieMenuOpen()`** - Gets whether the `PieMenu` has a sub-`PieMenu` open, and is thus transferring commands to that sub-`PieMenu`.  
	**`SetAnimationModeToNormal()`** - Sets the `PieMenu` back to normal animation mode, and disables it so it's ready for use.  
	**`DoDisableAnimation()`** - Makes the `PieMenu` do its disabling animation.  
	**`Wobble()`** - Makes the `PieMenu` do its wobbling animation.  
	**`FreezeAtRadius(radius)`** - Makes the `PieMenu` freeze open at the given radius.  
	**`GetPieCommand()`** - Gets the command given to the `PieMenu`, either by pressing a `PieSlice` button, or by selecting a `PieSlice` and closing the `PieMenu`.  
	**`GetFirstPieSliceByPresetName(presetName)`** - Searches through the `PieSlice`s in the `PieMenu` and returns the first one with the given `PresetName`.  
	**`GetFirstPieSliceByType(pieSliceType)`** - Searches through the `PieSlice`s in the `PieMenu` and returns the first one with the given `PieSlice` `Type`.

	**`AddPieSlice(pieSliceToAdd, pieSliceOriginalSource, optional_onlyCheckPieSlicesWithSameOriginalSource, optional_allowQuadrantOverflow)`** - Adds the given `PieSlice` to the `PieMenu`, returning whether or not the `PieSlice` was added.  
	The `pieSliceOriginalSource` is the object that added the `PieSlice` to the `PieMenu`, and it's very important you set this properly (much of the time you'll want it to be `self` if, say, you have a gun adding a `PieSlice`), otherwise you can end up with ghost `PieSlice`s.  
	`allowQuadrantOverflow` is optional and defaults to false, it determines whether the `PieSlice` can only be added in its specified direction (false), or if it can overflow if that direction is full of `PieSlice`s (true).

	**`AddPieSliceIfPresetNameIsUnique(pieSliceToAdd, pieSliceOriginalSource, optionalAllowQuadrantOverflow)`** - Like `AddPieSlice`, this adds the given `PieSlice` to the `PieMenu` and returns whether or not the `PieSlice` was added, but only if the `PieMenu` doesn't contain a `PieSlice` with this `PieSlice`'s preset name, optionally only checking `PieSlice`s with the same original source (by default it checks all `PieSlice`s in the `PieMenu`).  
	`PieSlice`s with no preset name will always be added by this.

	**`RemovePieSlice(pieSliceToRemove)`** - Removes the given `PieSlice` from the `PieMenu`, and returns it to Lua so you can add it to another `PieMenu` if you want.  
	**`RemovePieSlicesByPresetName(presetNameToRemoveBy)`** - Removes any `PieSlice`s with the given preset name from the `PieMenu`. Note that, unlike `RemovePieSlice`, the `PieSlice` is not returned, since multiple `PieSlices` can be removed this way. Instead, this returns true if any `PieSlice`s were removed.  
	**`RemovePieSlicesByType(pieSliceTypeToRemoveBy)`** - Removes any `PieSlice`s with the given `PieSlice` `Type` from the `PieMenu`. Note that, unlike `RemovePieSlice`, the `PieSlice` is not returned, since multiple `PieSlices` can be removed this way. Instead, this returns true if any `PieSlice`s were removed.  
	**`RemovePieSlicesByOriginalSource(originalSource)`** - Removes any `PieSlice`s with the original source from the `PieMenu`. Note that, unlike `RemovePieSlice`, the `PieSlice` is not returned, since multiple `PieSlices` can be removed this way. Instead, this returns true if any `PieSlice`s were removed.  
	
	**`ReplacePieSlice`(pieSliceToReplace, replacementPieSlice)`** - Replaces the specified `PieSlice` to replace, if it exists in the `PieMenu`, with the replacement `PieSlice` and returns the replaced `PieSlice` for use (e.g. for adding to a different `PieMenu`). The replacement `PieSlice` takes the replaced `PieSlice`'s original source, direction, middle slice eligibility, angles and slot count, so it seamlessly replaces it.

- `PieSlice`s have been modified to support `PieMenu`s being defined in INI. They have the following properties:  

	**`Type`** (INI, Lua R/W) - Gets or sets the `PieSlice`'s type, useful for invoking hardcoded game actions (e.g. pickup).  
	**`Direction`** (INI, Lua R/W) - Gets or sets the `PieSlice`'s direction, i.e what direction the `PieSlice` should be added in to a `PieMenu`. Defaults to `Any`, which means it will be added to the least populated direction. Note that if you set this via Lua, you will need to remove and readd the `PieSlice` for it to take effect.  
	**`CanBeMiddleSlice`** (INI, Lua R/W) - Gets or sets whether or not this `PieSlice` can be the middle slice (i.e. a cardinal one like reload) in its direction, when added to a `PieMenu`. Defaults to true. Note that if you set this via Lua, you will need to remove and readd the `PieSlice` for it to take effect.  
	**`Enabled`** (INI, Lua R/W) - Gets or sets whether or not this `PieSlice` is enabled and usable.  
	**`ScriptPath`** (INI, Lua R/W) - Gets or sets the filepath to the script that should be run when this `PieSlice` is activated. A script function name is also required for this to work.  
	**`ScriptFunctionName`** (INI Lua R/W) - Gets or sets the name of the function that should be run when this `PieSlice` is activated. A script path is also required for this to work.  
	**`SubPieMenu`** (INI Lua R/W) - Gets or sets the sub-`PieMenu` that should be opened when this `PieSlice` is activated. Note that `PieSlice`s with sub-`PieMenu`s will not perform any other actions, though they will run scripts.  
	**`Icon`** (INI) - The icon for this `PieSlice` to show in its `PieMenu`.  
	**`OriginalSource`** (Lua R) - The object that added this `PieSlice` to its `PieMenu`.
	
- Added `Directions` enum with the following values:  
	```
	(-1) None
	(0)  Up
	(1)  Down
	(2)  Left
	(3)  Right
	(4)  Any
	```

- Added `GameActivity` INI property `BuyMenuEnabled` to match the Lua property, and made the buy menu `PieSlice` disappear if `BuyMenuEnabled` is false.  
	Note that, if you toggle this from off to on in Lua for a running `GameActivity`, you'll need to re-add the buy menu `PieSlice` manually.
	
- Added `Controller` Lua function `IsGamepadControlled()`, that lets you tell if a `Controller` is being controlled by any of the gamepad inputs, much like the pre-existing `IsMouseControlled()` function.

- Added some useful global angle helper functions:  
	```lua
	NormalizeAngleBetween0And2PI(angle) -- Takes an angle in radians, and returns that angle modified so it's not negative or larger than 2PI.	
	NormalizeAngleBetweenNegativePIAndPI(angle) -- Takes an angle in radians, and returns that angle modified so angles larger than PI are instead represented as negative angles, and no angle is larger than PI or smaller than -PI.
	AngleWithinRange(float angleToCheck, float startAngle, float endAngle) -- Returns whether or not the angleToCheck is between the startAngle and endAngle, in a counter-clockwise direction (e.g. 0.5rad is between 0rad and 1rad, and 0.3rad is between 2.5rad and 1 rad).	 
	ClampAngle(float angleToClamp, float startAngle, float endAngle) -- Returns the angleToClamp, clamped between the startAngle and endAngle.
	```
	
- Added support for nested block comments in INI. ([Issue #248](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/248))  
	The reader will track block comment open tags and crash if a file ends while a block is open, reporting the line it was opened on.

- Added thickness option to Line primitives. ([Issue #403](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/403))  
	New bindings with argument for thickness are:  
	`PrimitiveMan:DrawLinePrimitive(startPos, endPos, color, thickness)`  
	`PrimitiveMan:DrawLinePrimitive(player, startPos, endPos, color, thickness)`  
	Original bindings with no thickness argument are untouched and can be called as they were.

- Added rotation option to Text primitives.  
	New bindings with argument for rotation are:  
	`PrimitiveMan:DrawTextPrimitive(pos, text, bool useSmallFont, alignment, rotAngleInRadians)`  
	`PrimitiveMan:DrawTextPrimitive(player, pos, text, bool useSmallFont, alignment, rotAngleInRadians)`  
	Original bindings with no rotation argument are untouched and can be called as they were.

- Added option to draw bitmap from file instead of from `MOSPrite` based object to Bitmap primitives.  
	New bindings with argument for file path are:  
	`PrimitiveMan:DrawBitmapPrimitive(pos, filePath, rotAngleInRadians)`  
	`PrimitiveMan:DrawBitmapPrimitive(pos, filePath, rotAngleInRadians, hFlipped, vFlipped)`  
	`PrimitiveMan:DrawBitmapPrimitive(player, pos, filePath, rotAngleInRadians)`  
	`PrimitiveMan:DrawBitmapPrimitive(player, pos, filePath, rotAngleInRadians, hFlipped, vFlipped)`  
	Note that the `frame` argument does not exist in these bindings.  
	Original bindings with no filepath argument are untouched and can be called as they were.

- Added new primitive drawing functions to `PrimitiveMan`:  
	```lua
	-- Polygon
	PrimitiveMan:DrawPolygonPrimitive(Vector startPos, color, { Vector vertexRelPos, ... })
	PrimitiveMan:DrawPolygonPrimitive(player, Vector startPos, { Vector vertexRelPos, ... })

	PrimitiveMan:DrawPolygonFillPrimitive(Vector startPos, color, { Vector vertexRelPos, ... })
	PrimitiveMan:DrawPolygonFillPrimitive(player, Vector startPos, { Vector vertexRelPos, ... })
	```
	The vertices table contains `Vector`s with the position of each vertex of the polygon **RELATIVE** to the starting position. The starting position will be automatically added to each vertex position, doing so manually will lead to unexpected results.  
	A minimum of 2 vertices (which would result in a line) are required to draw a polygon primitive. A console error will be printed and drawing will be skipped if less are provided.  
	There may be a limit for the number of vertices in `PolygonFillPrimitive` because it has different handling but it was not reached during testing.

	The order of vertices is of high importance. Bad ordering will lead to unexpected results.  
	For example: `{ Vector(10, 0), Vector(10, 10), Vector(0, 10), Vector(0, 0) }` will result in a square, while `{ Vector(10, 0), Vector(0, 10), Vector(10, 10), Vector(0, 0) }` will result in an hourglass shape.  
	Note that **all** vertices of the shape must be specified, as the last vertex will be connected to the first vertex and not to the starting position (whether it is used as center or as a corner) to complete the shape.  
	Omitting the last `Vector(0, 0)` in the above example would result in a right angle triangle.

	**The `Vector`s in the vertices table are single use! They will be deleted after being drawn, so they cannot be re-used!**

	Usage example:
	```lua
	local myVertices = {
		Vector(10, 0),
		Vector(10, 10),
		Vector(0, 10),
		Vector(0, 0)
	};
	PrimitiveMan:DrawPolygonPrimitive(self.Pos, 13, myVertices);

	-- myVertices no longer contains valid Vectors for this call, they were deleted after being drawn by the previous call.
	PrimitiveMan:DrawPolygonFillPrimitive(self.Pos, 13, {
		Vector(10, 0),
		Vector(10, 10),
		Vector(0, 10),
		Vector(0, 0)
	});
	```

- Added blended drawing functions to `PrimitiveMan`:  
	There are 10 blending modes available to produce different color and transparency effects for both true primitives and bitmap based primitives.  
	Blended drawing effects are not the same as post-processing (glows), as they are all drawn in indexed color mode and will produce widely different results.

	**Note that blended drawing is very expensive and chugs FPS like no tomorrow. It should not be abused!**

	There are 3 blended drawing function overloads:

	* `PrimitiveMan:DrawPrimitives(blendMode, blendAmountR, blendAmountG, blendAmountB, blendAmountA, { primitiveObj, ... })`  
	This is the fully fledged blended drawing function which allows individual control over each color channel blend amount.  
	Blend amounts are in percentages, where 0 means no blending and 100 means full blending (e.g. `blendAmountA = 100` will result in a fully transparent primitive, as if it was not drawn at all).  
	The blend mode and amounts will be applied to all the primitives in the primitive table.  
	Note that blend amounts are internally rounded to multiples of 5 (e.g. 32 will round to 30, 33 will round to 35) to reduce memory usage and because smaller steps are hardly noticeable.
	
	* `PrimitiveMan:DrawPrimitives(blendMode, blendAmountRGBA, { primitiveObj, ... })`  
	This overload allows selecting a blend mode and applies the blend amount to all color channels at once.  
	This overload is for convenience when using certain modes (e.g. `Invert` and `Dissolve`). See blend mode specifics further below.
	
	* `PrimitiveMan:DrawPrimitives(transBlendAmount, { primitiveObj, ... })`  
	This overload uses the transparency blending mode and applies the blend amount to all color channels at once.  
	Transparency blending is likely to be the most commonly used mode so this exists for convenience.

	The blending modes are defined in the new `DrawBlendMode` enum as follows:
	```
	(0)  NoBlend
	(1)  Burn
	(2)  Color
	(3)  Difference
	(4)  Dissolve
	(5)  Dodge
	(6)  Invert
	(7)  Luminance
	(8)  Multiply
	(9)  Saturation
	(10) Screen
	(11) Transparency
	(12) BlendModeCount
	```
	The blending modes are common and information on what the result of each is can be found with a quick search.

	Some blend mode specifics:
	* `Invert` and `Dissolve` modes only use alpha channel blend amount. RGB channels blend amounts are ignored in these modes.  
	* `Transparency` mode ignores alpha channel blend amount. Only RGB channels blend amounts are used in this mode.  
	* Some trial and error is expected to produce desired results in other modes.

	The primitives table must be filled with `GraphicalPrimitive` objects. For this the constructors for all the supported primitives have been exposed:
	```lua
	LinePrimitive(player, startPos, endPos, color)
	ArcPrimitive(player, centerPos, startAngle, endAngle, radius, thickness, color)
	SplinePrimitive(player, startPos, guideA, guideB, endPos, color)
	BoxPrimitive(player, topLeftPos, bottomRightPos, color)
	BoxFillPrimitive(player, topLeftPos, bottomRightPos, color)
	RoundedBoxPrimitive(player, topLeftPos, bottomRightPos, cornerRadius, color)
	RoundedBoxFillPrimitive(player, topLeftPos, bottomRightPos, cornerRadius, color)
	CirclePrimitive(player, centerPos, radius, color)
	CircleFillPrimitive(player, centerPos, radius, color)
	EllipsePrimitive(player, centerPos, horizRadius, vertRadius, color)
	EllipseFillPrimitive(player, centerPos, horizRadius, vertRadius, color)
	TrianglePrimitive(player, pointA, pointB, pointC, color)
	TriangleFillPrimitive(player, pointA, pointB, pointC, color)
	TextPrimitive(player, pos, text, useSmallFont, alignment, rotAngle)
	BitmapPrimitive(player, centerPos, moSprite, rotAngle, frame, hFlipped, vFlipped)
	BitmapPrimitive(player, centerPos, filePath, rotAngle, hFlipped, vFlipped)
	```
	Note that `PolygonPrimitive`, `IconPrimitive` and `LinePrimitive` with thickness do not support blended drawing.

	**The `GraphicalPrimitive`s in the primitives table are single use! They will be deleted after being drawn, so they cannot be re-used!**

	Usage example:
	```lua
	local myPrimitives = {
		CircleFillPrimitive(-1, self.Pos + Vector(-100, 0), 20, 13),
		BitmapPrimitive(-1, self.Pos + Vector(100, 0), "Base.rte/Craft/Rockets/MK2/RocketMK2000.png", 0, false, false)
	};
	PrimitiveMan:DrawPrimitives(DrawBlendMode.Screen, 50, 0, 50, 0, myPrimitives);

	-- myPrimitives no longer contains valid primitives for this call, they were deleted after being drawn by the previous call.
	PrimitiveMan:DrawPrimitives(DrawBlendMode.Dissolve, 50, {
		CircleFillPrimitive(-1, self.Pos + Vector(-100, -100), 20, 13),
		BitmapPrimitive(-1, self.Pos + Vector(100, -100), "Base.rte/Craft/Rockets/MK2/RocketMK2000.png", 0, false, false)
	});

	PrimitiveMan:DrawPrimitives(50, {
		CircleFillPrimitive(-1, self.Pos + Vector(-100, -200), 20, 13),
		BitmapPrimitive(-1, self.Pos + Vector(100, -200), "Base.rte/Craft/Rockets/MK2/RocketMK2000.png", 0, false, false)
	});

	-- NoBlend will draw the primitives without any blending mode (solid mode). Any color channel blend amounts are ignored.
	PrimitiveMan:DrawPrimitives(DrawBlendMode.NoBlend, 0, 0, 0, 0, {
		CircleFillPrimitive(-1, self.Pos + Vector(-100, -300), 20, 13),
		BitmapPrimitive(-1, self.Pos + Vector(100, -300), "Base.rte/Craft/Rockets/MK2/RocketMK2000.png", 0, false, false)
	});
	-- It is equivalent to calling the individual draw functions like so:
	-- PrimitiveMan:DrawCircleFillPrimitive(-1, self.Pos + Vector(-100, -200), 20, 13);
	-- PrimitiveMan:DrawBitmapPrimitive(-1, self.Pos + Vector(100, -200), "Base.rte/Craft/Rockets/MK2/RocketMK2000.png", 0, false, false);
	```

- New `Vector` Lua (R/O) property `SqrMagnitude` which returns the squared magnitude of the `Vector`.  
	Should be used for more efficient comparison with `vector.SqrMagnitude > (floatValue * floatValue)` over `vector.Magnitude > floatValue`.

- New `Vector` Lua convenience functions for more efficient magnitude comparison.  
	```lua
	-- These perform vector.SqrMagnitude > or < (floatValue * floatValue).
	vector:MagnitudeIsGreaterThan(floatValue) -- Note that you can use (not vector:MagnitudeIsGreaterThan(floatValue)) in place of (vector.SqrMagnitude <= (floatValue * floatValue)).
	vector:MagnitudeIsLessThan(floatValue) -- Note that you can use (not vector:MagnitudeIsLessThan(floatValue)) in place of (vector.SqrMagnitude >= (floatValue * floatValue)).
	```

- The game now supports saving and loading. The easiest way to do this is to quick-save with `F5`, and quick-load with `F9`. Console clearing is now done with `F10`.
	```lua
	ActivityMan:SaveGame(fileName) -- Saves the currently playing Scene and Activity. The save result will be printed to the console.
	ActivityMan:LoadGame(fileName) -- Loads and resumes a previously saved Scene and Activity.
	```
	The `Activity` start function now looks like `function activityName:StartActivity(isNewGame)`. The new `isNewGame` parameter is true if a game is beingly newly started (or restarted), and false if it's being loaded.  

	Scripts on `Activities` now have a new callback function `OnSave` (in addition to `Create`, `Update`, etc), which is called whenever a scene is saved. This function must exist for the `Activity` to be saveable!  
	To support saving and loading, `Activity` now has several Lua convenience functions to for dealing with script variables:
	```lua
	Activity:SaveNumber(stringKey, floatValue) -- Saves a float value which can later be retrieved using stringKey.
	Activity:LoadNumber(stringKey) -- Retrieves a previously saved float value with key stringKey.

	Activity:SaveString(stringKey, stringValue) -- Saves a string value which can later be retrieved using stringKey.
	Activity:LoadString(stringKey) -- Retrieves a previously saved string value with key stringKey.
	```
	A new `GlobalScript` has been added that will automatically save the game every three minutes. To turn it on, enable the Autosaving `GlobalScript` in the main menu mod manager's Global Scripts section.  
	To load games saved by this script, open the console and enter the command `ActivityMan:LoadGame("Autosave")`, or use the `Ctrl + F9` shortcut.
	
- New hardcoded `MovableObject` function `OnGameSave(self)` that gets run for each `MovableObject` with it when the game is saved. This can be used in tandem with custom values (for `MOSRotatings` and child classes) to store data, which can be read during `Create` when the game is loaded.
	
- New Lua `AEmitter` properties:  
	**TotalParticlesPerMinute** (R/O) - The rate at which all of the `Emission`s of this `AEmitter` combined, emit their particles.  
	**TotalBurstSize** (R/O) - The number of particles that will be emitted by all the `Emission`s of this `AEmitter` combined, in one shot when a burst is triggered.  
	**EmitCount** (R/O) - The number of emissions emitted since emission was last enabled.  
	**EmitOffset** (R/W) - The offset (`Vector`) of the emission point from this `AEmitter`'s sprite center.  

- New `PresetMan` Lua function `ReloadEntityPreset(presetName, className, optionalDefinedInModule)` that allows hot-reloading `Entity` INI presets (along with all other entity presets referenced in the reloaded entity preset).  
	If the `optionalDefinedInModule` argument is not specified, the game will look through every `DataModule` to find an `Entity` preset that matches the name and type.  
	Once an `Entity` preset has been reloaded via the function, the key combination `Ctrl + F2` can be used to quickly reload it as many times as necessary.  
	Note that any changes made to the `Entity` preset will not be reflected in existing copies of the `Entity`, only in new ones created after the reload.  
	Also note that this will reload the `Entity`'s sprites (and of all other referenced entity presets), which will be reflected immediately in all existing copies of the `Entity`.
	
- New INI and Lua (R/W) `Actor` property `AIBaseDigStrength`, used to determine the strength of the terrain the `Actor` can attempt to move through without digging tools. Normally used for moving through things like terrain debris and corpses. Defaults to 35.

- New optional parameter `ignoreMaterial` for Lua function `SceneMan:CastMaxStrengthRay(start, end, skip, ignoreMaterial)`, which allows specifying a material that the ray will ignore. Defaults to the door material, for legacy compatibility purposes.

- New `Settings.ini` property `PathFinderGridNodeSize` to define the size of the pathfinder's graph nodes, in pixels. 

- New `Settings.ini` property `AIUpdateInterval` to define how often actor AI will update, in simulation updates. Higher values may give better performance with large actor counts, at a cost of AI capability and awareness.  
This can be accessed via the new Lua (R/W) `SettingsMan` property `AIUpdateInterval`.

- New Lua (R) `TimerMan` properties `AIDeltaTimeMS` and `AIDeltaTimeSecs` to get the time that has passed since the last AI update.

- Added `MOSRotating` INI property `DetachAttachablesBeforeGibbingFromWounds` that makes `Attachables` fall off before the `MOSRotating` gibs from having too many wounds, for nice visuals. Defaults to true.

- New `MOSRotating` Lua property `Gibs` (R/O) to access an iterator of the `MOSRotating`'s `Gib`s.

- Expose `Gib` to Lua.  
	You can read and write the following properties:  
	```
	gib.ParticlePreset = movableObject;
	gib.Offset = vector;
	gib.Count = intValue;
	gib.Spread = angleInRadians;
	gib.MinVelocity = floatValue;
	gib.MaxVelocity = floatValue;
	gib.LifeVariation = floatValue;
	gib.InheritsVel = bool;
	gib.IgnoresTeamHits = bool;
	gib.SpreadMode = SpreadMode;
	```

	The `SpreadMode` property accepts values from the `SpreadMode` enum:  
	```
	(0) Gib.SpreadRandom
	(1) Gib.SpreadEven
	(2) Gib.SpreadSpiral
	```

	The collection of a `MOSRotating`'s `Gib`s can be accessed via `mosRotating.Gibs`.

- New `Settings.ini` property `ServerUseDeltaCompression = 0/1` to enable delta compression in dedicated server mode which reduces bandwidth usage. Enabled by default.

- New `Settings.ini` property `SubPieMenuHoverOpenDelay` that determines how long, in milliseconds,a `PieSlice` with a sub-`PieMenu` must be hovered over for the sub-`PieMenu` to automatically open. Default is 1000 milliseconds.

- Added `PieSlice` Lua function `ReloadScripts()`. Works the same as the `MovableObject` function, but for `PieSlice`s.

- Added key combinations for resetting time scales to defaults while performance stats are visible.  
	`Ctrl + 1` to reset the time scale.  
	`Ctrl + 3` to reset the `RealToSimCap`.  
	`Ctrl + 5` to reset the `DeltaTime`.

- Added `Alt + P` key combination for toggling advanced performance stats (graphs) visibility while performance stats are visible.

- Added new `UPS` (Updates per second) measurement to the performance stats which is probably the most reliable performance indicator.  
	The sim update target is ~60 UPS (defined by `DeltaTime`).  
	When UPS dips due to load there will be noticeable FPS impact because more time is spent updating the sim and less time is left to draw frames before the next sim update.  
	When UPS dips to ~30 the FPS will be equal to UPS because there is only enough time to draw one frame before it is time for the next sim update.  
	When UPS is capped at the target, FPS will be greater than UPS because there is enough time to perform multiple draws before it is time for the next sim update.  
	Results will obviously vary depending on system performance.

- Added `LuaMan` Lua functions `GetDirectoryList(pathToGetDirectoryNamesIn)` and `GetFileList(pathToGetFileNamesIn)`, that get the names of all directories or files at the specified file path.  

- Added a new Lua scripted function for `Actor`s: `OnControllerInputModeChange(self, previousControllerMode, previousControllingPlayer)` that triggers when an `Actor`'s `Controller`'s input state changes (between AI/Player/Network control etc). This provides a script hook that fires when a player starts/stops controlling an `Actor`.

- Added `ACrab` INI properties for setting individual foot `AtomGroup`s, as opposed to setting the same foot `AtomGroup`s for both `Legs` on the left or right side.  
	These are `LeftFGFootGroup`, `LeftBGFootGroup`, `RightFGFootGroup` and `RightBGFootGroup`.

- Buy Menu Quality-of-Life improvements:  
	Shift-clicking an item (or Shift + Fire in keyboard-only) in the cart will now empty the entire cart.  
	Items in the cart will be indented to signify what actor's inventory they belong to.  
	Middle-clicking (or pressing the Pickup key) on an item will duplicate it. This also duplicates an actor's inventory.  
	You can now reorganize the cart by click-dragging. For kbd-only you can do this by holding the sharp aim key and pressing up/down.

- Added to Lua enum `ControlState` the state `RELEASE_FACEBUTTON`.

- Added screen-shake. The screen-shake strength can be tweaked or disabled in the options menu.  
	New `MOSRotating` INI property `GibScreenShakeAmount`, which determines how much this will shake the screen when gibbed. This defaults to automatically calculating a screen-shake amount based on the energy involved in the gib.  
	New `HDFirearm` INI property `RecoilScreenShakeAmount`, which determines how much this weapon will shake the screen when fired. This defaults to automatically calculating a screen-shake amount based on the recoil energy.  

	New `Settings.ini` screen-shake properties:  
	`ScreenShakeStrength` - a global multiplier applied to screen shaking strength.  
	`ScreenShakeDecay` - how quickly screen shake falls off.  
	`MaxScreenShakeTime` - the amount of screen shake time, i.e. the maximum number of seconds screen shake will happen until ScreenShakeDecay reduces it to zero.  
	`DefaultShakePerUnitOfGibEnergy` - how much the screen should shake per unit of energy from gibbing (i.e explosions), when the screen shake amount is auto-calculated.  
	`DefaultShakePerUnitOfRecoilEnergy` - how much the screen should shake per unit of energy for recoil, when the screen shake amount is auto-calculated.  
	`DefaultShakeFromRecoilMaximum` - the maximum amount of screen shake recoil can cause, when the screen shake amount is auto-calculated. This is ignored by per-firearm shake settings.

- Added new Lua manager `CameraMan`, to handle camera movement.
	New Lua functions on CameraMan:
	```lua
	AddScreenShake(screenShakeAmount, screen); -- Can be used to shake a particular screen.
	AddScreenShake(screenShakeAmount, position); -- Applies screenshake at a position in the game world. All screens looking near this position will have their screen shaken.
	```
	Several `SceneMan` Lua functions have been moved into CameraMan. For the full list, see the Changed section below.

- Added `MovableMan` Lua functions `GetMOsInRadius(position, radius, ignoreTeam, getsHitByMOsOnly)` and `GetMOsInBox(box, ignoreTeam, getsHitByMOsOnly)` that'll return all of the MOs either within a circular radius of a position, or in an axis-aligned-bounding-box. The `ignoreTeam` parameter defaults to `Team.NOTEAM`. The `getsHitByMOsOnly` defaults to false, which will get every `MovableObject`.

- `SceneMan`s `GetMOIDPixel(x, y, ignoreTeam)` Lua function has a new optional `ignoreTeam` parameter. This defaults to `Team.NOTEAM`.

- Added alternative `Actor` Lua function `RemoveInventoryItem(moduleName, presetName)`, that lets you specify the module and preset name of the inventory item, instead of just the preset name.

- Added alternative `AHuman` Lua function `EquipNamedDevice(moduleName, presetName, doEquip)`, that lets you specify the module and preset name of the `HeldDevice` to equip, instead of just the preset name.

- Added Lua access (R/W) to `Attachable` property `DeleteWhenRemovedFromParent`, which determines whether the given `Attachable` should delete itself when it's removed from its current parent.

- Added Lua convenience function `RoundToNearestMultiple(num, multiple)` which returns a number rounded to the nearest specified multiple.  
	Note that this operates on integers, so fractional parts will be truncated towards zero by type conversion.

- Added `Actor` INI and Lua property (R/W) `PlayerControllable`, that determines whether the `Actor` can be swapped to by human players. Note that Lua can probably break this, by forcing the `Controller`s of `Actor`s that aren't `PlayerControllable` to the `CIM_PLAYER` input mode.

- Added alternative `MovableMan:GetClosestTeamActor(team, player, scenePoint, maxRadius, getDistance, onlyPlayerControllableActors, actorToExclude)` that acts like the existing version, but allows you to specify whether or not to only get `Actors` that are `PlayerControllable`.

- New `Attachable` INI and Lua property `IgnoresParticlesWhileAttached`, which determines whether the `Attachable` should ignore collisions (and penetrations) with single-atom particles. Useful for preventing `HeldDevice`s from being destroyed by bullets while equipped.

- Added `AHuman` INI and Lua (R/W) property `DeviceArmSwayRate`, that defines how much `HeldDevices` will sway when walking. 0 is no sway, 1 directly couples sway with leg movement, >1 may be funny. Defaults to 0.75.

- Added `AHuman` INI and Lua (R/W) property `ReloadOffset`, that defines where `Hands` should move to when reloading, if they're not holding a supported `HeldDevice`.

- Added `AHuman` Lua function `FirearmsAreReloading(onlyIfAllFirearmsAreReloading)` which returns whether or not this `AHuman`'s `HeldDevices` are currently reloading. If the parameter is set to true and the `AHuman` is holding multiple `HeldDevices`, this will only return true if all of them are reloading.

- Added `AHuman` Lua function `ReloadFirearms(onlyReloadEmptyFirearms)`. This behaves the same as the pre-existing `ReloadFirearms` function, but if the parameter is set to true, only `HDFirearms` that are empty will be reloaded.

- Added `AHuman` Lua property (R/W) `UpperBodyState`, that lets you get and set the `AHuman`'s `UpperBodyState`. If you don't know what this does, you probably don't need or want it.

- Added `AHuman` Lua property (R/W) `MovementState`, that lets you get and set the `AHuman`'s `MovementState`. If you don't know what this does, you probably don't need or want it.

- Added `AHuman` Lua property (R/W) `ProneState`, that lets you get and set the `AHuman`'s `ProneState`. If you don't know what this does, you probably don't need or want it.

- Added `Actor` Lua property (R/W) `LimbPushForcesAndCollisionsDisabled`. If this is true, any of the `Actor`'s `Arm`s and `Leg`s won't do any movement or collide with terrain, and will just swing around with momentum.

- Added `HeldDevice` INI and Lua (R/W) property `DualReloadable`, that determines whether or not a one-handed `HeldDevice` can be dual-reloaded (i.e. old reload behaviour). Note that for dual-reload to happen, both equipped `HDFirearms` must have this flag enabled.

- Added `HeldDevice` INI and Lua (R/W) property `OneHandedReloadTimeMultiplier`, that determines how much faster or slower an `HeldDevice` is when reloading one-handed (i.e. if it's one-handed and the other `Arm` is missing, or is holding something).

- Added `HeldDevice` INI and Lua (R/W) property `Supportable`, that determines whether or not a `HeldDevice` can be supported by a background `Arm`.

- Added `Timer` Lua function `GetSimTimeLimitMS()` that gets the sim time limit of the `Timer` in milliseconds.

- Added `Timer` Lua function `GetSimTimeLimitS()` that gets the sim time limit of the `Timer` in seconds.

- Ground pressure calculations revamp. Ground pressure, by default, is now calculated using a pseudo-3d model which takes into account the depth of an object colliding with terrain. This means actors cause much less terrain deformation from walking, especially large actors walking on smooth ground.

	New `AtomGroup` INI property `AreaDistributionType`, with the following options: 
	```
	Linear // Legacy CC behaviour. This is best for long but flat objects, like guns.
	Circle // Calculates ground pressure assuming objects are cylindrical with a diameter equal to the object's width. This is best for roundish objects, like feet.
	Square // Similar to Circle, but instead assuming that the object is a cuboid with a depth equal to the object's width.
	```
	New `AtomGroup` INI propery `AreaDistributionSurfaceAreaMultiplier`. This is a multiplier to the calculated surface area, i.e how "blunt" an object is. Large values lead to objects having lower ground pressure, and so dig into the terrain less. 
	
	The default values are `AreaDistributionType = Circle` and `AreaDistributionSurfaceAreaMultiplier = 0.5`, meaning that an object is assumed to be an oval with a depth of half it's width.

- New `SceneMan` Lua function `DislodgePixel(posX, posY)` that removes a pixel of terrain at the passed in coordinates and turns it into a `MOPixel`. Returns the dislodged pixel as a `MovableObject`, or `nil` if no pixel terrain was found at the passed in position.

- New `HDFirearm` Lua property `CanFire` which accurately indicates whether the firearm is ready to fire off another round.

- New `HDFirearm` Lua property `MSPerRound` which returns the minimum amount of MS in between shots, relative to `RateOfFire`.

- New `HDFirearm` INI and Lua (R/W) properties `ReloadAngle` and `OneHandedReloadAngle` which determine the width of the reload animation angle in radians, the latter being used when the device is held with no supporting `Arm` available. 0 means the animation is disabled. 

- New `HDFirearm` Lua (R) property `CurrentReloadAngle` which gets the reload angle being currently used. I.e. the `ReloadAngle` when there's a supporting `Arm` available, and the `OneHandedReloadAngle` when there isn't.

- New `HDFirearm` Lua property `MSPerRound` which returns the minimum amount of MS in between shots, relative to`RateOfFire`.

- New `Attachable` INI and Lua (R/W) property `GibWhenRemovedFromParent` which gibs the `Attachable` in question when it's removed from its parent. `DeleteWhenRemovedFromParent` will always override this.

- New `Settings.ini` property `AutomaticGoldDeposit` which determines whether gold gathered by actors is automatically added into the team's funds. False means that gold needs to be manually transported into orbit via craft, the old school way. Enabled by default.  
	A noteworthy change in comparison to previous logic is that gold is no longer converted into objects, and `GoldCarried` is now automatically transferred into craft upon entering them. This effectively allows the same actor to resume prospecting without having to return to orbit with the craft.  
	Regardless of the setting, this behavior is always disabled for AI-only teams for the time being, until the actor AI is accommodated accordingly.

- New `Actor` Lua function `AddGold(goldOz)` which adds the passed-in amount of gold either to the team's funds, or the `GoldCarried` of the actor in question, depending on whether automatic gold depositing is enabled. This effectively simulates the actor collecting gold.

- New `Actor` Lua function `DropAllGold()` which converts all of the actor's `GoldCarried` into particles and spews them on the ground.

- Added `Alt + F2` key combination to reload all cached sprites. This allows you to see changes made to sprites immediately in-game.

- New `MovableObject` Lua (R) property `DistanceTravelled` which returns the amount of pixels the object has travelled since its creation.

- Added `Activity` Lua function `ForceSetTeamAsActive(team)`, which forcefully sets a team as active. Necessary for `Activity`s that don't want to define/show all used teams, but still want `Actor`s of hidden teams to work properly.

- Added `GameActivity` INI property `DefaultGoldMaxDifficulty`, which lets you specify the default gold when the difficulty slider is maxed out.

- Added `HDFirearm` Lua (R/W) property `BaseReloadTime` that lets you get and set the `HDFirearm`'s base reload time (i.e. the reload time before it's adjusted for one-handed reloads where appropriate).

- Added `Actor` INI and Lua property (R/W) `PlayerControllable`, that determines whether the `Actor` can be swapped to by human players. Note that Lua can probably break this, by forcing the `Controller`s of `Actor`s that aren't `PlayerControllable` to the `CIM_PLAYER` input mode.

- Added alternative `MovableMan:GetClosestTeamActor(team, player, scenePoint, maxRadius, getDistance, onlyPlayerControllableActors, actorToExclude)` that acts like the existing version, but allows you to specify whether or not to only get `Actors` that are `PlayerControllable`.

- Added new `Timer` constructors `timer = Timer(elapsedSimTimeMS)` and `timer = Timer(elapsedSimTimeMS, simTimeLimitMS)` that let you setup `Timer`s more cleanly.

- Added `Timer` Lua (R) properties `RealTimeLimitProgress` and `SimTimeLimitProgress`, that get how much progress the `Timer` has made towards its `RealTimeLimit` or `SimTimeLimit`. 0 means no progress, 1.0 means the timer has reached or passed the time limit.

- New `Settings.ini` property `EnableVSync` to enable vertical synchronization. Enabled by default.

- New `Settings.ini` property `IgnoreMultiDisplays` to ignore all displays except the one the window is currently positioned at when changing resolution.

- Added Lua (R/W) properties for `ACrab` `AimRangeUpperLimit` and `AimRangeLowerLimit`. INI bindings already existed and are mentioned in an earlier changelog entry.

- Added `TerrainObject` INI property `ClearChildObjects` that lets you clear child objects when doing a `CopyOf` of another `TerrainObject`.

- Added `Actor` Lua (R) property `MovePathEnd` that gets you the last point in the `Actor`'s move path.

- Added `Actor` Lua (R) property `SceneWaypoints` that lets you iterate over the `Actor`'s scene waypoints.

- Added `MovableObject` Lua (R) property `HasEverBeenAddedToMovableMan` that tells you whether or not the `MovableObject` has ever been added to `MovableMan`.

- Added alternate version of `Scene` Lua function `CalculatePath(startPos, endPos, movePathToGround, digStrength, team)` that works as the previous one, but lets you specify the team to calculate the path for, allowing you to ignore doors on your team.

- Added `Controller` Lua function `IsKeyboardOnlyControlled` that tells you whether the `Controller` is being controlled by keyboard only. Previously the only way to do this was to check that it's not mouse controlled and not gamepad controlled.

- Added `Controller` control state `PIE_MENU_OPENED` that is true for the first Update in which the `PieMenu` is opened.

- Added `Activity` Lua function `GetPlayerController`, which gets you the `Controller` used for GUI stuff and when there's no `Actor` selected in an `Activity`. Be aware, it's very likely possible to cause problems by doing dumb things with this.

</details>

<details><summary><b>Changed</b></summary>

- Codebase now uses the C++20 standard.

- Dramatic performance enhancements, especially with high actor counts and large maps. FPS has been more-than-doubled.

- Greatly reduce online multiplayer bandwidth usage.

- Swapped MoonJIT to LuaJIT. Compiled from [d0e88930ddde28ff662503f9f20facf34f7265aa](https://github.com/LuaJIT/LuaJIT/commit/d0e88930ddde28ff662503f9f20facf34f7265aa).

- Swapped to SDL2 for window management and input handling.

- `Settings.ini` and player loadouts (BuyMenu presets) are now stored in the `Userdata` directory instead of `Base.rte`.

- Lua scripts are now run in a more efficient way. As part of this change, `PieSlice` scripts need to be reloaded like `MovableObject` scripts (i.e. using `pieSlice:ReloadScripts()`, in order for their changes to be reflected in-game.  
	`PresetMan:ReloadAllScripts()` will reload `PieSlice` preset scripts, like it does for `MovableObject`s.

- The landing zone cursor will now show the width of the selected delivery craft.

- Completely replaced `ScriptFile` with `ScriptPath`.

- Changed the `Vector` function `ClampMagnitude` so its parameter order makes sense, it's now `Vector:ClampMagnitude(lowerMagnitudeLimit, upperMagnitudeLimit)`.

- Changed the `MovableMan` function `AddItem` so it now only accepts `HeldDevice`s and sub-classes (i.e. `HDFirearm`, `ThrownDevice`, `TDExplosive`), because it always expected that anyway, and it's good to enforce it.

- `Scene` background layer presets in INI are now defined as `SLBackground` rather than `SceneLayer`.

- `TerrainDebris` INI property `OnlyOnSurface = 0/1` replaced with `DebrisPlacementMode = 0-6`.  
	Placement modes are:  
	```
	0 (NoPlacementRestrictions) // Debris will be placed anywhere where there is target material (currently not strictly enforced when being offset by min/max depth, so can end up being placed mid-air/cavity unless set to OnlyBuried = 1).

	1 (OnSurfaceOnly) // Debris will be placed only on the surface (scanning from top to bottom) where no background cavity material (material index 1) was encountered before the target material.

	2 (OnCavitySurfaceOnly) // Debris will be placed only on the surface (scanning from top to bottom) where background cavity material (material index 1) was encountered before the target material.

	3 (OnSurfaceAndCavitySurface) // Debris will be placed only on the surface (scanning from top to bottom) regardless whether background cavity material (material index 1) was encountered before the target material.

	4 (OnOverhangOnly) // Debris will be placed only on overhangs (scanning from bottom to top) where no background cavity material (material index 1) was encountered before the target material.

	5 (OnCavityOverhangOnly) // Debris will be placed only on overhangs (scanning from bottom to top) where background cavity material (material index 1) was encountered before the target material.

	6 (OnOverhangAndCavityOverhang) // Debris will be placed only on overhangs (scanning from bottom to top) regardless whether background cavity material (material index 1) was encountered before the target material.
	```

- `Material` INI property `TextureFile` renamed to `FGTextureFile` to accommodate new background texture property.

- `TerrainObject`s no longer have a hard requirement for `FG` and `Mat` layer sprites. Any layer may be omitted as long as at least one is defined.

- `Scene` layer data will now be saved as compressed PNG to reduce file sizes of MetaGame saves and is threaded to prevent the game from freezing when layer data is being saved. 

- Lua function `BuyMenuGUI:SetHeaderImage` renamed to `SetBannerImage`.

- Lua functions run by `PieSlice`s will now have the following signature: `pieSliceFunction(pieMenuOwner, pieMenu, pieSlice)`. The details for these are as follows:  
	`pieMenuOwner` - The `Actor` owner of this `PieMenu`, or the `MovableObject` affected object of it if it has no owner.
	`pieMenu` - The `PieMenu` that is being used, and is calling this function. Note that this may be a sub-`PieMenu`.  
	`pieSlice` - The `PieSlice` that has been activated to call this function.  

- `OnPieMenu(self)` event function has been changed to `WhilePieMenuOpen(self, openedPieMenu)` and will run as long as the `PieMenu` is open.

- Any `Attachable` on an `Actor` (not just `HeldDevice`s) can now have a `WhilePieMenuOpen(self, openedPieMenu)` function, and can add `PieSlice`s and run functions when they're pressed.

- `PieSliceIndex` enum has been renamed to `SliceType` to better match the source. More relevantly for modders, its values have also been renamed, they are as follows:	 
	```
	(0)  NoType

	// The following are used for inventory management:
	(1)  Pickup
	(2)  Drop
	(3)  NextItem
	(4)  PreviousItem
	(5)  Reload

	// The following are used for menu and GUI activation:
	(6)  BuyMenu
	(7)  FullInventory
	(8)  Stats
	(9)  Map
	(10) Ceasefire

	// The following is used for squad management:
	(11) FormSquad

	// The following are used for AI mode management:
	(12) AIModes
	(13) Sentry
	(14) Patrol
	(15) BrainHunt
	(16) GoldDig
	(17) GoTo
	(18) Return
	(19) Stay
	(20) Deliver
	(21) Scuttle

	// The following are used for game editors:
	(22) EditorDone
	(23) EditorLoad
	(24) EditorSave
	(25) EditorNew
	(26) EditorPick
	(27) EditorMove
	(28) EditorRemove
	(29) EditorInFront
	(30) EditorBehind
	(31) EditorZoomIn
	(32) EditorZoomOut
	(33) EditorTeam1
	(34) EditorTeam2
	(35) EditorTeam3
	(36) EditorTeam4
	```

- Major improvements to pathfinding performance and AI decision making.

- Having the pie menu open no longer blocks user input when using mouse+keyboard or a controller.

- `MOSRotating` based presets without an `AtomGroup` definition will now crash with error message during loading.

- Over-indentation in INI will crash with error message if detected during loading instead of skipping entire blocks or in some cases the rest of the file.  
	There is never a case where there should be a positive difference of more than one tab between lines, but this means that lines like `IncludeFile` which would previously load fine even if over-indented will also crash, but you should never have those indented to begin with.  

	Examples of structure that will cause a crash:  
	```ini
	AddSomething = Something
			PresetName = Thing // Over-indented. Will crash.
		SomeProperty = Whatever

	AddSomething = Something
		PresetName = Thing
		SomeObjectProperty = Something
				CopyOf = Thing // Over-indented. Will crash.
	```

- Improve accuracy of the `MSPF` measurement in performance stats, which also improves the accuracy of the `FPS` measurement.  
	The `MSPF` measurement now displays 3 values:  
	`Frame` (previously `MSPF`) - The total frame time (game loop iteration), in milliseconds.  
	`Update` - The total time spent updating the sim during the frame (as the sim can be updated multiple times per frame), in milliseconds.  
	`Draw` - The time spend drawing during the frame, in milliseconds.

- Advanced performance stats (graphs) will now scale to `RealToSimCap`.

- The keyboard shortcut for clearing the console is now `F10`, since `F5` is used for quick-saving (`F9` quick-loads).

- `BitmapPrimitive` drawing functions now accept `MOSprite` instead of `Entity` for the object they get the bitmap to draw from.  
	This changes nothing regarding the bindings, but will now print an error to the console when attempting to draw a non-`MOSprite` based object (e.g. `MOPixel`), instead of silently skipping it.

- Unless set to dual-reload, one-handed `HDFirearms` will now reload one-at-a-time. To maintain this behaviour in Lua scripts, it is recommend to use `AHuman:ReloadFirearms()` instead of reloading `HeldDevices` directly, as the latter will ignore restrictions.

- Made a lot of changes to `Arms` - they can now only hold `HeldDevices` (and subclasses like `HDFirearms` and `ThrownDevices`), and `AHumans` have a lot of `Arm` animations, including sway when walking and holding something, smoother `Arm` movement and reload animations.  
	They now have the following INI properties:  
	```
	MaxLength - The max length of the Arm in pixels.
	MoveSpeed - How quickly the Arm moves between targets. 0 means no movement, 1 means instant movement.
	HandIdleOffset - The idle offset this Arm's hand will move to if it has no targets, and nothing else affecting its idle offset (e.g. it's not holding or supporting a HeldDevice). IdleOffset is also allowed for compatibility.
	HandSprite - The sprite file for this Arm's hand. Hand is also allowed for compatibility.
	GripStrength - The Arm's grip strength when holding HeldDevices. Further described below, in the entry where it was added.
	ThrowStrength - The Arm's throw strength when throwing ThrownDevices. Further described below, in the entry where it was added.
	HeldDevice - Allows you to set the HeldDevice attached to this Arm.
	```
	They now have the following Lua properties and functions:  
	**`MaxLength`** (R) - Allows getting the `Arm`'s maximum length.  
	**`MoveSpeed`** (R/W) - Allows getting and setting the `Arm`'s movement speed. 0 means no movement, 1 means instant movement.  
	**`HandIdleOffset`** (R/W) - Allows getting and setting the `Arm`'s default idle hand offset, i.e. where the hand will go when it has no targets and isn't holding or supporting anything.  
	**`HandPos`** (R/W) - Gets and sets the current position of the hand. Note that this will override any animations and move the hand to the position instantly, so it's generally not recommended.  
	**`HasAnyHandTargets`** (R) - Gets whether or not this `Arm` has any hand targets, i.e. any positions the `Arm` is supposed to try to move its hand to.  
	**`NumberOfHandTargets`** (R) - Gets the number of hand targets this `Arm` has.  
	**`NextHandTargetDescription`** (R/W) - Gets the description of the next target this `Arm`'s hand is moving to, or an empty string if there are no targets.  
	**`NextHandTargetPosition`** (R/W) - Gets the position of the next target this `Arm`'s hand is moving to, or `Vector(0, 0)` if there are no targets.  
	**`HandHasReachedCurrentTarget`** (R) - Gets whether or not this `Arm`'s hand has reached its current target. This may not be reliably accessible from Lua since it can often get reset before being read from Lua, if the target has no delay. Note that this will be true if there are no targets but that hand has reached its appropriate idle offset.  
	**`GripStrength`** (R/W) - Gets and sets the `Arm`'s grip strength when holding `HeldDevices`. Further described below, in the entry where it was added.  
	**`ThrowStrength`** (R/W) - Gets and sets the `Arm`'s throw strength when throwing `ThrownDevices`. Further described below, in the entry where it was added.  
	**`HeldDevice`** (R/W) - Gets and sets the `HeldDevice` held by this `Arm`.  
	**`SupportedHeldDevice`** (R) - Gets the `HeldDevice` this `Arm` is supporting. For obvious reasons, this will be empty if this is not the BG `Arm` or if it has a `HeldDevice` of its own.  
	**`AddHandTarget(description, positionOnScene)`**  - Adds a target for this `Arm`'s hand to move to. The target goes to the back of the queue, allowing for multiple animations to be added in succession. The description is arbitrary, but useful for identification, and if the target being added has the same description as the target at the end of the queue, they will be merged to avoid duplication.  
	**`AddHandTarget(description, positionOnScene, delayAtTarget)`**  - Adds a target for this `Arm`'s hand to move to as above, but the hand will wait at the target for the specified amount of time.  
	**`RemoveNextHandTarget()`**  - Removes the next hand target from the queue, if there are any.  
	**`ClearHandTargets()`**  - Empties the queue of hand targets. Once the queue is empty, the hand will move towards its appropriate idle offset.

- The following `SceneMan` functions have been moved to `CameraMan`:
	```lua
	SetOffset(offsetVector, screenId);
	GetScreenOcclusion(screenId);
	SetScreenOcclusion(occlusionVector, screenId);
	GetScrollTarget(screenId);
	SetScrollTarget(targetPosition, screenId);
	TargetDistanceScalar(point);
	CheckOffset(screenId);
	SetScroll(center, screenId);
	```

- `HDFirearm` Lua property `ReloadTime` is no longer writable. Use `BaseReloadTime` instead. INI property `ReloadTime` has been renamed to `BaseReloadTime`, though `ReloadTime` still works as well.

- `GameActivity` default gold INI properties have been renamed, so they all have `Difficulty` at the end. The full set of properties is:  
	`DefaultGoldCakeDifficulty`, `DefaultGoldEasyDifficulty`, `DefaultGoldMediumDifficulty`, `DefaultGoldHardDifficulty`, `DefaultGoldNutsDifficulty`, `DefaultGoldMaxDifficulty`.

- `UInputMan` Lua functions `KeyPressed`, `KeyReleased` and `KeyHeld` now take `SDL_Keycode` values instead of Allegro scancodes.  
	Keycodes take keyboard layout into account and should be the preferred way of detecting input.

	If detecting by scancode (physical key location independent of layout) is absolutely necessary, the following functions have been added:  
	`ScancodePressed`, `ScancodeReleased`, `ScancodeHeld`

	Info on the keycode and scancode Lua tables and how to access them be found here: [SDL Keycode and Scancode enum values in Lua](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data/wiki/SDL-Keycode-and-Scancode-enum-values-in-Lua).

- Replace `PrintScreen` with `F12` for dumping a single screenshot, as `PrintScreen` was unreliable.

- `AHuman`, `ACrab` and `ACRockt` will now attempt to fallback to using each `Leg`'s `Foot` attachable's `AtomGroup` as the appropriate `FootGroup`.  
	This allows using auto-generated `AtomGroup`s instead of manually defining each `Atom` in a `FootGroup` when creating actors with larger or irregularly shaped feet simply by removing the `FootGroup` properties from the actor preset.

- Failing to create actor `FootGroup`s during loading will now crash with error message instead of straight to desktop.

- `Gib` property `InheritsVel` now works as a `float` scalar from 0 to 1, defining the portion of velocity inherited from the parent object.

- Jetpack burst fuel consumption is now scaled according to the total burst size instead of always being tenfold.  
	Bursts during downtime from burst spacing are now less punishing, scaling according to half of the burst size.

- New `Activity` Lua function `activity:SetPlayerHadBrain(player, whetherOrNotPlayerHadBrain)`, which sets whether or not the given player had a brain. Probably mostly useful for dealing with loading a game with multiple players, where one player is dead and you have to sort out brain assignment.

- Changed `LuaMan:FileOpen` access modes so it only allows `"r", "r+", "w", "w+", "a", "a+"`, i.e. specifying type (text, binary) is not supported. See [this reference page](https://cplusplus.com/reference/cstdio/fopen) for details on the access modes.

</details>

<details><summary><b>Fixed</b></summary>

- Improved support for varied resolutions and aspect ratios. 1366x768 users rejoice.

- Multi-display fullscreen now works regardless of window position or which screen in the arrangement is set as primary.  
	Still limited to horizontal arrangements that are top or bottom edge aligned (or anywhere in between for arrangements with different height displays).

- Controller hot-plug and disconnect is now properly detected at any point and will attempt to reconnect devices to the same gamepad slot.

- Fixed material view not drawing correctly when viewed in split-screen. ([Issue #54](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/54))

- Fix `TerrainObject`s not wrapping when placed over the Y seam on Y-wrapped scenes.

- Fix black striping in online multiplayer when client screen width isn't divisible by transmitted box width.

- Fixed issue where actors refused to pathfind around enemy doors. ([Issue #396](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/396))

- Fix advanced performance stats (graphs) peak values stuck at 0.

- Fix `MOSRotating` `GetWounds()` Lua function missing its implementation.

- Fixed `Entity.ModuleName` returning an empty string for `Entities` defined in `Base.rte`. They now return "Base.rte", as they should.

- Fixed `MOSRotating`s registering all penetrations in one frame even when exceeding gibbing conditions. They now omit all collisions after being flagged for deletion, allowing particles like grenade fragments to penetrate other objects.

</details>

<details><summary><b>Removed</b></summary>

- Removed `SLTerrain` and `SLBackground` INI property `Offset`. Internal and shouldn't have been exposed.

- Removed `SLTerrain` INI alt property `AddTerrainObject`. Use `PlaceTerrainObject` for consistency with similar properties.

- Removed `TerrainObject` INI property `DisplayAsTerrain`. Wasn't implemented and did nothing.

- Removed `SceneMan` Lua function `AddTerrainObject`. `SceneMan:AddSceneObject` should be used instead.

- Removed `Activity` Lua function `EnteredOrbit`. This tells the `Activity` to consider an `ACraft` as having entered orbit, and should never actually have been accessible to Lua.

- Removed `OnPieMenu` listeners for `Activity`s and `GlobalScript`s, and removed the `ProvidesPieMenuContext` concept and everything around it. These things should no longer be necessary since you can modify `PieMenu`s on the fly at any time, and they made this already complex set of code even more complicated.

- Removed `SceneMan` Lua functions `SetOffsetX(x, screenId)` and `SetOffsetY(y, screenId)`. Use `CameraMan:SetOffset(offsetVector, screenId)` instead.

- Removed `whichStick` parameter for the following `UInputMan` Lua functions:  
	`JoyDirectionPressed`, `JoyDirectionReleased`, `JoyDirectionHeld`, `AnalogAxisValue`  
	No longer used or meaningful.

- Removed `UInputMan` Lua function `WhichKeyHeld`.

- Dedicated fullscreen has been removed (again) along with the following `Settings.ini` properties:
	`ForceVirtualFullScreenGfxDriver`, `ForceDedicatedFullScreenGfxDriver`

</details>

***

## [0.1.0 pre-release 4.0][0.1.0-pre4.0] - 2022/02/28

<details><summary><b>Added</b></summary>

- Executable can be compiled as 64bit.

- New `Settings.ini` property `MeasureModuleLoadTime = 0/1` to measure the duration of module loading (archived module extraction included). For benchmarking purposes.

- `Color` object's RGB values can now be set with index number.  
	```
	Color/TrailColor = Color
		Index = 0-255 // Corresponds with index in palette.bmp
	```

- New `Settings.ini` property `ForceDedicatedFullScreenGfxDriver` to force the game to run in previously removed dedicated fullscreen mode, allowing using lower resolutions (and 1366x768) while still maintaining fullscreen.

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

- Added `MOSRotating` `GibBlastStrength` INI and Lua (R/W) property. This lets you define how much force created `Gibs` and any `Attachables` will get launched with when the `MOSRotating` gibs.

- New INI and Lua (R/W) properties for `Attachables`:  
	`ParentBreakWound = AEmitter...` - allows you to optionally define different `BreakWounds` for the `Attachable` and its parent. By default it matches `BreakWound` for ease of use. `BreakWound` is also now R/W accessible to Lua.  
	`InheritsHFlipped = -1/0/1` - allows you to define whether the `Attachable` will inherit its parent's HFlipped value or not.  
	-1 means reversed inheritance (i.e. if the parent's HFlipped value is true, this `Attachable`'s HFlipped value will be false), 0 means no inheritance, 1 means normal inheritance. Defaults to 1 to preserve normal behavior.  
	`InheritedRotAngleRadOffset = angle` - and `InheritedRotAngleDegOffset = angle` allow you specify an offset to keep an `Attachable`'s rotation at when `InheritsRotAngle` is set to true. For example, `InheritedRotAngleDegOffset = 90` would make the `Attachable` always face perpendicular to its parent. In Lua there's only `InheritedRotAngleOffset`, which takes/returns radians to avoid confusion. Note that this property does nothing if the `Attachable`'s `InheritsRotAngle` is set to false or the `Attachable` has no parent.  
	`GibWithParentChance = 0 - 1` - allows you to specify whether this `Attachable` should be gibbed when its parent does and what the chance of that happening is. 0 means never, 1 means always.  
	`ParentGibBlastStrengthMultiplier = number` - allows you to specify a multiplier for how strongly this `Attachable` will apply its parent's gib blast strength to itself when the parent gibs. Usually this would be a positive number, but it doesn't have to be.

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
	```lua
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
	**`Actor`** - `BodyHitSound`, `PainSound`, `DeathSound`, `DeviceSwitchSound`, `AlarmSound`  
	**`AHuman & ACrab`** - `StrideSound`  
	**`HDFirearm`** - `FireSound`, `FireEchoSound`, `EmptySound`, `ReloadStartSound`, `ReloadEndSound`, `ActiveSound`, `DeactivationSound`, `PreFireSound`  
	**`AEmitter`** - `EmissionSound`, `BurstSound`, `EndSound`  
	**`ACraft`** - `HatchOpenSound`, `HatchCloseSound`, `CrashSound`  
	**`MOSRotating`** - `GibSound`  
	**`ADoor`** - `DoorMoveStartSound`, `DoorMoveSound`, `DoorDirectionChangeSound`, `DoorMoveEndSound`
  
- Added Lua function `RoundFloatToPrecision`. Utility function to round and format floating point numbers for display in strings.  
`RoundFloatToPrecision(floatValue, digitsPastDecimal, roundingMode) -- Rounding mode 0 for system default, 1 for floored remainder, 2 for ceiled remainder, 3 for ceiled remainder with the last decimal place rounded to the nearest 0 or 5 (i.e. if option 2 gave 10.1, this would give 10.5, if it gave 10.369 this would give 10.370, etc.)`

- The Lua console (and all text boxes) now support using `Ctrl` to move the cursor around and select or delete text.

- Added `mosRotating:RemoveAttachable(attachableOrUniqueID, addToMovableMan, addBreakWounds)` method that allows you to remove an `Attachable` and specify whether it should be added to `MovableMan` or not, and whether breakwounds should be added (if defined) to the `Attachable` and parent `MOSRotating`. This method returns the removed `Attachable`, see the `Changed` section for important details on that.

- Added `mosRotating:RemoveEmitter(attachableOrUniqueID, addToMovableMan, addBreakWounds)` method that is identical to the `RemoveAttachable` function mentioned above.  

- Added `attachable:RemoveFromParent()` and `attachable:RemoveFromParent(addToMovableMan, addBreakWounds)` that allow you to remove `Attachables` from their parents without having to use `GetParent` first. Their return value is the same as `RemoveAttachable` above.

- Added `Settings.ini` debug properties to allow modders to turn on some potentially useful information visualizations.  
	`DrawAtomGroupVisualizations` - any `MOSRotating` will draw its `AtomGroup` to the standard view.  
	`DrawHandAndFootGroupVisualizations` - any `Actor` subclasses with  will draw its hand and foot `AtomGroup`s to the standard view.  
	`DrawLimbPathVisualizations` - any  `AHumans` or `ACrabs` will draw some of their `LimbPaths` to the standard view.  
	`DrawRayCastVisualizations` - any rays cast by `SceneMan` will be drawn to the standard view.  
	`DrawPixelCheckVisualizations` - any pixel checks made by `SceneMan:GetTerrMatter` or `SceneMan:GetMOIDPixel` will be drawn to the standard view.

- Added a fully featured inventory view for managing `AHuman` inventories (to be expanded to other things in future).

- New `Settings.ini` property `CaseSensitiveFilePaths = 0/1` to enable/disable file path case sensitivity in INIs. Enabled by default.  
	It is **STRONGLY** ill-advised to disable this behavior as it makes case sensitivity mismatches immediately obvious and allows fixing them with ease to ensure a path related crash free cross-platform experience.  
	Only disable this if for some reason case sensitivity increases the loading times on your system (which it generally should not). Loading times can be benchmarked using the `Settings.ini` property `MeasureModuleLoadTime`. The result will be printed to the console.

- Added `MovableObject` Lua function `EnableOrDisableAllScripts` that allows you to enable or disable all scripts on a `MovableObject` based on the passed in value.

- Added `AEmitter` and `PEmitter` Lua (R/W) properties `NegativeThrottleMultiplier` and `PositiveThrottleMultiplier` that affect the emission rate relative to throttle.

- Added `Attachable` Lua function and INI property `InheritsFrame` which lets `Attachables` inherit their parent's frame. It is set to false by default.

- Added `MovableObject` Lua (R/W) and INI properties `ApplyWoundDamageOnCollision` and `ApplyWoundBurstDamageOnCollision` which allow `MovableObject`s to apply the `EntryWound` damage/burst damage that would occur when they penetrate another object, without actually creating a wound.

- `Turret`s can now support an unlimited number of mounted `HeldDevice`s. Properties have been added to Lua and INI to support this:  
	`AddMountedDevice = ...` (INI) and `turret:AddMountedDevice` (Lua) - this adds the specified `HeldDevice` or `HDFirearm` as a mounted device on the `Turret`.  
	`turret:GetMountedDevices` (Lua) - this gives you access to all the mounted `HeldDevice`s on the `Turret`. You can loop through them with a for loop, and remove or modify them as needed.  
	Note that `MountedDevice = ...` (INI) and `turret.MountedDevice` (Lua R/W) deals with the first mounted `HeldDevice`, which is treated as the primary one for things like sharp-aiming.
	
- Added `Turret` Lua (R/W) and INI property `MountedDeviceRotationOffset` that lets you specify a standard rotation offset for all mounted `HeldDevices` on a turret.

- Added option for players to vote to restart multiplayer activities by holding the backslash key, `\`. Requires all players to vote to pass.  
	This is an alternative to the existing ability to vote to end the activity and return to the multiplayer lobby, by holding `Backspace` key.

- New `Settings.ini` properties `MuteMaster = 0/1`, `MuteMusic = 0/1` and `MuteSound = 0/1` to control muting of master/music/sound channels without changing the volume property values.	

- New `Settings.ini` property `TwoPlayerSplitscreenVertSplit = 0/1` to force two player splitscreen into a vertical split mode (horizontal by default).

- Controller hot-plugging is now supported (Windows only).

- Console text can be set to use a monospace font through `Settings.ini` property `ConsoleUseMonospaceFont = 0/1` or through the in-game settings.

- New `ThrownDevice` INI property `StrikerLever`, which is the same as `Shell` for `Round` in `HDFirearm`, but for grenades. Represents the lever/pin coming off when activated.

- New `Arm` INI and Lua (R/W) property `ThrowStrength` which now calculates how far `ThrownDevice`s are thrown, which also takes to account the weight of the device. `ThrownDevice`s can still define `MaxThrowVel` and `MinThrowVel` to override this.

- New `Settings.ini` property `DisableLuaJIT = 0/1` to disable LuaJIT (MoonJIT) to (potentially) improve performance on machines that seem to struggle with it.

- New `Settings.ini` property `ShowEnemyHUD` which allows disabling of enemy actor HUD in its entirety.

- New `DataModule` INI and Lua (R) property `IsFaction` which determines whether a module is a playable faction (in MetaGame, etc.). This replaces the need to put "Tech" in the module name. Defaults to false (0).

- New `MOSRotating` INI and Lua (R) property `WoundCountAffectsImpulseLimitRatio` which can be used to make objects more prone to gibbing from impulse when they have also received wounds.

- New `Gib` INI property `SpreadMode` which sports two new spread logic variants which alter the way velocity is applied to the `GibParticle`s when they spawn. This can be used to create richer explosion effects.  
	`SpreadMode = 0` is the default, fully randomized spread according to `MinVelocity`, `MaxVelocity` and `Spread` values. Think: a piece of grenade fragment, launching out in an arbitrary direction.  
	`SpreadMode = 1` is the same as the default, but with evenly spaced out angles. Think: an air blast shockwave, dispersing evenly outward from the explosion.  
	`SpreadMode = 2` has an entirely different behavior of its own, which utilizes the fermat spiral as means to evenly disperse the particles in a circular area, according to `MaxVelocity` and `MinVelocity`. Since this mode will always result in a full, 360-degree spread, the `Spread` property can be used to add randomization to the gib particles. Think: a cloud of smoke.

- New `Actor` INI and Lua (R/W) property `StableRecoverDelay` which determines how long it takes for an actor to regain `STABLE` status after being rendered `UNSTABLE`.

- New `AHuman` Lua (R) property `ThrowProgress` which returns the current throw chargeup progress as a scalar from 0 to 1.

- New `HDFirearm` INI and Lua (R/W) property `ShellVelVariation` which can be used to randomize the magnitude at which shells are ejected.

- New `HDFirearm` Lua (R) property `ReloadProgress` which returns the current reload progress as a scalar from 0 to 1.

- New `HDFirearm` INI and Lua (R/W) property `Reloadable` which can be used to disable the ability to reload said device.

- New `HDFirearm` Lua (R) property `RoundInMagCapacity` which returns the maximum capacity of the `Magazine` or, if there's not currently a `Magazine`, the maximum capacity of the next `Magazine`.  
	This means that the property will always return the maximum ammo capacity of the device, even when reloading.

- New `Entity` Lua (R) property `ModuleName` which returns the filename of the data module from which the entity originates from.

- `Arm`s will now react to the recoil of `HeldDevice`s. This is affected by the `Arm`'s `GripStrength` and the `HeldDevice`'s `RecoilTransmission`, in the same way as recoil itself.

- `HDFirearm` reload progress now shows up as a HUD element.

- New `Round` INI property `LifeVariation` which can be used to randomize the `Lifetime` of shot particles.

- Exposed `MOSprite` property `PrevRotAngle` to Lua (R).

- New `ACraft` INI and Lua (R/W) property `ScuttleOnDeath` which can be used to disable the automatic self-destruct sequence when the craft's health drops down to zero.

- New `Settings.ini` property `UnheldItemsHUDDisplayRange = numPixels` that hides the HUD of stranded items at a set distance. Default is 500 (25 meters).
	Value of -1 or anything below means all HUDs will be hidden and the only indication an item can be picked up will be on the `Actor`'s HUD when standing on top of it.
	Value of 0 means there is no range limit and all items on Scene will display the pick-up HUD.
	Valid range values are 1-1000, anything above will be considered as no range limit.

- Various improvements to the Buy Menu. You can now navigate tabs with the actor swap buttons, and the menu will smartly navigate when you add an `Actor` to your shop list, so you can quickly select weapons, etc..  
	There is also a new `Settings.ini` property, `SmartBuyMenuNavigation = 0/1`, which allows you to turn off this smart buy menu navigation, in case you prefer not to have it. 

- Exposed `ACraft` property `HatchDelay` to Lua (R/W).

- New `Settings.ini` property `SimplifiedCollisionDetection = 0/1` to enable more performant but less accurate MO collision detection (previously `PreciseCollisions = 0`). Disabled by default.

- New INI property `BuyableMode` to specify in which buy lists a `Buyable = 1` item should appear in.  
	`BuyableMode = 0 // No restrictions` - item will appear in both lists as usual. Default value, does not need to be explicitly specified.
	`BuyableMode = 1 // BuyMenu only` - item will not appear in any group in the object picker during the editing phase, but will be available to purchase from the buy menu.  
	`BuyableMode = 2 // ObjectPicker only` - item will not appear in any tab in the buy menu when making an order, but will be available for placement from the object picker during editing phase.

- New `MovableMan` Lua function `KillAllTeamActors`, which kills all `Actor`s on the given `Team`.

- New `ACRocket` INI property `MaxGimbalAngle`, which enables automatic stabilization via tilting of the main engine.

- Added Lua bindings for `scene.Areas` and `area.Boxes` that you can iterate through to get all the `Areas` in a `Scene` and all the `Boxes` in an `Area`.

- Added `Area` Lua function `area:RemoveBox(boxToRemove)` which removes the given `Box` from the `Area`. Note that this removal is done by comparing the `Box`'s `Corner`, `Width` and `Height`, so you're actually removing the first `Box` that matches the passed-in boxToRemove.

- Added `Area` Lua property `area.FirstBox` that returns the first `Box` in this `Area`. Useful for `Areas` that only have one `Box`.

- Added `Area` Lua properties for `area.Center` and `area.RandomPoint`. They're exactly the same as the existing `area:GetCenterPoint()` and `area:GetRandomPoint()` functions, but a bit more convenient.

- Added `SceneMan` Lua function `SceneMan:WrapBox(boxToWrap)` which takes a `Box` and, if it passes over the seam, splits it into multiple boxes and returns them. Useful for creating `Boxes` without having to worry about the seam.

- Added Lua binding for `AudioMan:StopMusic()`, which stops all playing music. `AudioMan:StopAll()` used to do this, but now it actually stops all sounds and music.

- New `Actor` Lua (R) property `SharpAimProgress`, which returns the current sharp-aiming progress as a scalar from 0 to 1.

- New `HeldDevice` Lua (R/W) property `Supported`, which indicates whether or not the device is currently being supported by a background hand.

- New `HeldDevice` Lua function `IsEmpty`, which indicates whether the device is devoid of ammo. Can be used to skip an extra step to check for a `Magazine`. Will always return `false` for non-`HDFirearm` devices.

- New `SoundContainer` INI and Lua (R/W) property `PitchVariation`, which can be used to randomize the pitch of the sounds being played.

- New `AHuman` and `ACrab` INI and Lua (R/W) property `JetReplenishRate`, which determines how fast jump time (i.e. jetpack fuel) is replenished during downtime.

- Added `Entity` Lua function `entity:RemoveFromGroup(groupToRemoveFrom)` which removes the given group from the `Entity`. The reverse of `AddToGroup`.

- New `AHuman` Lua functions `GetWalkAngle(layer)` and `SetWalkAngle(layer, angle)`, which can be used to read and override walk path rotation of both Legs/Layers respectively. Note that the walk path rotation is automatically updated on each step to match the curvature of the terrain, so this value resets every update.

- New `AHuman` INI and Lua (R/W) property `ArmSwingRate`, which now controls the arms' walking animation, according to each arm's `IdleOffset`. `1` is the default value, `0` means that the arms stay still.

- New `Attachable` Lua (R) property `JointPos`, which gets the position of the object's joint in scene coordinates.

- New `AHuman` Lua (R) property `IsClimbing`, which indicates whether the actor is currently climbing using either of the arms.

- New `AHuman` Lua functions `UnequipFGArm()` and `UnequipArms()` which unequip the currently held item(s) and put them into the actor's inventory.

- You can now execute multiple copies of your delivery order by holding UP or DOWN while choosing the landing zone.

- New `MOSprite` INI property `IconFile`, which can be used to define a separate sprite to be displayed in GUI elements, such as the Buy Menu.  
	Defined similarly to `SpriteFile`, i.e. `IconFile = ContentFile` followed up by a `FilePath` to the sprite.
	
- New `MOSprite` Lua functions `GetIconWidth()` and `GetIconHeight()` which return the dimensions of its GUI representation.

- New `PrimitiveMan` Lua functions `DrawIconPrimitive(player, pos, entity)` and `DrawIconPrimitive(pos, entity)` which can be used to draw the GUI representation of the passed in entity.

- New `AEmitter` and `PEmitter` Lua (R) property `ThrottleFactor`, which gets the throttle strength as a multiplier value that factors in either the positive or negative throttle multiplier according to throttle.

- New `AHuman` Lua (R) property `EquippedMass`, which returns the total mass of any `HeldDevice`s currently equipped by the actor.

- New Settings.ini flag `UseExperimentalMultiplayerSpeedBoosts = 1/0`. When turned on, it will use some code that **may** speed up multiplayer.

- New `FrameMan` Lua function `SplitStringToFitWidth(stringToSplit, widthLimit, useSmallFont)`, which lets you split up a string so it fits within a given width limit for the specified font. Does not try to perfectly fit strings, and can be wonky if the width limit is small. Mostly used to ensure text fits on the screen!

</details>

<details><summary><b>Changed</b></summary>

- `ACrab` actors will now default to showing their `Turret` sprite as their GUI icon. If no turret is defined, the `ACrab`'s own sprite will be used.  
	In a similar fashion, `AHuman` will now default to its torso sprite as its GUI representation if no `Head` has somehow been defined.

- `ThrownDevice`s will now use `StanceOffset`, `SharpStanceOffset` and `SupportOffset` in the same way as any other `HeldDevice`. In addition, `EndThrowOffset` will be used to set the BG hand position while sharp aiming or throwing.

- `AHuman` throwing angle will no longer be affected by the rotation of the body.

- Exposed `MovableObject` property `RestThreshold` to Lua (R/W).

- `ACRocket`s can now function without a full set of thrusters. This also means that "Null Emitter" thrusters are no longer required for rockets.

- Changed `MOSprite` property `SpriteAnimMode` `Enum` `LOOPWHENMOVING` to `LOOPWHENACTIVE` as it also describes active devices.

- Changed `Activity` Lua (R) properties `Running`, `Paused` and `ActivityOver` to `IsRunning`, `IsPaused` and `IsOver` respectively.  (NOTE: corresponding `ActivityMan` functions remain unchanged)

- Exposed `ThrownDevice` properties `StartThrowOffset` and `EndThrowOffset` to Lua (R/W).

- `HeldDevice`s can now show up as "Tools" in the buy menu, rather than just as "Shields".

- Keyboard-only controlled `AHuman`s and `ACrab`s can now strafe while sharp-aiming.

- Lowered the default `AHuman` Head damage multiplier from 5 to 4.

- "Fixed" grenades and other fast-moving objects bouncing violently off of doors and other stationary objects.

- `AEmitter` and `PEmitter` throttle logic has changed:  
	The properties `MinThrottleRange` and `MaxThrottleRange` have been changed to `NegativeThrottleMultiplier` and `PositiveThrottleMultiplier` respectively.  
	The new logic uses the multipliers to multiply the emission rate relative to the absolute throttle value. `NegativeThrottleMultiplier` is used when throttle is negative, and vice versa.

- Doors in `Team = -1` will now open up for all actors.

- `MovableMan` function `KillAllActors` (commonly found in activities) has been appropriately renamed `KillAllEnemyActors`.

- Wound limit gibbing logic has changed for `MOSRotating` (and all its subclasses), where objects will now gib when they reach their `GibWoundLimit` rather than when they surpass it. This allows for one-wound gibbing, which was previously infeasible. For objects with low `GibWoundLimit`s, you may want to adjust limits to account for this change.

- `TDExplosive`s will no longer default to a looping animation when activated. Instead, they change to the second frame (i.e 001), similarly to `HDFirearm`. Set `SpriteAnimMode` to `4` if you wish to enable the looping active animation.

- `AHuman` can now manually reload BG devices.

- Jetpack thrust angle is now properly clamped when controlled with an analog stick.

- Aim reticle dots can now be hidden per device by setting `SharpLength` to 0.

- Craft will now automatically scuttle when opening doors at a 90° angle rather than 45°.

- `AHuman`s can now sharp-aim slightly while walking, however not while reloading.

- Recoil when firing weapons now affects sharp aim.

- The distance arguments for `MovableMan` functions `GetClosestActor` and `GetClosestTeamActor` are now of type `Vector` rather than `float`.

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
	In place of these functions, you can get all wounds with `GetWounds`, you can get the wound count with `GetWoundCount` (or using the pre-existing WoundCount property), and remove wounds with `RemoveWounds`. You can also get the total gib wound limit with `GetGibWoundLimit` (or using the pre-existing GibWoundLimit property).  
	All of these functions have two variants, one lets you just specify any normal arguments (e.g. number of wounds to remove), the other lets you also specify whether you want to include any of the following, in order: `Attachables` with a positive `DamageMultiplier` (i.e. `Attachables` that damage their parent), `Attachables` with a negative `DamageMultiplier` (i.e. `Attachables` that heal their parent) or `Attachables` with no `DamageMultiplier` (i.e. `Attachables` that don't affect their parent).  
	Without any arguments, `GetWoundCount` and `RemoveWounds` will only include `Attachables` with a positive `DamageMultiplier` in their counting calculations, and `GetGibWoundLimit` will not include any `Attachables` in its counting calculations. The property variants (e.g. `mosr.WoundCount`) behave the same way as the no-argument versions.  
	Note that this process is recursive, so if an `Attachable` that satisfies the conditions has `Attachable`s that also satisfy the conditions, their wounds will be included in the results.

- Renamed `Turret` INI property `MountedMO` to `MountedDevice` to better match the new reality that `Turrets` can only mount `HeldDevices` and their child classes.

- Renamed `ACrab` `LFGLeg`, `LBGLeg`, `RFGLeg` and `RBGLeg` Lua properties to `LeftFGLeg`, `LeftBGLeg`, `RightFGLeg`, `RightBGLeg` respectively, to be more consistent with other naming.  
	For the time being, the INI properties (as well as the ones for setting `FootGroups` and `LimbPaths`) support both single letter and written out versions (i.e. `LStandLimbPath` and `LeftStandLimbPath` are both supported). This single letter versions will probably be deprecated over time.

- To better align with the other changes, hardcoded `Attachable` INI definitions for `ACDropShips` and `ACRockets` can now support spelled out words. The following options are all supported in INI:  
	**`ACDropShip`** - `RThruster`/`RightThruster`/`RightEngine`, `LThruster`/`LeftThruster`/`LeftEngine`, `URThruster`/`UpRightThruster`, `ULThruster`/`UpLeftThruster`, `RHatchDoor`/`RightHatchDoor`, `LHatchDoor`/`LeftHatchDoor`  
	**`ACRocket`** - `RLeg`/`RightLeg`, `LLeg`/`LeftLeg`, `RFootGroup`/`RightFootGroup`, `LFootGroup`/`LeftFootGroup`, `MThruster`/`MainThruster`, `RThruster`/`RightThruster`, `LThruster`/`LeftThruster`, `URThruster`/`UpRightThruster`, `ULThruster`/`UpLeftThruster`

- `MovableMan:AddMO` will now add `HeldDevices` (or any child class of `HeldDevice`) to its `Items` collection, making it able to provide the functionality of `AddParticle`, `AddActor` and `AddItem`.

- Changed and cleaned up how gibbing works and how it affects `Attachables`. In particular, limbs will better inherit velocity during gibbing and things are more customizable. See `Attachable` properties for more details.  
	As an added bonus,  `Attachables` on `ACDropShips` and `ACRockets` can now be shot down when the craft gibs; fight back against the baleful dropship engines!
	
- Improved native recoil handling! Guns transfer recoil to arms/turrets, which transfer it to AHumans/ACrabs, all of it properly accounts for joint strengths (or grip strengths) and offsets at every step. Future work will be done on this to improve it. ([Issue #7](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/7) and [Issue #8](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/8)).

- `Attachables` now use their `GibImpulseLimit` as well as their `JointStrength` when determining whether they should be detached by strong forces. To maintain backwards compatibility, if the `GibImpulseLimit` is less than the `JointStrength`, the `JointStrength` will be used instead for this purpose.

- The `FacingAngle` function has been moved from `Actor` to `MOSprite` so it can be used more widely.

- `Lifetime` and `ToDelete` now work on wounds, giving modders more control over them.

- Some functionality has been moved from `AudioMan` to `SoundContainer` for consistency. As such, the following `AudioMan` Lua bindings have been replaced:  
	`AudioMan:FadeOutSound(fadeOutTime);` has been replaced with `soundContainer:FadeOut(fadeOutTime);`  
	`AudioMan:StopSound(soundContainer);` and `AudioMan:StopSound(soundContainer, player);` have been replaced with `soundContainer:Stop();` and `soundContainer:Stop(player);`
	
- Pressing escape when a buy menu is open now closes it instead of pausing the game.

- `GetParent` will now return an `MOSRotating` instead of a `MovableObject` so it doesn't need to be casted with `ToMOSRotating`. Additionally, it will always return null for objects with no parents, instead of returning the self object for things that weren't `Attachables`.  
	This makes things more consistent and reasonable throughout and will rarely, if ever, cause Lua problems.

- Previews generated by the `SceneEditor` are now the same as `ScenePreviewDumps`. Also, both are now saved as PNGs.

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

- All `mosRotating:RemoveAttachable`, `mosRotating:RemoveEmitter` and `attachable:RemoveFromParent` functions will return the removed `Attachable` if it hasn't been added to `MovableMan`, or nil if it has. If the `Attachable` is returned, it will belong to Lua like it would if it were newly Created. You could then, for example, add it to MovableMan, an inventory, or attach it to something else.

- `Settings.ini` property `MenuTransitionDuration` renamed to `MenuTransitionDurationMultiplier`.

- `Settings.ini` property `DisableLoadingScreen` renamed to `DisableLoadingScreenProgressReport`.

- Scenario scene markers are now color coded to help distinguish them visually:  
	`Base.rte` scenes are yellow as always.  
	`Missions.rte` scenes are now green.  
	`Scenes.rte` or any other mod/user scenes are now cyan.

- Main menu and sub-menus were given a major facelift.

- Settings menu was reworked to make it less useless.

- Esc has been disabled in server mode to not disrupt simulation for clients, use Alt+F4 or the window close button to exit.

- Placing "Tech" in a `DataModule`'s `ModuleName` no longer makes the module a playable faction (in MetaGame, etc.). The `IsFaction` property should be used instead.  
	The word "Tech" will also not be omitted from the module name when displayed in any faction selection dropdown list.
	
- Renamed Lua methods `GetRadRotated` and `GetDegRotated` to `GetRadRotatedCopy` and `GetDegRotatedCopy` for clarity.

- Added support for multiple lines in DataModule descriptions in their Index.inis. See earlier entry on multiple lines in descriptions for details on how to use this.

- `FrameMan` screen text will now always try to fit on the screen, splitting into multiple lines if needed. If you want more control of this, split your screen text manually with the `"\n"` new line character.

</details>


<details><summary><b>Fixed</b></summary>

- Fixed the logic for `Gib` and `Emission` property `LifeVariation` where it would round down to zero, giving particles infinite lifetime.

- Fixed legs going bonkers for one frame when turning around.

- `HFlipped` is now properly assigned to emissions, gibs and particles that are shot from a `HDFirearm`'s `Round` when the source object is also flipped.

- `MovableObject:SetWhichMOToNotHit` will now work properly for Attachables. They will also not hit the relevant MO. When they're removed, Attachables will check if they have the same MO for this value and, if so, unset it so they can hit that MO.

- Craft sucking up objects now works properly again.

- Getting the `Mass` of a `MOSRotating` has now been made more efficient. Additionally, `Attachables` of `Attachables` will now be properly included in Mass, so some things have gotten a lot heavier (e.g. Dummy Dreadnought).

- The moment of inertia of `AtomGroups` now updates when the mass or Atoms change, meaning losing `Attachables` or changing mass will properly affect how rotational forces apply to MOSRotatings.

- `WoundDamageMultipliers` on projectiles will now properly stack with wounds' `DamageMultiplier`. Prior to this, if you set the `DamageMultiplier` of a wound on some object, it'd be overwritten by the hitting projectile's `WoundDamageMultiplier`. Now they multiply together properly.

- `Radius` and `Diameter` now account for `Attachables` on objects that can have them. If you want just the `Radius` or `Diameter` of the object, use `IndividualRadius` and `IndividualDiameter` (only available for `MOSRotating` and subclasses). This means that `Radius` and `Diameter` will now give you a good estimation of an object's total size.

- Fixed various audio bugs that were in Pre3, and fixed clicking noise on sounds that played far away. The game should sound way better now!

- Mobile sounds (i.e. generally things that aren't GUI related) will now pause and resume when you pause and resume your activity.

- The `DeactivationSound` of `HDFirearms` now respects its `SoundOverlapMode` instead of never allowing overlap. If you don't want it overlapping, set it up accordingly.

- Enabled DPI Awareness to fix issues with resolution settings when Windows scaling is enabled.

- Fixed a bug that caused the game to crash when the crab bomb effect was triggered while there were multiple crab bomb eligible Craft in an activity.

- Renamed `Attachable` INI property `CollidesWithTerrainWhenAttached` to more correct, consistent `CollidesWithTerrainWhileAttached`.

- You can now modify all hardcoded `Attachable` `CopyOf` INI definitions without setting a new `PresetName`. This means you could, for example, `CopyOf` a predefined `Leg` and change it, without having to set a new `PresetName`. This is optional, and comes with the obvious limitation of not being able to find that modified copy in-game with Lua.

- `OnCollideWithMO` now works for `MOPixels` and `MOSParticles` so you can use it to check if your bullets collide with things.

- `OnCollideWithMO` and `OnCollideWithTerrain` (and other special functions) will run more reliably right after the object is spawned. E.g. `OnCollideWithTerrain` on a bullet should now work even if your gun is jammed into terrain when you shoot.

- You can now sharp-aim through friendlies when playing as any team, instead of just as red team.

- The reload hotkey now works even if you're on top of a pickupable object.

- Improved LZ behaviour on wrapping maps, so your buy cursor will no longer annoyingly wrap around the LZ area.

- Fixed a bug with metagame saves that caused `Player` and `Team` numbers to be off by 1.

- Vote counts to end a multiplayer activity now display as intended. 

- Fixed a bug where choosing `-Random-` as a player's tech and pressing start game had a 1 in (number of techs + 1) chance to crash the game.

- Console error spam will no longer cripple performance over time.

- `AudioMan:StopAll()` now actually stops all sounds, instead of just stopping music.

- Fixed incorrect mouse bounds during splitscreen when the mouse player was not Player 1.

</details>


<details><summary><b>Removed</b></summary>

- Removed obsolete graphics drivers and their `Settings.ini` properties `ForceOverlayedWindowGfxDriver` and `ForceNonOverlayedWindowGfxDriver`.

- Removed `Attachable` Lua write capability for `AtomSubGroupID` as changing this can cause all kinds of problems.

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

- Removed hardcoded INI constraint that forced `Mass` of `MovableObjects` to not be 0. Previously, anytime a `Mass` of 0 was read in from INI, it was changed to 0.0001, now 0 `Mass` is allowed and supported.

- Removed the quit confirmation dialog from the scenarios screen. Now pressing escape will return you to the main menu.

- Removed `Settings.ini` properties `HSplitScreen` and `VSplitScreen`. Superseded by `TwoPlayerSplitscreenVertSplit`.

</details>

***

## [0.1.0 pre-release 3.0][0.1.0-pre3.0] - 2020/12/25

<details><summary><b>Added</b></summary>

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
	```lua
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
	`soundSet:AddSound("Path/to/sound.flac")` (Lua) - Adds the sound at the given path with no offset, 0 minimum audible distance, and default attenuation start distance.  
	`soundSet:AddSound("Path/to/sound.flac", offset, minimumAudibleDistance, attenuationStartDistance)` (Lua) - Adds the sound at the given path with the given parameters.  
	`soundSet:AddSoundSet(soundSetToAdd)` (Lua) - Adds the given `SoundSet` as a sub `SoundSet` of this `SoundSet`.  
	`soundSet:RemoveSound("Path/to/sound.flac")` (Lua) - Removes any sounds with the given filepath from the `SoundSet`, returning whether or not any where removed. Does not remove sounds from sub-`SoundSet`s.  
	`soundSet:RemoveSound("Path/to/sound.flac", removeFromSubSoundSets)` (Lua) - Removes any sounds with the given filepath from the `SoundSet`, returning whether or not any where removed. Optionally removes matching sounds from any sub-`SoundSet`s and their sub-`SoundSet`s and so on.  
	
	Additionally, `AddSound` and `AddSoundSet` INI properties work for `SoundSets`. They are exactly the same as they are for `SoundContainers`.

- You can get the top level `SoundSet` of a `SoundContainer` with `soundContainer:GetTopLevelSoundSet` and manipulate it as described above. You can also set it to fully overwrite it with `soundContainer:SetTopLevelSoundSet`. This allows you full interaction with all levels of `SoundSets` in a `SoundContainer`.

</details>


<details><summary><b>Changed</b></summary>

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
	```lua
	for attachable in ToMOSRotating(self:GetParent()).Attachables do
		...
	end
	```
	Or
	```lua
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

</details>


<details><summary><b>Fixed</b></summary>

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

</details>


<details><summary><b>Removed</b></summary>

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

</details>

***

## [0.1.0 pre-release 2][0.1.0-pre2] - 2020/05/08

<details><summary><b>Added</b></summary>

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

		SoundSelectionCycleMode = RANDOM (default) | FORWARDS | ALL // How the SoundContainer will cycle through its `SoundSets` whenever it's told to select a new one. The first is prior behaviour of picking sounds at random, the second cycles through SoundSets in the order they were added, and the third plays all SoundSets at once.

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
	```lua
	soundContainer:HasAnySounds() -- Returns whether or not the SoundContainer has any sounds in it. Returns True or false.
	```
	```lua
	soundContainer:IsBeingPlayed() -- Returns whether or not any sounds in the SoundContainer are currently being played. Returns True or False.
	```
	```lua
	soundContainer:Play(optionalPosition, optionalPlayer) -- Plays the sounds belonging to the SoundContainer's currently selected SoundSet. The sound will play at the position and for the player specified, or at (0, 0) for all players if parameters aren't specified.
	```
	```lua
	soundContainer:Stop(optionalPlayer) -- Stops any playing sounds belonging to the SoundContainer, optionally only stopping them for a specified player.
	```
	```lua
	soundContainer:AddSound(filePath, optional soundSetToAddSoundTo, optionalSoundOffset, optionalAttenuationStartDistance, optionalAbortGameIfSoundIsInvalid) -- Adds the sound at the given filepath to the SoundContainer. If a SoundSet index is specified it'll add it to that SoundSet. If an offset or attenuation start distance are specified they'll be set, as mentioned in the INI section above. If set to abort for invalid sounds, the game will error out if it can't load the sound, otherwise it'll show a console error.
	```
	```lua
	soundContainer:SetPosition(position) -- Sets the position at which the SoundContainer's sounds will play.
	```
	```lua
	soundContainer:SelectNextSoundSet() -- Selects the next SoundSet to play when soundContainer:Play(...) is called, according to the INI defined CycleMode.
	```
	```lua
	soundContainer.Loops -- Set or get the number of loops for the SoundContainer, as mentioned in the INI section above.
	```
	```lua
	soundContainer.Priority -- Set or get the priority of the SoundContainer, as mentioned in the INI section above.
	```
	```lua
	soundContainer.AffectedByGlobalPitch -- Set or get whether the SoundContainer is affected by global pitch, as mentioned in the INI section above.
	```
- `MovableObjects` can now run multiple scripts by putting multiple `AddScript = FilePath.lua` lines in the INI definition. ([Issue #109](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/pull/109))  
	Scripts will have their appropriate functions run in the order they were added. Note that all scripts share the same `self`, so care must be taken when naming self variables.  
	Scripts can be checked for with `movableObject:HasScript(filePath);` and added and removed with `movableObject:AddScript(filePath);` and `movableObject:RemoveScript(filePath);`. They can also be enabled and disabled in Lua (preserving their ordering) with `movableObject:EnableScript(filePath);` and `movableObject:DisableScript(filePath);`.

- Scripts on `MovableObjects` and anything that extends them (i.e. most things) now support the following new functions (in addition to `Create`, `Update`, `Destroy` and `OnPieMenu`). They are added in the same way as the aforementioned scripts:  
	```lua
	OnScriptRemoveOrDisable(self, scriptWasRemoved) -- This is run when the script is removed or disabled. The scriptWasRemoved parameter will be True if the script was removed and False if it was disabled.
	```
	```lua
	OnScriptEnable(self) -- This is run when the script was disabled and has been enabled.
	```
	```lua
	OnCollideWithTerrain(self, terrainMaterial) -- This is run when the MovableObject this script on is in contact with terrain. The terrainMaterial parameter gives you the material ID for the terrain collided with. It is suggested to disable this script when not needed to save on overhead, as it will be run a lot!
	```
	```lua
	OnCollideWithMO(self, collidedMO, collidedRootMO) -- This is run when the MovableObject this script is on is in contact with another MovableObject. The collidedMO parameter gives you the MovableObject that was collided with, and the collidedRootMO parameter gives you the root MovableObject of that MovableObject (note that they may be the same). Collisions with MovableObjects that share the same root MovableObject will not call this function.
	```

- Scripts on `Attachables` now support the following new functions:  
	```lua
	OnAttach(self, newParent) -- This is run when the Attachable this script is on is attached to a new parent object. The newParent parameter gives you the object the Attachable is now attached to.
	```
	```lua
	OnDetach(self, exParent) -- This is run when the Attachable this script is on is detached from an object. The exParent gives you the object the Attachable was attached to.
	```

</details>


<details><summary><b>Changed</b></summary>

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

</details>


<details><summary><b>Fixed</b></summary>

- Fixed LuaBind being all sorts of messed up. All lua bindings now work properly like they were before updating to the v141 toolset.

- Explosives (and other thrown devices) will no longer reset their explosion triggering timer when they're picked up. ([Issue #71](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/71))

- Sprite Animation Mode `ALWAYSPINGPONG` now works properly. Sprite animation has also been moved to `MOSprite` instead of `MOSRotating`, they they'll be able to properly animate now. ([Issue#77](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/77))

- Fixed `BG Arm` flailing when reloading one-handed weapon, so shields are no longer so useless.

- Fixed crash when clearing an already empty preset list in the buy menu.

- Temporary fix for low mass attachables/emitters being thrown at ridiculous speeds when their parent is gibbed.

- The audio system now better supports splitscreen games, turning off sound panning for them and attenuating according to the nearest player.

- The audio system now better supports wrapping maps so sounds handle the seam better. Additionally, the game should be able to function if the audio system fails to start up.

- Scripts on attached attachables will only run if their parent exists in MovableMan. ([Issue #83](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/issues/83))

</details>


<details><summary><b>Removed</b></summary>

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

</details>

***

## [0.1.0 pre-release 1][0.1.0-pre1] - 2020/01/27

<details><summary><b>Added</b></summary>

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

</details>

<details><summary><b>Changed</b></summary>

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

</details>


<details><summary><b>Fixed</b></summary>

- SFX slider now works properly.

- BGM now loops properly.

- Sound pitching now respects sounds that are not supposed to be affected by pitch.

- Using `actor:Clone()` now works properly, there are no longer issues with controlling/selecting cloned actors.

- `TDExplosive.ActivatesWhenReleased` now works properly.

- Various bug fixed related to all the Attachable and Emitter changes, so they can now me affected reliably and safely with lua.

- Various minor other things that have gotten lost in the shuffle.

</details>


<details><summary><b>Removed</b></summary>

- All licensing-related code has been removed since it's no longer needed.

- Wounds can no longer be added via ini, as used to be doable buggily through ini `AddEmitter`.

- All usage of the outdated Slick Profiler has been removed.

- `TDExplosive.ParticleNumberToAdd` property has been removed.

</details>

***

Note: For a log of changes made prior to the commencement of the open source community project, look [here.](https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/wiki/Previous-Closed-Source-Changelog)


[unreleased]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/compare/master...cortex-command-community:development
[0.1.0-pre1]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data/releases/tag/v0.1.0-pre1
[0.1.0-pre2]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Data/releases/tag/v0.1.0-pre2
[0.1.0-pre3.0]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/releases/tag/v0.1.0-pre3.0
[0.1.0-pre4.0]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/releases/tag/v0.1.0-pre4.0
[0.1.0-pre5.0]: https://github.com/cortex-command-community/Cortex-Command-Community-Project-Source/releases/tag/v0.1.0-pre5.0
