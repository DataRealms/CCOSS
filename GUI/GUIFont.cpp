#include "GUI.h"

using namespace RTE;

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIFont::GUIFont(const std::string &Name) {
	m_Screen = nullptr;
	m_Font = nullptr;
	m_FontHeight = 0;
	m_Name = Name;
	m_Kerning = 0;
	m_Leading = 0;
	m_ColorCache.clear();

	m_MainColor = 15; // Color index of the main font color
	m_CurrentColor = m_MainColor;
	m_CurrentBitmap = nullptr;

	m_CharIndexCap = 256;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool GUIFont::Load(GUIScreen *Screen, const std::string &Filename) {
	assert(Screen);

	m_Screen = Screen;

	// Load the font image
	m_Font = m_Screen->CreateBitmap(Filename);
	if (!m_Font) {
		return false;
	}
	m_CurrentBitmap = m_Font;

	// Clear the cache
	m_ColorCache.clear();

	// Convert the MainColor
	m_MainColor = Screen->ConvertColor(m_MainColor, m_CurrentBitmap->GetColorDepth());
	m_CurrentColor = m_MainColor;

	// Set the color key to be the same color as the Top-Right hand corner pixel
	unsigned long BackG = m_Font->GetPixel(m_Font->GetWidth() - 1, 0);
	m_Font->SetColorKey(BackG);

	// The red separator MUST be on the Top-Left hand corner
	unsigned long Red = m_Font->GetPixel(0, 0);

	// Find the separating gap of the font lines
	int y;
	m_FontHeight = 0;
	for (y = 1; y < m_Font->GetHeight(); y++) {
		if (m_Font->GetPixel(0, y) == Red) {
			m_FontHeight = y;
			break;
		}
	}

	// Pre-calculate the character details
	memset(m_Characters, 0, sizeof(Character) * 256);

	int x = 1;
	y = 0;
	int charOnLine = 0;
	for (int chr = 32; chr < 256; chr++) {
		m_Characters[chr].m_Offset = x;

		// Find the next red pixel
		int w = 0;
		for (int n = x; n < m_Font->GetWidth(); n++, w++) {
			if (m_Font->GetPixel(n, y) == Red) {
				break;
			}
		}

		// Calculate the maximum height of the character
		int Height = 0;
		for (int j = y; j < y + m_FontHeight; j++) {
			for (int i = x; i < x + w; i++) {
				unsigned long Pixel = m_Font->GetPixel(i, j);
				if (Pixel != Red && Pixel != BackG) { Height = std::max(Height, j - y); }
			}
		}

		m_Characters[chr].m_Height = Height;
		m_Characters[chr].m_Width = w - 1;
		x += w + 1;

		m_CharIndexCap = chr + 1;

		charOnLine++;
		if (charOnLine >= 16) {
			charOnLine = 0;
			x = 1;
			y += m_FontHeight;
			// Stop if we run out of bitmap
			if ((y + m_FontHeight) > m_Font->GetHeight()) {
				break;
			}
		}
	}

	return true;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIFont::Draw(GUIBitmap *Bitmap, int X, int Y, const std::string &Text, unsigned long Shadow) {
	unsigned char c;
	GUIRect Rect;
	GUIBitmap *Surf = m_CurrentBitmap;
	int initX = X;

	assert(Surf);

	// Make the shadow color
	FontColor *FSC = nullptr;
	if (Shadow) {
		FSC = GetFontColor(Shadow);
		if (!FSC) {
			CacheColor(Shadow);
			FSC = GetFontColor(Shadow);
		}
	}

	// Go through every character
	for (int i = 0; i < Text.length(); i++) {
		c = Text.at(i);

		if (c == '\n') {
			Y += m_FontHeight;
			X = initX;
		}
		if (c == '\t') { X += m_Characters[' '].m_Width * 4; }
		if (c < 0) { c += m_CharIndexCap; }
		if (c < 32 || c >= m_CharIndexCap) {
			continue;
		}

		int CharWidth = m_Characters[c].m_Width;
		int offX = m_Characters[c].m_Offset;
		int offY = ((c - 32) / 16) * m_FontHeight;
		SetRect(&Rect, offX, offY, offX + CharWidth, offY + m_FontHeight);

		// Draw the shadow
		if (Shadow && FSC) { FSC->m_Bitmap->DrawTrans(Bitmap, X + 1, Y + 1, &Rect); }

		// Draw the main color
		Surf->DrawTrans(Bitmap, X, Y, &Rect);

		// Find the starting position
		X += CharWidth + m_Kerning;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIFont::DrawAligned(GUIBitmap *Bitmap, int X, int Y, const std::string &Text, int HAlign, int VAlign, int MaxWidth, unsigned long Shadow) {
	std::string TextLine = Text;
	int lineStartPos = 0;
	int lineEndPos = 0;
	int lineWidth = 0;
	int yLine = Y;
	int lastSpacePos = 0;

	// Adjust the starting of the Y based on vertical alignment
	if (VAlign == Middle) {
		yLine -= (CalculateHeight(TextLine, MaxWidth) / 2);
	} else if (VAlign == Bottom) {
		yLine -= CalculateHeight(TextLine, MaxWidth);
	}

	while (lineStartPos < Text.size()) {
		// Find the next newline, if any
		lineEndPos = Text.find('\n', lineStartPos);
		// Grab the whole line
		TextLine = Text.substr(lineStartPos, (lineEndPos == std::string::npos ? Text.size() : lineEndPos) - lineStartPos);
		// Figure its width, in pixels
		lineWidth = CalculateWidth(TextLine);

		// See if it's too wide to fit within the maxWidth
		if (MaxWidth > 0 && lineWidth > MaxWidth) {
			// Find the last space that makes the line within the max width
			do {
				// Find last space
				lastSpacePos = Text.rfind(' ', lineEndPos - 1);
				// if there was no space encountered before hitting beginning, then just leave the too long line as is and let it exceed the max width
				if (lastSpacePos == std::string::npos || lastSpacePos <= lineStartPos) {
					lineEndPos = Text.size();
					break;
				}
				// Update the new end position
				lineEndPos = lastSpacePos;
				// Get the new, shorter line
				TextLine = Text.substr(lineStartPos, lineEndPos - lineStartPos);
				// Figure the new line width, in pixels
				lineWidth = CalculateWidth(TextLine);
			} while (lineWidth > MaxWidth);

			// Update the new start position for next line
			lineStartPos = lineEndPos + 1;
			// Make sure it's not starting on a space
			while (lineStartPos < Text.size() && Text.at(lineStartPos) == ' ') {
				lineStartPos++;
			}
		} else {
			// Advance the line start to the next line, or to the end of the text if there are no more lines
			lineStartPos = lineEndPos == std::string::npos ? Text.size() : (lineEndPos + 1);
		}

		// If the line is scrolled above the bitmap top, then don't try to draw anything
		if ((yLine + m_FontHeight) >= 0) {
			switch (HAlign) {
				// Left HAlignment: Where X is the starting point of the text
				case Left:
					Draw(Bitmap, X, yLine, TextLine, Shadow);
					break;

					// Center HAlignment: Where X is the center point of the text
				case Centre:
					Draw(Bitmap, X - lineWidth / 2, yLine, TextLine, Shadow);
					break;

					// Right HAlignment: Where X is the end point of the text
				case Right:
					Draw(Bitmap, X - lineWidth, yLine, TextLine, Shadow);
					break;
				default:
					break;
			}
		}

		// Add the height
		yLine += m_FontHeight;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIFont::SetColor(unsigned long Color) {
	// Only check the change if the color is different
	if (Color != m_CurrentColor) {

		// Find the cached color
		FontColor *FC = GetFontColor(Color);

		// Use the cached color, otherwise just draw the default bitmap
		if (FC) {
			m_CurrentBitmap = FC->m_Bitmap;
			m_CurrentColor = Color;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIFont::CalculateWidth(const std::string &Text) {
	unsigned char c;
	int Width = 0;
	int WidestLine = 0;

	// Go through every character
	for (int i = 0; i < Text.length(); i++) {
		c = Text.at(i);
		// Reset line counting if newline encountered
		if (c == '\n') {
			if (Width > WidestLine) { WidestLine = Width; }
			Width = 0;
			continue;
		}
		if (c < 0) { c += m_CharIndexCap; }

		if (c < 32 || c >= m_CharIndexCap) {
			continue;
		}

		Width += m_Characters[c].m_Width;

		// Add kerning
		Width += m_Kerning;
	}
	if (Width > WidestLine) { WidestLine = Width; }

	return WidestLine;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIFont::CalculateWidth(const char Character) {
	if (Character >= 32 && Character < m_CharIndexCap) {
		return m_Characters[Character].m_Width + m_Kerning;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIFont::CalculateHeight(const std::string &Text, int MaxWidth) {
	if (Text.empty()) {
		return 0;
	}
	unsigned char c;
	int Width = 0;
	int Height = m_FontHeight;
	int lastSpacePos = 0;

	// Go through every character
	for (int i = 0; i < Text.length(); i++) {
		c = Text.at(i);

		// Add the new line's height if newline encountered
		if (c == '\n') {
			Width = 0;
			Height += m_FontHeight;
			continue;
		}
		if (c < 32 || c >= m_CharIndexCap) {
			continue;
		}
		if (c == ' ') { lastSpacePos = i; }

		Width += m_Characters[c].m_Width + m_Kerning;
		if (MaxWidth > 0 && Width > MaxWidth) {
			// Rewind to the last space, and do line break, but only if we've passed a space since last wrap
			if (lastSpacePos > 0) {
				i = lastSpacePos;
				lastSpacePos = 0;
				Width = 0;
				Height += m_FontHeight;
			}
			continue;
		}
	}

	return Height;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIFont::CacheColor(unsigned long Color) {
	// Make sure we haven't already cached this color and it isn't a 0 color
	if (GetFontColor(Color) != nullptr || !Color) {
		return;
	}

	// Add a new color to the cache
	FontColor FC;
	FC.m_Color = Color;
	FC.m_Bitmap = m_Screen->CreateBitmap(m_Font->GetWidth(), m_Font->GetHeight());

	// Created the bitmap?
	if (!FC.m_Bitmap) {
		return;
	}

	// Copy the bitmap
	m_Font->Draw(FC.m_Bitmap, 0, 0, nullptr);

	// Set the color key to be the same color as the Top-Right hand corner pixel
	unsigned long BackG = FC.m_Bitmap->GetPixel(FC.m_Bitmap->GetWidth() - 1, 0);
	FC.m_Bitmap->SetColorKey(BackG);

	// Go through the bitmap and change the pixels
	for (int y = 0; y < FC.m_Bitmap->GetHeight(); y++) {
		for (int x = 0; x < FC.m_Bitmap->GetWidth(); x++) {
			if (FC.m_Bitmap->GetPixel(x, y) == m_MainColor) { FC.m_Bitmap->SetPixel(x, y, Color); }
		}
	}

	// Add the color to the list
	m_ColorCache.push_back(FC);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

GUIFont::FontColor * GUIFont::GetFontColor(unsigned long Color) {
	std::vector<FontColor>::iterator it;
	FontColor *F = nullptr;
	for (it = m_ColorCache.begin(); it != m_ColorCache.end(); it++) {
		F = &(*it);

		// Colors match?
		if (F->m_Color == Color) {
			return F;
		}
	}
	// Not found
	return nullptr;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIFont::GetFontHeight() const {
	return m_FontHeight;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

std::string GUIFont::GetName() const {
	return m_Name;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int GUIFont::GetKerning() const {
	return m_Kerning;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void GUIFont::Destroy() {
	if (m_Font) {
		m_Font->Destroy();
		delete m_Font;
		m_Font = nullptr;
	}

	// Go through the color cache and destroy the bitmaps
	std::vector<FontColor>::iterator it;
	FontColor *FC = 0;
	for (it = m_ColorCache.begin(); it != m_ColorCache.end(); it++) {
		FC = &(*it);
		if (FC && FC->m_Bitmap) {
			FC->m_Bitmap->Destroy();
			delete FC->m_Bitmap;
		}
	}
	m_ColorCache.clear();
}