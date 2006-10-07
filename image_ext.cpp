/*
 
 image_ext.cpp: extra functionality for wxImage
 Copyright (C) 2006  Gregory Smith
 
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version 2
 of the License, or (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 
 */

#include "image_ext.h"

void wxImageExt::White()
{
	int NumBytes = GetWidth() * GetHeight() * 3;
	for (int i = 0; i < NumBytes; i++) 
	{
		GetData()[i] = 0xff;
	}
}

void wxImageExt::RemoveAlpha()
{
	wxImageExt image;
	image.Create(GetWidth(), GetHeight());
	memcpy(image.GetData(), GetData(), GetWidth() * GetHeight() * 3);
	
	*this = image;
}

void wxImageExt::MaskToAlpha()
{
	wxImageExt image;
	image.Create(GetWidth(), GetHeight());
	image.InitAlpha();
	image.SetMask(false);
	
	memcpy(image.GetData(), GetData(), GetWidth() * GetHeight() * 3);
	
	for (int x = 0; x < GetWidth(); x++)
	{
		for (int y = 0; y < GetHeight(); y++)
		{
			if (IsTransparent(x, y))
			{
				image.GetAlpha()[x + y * GetWidth()] = 0x00;
			} 
			else
			{
				image.GetAlpha()[x + y * GetWidth()] = 0xff;
			}
		}
	}
	
	*this = image;
}

void wxImageExt::PropRescale(int width, int height)
{
	float scale = (float) MAX(width, height) / MAX(GetWidth(), GetHeight());
	Rescale((int) (GetWidth() * scale), (int) (GetHeight() * scale));
	Resize(wxSize(width, height), wxPoint((width - GetWidth()) / 2, (height - GetHeight()) / 2));
}

void wxImageExt::FromAlpha(const wxImageExt& source)
{
	Create(source.GetWidth(), source.GetHeight());
	int NumPixels = GetWidth() * GetHeight();
	for (int i = 0; i < NumPixels; i++)
	{
		GetData()[i * 3 + 0] = GetData()[i * 3 + 1] = GetData()[i * 3 + 2] = source.GetAlpha()[i];
	}
}

void wxImageExt::ToAlpha(wxImageExt& dest) const
{
	if (!dest.HasAlpha())
	{
		dest.InitAlpha();
	}
	
	int NumPixels = GetWidth() * GetHeight();
	for (int i = 0; i < NumPixels; i++)
	{
		unsigned int Average = (GetData()[i * 3 + 0] + GetData()[i * 3 + 1] + GetData()[i * 3 + 2]) / 3;
		dest.GetAlpha()[i] = Average & 0xff;
	}
}

void wxImageExt::MakeOpacTypeTwo()
{
	wxImageExt image;
	image.Create(GetWidth(), GetHeight());
	int NumPixels = GetWidth() * GetHeight();
	for (int i = 0; i < NumPixels; i++)
	{
		unsigned int Average = (GetData()[i * 3 + 0] + GetData()[i * 3 + 1] + GetData()[i * 3 + 2]) / 3;
		image.GetData()[i * 3 + 0] = image.GetData()[i * 3 + 1] = image.GetData()[i * 3 + 2] = Average & 0xff;
	}
	
	*this = image;
}

void wxImageExt::MakeOpacTypeThree()
{
	wxImageExt image;
	image.Create(GetWidth(), GetHeight());
	int NumPixels = GetWidth() * GetHeight();
	for (int i = 0; i < NumPixels; i++)
	{
		unsigned char Max = MAX(GetData()[i * 3 + 0], MAX(GetData()[i * 3 + 1], GetData()[i * 3 + 2]));
		image.GetData()[i * 3 + 0] = image.GetData()[i * 3 + 1] = image.GetData()[i * 3 + 2] = Max;
	}
	
	*this = image;
}

// thanks to the Virtual Terrain Project for these algorithms, and the code looks
// pretty similar too...

