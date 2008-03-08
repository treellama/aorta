/////////////////////////////////////////////////////////////////////////////
// Name:        imagdds.cpp
// Purpose:     wxImage DDS handler
// Author:      Gregory Smith
// Copyright:   (c) Gregory Smith
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

// currently this file has some Aleph One specializations; some formats are
// ignored or treated differently...caveat emptor

#include "imagdds.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif

#include <squish.h>

#include "Filter.h"
#include "FloatImage.h"

#include "math.h"
#include <algorithm>
#include <vector>
using namespace std;

enum InternalFormat {
	Format_None,
	Format_DXT1,
	Format_DXT3,
	Format_DXT5,
	Format_RGB,
	Format_RGBA
};

#define MAKE_FOURCC(a,b,c,d) (((wxUint32(d) << 24) | (wxUint32)(c) << 16) | ((wxUint32)(b) << 8) | (wxUint32)(a))

static inline int NextPowerOfTwo(int n)
{
	int p = 1;
	while(p < n) {p <<= 1;}
	return p;
}

bool wxDDSHandler::ReadHeader(wxInputStream& stream, DDSURFACEDESC2 &ddsd)
{
    // try to read the whole thing, then swap it
    stream.Read(&ddsd, sizeof(ddsd));
    if (stream.LastRead() != sizeof(ddsd)) return FALSE;
    
    ddsd.dwSize = wxINT32_SWAP_ON_BE(ddsd.dwSize);
    ddsd.dwFlags = wxINT32_SWAP_ON_BE(ddsd.dwFlags);
    ddsd.dwHeight = wxINT32_SWAP_ON_BE(ddsd.dwHeight);
    ddsd.dwWidth = wxINT32_SWAP_ON_BE(ddsd.dwWidth);
    ddsd.dwPitchOrLinearSize = wxINT32_SWAP_ON_BE(ddsd.dwPitchOrLinearSize);
    ddsd.dwDepth = wxINT32_SWAP_ON_BE(ddsd.dwDepth);
    ddsd.dwMipMapCount = wxINT32_SWAP_ON_BE(ddsd.dwMipMapCount);
    
    ddsd.ddpfPixelFormat.dwSize = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwSize);
    ddsd.ddpfPixelFormat.dwFlags = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwFlags);
    ddsd.ddpfPixelFormat.dwFourCC = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwFourCC);
    ddsd.ddpfPixelFormat.dwRGBBitCount = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwRGBBitCount);
    
    ddsd.ddsCaps.dwCaps1 = wxINT32_SWAP_ON_BE(ddsd.ddsCaps.dwCaps1);
    ddsd.ddsCaps.dwCaps2 = wxINT32_SWAP_ON_BE(ddsd.ddsCaps.dwCaps2);
    
    return TRUE;
    
}

bool wxDDSHandler::WriteHeader(wxOutputStream &stream, DDSURFACEDESC2 &ddsd)
{
    wxUint32 dwMagic = (wxINT32_SWAP_ON_BE(MAKE_FOURCC('D', 'D', 'S', ' ')));
    stream.Write(&dwMagic, sizeof(dwMagic));

    ddsd.dwSize = wxINT32_SWAP_ON_BE(ddsd.dwSize);
    ddsd.dwFlags = wxINT32_SWAP_ON_BE(ddsd.dwFlags);
    ddsd.dwHeight = wxINT32_SWAP_ON_BE(ddsd.dwHeight);
    ddsd.dwWidth = wxINT32_SWAP_ON_BE(ddsd.dwWidth);
    ddsd.dwPitchOrLinearSize = wxINT32_SWAP_ON_BE(ddsd.dwPitchOrLinearSize);
    ddsd.dwDepth = wxINT32_SWAP_ON_BE(ddsd.dwDepth);
    ddsd.dwMipMapCount = wxINT32_SWAP_ON_BE(ddsd.dwMipMapCount);
    
    ddsd.ddpfPixelFormat.dwSize = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwSize);
    ddsd.ddpfPixelFormat.dwFlags = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwFlags);
    ddsd.ddpfPixelFormat.dwFourCC = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwFourCC);
    ddsd.ddpfPixelFormat.dwRGBBitCount = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwRGBBitCount);
    
    ddsd.ddsCaps.dwCaps1 = wxINT32_SWAP_ON_BE(ddsd.ddsCaps.dwCaps1);
    ddsd.ddsCaps.dwCaps2 = wxINT32_SWAP_ON_BE(ddsd.ddsCaps.dwCaps2);

    stream.Write(&ddsd, sizeof(ddsd));
    return TRUE;
}

