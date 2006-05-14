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
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statline.h>

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#endif

#include "image_ext.h"

class MainApp: public wxApp // MainApp is the class for our application 
{
	// MainApp just acts as a container for the window, 
public: 
	// or frame in MainFrame 
	virtual bool OnInit(); 
};

class MainFrame: public wxFrame // MainFrame is the class for our window, 
{ 
	// It contains the window and all objects in it 
public: 
	MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size, long style); 
	wxNotebook *notebook;
	wxMenuBar *menuBar;
	wxMenu *fileMenu;
	void OnExit(wxCommandEvent& event);
	void OnAbout(wxCommandEvent &) { }

	DECLARE_EVENT_TABLE()

}; 

class BasicPage: public wxNotebookPage
{
public: 
	BasicPage(wxWindow* parent, wxWindowID id, const wxPoint &pos, const wxSize &size);
	
	void OnLoadNormal(wxCommandEvent &);
	void OnLoadMask(wxCommandEvent &);
	void OnClearMask(wxCommandEvent &);
	void OnOpacTypeTwo(wxCommandEvent &);
	void OnOpacTypeThree(wxCommandEvent &);
	void OnSaveAs(wxCommandEvent &);
	
	void UpdateNormalDisplay();
	void UpdateMaskDisplay();
	
	void SetMaskButtonEnablement(bool enabled);
	
	DECLARE_EVENT_TABLE()
		
private:
	wxImageExt normalImage;
	wxImageExt normalImageDisplay;
	wxButton *normalImageButton;
	wxStaticBitmap *normalImageStatic;
	
	wxImageExt maskImage;
	wxImageExt maskImageDisplay;
	wxButton *maskImageButton;
	wxButton *clearMaskButton;
	wxButton *opacTypeTwo;
	wxButton *opacTypeThree;
	wxStaticBitmap *maskImageStatic;
	
	wxButton *saveAsButton;
};

enum
{
	AORTA_EXIT = wxID_HIGHEST + 1,
	AORTA_ABOUT,
	BUTTON_NormalImage,
	BUTTON_MaskImage,
	BUTTON_ClearMask,
	BUTTON_OpacTypeTwo,
	BUTTON_OpacTypeThree,
	BUTTON_SaveAs
};

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(AORTA_EXIT, MainFrame::OnExit)
EVT_MENU(AORTA_ABOUT, MainFrame::OnAbout)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(BasicPage, wxNotebookPage)
EVT_BUTTON(BUTTON_NormalImage, BasicPage::OnLoadNormal)
EVT_BUTTON(BUTTON_MaskImage, BasicPage::OnLoadMask)
EVT_BUTTON(BUTTON_ClearMask, BasicPage::OnClearMask)
EVT_BUTTON(BUTTON_OpacTypeTwo, BasicPage::OnOpacTypeTwo)
EVT_BUTTON(BUTTON_OpacTypeThree, BasicPage::OnOpacTypeThree)
EVT_BUTTON(BUTTON_SaveAs, BasicPage::OnSaveAs)
END_EVENT_TABLE()
	

#endif
