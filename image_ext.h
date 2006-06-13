/*
 
 image_ext.h: extra functionality for wxImage
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

#ifndef IMAGE_EXT_H
#define IMAGE_EXT_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#include <wx/image.h>
#include <wx/progdlg.h>
#endif

#ifndef MAX
#define MAX(x, y) (y < x ? x : y)
#endif
#ifndef MIN
#define MIN(a,b) ((a)<=(b) ? (a) : (b))
#endif
#define FLOOR(value,floor) MAX(value,floor)
#define CEILING(value,ceiling) MIN(value,ceiling)

#define PIN(value,floor,ceiling) (CEILING(FLOOR((value),(floor)),(ceiling)))

class wxImageExt : public wxImage
{
public:
	wxImageExt() : wxImage() { }
	wxImageExt(const wxImageExt& image) : wxImage(image) { }
	wxImageExt(int width, int height, bool clear=true) : wxImage(width, height, clear) { }
	
	void White(); // make the image entirely opaque white
	void RemoveAlpha(); // remove the alpha channel
	void MaskToAlpha(); // convert the mask to an alpha channel
	void PropRescale(int width, int height); // scale to fit but maintain proportions
	void FromAlpha(const wxImageExt& source); // create a grayscale representation of source's alpha
	void ToAlpha(wxImageExt& dest) const; // export our grayscale representation to the dest's alpha channel
	void MakeOpacTypeTwo();
	void MakeOpacTypeThree();

	void ReconstructColors(const wxColour&);
	void PrepareForMipmaps();
};

#endif