bool wxDDSHandler::DoCanRead(wxInputStream& stream)
{
    wxUint32 dwMagic;
    stream.Read(&dwMagic, sizeof(dwMagic));
    if (stream.LastRead() != sizeof(dwMagic)) return FALSE;
    
    if (wxINT32_SWAP_ON_BE(dwMagic) != MAKE_FOURCC('D', 'D', 'S', ' ')) return FALSE;

    DDSURFACEDESC2 ddsd;
    if (!ReadHeader(stream, ddsd)) return FALSE;

    // validate the sizes
    if (ddsd.dwSize != 124) return FALSE;
    if (ddsd.ddpfPixelFormat.dwSize != 32) return FALSE;

    if (ddsd.ddsCaps.dwCaps2 & DDSCAPS2_VOLUME) return FALSE;
    if (ddsd.ddsCaps.dwCaps2 & DDSCAPS2_CUBEMAP) return FALSE;
    
    if (ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) {
	return (ddsd.ddpfPixelFormat.dwRGBBitCount == 24 || ddsd.ddpfPixelFormat.dwRGBBitCount == 32);
    }
    else if ((ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC) &&
	     (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '1') ||
	      ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '2') ||
	      ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '3') ||
	      ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '4') ||
	      ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '5')))
	return TRUE;
    else
	return FALSE;
    
}

static void UnpremultiplyAlpha(wxImage *image)
{
    for (int x = 0; x < image->GetWidth(); x++) {
	for (int y = 0; y < image->GetHeight(); y++) {
	    if (image->GetAlpha(x, y) == 0) continue;
	    short red = image->GetRed(x, y);
	    short green = image->GetGreen(x, y);
	    short blue = image->GetBlue(x, y);

	    red = std::min(255, 255 * red / image->GetAlpha(x, y));
	    green = std::min(255, 255 * green / image->GetAlpha(x, y));
	    blue = std::min(255, 255 * blue / image->GetAlpha(x, y));

	    image->SetRGB(x, y, (unsigned char) red, (unsigned char) green, (unsigned char) blue);
	}
    }
}

bool wxDDSHandler::LoadFile(wxImage *image, wxInputStream& stream, bool verbose, int index)
{
    wxUint32 dwMagic;
    stream.Read(&dwMagic, sizeof(dwMagic));
    if (stream.LastRead() != sizeof(dwMagic)) return FALSE;

    if (wxINT32_SWAP_ON_BE(dwMagic) != MAKE_FOURCC('D', 'D', 'S', ' ')) return FALSE;
    
    DDSURFACEDESC2 ddsd;
    if (!ReadHeader(stream, ddsd)) return FALSE;
    
    bool unpremultiplyAlpha = false;
    
    // just read the first mipmap
    InternalFormat internalFormat = Format_None;
    if (ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
	if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '1'))
	    internalFormat = Format_DXT1;
	else if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '2')) {
	    unpremultiplyAlpha = true;
	    internalFormat = Format_DXT3;
	}
	else if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '3'))
	    internalFormat = Format_DXT3;
	else if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '4')) {
	    unpremultiplyAlpha = true;
	    internalFormat = Format_DXT5;
	}
	else if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '5'))
	    internalFormat = Format_DXT5;
    } else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) {
	if (ddsd.ddpfPixelFormat.dwRGBBitCount == 24) {
	    internalFormat = Format_RGB;
	} else if (ddsd.ddpfPixelFormat.dwRGBBitCount == 32) {
	    internalFormat = Format_RGBA;
	}
    }
    if (internalFormat == Format_None) return FALSE;

    int width = ddsd.dwWidth;
    int height = ddsd.dwHeight;

    if (internalFormat == Format_RGB || internalFormat == Format_RGBA) {
	int pitch;
	if (ddsd.dwFlags & DDSD_PITCH) {
	    pitch = ddsd.dwPitchOrLinearSize;
	} else {
	    pitch = ddsd.dwPitchOrLinearSize / ddsd.dwHeight;
	}

	if (pitch != ((internalFormat == Format_RGB) ? 3 : 4) * width)
	{
	    fprintf(stderr, "we don't know how to do weird pitch\n");
	    return FALSE;
	}

	image->Create(width, height);
	if (internalFormat == Format_RGBA) 
	    image->InitAlpha();
	for (int y = 0; y < height; y++) {
	    for (int x = 0; x < width; x++) {
		unsigned char b = stream.GetC();
		if (stream.LastRead() != 1) {
		    return FALSE;
		}
		unsigned char g = stream.GetC();
		if (stream.LastRead() != 1) {
		    return FALSE;
		}
		unsigned char r = stream.GetC();
		if (stream.LastRead() != 1) {
		    return FALSE;
		}

		image->SetRGB(x, y, r, g, b);

		if (internalFormat == Format_RGBA)
		{
		    image->SetAlpha(x, y, stream.GetC());
		    if (stream.LastRead() != 1) {
			return FALSE;
		    }
		}
	    }
	}
    
	return TRUE;
    }
    
    int bpp = (internalFormat == Format_DXT1) ? 4 : 8;
    int potWidth = NextPowerOfTwo(width);
    int potHeight = NextPowerOfTwo(height);

    int compressedBufferSize = (potWidth * potHeight * bpp) / 8;

    vector<unsigned char> compressedBuffer(width * height * bpp / 8);
    stream.Read(&compressedBuffer.front(), compressedBuffer.size());
    if (stream.LastRead() != compressedBuffer.size()) {
	return FALSE;
    }

    image->Create(width, height);
    vector<unsigned char> uncompressedBuffer(width * height * 4);
    int flags;
    if (internalFormat == Format_DXT1) {
	flags = squish::kDxt1;
    } else {
	if (internalFormat == Format_DXT3) {
	    flags = squish::kDxt3;
	} else {
	    flags = squish::kDxt5;
	}
	image->InitAlpha();
    }
	  
    squish::DecompressImage(&uncompressedBuffer.front(), width, height, &compressedBuffer.front(), flags);
    
    for (int x = 0; x < width; x++) {
	for (int y = 0; y < height; y++) {
	    image->GetData()[(x + y * width) * 3 + 0] = uncompressedBuffer[(x + y * width) * 4 + 0];
	    image->GetData()[(x + y * width) * 3 + 1] = uncompressedBuffer[(x + y * width) * 4 + 1];
	    image->GetData()[(x + y * width) * 3 + 2] = uncompressedBuffer[(x + y * width) * 4 + 2];
	    if (image->HasAlpha()) {
		image->GetAlpha()[(x + y * width)] = uncompressedBuffer[(x + y * width) * 4 + 3];
	    }
	}
	
    }

    if (unpremultiplyAlpha)
	UnpremultiplyAlpha(image);

    return TRUE;
}

