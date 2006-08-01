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
#ifndef __WIN32__
#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES 1
#endif
#endif
#include <wx/glcanvas.h>
#endif

#ifdef __APPLE__
#include <OpenGL/glu.h>
#endif

#include "math.h"
#include <vector>
using namespace std;

#ifndef MAX
#define MAX(x, y) (y < x ? x : y)
#endif

#define MAKE_FOURCC(a,b,c,d) (((wxUint32(d) << 24) | (wxUint32)(c) << 16) | ((wxUint32)(b) << 8) | (wxUint32)(a))

static inline int NextPowerOfTwo(int n)
{
	int p = 1;
	while(p < n) {p <<= 1;}
	return p;
}

// I don't know of any way to get an opengl context without a window, so:
class SubtleOpenGLContext
{
public:
    SubtleOpenGLContext() {
	frame = new wxFrame(NULL, -1, _T(""), wxPoint(0, 0), wxSize(1, 1), wxFRAME_NO_TASKBAR);
	canvas = new wxGLCanvas(frame, -1, wxDefaultPosition, wxDefaultSize);
	frame->Show();
	canvas->SetCurrent();

	glHint(GL_TEXTURE_COMPRESSION_HINT_ARB, GL_NICEST);

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glGenTextures(1, &textureRef);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureRef);
    }
    
    ~SubtleOpenGLContext() {
	glDeleteTextures(1, &textureRef);
	frame->Close();
    }
private:
    wxFrame *frame;
    wxGLCanvas *canvas;
    GLuint textureRef;
};

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
	      ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '3') ||
	      ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '5')))
	return TRUE;
    else
	return FALSE;
    
}

bool wxDDSHandler::LoadFile(wxImage *image, wxInputStream& stream, bool verbose, int index)
{
    wxUint32 dwMagic;
    stream.Read(&dwMagic, sizeof(dwMagic));
    if (stream.LastRead() != sizeof(dwMagic)) return FALSE;

    if (wxINT32_SWAP_ON_BE(dwMagic) != MAKE_FOURCC('D', 'D', 'S', ' ')) return FALSE;
    
    DDSURFACEDESC2 ddsd;
    if (!ReadHeader(stream, ddsd)) return FALSE;
    
    // just read the first mipmap
    GLenum internalFormat = GL_NONE;
    if (ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
	if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '1'))
	    internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	else if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '3'))
	    internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	else if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '5'))
	    internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    } else if (ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) {
	if (ddsd.ddpfPixelFormat.dwRGBBitCount == 24) {
	    internalFormat = GL_RGB;
	} else if (ddsd.ddpfPixelFormat.dwRGBBitCount == 32) {
	    internalFormat = GL_RGBA;
	}
    }
    if (internalFormat == GL_NONE) return FALSE;

    int width = ddsd.dwWidth;
    int height = ddsd.dwHeight;

    if (internalFormat == GL_RGB || internalFormat == GL_RGBA) {
	int pitch;
	if (ddsd.dwFlags & DDSD_PITCH) {
	    pitch = ddsd.dwPitchOrLinearSize;
	} else {
	    pitch = ddsd.dwPitchOrLinearSize / ddsd.dwHeight;
	}

	if (pitch != ((internalFormat == GL_RGB) ? 3 : 4) * width)
	{
	    fprintf(stderr, "we don't know how to do weird pitch\n");
	    return FALSE;
	}

	image->Create(width, height);
	if (internalFormat == GL_RGBA) 
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

		if (internalFormat == GL_RGBA)
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
    
    int bpp = (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ? 4 : 8;
    int potWidth = NextPowerOfTwo(width);
    int potHeight = NextPowerOfTwo(height);

    int compressedBufferSize = (potWidth * potHeight * bpp) / 8;

    GLbyte *compressedBuffer = new GLbyte[compressedBufferSize];
    for (int row = 0; row < height / 4; row++) {
	stream.Read(&compressedBuffer[row * potWidth / 4 * bpp * 2], width / 4 * bpp * 2);
	if (stream.LastRead() != width / 4 * bpp * 2) {
	    delete[] compressedBuffer;
	    return FALSE;
	}
    }

    {
	SubtleOpenGLContext context;
	
	glCompressedTexImage2DARB(GL_TEXTURE_2D, 0, internalFormat, potWidth, potHeight, 0, compressedBufferSize, compressedBuffer);
	
	if (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) {
	    image->Create(width, height);
	    GLbyte *uncompressedBuffer = new GLbyte[potWidth * potHeight * 3];
	    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGB, GL_UNSIGNED_BYTE, uncompressedBuffer);
	    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
		    image->GetData()[(x + y * width) * 3 + 0] = uncompressedBuffer[(x + y * potWidth) * 3 + 0];
		    image->GetData()[(x + y * width) * 3 + 1] = uncompressedBuffer[(x + y * potWidth) * 3 + 1];
		    image->GetData()[(x + y * width) * 3 + 2] = uncompressedBuffer[(x + y * potWidth) * 3 + 2];
		}
	    }

	    delete [] uncompressedBuffer;
	} else if (internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT3_EXT || internalFormat == GL_COMPRESSED_RGBA_S3TC_DXT5_EXT) {
	    GLbyte *uncompressedBuffer = new GLbyte[potWidth * potHeight * 4];
	    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, uncompressedBuffer);
	    
	    image->Create(width, height);
	    image->InitAlpha();
	    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
		    image->GetData()[(x + y * width) * 3 + 0] = uncompressedBuffer[(x + y * potWidth) * 4 + 0];
		    image->GetData()[(x + y * width) * 3 + 1] = uncompressedBuffer[(x + y * potWidth) * 4 + 1];
		    image->GetData()[(x + y * width) * 3 + 2] = uncompressedBuffer[(x + y * potWidth) * 4 + 2];
		    image->GetAlpha()[x + y * width] = uncompressedBuffer[(x + y * potWidth) * 4 + 3];
		}
	    }
	    
	    delete[] uncompressedBuffer;
	}

    }

    delete[] compressedBuffer;
    return TRUE;
}

