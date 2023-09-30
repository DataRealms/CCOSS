#ifndef _ALLEGROTOOLS_H_
#define _ALLEGROTOOLS_H_
namespace RTE {
	#pragma region True Alpha Blending

		/// <summary>
		/// Workaround for allegro's missing true alpha blender, use instead of set_alpha_blender when alpha values are desired or necessary after draw:
		/// ``` set_blender_mode_ex(_blender_black, _blender_black, _blender_black, TrueAlphaBlender, _blender_black, _blender_black, _blender_black, 0, 0, 0, 0);```
		/// </summary>
		unsigned long TrueAlphaBlender(unsigned long x, unsigned long y, unsigned long n);
		/// <summary>
		/// Sets the 32bit allegro blender mode to TrueAlphaBlender
		/// </summary>
		void SetTrueAlphaBlender();
	#pragma endregion
}

#endif