static void PremultiplyAlpha(wxImage *image)
{
    for (int x = 0; x < image->GetWidth(); x++) {
	for (int y = 0; y < image->GetHeight(); y++) {
	    short red = image->GetRed(x, y);
	    short green = image->GetGreen(x, y);
	    short blue = image->GetBlue(x, y);
	    
	    red = (image->GetAlpha(x, y) * red + 127) / 255;
	    green = (image->GetAlpha(x, y) * green + 127) / 255;
	    blue = (image->GetAlpha(x, y) * blue + 127) / 255;
	    
	    image->SetRGB(x, y, (unsigned char) red, (unsigned char) green, (unsigned char) blue);
	}    
    }
}

bool wxDDSHandler::SaveFile(wxImage *image, wxOutputStream& stream, bool verbose)
{

    wxBusyCursor getBusy;

    bool mipmap = image->HasOption(wxIMAGE_OPTION_DDS_USE_MIPMAPS) && 
	image->GetOptionInt(wxIMAGE_OPTION_DDS_USE_MIPMAPS);
    bool compress = image->HasOption(wxIMAGE_OPTION_DDS_COMPRESS) &&
	image->GetOptionInt(wxIMAGE_OPTION_DDS_COMPRESS);
    bool premultiply = image->HasOption(wxIMAGE_OPTION_DDS_PREMULTIPLY_ALPHA) &&
	image->GetOptionInt(wxIMAGE_OPTION_DDS_PREMULTIPLY_ALPHA);

    if (compress)  {
	if ((image->GetHeight() & 3) || (image->GetWidth() & 3)) {
	    image->Rescale((image->GetWidth() + 3) & ~3, (image->GetHeight() + 3) & ~3);
	}
    }
    
    DDSURFACEDESC2 ddsd;
    memset(&ddsd, 0, sizeof(ddsd));
    ddsd.dwSize = 124;
    ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_PIXELFORMAT;
    ddsd.dwHeight = image->GetHeight();
    ddsd.dwWidth = image->GetWidth();
    if (compress) {
	ddsd.dwFlags |= DDSD_LINEARSIZE;
	if (image->HasAlpha()) {
	    ddsd.dwPitchOrLinearSize = image->GetWidth() / 4 * image->GetHeight() / 4 * 16;
	    
	    ddsd.ddpfPixelFormat.dwFourCC = MAKE_FOURCC('D', 'X', 'T', '5');
	} else {
	    ddsd.dwPitchOrLinearSize = image->GetWidth() / 4 * image->GetHeight() / 4 * 8;
	    ddsd.ddpfPixelFormat.dwFourCC = MAKE_FOURCC('D', 'X', 'T', '1');
	}
    } else {
	ddsd.dwFlags |= DDSD_PITCH;
	if (image->HasAlpha()) {
	    ddsd.dwPitchOrLinearSize = image->GetWidth() * 4;
	} else {
	    ddsd.dwPitchOrLinearSize = image->GetWidth() * 3;
	}
    }
    
    int mipmap_count;
    
    if (mipmap) {
	    mipmap_count = ddsd.dwMipMapCount = NumMipmaps(*image);
	    ddsd.dwFlags |= DDSD_MIPMAPCOUNT;
    }
    
    ddsd.ddpfPixelFormat.dwSize = 32;
    if (compress) {
	ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;
    } else {
	ddsd.ddpfPixelFormat.dwFlags = DDPF_RGB;
	ddsd.ddpfPixelFormat.dwRBitMask = wxINT32_SWAP_ON_BE(0x00ff0000);
	ddsd.ddpfPixelFormat.dwGBitMask = wxINT32_SWAP_ON_BE(0x0000ff00);
	ddsd.ddpfPixelFormat.dwBBitMask = wxINT32_SWAP_ON_BE(0x000000ff);
	if (image->HasAlpha())
	{
	    ddsd.ddpfPixelFormat.dwRGBBitCount = 32;
	    ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = wxINT32_SWAP_ON_BE(0xff000000);
	    ddsd.ddpfPixelFormat.dwFlags |= DDPF_ALPHAPIXELS;
	} else {
	    ddsd.ddpfPixelFormat.dwRGBBitCount = 24;
	}
    }
    
    ddsd.ddsCaps.dwCaps1 = DDSCAPS_TEXTURE;
    if (mipmap) ddsd.ddsCaps.dwCaps1 |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
    
    WriteHeader(stream, ddsd);
    
    if (premultiply) 
	PremultiplyAlpha(image);

    long filter = wxIMAGE_OPTION_DDS_FILTER_BOX;
    if (image->HasOption(wxIMAGE_OPTION_DDS_MIPMAP_FILTER))
	filter = image->GetOptionInt(wxIMAGE_OPTION_DDS_MIPMAP_FILTER);

    long wrap_mode = wxIMAGE_OPTION_DDS_WRAP_CLAMP;
    if (image->HasOption(wxIMAGE_OPTION_DDS_MIPMAP_WRAP_MODE))
	wrap_mode = image->GetOptionInt(wxIMAGE_OPTION_DDS_MIPMAP_WRAP_MODE);

    wxImage minImage = *image;
    
    for (int level = 0; level < ((mipmap) ? mipmap_count : 1); level++) {
	if (level) minImage = Minify(minImage, filter, wrap_mode);
	if (compress) {
	    if (image->HasAlpha()) {
		WriteDXT5(minImage, stream);
	    } else {
		WriteDXT1(minImage, stream);
	    }
	} else {
	    WriteRGBA(minImage, stream);
	}
    }
}

