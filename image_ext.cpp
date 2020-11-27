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
#include <cmath>
#include <vector>

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
	float scale = (float) std::max(width, height) / std::max(GetWidth(), GetHeight());
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
		unsigned char Max = std::max(GetData()[i * 3 + 0], std::max(GetData()[i * 3 + 1], GetData()[i * 3 + 2]));
		image.GetData()[i * 3 + 0] = image.GetData()[i * 3 + 1] = image.GetData()[i * 3 + 2] = Max;
	}
	
	*this = image;
}

static inline unsigned char WrapBlue(const wxImageExt& image, int x, int y, int w, int h) {
	if (x < 0) x += w; else if (x > w - 1) x -= w;
	if (y < 0) y += h; else if (y > h - 1) y -= h;
	return image.GetData()[(y * w + x) * 3 + 2];
}

void wxImageExt::MakeNormalMap()
{
	wxImageExt image;
	image.Create(GetWidth(), GetHeight());

	float fs, ft, fr, fd;
	unsigned char mat[3][3];

	int width = GetWidth();
	int height = GetHeight();

	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			mat[0][0] = WrapBlue(*this, x - 1, y - 1, width, height);
			mat[0][1] = WrapBlue(*this, x, y - 1, width, height);
			mat[0][2] = WrapBlue(*this, x + 1, y - 1, width, height);
			mat[1][0] = WrapBlue(*this, x - 1, y, width, height);
			mat[1][2] = WrapBlue(*this, x + 1, y, width, height);
			mat[2][0] = WrapBlue(*this, x - 1, y + 1, width, height);
			mat[2][1] = WrapBlue(*this, x, y + 1, width, height);
			mat[2][2] = WrapBlue(*this, x + 1, y + 1, width, height);

			// Sobel operator horizontal
			fs = (1.0 * mat[0][0] - 1.0 * mat[0][2] + 
			      2.0 * mat[1][0] - 2.0 * mat[1][2] +
			      1.0 * mat[2][0] - 1.0 * mat[2][2]) / 255.0;

			ft = (1.0 * mat[2][0] - 1.0 * mat[0][0] +
			      2.0 * mat[2][1] - 2.0 * mat[0][1] +
			      1.0 * mat[2][2] - 1.0 * mat[0][2]) / 255.0;

			fr = 0.5 * std::sqrt(1.0 + fs * fs + ft * ft);

			// normalize vector (r, s, t)
			fd = 1.0 / std::sqrt(fr * fr + fs * fs + ft * ft);
			fs *= fd;
			ft *= fd;
			fr *= fd;

			image.SetRGB(x, y, (unsigned char) (0x80 + fs * 0x7f), (unsigned char) (0x80 + ft * 0x7f), (unsigned char) (0x80 + fr * 0x7f));
		}
	}		
	
	*this = image;
			
}

// thanks to the Virtual Terrain Project for this algorithm, and the code looks
// pretty similar too...