void wxImageExt::ReconstructColors(const wxColour& bgColor)
{
	// restore the color of edge texels by guessing correct non-background color
    wxImageExt result;
	
    result.Create(GetWidth(), GetHeight());
    result.InitAlpha();
    for (int x = 0; x < GetWidth(); x++) {
		for (int y = 0; y < GetHeight(); y++) {
			if (GetAlpha(x, y) == 0) {
				result.SetAlpha(x, y, 0);
				result.SetRGB(x, y, bgColor.Red(), bgColor.Green(), bgColor.Blue());
			} else if (GetAlpha(x, y) == 0xff) {
				result.SetAlpha(x, y, 0xff);
				result.SetRGB(x, y, GetRed(x, y), GetGreen(x, y), GetBlue(x, y));
			} else {
				float blend_factor = GetAlpha(x, y) / 255.0f;
				result.SetAlpha(x, y, GetAlpha(x, y));
				short rDiff = GetRed(x, y) - bgColor.Red();
				short gDiff = GetGreen(x, y) - bgColor.Green();
				short bDiff = GetBlue(x, y) - bgColor.Blue();
				
				result.SetRGB(x, y, PIN(bgColor.Red() + (int) (rDiff * (1.0f / blend_factor)), 0, 255), PIN(bgColor.Green() + (int) (gDiff * (1.0f / blend_factor)), 0, 255), PIN(bgColor.Blue() + (int) (bDiff * (1.0f / blend_factor)), 0, 255));
			}
		}
    }
	
	*this = result;
}

void wxImageExt::PrepareForMipmaps()
{
    if (!HasAlpha()) return;
    wxBusyCursor();

    wxImageExt result  = *this;
    // scan for background pixels
    int numBackgroundPixels = 0;
    for (int x = 0; x < GetWidth(); x++) {
	for (int y = 0; y < GetWidth(); y++) {
	    if (GetAlpha(x, y) == 0) {
		numBackgroundPixels++;
	    }
	}
    }
    
    wxProgressDialog pd(_T("Processing"), _T("Processing background pixels"), numBackgroundPixels, NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_ELAPSED_TIME);

    wxImageExt tempImage;
    tempImage.Create(GetWidth(), GetHeight());
    tempImage.InitAlpha();
    int filled_in = 1;
    while (filled_in) {
	filled_in = 0;
	int numBackgroundPixelsRemaining = 0;
	tempImage.Create(GetWidth(), GetHeight(), true);
	
	short rSum, gSum, bSum, aSum;
	int surround;
	for (int i = 0; i < GetWidth(); i++) {
	    for (int j = 0; j < GetHeight(); j++) {
		if (result.GetAlpha(i, j) != 0)
		    continue;

		numBackgroundPixelsRemaining++;
		rSum = gSum = bSum = aSum = 0;
		surround = 0;
		for (int x = -1; x <= 1; x++)
		    for (int y = -1; y <= 1; y++) {
			if (x == 0 && y == 0) continue;
			if (i + x < 0) continue;
			if (i + x > GetWidth() - 1) continue;
			if (j + y < 0) continue;
			if (j + y > GetHeight() - 1) continue;
			if (result.GetAlpha(i + x, j + y) != 0) {
			    rSum += result.GetRed(i + x, j + y);
			    gSum += result.GetGreen(i + x, j + y);
			    bSum += result.GetBlue(i + x, j + y);
			    aSum += result.GetAlpha(i + x, j + y);
			    surround++;
			}
		    }
		
		if (surround > 2)
		{
		    tempImage.SetRGB(i, j, rSum / surround, gSum / surround, bSum / surround);
		    tempImage.SetAlpha(i, j, aSum / surround);
		}
	    }
	}

	for (int i = 0; i < GetWidth(); i++)
	{
	    for (int j = 0; j < GetHeight(); j++)
	    {
		if (result.GetAlpha(i, j) == 0) {
		    if (tempImage.GetRed(i, j) != 0 || tempImage.GetGreen(i, j) != 0 || tempImage.GetBlue(i, j) != 0) {
			result.SetRGB(i, j, tempImage.GetRed(i, j), tempImage.GetGreen(i, j), tempImage.GetBlue(i, j));
			result.SetAlpha(i, j, 1);
			filled_in++;
		    }
		}
	    }
	}
	pd.Update(numBackgroundPixels - numBackgroundPixelsRemaining);
    }

    pd.Update(numBackgroundPixels);

    for (int i = 0; i < GetWidth(); i++) {
	for (int j = 0; j < GetHeight(); j++) {
	    if (result.GetAlpha(i, j) == 1) result.SetAlpha(i, j, 0);
	}
    }

    *this = result;
}

void wxImageExt::UnpremultiplyAlpha()
{
	for (int x = 0; x < GetWidth(); x++) {
	for (int y = 0; y < GetHeight(); y++) {
	    if (GetAlpha(x, y) == 0) continue;
	    short red = GetRed(x, y);
	    short green = GetGreen(x, y);
	    short blue = GetBlue(x, y);

	    red = std::min(255, 255 * red / GetAlpha(x, y));
	    green = std::min(255, 255 * green / GetAlpha(x, y));
	    blue = std::min(255, 255 * blue / GetAlpha(x, y));

	    SetRGB(x, y, (unsigned char) red, (unsigned char) green, (unsigned char) blue);
	}
    }
}