int wxDDSHandler::NumMipmaps(const wxImage &image)
{
    return (1 + (int) floor(log(std::max(image.GetWidth(), image.GetHeight())) / log(2)));
}

static vector<unsigned char> BuildRGBAImage(const wxImage& image)
{
    vector<unsigned char> buffer(image.GetWidth() * image.GetHeight() * 4);
    for (int x = 0; x < image.GetWidth(); x++) {
	for (int y = 0; y < image.GetHeight(); y++) {
	    buffer[(x + y * image.GetWidth()) * 4 + 0] = image.GetRed(x, y);
	    buffer[(x + y * image.GetWidth()) * 4 + 1] = image.GetGreen(x, y);
	    buffer[(x + y * image.GetWidth()) * 4 + 2] = image.GetBlue(x, y);
	    if (image.HasAlpha())
		buffer[(x + y * image.GetWidth()) * 4 + 3] = image.GetAlpha(x, y);
	    else
		buffer[(x + y * image.GetWidth()) * 4 + 3] = 0xff;
	}
    }

    return buffer;
}


void wxDDSHandler::WriteDXT1(const wxImage& image, wxOutputStream& stream)
{
    // use DXT3 to force 4 colors
    vector<unsigned char> compressedBuffer(squish::GetStorageRequirements(image.GetWidth(), image.GetHeight(), squish::kDxt3));
    squish::CompressImage(&BuildRGBAImage(image).front(), image.GetWidth(), image.GetHeight(), &compressedBuffer.front(), squish::kDxt3 | squish::kColourIterativeClusterFit);

    // skip the alpha blocks, and just write the colors
    unsigned int i = 0;
    while (i <= compressedBuffer.size() - 16)
    {
	i += 8;
	stream.Write(&compressedBuffer[i], 8);
	i += 8;
    }
}


