/////////////////////////////////////////////////////////////////////////////
// Name:        imagdds.cpp
// Purpose:     wxImage DDS handler
// Author:      Gregory Smith
// Copyright:   (c) Gregory Smith
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

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
    }
    
    ~SubtleOpenGLContext() {
	frame->Close();
    }
private:
    wxFrame *frame;
    wxGLCanvas *canvas;
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
    ddsd.ddpfPixelFormat.dwRBitMask = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwGBitMask);
    ddsd.ddpfPixelFormat.dwBBitMask = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwBBitMask);
    ddsd.ddpfPixelFormat.dwRGBAlphaBitMask = wxINT32_SWAP_ON_BE(ddsd.ddpfPixelFormat.dwRGBAlphaBitMask);
    
    ddsd.ddsCaps.dwCaps1 = wxINT32_SWAP_ON_BE(ddsd.ddsCaps.dwCaps1);
    ddsd.ddsCaps.dwCaps2 = wxINT32_SWAP_ON_BE(ddsd.ddsCaps.dwCaps2);
    
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
    
    if (ddsd.ddpfPixelFormat.dwFlags & DDPF_RGB) 
	return FALSE; // TBD
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
    GLint internalFormat = GL_NONE;
    if (ddsd.ddpfPixelFormat.dwFlags & DDPF_FOURCC) {
	if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '1'))
	    internalFormat = GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
	else if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '3'))
	    internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
	else if (ddsd.ddpfPixelFormat.dwFourCC == MAKE_FOURCC('D', 'X', 'T', '5'))
	    internalFormat = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    }
    if (internalFormat == GL_NONE) return FALSE;

    int bpp = (internalFormat == GL_COMPRESSED_RGB_S3TC_DXT1_EXT) ? 4 : 8;
    int width = ddsd.dwWidth;
    int height = ddsd.dwHeight;

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
	
	GLuint textureRef;
	glGenTextures(1, &textureRef);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, textureRef);
	
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

	glDeleteTextures(1, &textureRef);
	
    }

    delete[] compressedBuffer;
    return TRUE;
}
