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
	Rescale(GetWidth() * scale, GetHeight() * scale);
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

