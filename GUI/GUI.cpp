#include "GUI.h"

void SetRect(RECT *pRect, int left, int top, int right, int bottom)
{
    pRect->left = left;
    pRect->top = top;
    pRect->right = right;
    pRect->bottom = bottom;
}

int DDTstricmp(const char *s1, const char *s2)
{
    while ((*s1) && tolower((unsigned char)*s1) == tolower((unsigned char)*s2))
    {
        ++s1, ++s2;
    }

    return tolower((unsigned char)*s1) - tolower((unsigned char)*s2);
}