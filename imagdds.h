/////////////////////////////////////////////////////////////////////////////
// Name:        imagdds.h
// Purpose:     wxImage DDS handler
// Author:      Gregory Smith
// Copyright:   (c) Gregory Smith
// Licence:     wxWindows license
/////////////////////////////////////////////////////////////////////////////

#ifndef IMAGDDS_H_
#define IMAGDDS_H_

#include "wx/image.h"

#ifndef __DDRAW_INCLUDED__

#define DDSD_CAPS          0x00000001
#define DDSD_HEIGHT        0x00000002
#define DDSD_WIDTH         0x00000004
#define DDSD_PITCH         0x00000008
#define DDSD_PIXELFORMAT   0x00001000
#define DDSD_MIPMAPCOUNT   0x00002000
#define DDSD_LINEARSIZE    0x00008000
#define DDSD_DEPTH         0x00800000

#define DDPF_ALPHAPIXELS   0x00000001
#define DDPF_FOURCC        0x00000004
#define DDPF_RGB           0x00000040

#define DDSCAPS_COMPLEX    0x00000008
#define DDSCAPS_TEXTURE    0x00001000
#define DDSCAPS_MIPMAP     0x00400000

#define DDSCAPS2_CUBEMAP   0x00000200
#define DDSCAPS2_VOLUME    0x00200000

struct DDSURFACEDESC2
{
    wxUint32 dwSize;
    wxUint32 dwFlags;
    wxUint32 dwHeight;
    wxUint32 dwWidth;
    wxUint32 dwPitchOrLinearSize;
    wxUint32 dwDepth;
    wxUint32 dwMipMapCount;
    wxUint32 dwReserved1[11];
    
    struct {
	wxUint32 dwSize;
	wxUint32 dwFlags;
	wxUint32 dwFourCC;
	wxUint32 dwRGBBitCount;
	wxUint32 dwRBitMask;
	wxUint32 dwGBitMask;
	wxUint32 dwBBitMask;
	wxUint32 dwRGBAlphaBitMask;
    } ddpfPixelFormat;
    
    struct {
	wxUint32 dwCaps1;
	wxUint32 dwCaps2;
	wxUint32 Reserved[2];
    } ddsCaps;
    
    wxUint32 dwReserved2;
};

#endif

class wxDDSHandler : public wxImageHandler
{
public:
    enum {
	wxBITMAP_TYPE_DDS = wxBITMAP_TYPE_ANY + 1
    };
    
    wxDDSHandler() 
	{
	    m_name = _T("Direct Draw Surface file");
	    m_extension = _T("dds");
	    m_type = wxBITMAP_TYPE_DDS;
	}
#if wxUSE_STREAMS
    virtual bool SaveFile ( wxImage *image, wxOutputStream& stream, bool verbose=true ) { return FALSE; }
    virtual bool LoadFile ( wxImage *image, wxInputStream& stream, bool verbose=true, int index=-1);
    virtual bool DoCanRead( wxInputStream& stream );
private:
    bool ReadHeader(wxInputStream& stream, DDSURFACEDESC2 &ddsd);
#endif
    
};

#endif