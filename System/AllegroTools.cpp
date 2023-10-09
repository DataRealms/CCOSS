#include "AllegroTools.h"
#include "allegro.h"
#include "allegro/internal/aintern.h"
namespace RTE {
	unsigned long TrueAlphaBlender(unsigned long x, unsigned long y, unsigned long n) {
		// This is the original allegro alpha blender with added alpha component blending.
		unsigned long res;
		unsigned long green;
		unsigned long alphaX = geta32(x);
		unsigned long alphaY = geta32(y);
		unsigned long alpha = 0;

		n = geta32(x);

		if (n){ n++; }

		res = ((x & 0xFF00FF) - (y & 0xFF00FF)) * n / 256 + y;
		y &= 0xFF00;
		x &= 0xFF00;
		green = (x - y) * n / 256 + y;

		res &= 0xFF00FF;
		green &= 0xFF00;

		alpha = alphaX + (255 - alphaX) / 255.0f * alphaY;
		alpha <<= 24;

		return res | green | alpha;
	}

	void SetTrueAlphaBlender() {
		set_blender_mode_ex(_blender_black, _blender_black, _blender_black, TrueAlphaBlender, _blender_black, _blender_black, _blender_black, 0, 0, 0, 0);
	}
} // namespace RTE