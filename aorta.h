/*
 
 aorta.h: the Aleph One Replacement Texture Utility
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

#ifndef AORTA_H
#define AORTA_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP 
#include <wx/wx.h>
#include <wx/checkbox.h>
#include <wx/colordlg.h>
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/dirdlg.h>
#ifdef wxUSE_DRAG_AND_DROP
#include <wx/dnd.h>
#endif
#include <wx/filename.h>
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statline.h>

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#endif

#include "image_ext.h"

class MainFrame;

class MainApp: public wxApp // MainApp is the class for our application 
{
	// MainApp just acts as a container for the window, 
public: 
	// or frame in MainFrame 
	virtual bool OnInit(); 
#ifdef __WXMAC__
	void MainApp::MacOpenFile(const wxString &fileName);
#endif
private:
	MainFrame *MainWin;
};

class BasicPage;
class BatchPage;

class MainFrame: public wxFrame // MainFrame is the class for our window, 
{ 
	// It contains the window and all objects in it 
public: 
	MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size, long style); 
	wxNotebook *notebook;
	BasicPage *basicPage;
	BatchPage *batchPage;
	wxMenuBar *menuBar;
	wxMenu *fileMenu;
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent &);
	void OnLoadNormal(wxCommandEvent &);
	void OnLoadMask(wxCommandEvent &);
	void OnSaveAs(wxCommandEvent &);
	void LoadNormal(const wxString &);

	DECLARE_EVENT_TABLE()

}; 

class BasicPage: public wxPanel
{
public: 
	BasicPage(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size);
	
	void OnLoadNormal(wxCommandEvent &);
	void OnLoadMask(wxCommandEvent &);
	void OnClearMask(wxCommandEvent &);
	void OnOpacTypeTwo(wxCommandEvent &);
	void OnOpacTypeThree(wxCommandEvent &);
	void OnSaveAs(wxCommandEvent &);
	void OnUnpremultiplyAlpha(wxCommandEvent &);

	void LoadNormal(const wxString& path);
	void LoadMask(const wxString& path);
	
	void UpdateNormalDisplay();
	void UpdateMaskDisplay();
	
	void SetMaskButtonEnablement(bool enabled);
	
	DECLARE_EVENT_TABLE()
		
private:
	wxImageExt normalImage;
	wxImageExt normalImageDisplay;
	wxButton *normalImageButton;
	wxButton *unpremultiplyAlpha;
	wxStaticBitmap *normalImageStatic;
	wxStaticText *normalImageSize;
	wxStaticText *normalImageFilename;
	
	wxImageExt maskImage;
	wxImageExt maskImageDisplay;
	wxButton *maskImageButton;
	wxButton *clearMaskButton;
	wxButton *opacTypeTwo;
	wxButton *opacTypeThree;
	wxStaticBitmap *maskImageStatic;
	wxStaticText *maskStatus;
	
	wxButton *saveAsButton;
};

class BatchPage : public wxPanel
{
public:
	BatchPage(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size);

	void OnChooseSource(wxCommandEvent &);
	void OnChooseDestination(wxCommandEvent &);
	
	void ChooseSource(const wxString& files);
	void ChooseDestination(const wxString &folder);

	void OnConvert(wxCommandEvent &);

	void SaveFindMaskConfig(wxCommandEvent &);
	void SaveRecurseConfig(wxCommandEvent &);

        DECLARE_EVENT_TABLE()

private:
	void do_layout();

	wxButton *chooseFiles;
	wxStaticText *fileStatus;
	wxCheckBox *recurseCheckbox;

	wxCheckBox *findMasks;
	wxTextCtrl *maskString;

	wxButton *selectDestination;
	wxStaticText *destinationStatus;

	wxButton *convert;

	wxString source;
	wxString destination;
};

#if wxUSE_DRAG_AND_DROP
class DnDNormalImage : public wxFileDropTarget
{
public:
	DnDNormalImage(BasicPage *page) { m_page = page; }

	virtual bool OnDropFiles(wxCoord x, wxCoord y,
				 const wxArrayString& filenames);

private:
	BasicPage *m_page;
};

class DnDMask : public wxFileDropTarget
{
public:
	DnDMask(BasicPage *page) { m_page = page; }
	
	virtual bool OnDropFiles(wxCoord x, wxCoord y,
				 const wxArrayString& filenames);
private:
	BasicPage *m_page;
};

class DnDBatchFiles : public wxFileDropTarget
{
public:
	DnDBatchFiles(BatchPage *page) { m_page = page; }

	virtual bool OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames);
private:
	BatchPage *m_page;
};

class DnDBatchDestination : public wxFileDropTarget
{
public:
	DnDBatchDestination(BatchPage *page) { m_page = page; }

	virtual bool OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames);
private:
	BatchPage *m_page;
};


#endif

enum
{
	MENU_LoadNormal = wxID_HIGHEST + 1,
	MENU_SaveAs,
	BUTTON_NormalImage,
	BUTTON_UnpremultiplyAlpha,
	BUTTON_MaskImage,
	BUTTON_ClearMask,
	BUTTON_OpacTypeTwo,
	BUTTON_OpacTypeThree,
	BUTTON_SaveAs,
	BUTTON_ChooseBackground,
	BUTTON_NoHaloRemoval,
	BUTTON_PremultiplyAlpha,
	BUTTON_ColorFillBackground,
	BUTTON_GenerateMipmaps,
	BUTTON_ReconstructColors,

	BUTTON_ChooseFiles,
	BUTTON_ChooseDestination,
	BUTTON_Convert,
	BUTTON_FindMasks,
	CHECKBOX_Recurse,
	TEXT_MaskString
};



#endif
