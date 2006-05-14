/*

aorta.cpp: the Aleph One Replacement Texture Utility
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
#include "aorta.h" 

IMPLEMENT_APP(MainApp)  

bool MainApp::OnInit() 
{ 
	::wxInitAllImageHandlers();
	MainFrame *MainWin = new MainFrame(_("Aorta"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxRESIZE_BOX | wxMAXIMIZE_BOX)); 
	MainWin->Show(TRUE); 
	SetTopWindow(MainWin); 
		return TRUE;
} 

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size, long style) 
: wxFrame((wxFrame *) NULL, -1, title, pos, size, style) 
{ 
	fileMenu = new wxMenu;
	fileMenu->Append(AORTA_ABOUT, "&About Aorta...");
	fileMenu->AppendSeparator();
	fileMenu->Append(AORTA_EXIT, "&Quit");

	menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, "&File");
	SetMenuBar(menuBar);
	
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	notebook = new wxNotebook(this, -1);
	topsizer->Add(notebook, 1, wxEXPAND | wxALL, 10);
	
	BasicPage *basicPage = new BasicPage(notebook, -1, wxDefaultPosition, wxDefaultSize);
	notebook->AddPage(basicPage, _("Basic"), true);
	
	SetAutoLayout(TRUE);
	SetSizer(topsizer);
	topsizer->Fit(this);
	topsizer->SetSizeHints(this);
}

void MainFrame::OnExit(wxCommandEvent& event)
{
	Close(TRUE);
}

BasicPage::BasicPage(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
: wxNotebookPage(parent, id, pos, size)
{
	wxBoxSizer *pageSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *column[2];
	column[0] = new wxBoxSizer(wxVERTICAL);
	
	wxImageExt whiteImage(256, 256);
	whiteImage.White();
	
	normalImageStatic = new wxStaticBitmap(this, -1, wxBitmap(whiteImage));
	column[0]->Add(normalImageStatic, 0, wxALIGN_CENTER | wxEXPAND | wxALL, 10);
	normalImageButton = new wxButton(this, BUTTON_NormalImage, _T("Load normal..."));
	column[0]->Add(normalImageButton, 0, wxALIGN_CENTER | wxALL, 10);
	
	pageSizer->Add(column[0], 0, wxEXPAND | wxALL, 10);
	
	column[1] = new wxBoxSizer(wxVERTICAL);
	
	maskImageStatic = new wxStaticBitmap(this, -1, wxBitmap(whiteImage));
	column[1]->Add(maskImageStatic, 0, wxALIGN_CENTER | wxEXPAND | wxALL, 10);
	
	wxBoxSizer *loadClearSizer = new wxBoxSizer(wxHORIZONTAL);
	maskImageButton = new wxButton(this, BUTTON_MaskImage, _T("Load mask..."));
	loadClearSizer->Add(maskImageButton, 1, wxEXPAND | wxRIGHT, 10);
	clearMaskButton = new wxButton(this, BUTTON_ClearMask, _T("Clear mask"));
	loadClearSizer->Add(clearMaskButton, 1, wxEXPAND | wxLEFT, 10);
	column[1]->Add(loadClearSizer, 0, wxEXPAND | wxALIGN_CENTER | wxALL, 10);
	
	wxBoxSizer *opacTypeSizer = new wxBoxSizer(wxHORIZONTAL);
	opacTypeTwo = new wxButton(this, BUTTON_OpacTypeTwo, _T("Opac_type=2"));
	opacTypeThree = new wxButton(this, BUTTON_OpacTypeThree, _T("Opac_type=3"));
	opacTypeSizer->Add(opacTypeTwo, 1, wxEXPAND | wxRIGHT, 10);
	opacTypeSizer->Add(opacTypeThree, 1, wxEXPAND | wxLEFT, 10);
	column[1]->Add(opacTypeSizer, 0, wxEXPAND | wxALIGN_CENTER | wxALL, 10);
	SetMaskButtonEnablement(false);
	
	wxBoxSizer *saveSizer = new wxBoxSizer(wxHORIZONTAL);
	saveAsButton = new wxButton(this, BUTTON_SaveAs, _T("Save as..."));
	saveAsButton->Disable();
	saveSizer->AddStretchSpacer(1);
	saveSizer->Add(saveAsButton, 1, wxEXPAND | wxLEFT, 10);
	column[1]->Add(saveSizer, 0, wxEXPAND | wxALIGN_CENTER | wxALL, 10);
	
	pageSizer->Add(column[1], 0, wxEXPAND | wxTOP | wxBOTTOM, 10);
	
	SetAutoLayout(TRUE);
	SetSizer(pageSizer);
	pageSizer->Fit(this);
	pageSizer->SetSizeHints(this);
	Layout();
}

void BasicPage::OnLoadNormal(wxCommandEvent &)
{
	wxFileDialog *openFileDialog = new wxFileDialog( this,
													 _("Choose Image"),
													 _(""),
													 _(""),
													 _("Image Files ") + wxImage::GetImageExtWildcard() + "|All Files|*.*",
													 wxOPEN,
													 wxDefaultPosition);
	if (openFileDialog->ShowModal() == wxID_OK)
	{
		normalImage.LoadFile(openFileDialog->GetPath());
		if (normalImage.Ok())
		{
			if (normalImage.HasMask()) normalImage.MaskToAlpha();
			
			if (normalImage.HasAlpha())
			{
				// convert the mask to grayscale for display
				maskImage.FromAlpha(normalImage);
				normalImage.RemoveAlpha();
			}
			else
			{
				maskImage.Destroy();
			}

			UpdateNormalDisplay();
			UpdateMaskDisplay();
		} 
		else
		{
			normalImage.Destroy();
			maskImage.Destroy();
			UpdateNormalDisplay();
			UpdateMaskDisplay();
		}
	}
}

void BasicPage::OnLoadMask(wxCommandEvent &)
{
	wxFileDialog *openFileDialog = new wxFileDialog(this,
													_("Choose Mask"),
													_(""),
													_(""),
													_("Image Files ") + wxImage::GetImageExtWildcard(),
													wxOPEN,
													wxDefaultPosition);
	if (openFileDialog->ShowModal() == wxID_OK)
	{
		maskImage.LoadFile(openFileDialog->GetPath());
		if (maskImage.Ok())
		{
			if (maskImage.GetWidth() == normalImage.GetWidth() && maskImage.GetHeight() == normalImage.GetHeight())
			{
				maskImage.MakeOpacTypeTwo();
			}
			else
			{
				wxMessageBox(_T("The mask must be the same width and height as the image."), _T("Invalid mask"), wxOK);
				maskImage.Destroy();
			}
		}
		else
		{
			maskImage.Destroy();
		}
		UpdateMaskDisplay();
	}
}

void BasicPage::OnClearMask(wxCommandEvent &)
{
	maskImage.Destroy();
	UpdateMaskDisplay();
}

void BasicPage::OnOpacTypeTwo(wxCommandEvent &)
{
	maskImage = normalImage;
	maskImage.MakeOpacTypeTwo();
	UpdateMaskDisplay();
}

void BasicPage::OnOpacTypeThree(wxCommandEvent &)
{
	maskImage = normalImage;
	maskImage.MakeOpacTypeThree();
	UpdateMaskDisplay();
}

void BasicPage::OnSaveAs(wxCommandEvent &)
{
	if (!normalImage.Ok()) return;
	wxFileDialog *saveFileDialog = new wxFileDialog(this,
												   _("Save As"),
												   _(""),
												   _("untitled.png"),
												   _("PNG files (*.png)|*.png"),
												   wxSAVE | wxOVERWRITE_PROMPT,
												   wxDefaultPosition);
	if (saveFileDialog->ShowModal() == wxID_OK)
	{
		wxImageExt saveImage = normalImage;
		if (maskImage.Ok())
		{
			maskImage.ToAlpha(saveImage);
		}
		else
		{
			if (saveImage.HasAlpha())
			{
				exit(0);
			}
		}
		saveImage.SaveFile(saveFileDialog->GetPath(), wxBITMAP_TYPE_PNG);
	}
												   
}

void BasicPage::SetMaskButtonEnablement(bool enabled)
{
	if (enabled)
	{
		maskImageButton->Enable();
		clearMaskButton->Enable();
		opacTypeTwo->Enable();
		opacTypeThree->Enable();
	}
	else
	{
		maskImageButton->Disable();
		clearMaskButton->Disable();
		opacTypeTwo->Disable();
		opacTypeThree->Disable();
	}
}

void BasicPage::UpdateNormalDisplay()
{
	
	int width, height;
	normalImageStatic->GetSize(&width, &height);
	
	if (normalImage.Ok())
	{
		wxImageExt normalImageDisplay = normalImage;
		normalImageDisplay.PropRescale(width, height);
		normalImageStatic->SetBitmap(wxBitmap(normalImageDisplay));
		
		saveAsButton->Enable();
		SetMaskButtonEnablement(true);
	}
	else
	{
		wxImageExt normalImageDisplay(width, height);
		normalImageDisplay.White();
		normalImageStatic->SetBitmap(wxBitmap(normalImageDisplay));
		
		saveAsButton->Disable();
		SetMaskButtonEnablement(false);
	}
}

void BasicPage::UpdateMaskDisplay()
{
	int width, height;
	maskImageStatic->GetSize(&width, &height);
	
	wxImageExt maskImageDisplay;
	if (maskImage.Ok())
	{
		maskImageDisplay = maskImage;
	} 
	else if (normalImage.Ok())
	{
		maskImageDisplay.Create(normalImage.GetWidth(), normalImage.GetHeight());
		maskImageDisplay.White();
	}
	else
	{
		maskImageDisplay.Create(width, height);
		maskImageDisplay.White();
	}
	maskImageDisplay.PropRescale(width, height);
	maskImageStatic->SetBitmap(wxBitmap(maskImageDisplay));
	
}