void wxDDSHandler::WriteDXT5(const wxImage& image, wxOutputStream& stream)
{
    vector<unsigned char> compressedBuffer(squish::GetStorageRequirements(image.GetWidth(), image.GetHeight(), squish::kDxt5));
    squish::CompressImage(&BuildRGBAImage(image).front(), image.GetWidth(), image.GetHeight(), &compressedBuffer.front(), squish::kDxt5);
    stream.Write(&compressedBuffer.front(), compressedBuffer.size());
}

void wxDDSHandler::WriteRGBA(const wxImage& image, wxOutputStream& stream)
{
    for (int y = 0; y < image.GetHeight(); y++) {
	for (int x = 0; x < image.GetWidth(); x++) {
	    stream.PutC(image.GetBlue(x, y));
	    stream.PutC(image.GetGreen(x, y));
	    stream.PutC(image.GetRed(x, y));
	    if (image.HasAlpha())
	    {
		stream.PutC(image.GetAlpha(x, y));
	    }
	}
    }	
}

wxImage wxDDSHandler::Minify(wxImage &image, long filter, long wrap_mode)
{
    if (image.GetWidth() == 1 && image.GetHeight() == 1) return image;

    FloatImage f;

    f.allocate(4, image.GetWidth(), image.GetHeight());
    for (int x = 0; x < image.GetWidth(); ++x) {
	for (int y = 0; y < image.GetHeight(); ++y) {
	    f.setPixel(image.GetRed(x, y) / 255.0f, x, y, 0);
	    f.setPixel(image.GetGreen(x, y) / 255.0f, x, y, 1);
	    f.setPixel(image.GetBlue(x, y) / 255.0f, x, y, 2);
	    if (image.HasAlpha()) {
		f.setPixel(image.GetAlpha(x, y) / 255.0f, x, y, 3);
	    } else {
		f.setPixel(1.0f, x, y, 3);
	    }
	}
    }

    // convert channels 0 through 2 (R through B) to linear space
    f.toLinear(0, 2);

    wxImage minifiedImage;

    FloatImage::WrapMode wm = FloatImage::WrapMode_Clamp;
    if (wrap_mode == wxIMAGE_OPTION_DDS_WRAP_REPEAT)
	wm = FloatImage::WrapMode_Repeat;
    else if (wrap_mode == wxIMAGE_OPTION_DDS_WRAP_MIRROR)
	wm = FloatImage::WrapMode_Mirror;

    std::auto_ptr<FloatImage> minif;
    if (filter == wxIMAGE_OPTION_DDS_FILTER_TRIANGLE) {
	TriangleFilter filter;
	minif.reset(f.downSample(TriangleFilter(), wm));
    } else if (filter == wxIMAGE_OPTION_DDS_FILTER_KAISER) {
	minif.reset(f.downSample(KaiserFilter(3), wm));
    } else {
	    minif.reset(f.fastDownSample());
    }
    
    minif->toGamma(0, 2);
    minifiedImage.Create(minif->width(), minif->height());
    
    for (int x = 0; x < minif->width(); ++x) {
	for (int y = 0; y < minif->height(); ++y) {
	    unsigned int r = clamp((int) (minif->pixel(x, y, 0) * 255.0f), 0, 255);
	    unsigned int g = clamp((int) (minif->pixel(x, y, 1) * 255.0f), 0, 255);
	    unsigned int b = clamp((int) (minif->pixel(x, y, 2) * 255.0f), 0, 255);
	    minifiedImage.SetRGB(x, y, r, g, b);
	}
    }

    if (image.HasAlpha()) {
	minifiedImage.InitAlpha();
	for (int x = 0; x < minif->width(); ++x) {
	    for (int y = 0; y < minif->height(); ++y) {
		minifiedImage.SetAlpha(x, y, clamp((int) (minif->pixel(x, y, 3) * 255.0f), 0, 255));
	    }
	}
    }

    return minifiedImage;
}


    