void wxImageExt::ReconstructColors(const wxColour& bgColor)
{
	// restore the color of edge texels by guessing correct non-background color
	
    for (int x = 0; x < GetWidth(); x++) {
		for (int y = 0; y < GetHeight(); y++) {
			if (GetAlpha(x, y) == 0) {
				SetAlpha(x, y, 0);
				SetRGB(x, y, bgColor.Red(), bgColor.Green(), bgColor.Blue());
			} else if (GetAlpha(x, y) == 0xff) {
				SetAlpha(x, y, 0xff);
				SetRGB(x, y, GetRed(x, y), GetGreen(x, y), GetBlue(x, y));
			} else {
				float blend_factor = GetAlpha(x, y) / 255.0f;
				SetAlpha(x, y, GetAlpha(x, y));
				short rDiff = GetRed(x, y) - bgColor.Red();
				short gDiff = GetGreen(x, y) - bgColor.Green();
				short bDiff = GetBlue(x, y) - bgColor.Blue();
				
				SetRGB(x, y, PIN(bgColor.Red() + (int) (rDiff * (1.0f / blend_factor)), 0, 255), PIN(bgColor.Green() + (int) (gDiff * (1.0f / blend_factor)), 0, 255), PIN(bgColor.Blue() + (int) (bDiff * (1.0f / blend_factor)), 0, 255));
			}
		}
    }
	
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

static bool Downsample(wxImage &src, wxImage &dst)
{
	if (src.GetWidth() <= 2 || src.GetHeight() <= 2) return false;
	int holes = 0;
	for (int y = 0; y < src.GetHeight(); y++) {
		for (int x = 0; x < src.GetWidth(); x++) {
			if (src.GetAlpha(x, y) == 0) {
				holes++;
			}
		}
	}

	if (holes == 0) return false;

	dst.Create(src.GetWidth() / 2, src.GetHeight() / 2, true);
	dst.InitAlpha();

	for (int y = 0; y < dst.GetHeight(); y++) {
		for (int x = 0; x < dst.GetWidth(); x++) {

			const int x0 = 2 * x + 0;
			const int x1 = 2 * x + 1;
			const int y0 = 2 * y + 0;
			const int y1 = 2 * y + 1;

			if (src.GetAlpha(x0, y0) || src.GetAlpha(x1, y0) || src.GetAlpha(x0, y1) || src.GetAlpha(x1, y1))
			{
				unsigned int rSum = 0;
				unsigned int gSum = 0;
				unsigned int bSum  = 0;
				unsigned int aSum = 0;
				int total = 0;
				if (src.GetAlpha(x0, y0)) {
					rSum += src.GetRed(x0, y0);
					gSum += src.GetGreen(x0, y0);
					bSum += src.GetBlue(x0, y0);
					total++;
				}	
				if (src.GetAlpha(x1, y0)) {
					rSum += src.GetRed(x1, y0);
					gSum += src.GetGreen(x1, y0);
					bSum += src.GetBlue(x1, y0);
					total++;
				}
				if (src.GetAlpha(x0, y1)) {
					rSum += src.GetRed(x0, y1);
					gSum += src.GetGreen(x0, y1);
					bSum += src.GetBlue(x0, y1);
					total++;
				}
				if (src.GetAlpha(x1, y1)) {
					rSum += src.GetRed(x1, y1);
					gSum += src.GetGreen(x1, y1);
					bSum += src.GetBlue(x1, y1);
					total++;
				}

				dst.SetRGB(x, y, (unsigned char) (rSum / total), (unsigned char) (gSum / total), (unsigned char) (bSum / total));
				dst.SetAlpha(x, y, 255);
			} else {
				dst.SetRGB(x, y, 0, 0, 0);
				dst.SetAlpha(x, y, 0);
			}
		}
	}

	return true;
				     
}

// This is the filter used in the Lumigraph paper. The Unreal engine uses something similar.
void wxImageExt::PrepareForMipmaps()
{
	std::vector<wxImage> mipmaps;

	mipmaps.push_back(*this);

	wxImageExt temp;
	while (Downsample(mipmaps.back(), temp))
	{
		mipmaps.push_back(temp);
	}

	for (int y = 0; y < GetHeight(); y++) {
		for (int x = 0; x < GetWidth(); x++) {
                        
			int sx = x;
			int sy = y;
                        
			for (int l = 0; l < mipmaps.size(); l++) {
                                if (sx >= mipmaps[l].GetWidth()) {
                                        sx = mipmaps[l].GetWidth() - 1;
                                }

                                if (sy >= mipmaps[l].GetHeight()) {
                                        sy = mipmaps[l].GetHeight() - 1;
                                }

                                if (mipmaps[l].GetAlpha(sx, sy)) {
					SetRGB(x, y, mipmaps[l].GetRed(sx, sy), mipmaps[l].GetGreen(sx, sy), mipmaps[l].GetBlue(sx, sy));
					break;
				}
                                
				sx /= 2;
				sy /= 2;
			}
		}
	}
}