bool wxDDSHandler::SaveFile(wxImage *image, wxOutputStream& stream, bool verbose)
{
    bool mipmap = image->HasOption(wxIMAGE_OPTION_DDS_USE_MIPMAPS) && 
	image->GetOptionInt(wxIMAGE_OPTION_DDS_USE_MIPMAPS);

    if ((image->GetHeight() & 3) || (image->GetWidth() & 3)) {
	image->Rescale((image->GetWidth() + 3) & ~3, (image->GetHeight() + 3) & ~3);
    }

    {
	SubtleOpenGLContext glContext;

	DDSURFACEDESC2 ddsd;
	memset(&ddsd, 0, sizeof(ddsd));
	ddsd.dwSize = 124;
	ddsd.dwFlags = DDSD_CAPS | DDSD_HEIGHT | DDSD_WIDTH | DDSD_LINEARSIZE | DDSD_PIXELFORMAT;
	ddsd.dwHeight = image->GetHeight();
	ddsd.dwWidth = image->GetWidth();
	if (image->HasAlpha()) {
	    ddsd.dwPitchOrLinearSize = image->GetWidth() / 4 * image->GetHeight() / 4 * 16;
	    ddsd.ddpfPixelFormat.dwFourCC = MAKE_FOURCC('D', 'X', 'T', '5');
	} else {
	    ddsd.dwPitchOrLinearSize = image->GetWidth() / 4 * image->GetHeight() / 4 * 8;
	    ddsd.ddpfPixelFormat.dwFourCC = MAKE_FOURCC('D', 'X', 'T', '1');
	}
	
	int mipmap_count;
	
	if (mipmap) {
	    mipmap_count = ddsd.dwMipMapCount = NumMipmaps(image);
	    ddsd.dwFlags |= DDSD_MIPMAPCOUNT;
	}

	ddsd.ddpfPixelFormat.dwSize = 32;
	ddsd.ddpfPixelFormat.dwFlags = DDPF_FOURCC;

	ddsd.ddsCaps.dwCaps1 = DDSCAPS_TEXTURE;
	if (mipmap) ddsd.ddsCaps.dwCaps1 |= DDSCAPS_MIPMAP | DDSCAPS_COMPLEX;
	
	WriteHeader(stream, ddsd);
	wxImage minImage = *image;
	
	for (int level = 0; level < ((mipmap) ? mipmap_count : 1); level++) {
	    if (image->HasAlpha()) {
		WriteDXT5(minImage, stream);
	    } else {
		WriteDXT1(minImage, stream);
	    }
	    minImage = Minify(minImage);
	}

    }
}

int wxDDSHandler::NumMipmaps(const wxImage &image)
{
    return (1 + (int) floor(log(MAX(image.GetWidth(), image.GetHeight())) / log(2)));
}

void wxDDSHandler::WriteDXT1(const wxImage& image, wxOutputStream& stream)
{
    int potWidth = NextPowerOfTwo(image.GetWidth());
    int potHeight = NextPowerOfTwo(image.GetHeight());

    wxImage potImage = image.Size(wxSize(potWidth, potHeight), wxPoint(0, 0), 0, 0, 0);
    glHint(GL_TEXTURE_COMPRESSION_HINT_ARB, GL_NICEST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGB_S3TC_DXT1_EXT, potWidth, potHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, potImage.GetData());
    
    vector<GLbyte> compressedBuffer(MAX(potWidth / 4, 1) * MAX(potHeight / 4, 1) * 8);
    glGetCompressedTexImage(GL_TEXTURE_2D, 0, &compressedBuffer.front());

    for (int row = 0; row < (image.GetHeight() + 3 & ~3) / 4; row++) {
	stream.Write(&compressedBuffer[row * potWidth / 4 * 8], (image.GetWidth() + 3 & ~3) / 4 * 8);
    }
}

void wxDDSHandler::WriteDXT5(const wxImage& image, wxOutputStream& stream)
{
    int potWidth = NextPowerOfTwo(image.GetWidth());
    int potHeight = NextPowerOfTwo(image.GetHeight());

    vector<GLbyte> uncompressedBuffer(potWidth * potHeight * 4);
    for (int x = 0; x < image.GetWidth(); x++) {
	for (int y = 0; y < image.GetHeight(); y++) {
	    uncompressedBuffer[(x + y * potWidth) * 4 + 0] = image.GetRed(x, y);
	    uncompressedBuffer[(x + y * potWidth) * 4 + 1] = image.GetGreen(x, y);
	    uncompressedBuffer[(x + y * potWidth) * 4 + 2] = image.GetBlue(x, y);
	    uncompressedBuffer[(x + y * potWidth) * 4 + 3] = image.HasAlpha() ? image.GetAlpha(x, y) : 0xff;
	}
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, potWidth, potHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &uncompressedBuffer.front());

    vector<GLbyte> compressedBuffer(MAX(potWidth / 4, 1) * MAX(potHeight / 4, 1) * 16);
     glGetCompressedTexImage(GL_TEXTURE_2D, 0, &compressedBuffer.front());

     for (int row = 0; row < (image.GetHeight() + 3 & ~3) / 4; row++) {
	 stream.Write(&compressedBuffer[row * potWidth / 4 * 16], (image.GetWidth() + 3 & ~3) / 4 * 16);
    }
}

wxImage wxDDSHandler::Minify(wxImage &image)
{
    wxImage minifiedImage;
    minifiedImage.Create(MAX(image.GetWidth() >> 1, 1), MAX(image.GetHeight() >> 1, 1));

    gluScaleImage(GL_RGB, image.GetWidth(), image.GetHeight(), GL_UNSIGNED_BYTE, image.GetData(), minifiedImage.GetWidth(), minifiedImage.GetHeight(), GL_UNSIGNED_BYTE, minifiedImage.GetData());
    
    if (image.HasAlpha()) {
	minifiedImage.InitAlpha();
	gluScaleImage(GL_ALPHA, image.GetWidth(), image.GetHeight(), GL_UNSIGNED_BYTE, image.GetAlpha(), minifiedImage.GetWidth(), minifiedImage.GetHeight(), GL_UNSIGNED_BYTE, minifiedImage.GetAlpha());
    }

    return minifiedImage;

}
