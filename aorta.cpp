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
#include "DDSOptionsDialog.h"
#include "imagdds.h"

IMPLEMENT_APP(MainApp)  

bool MainApp::OnInit() 
{ 
	::wxInitAllImageHandlers();
	wxDDSHandler *ddsHandler = new wxDDSHandler;
	wxImage::AddHandler(ddsHandler);
	MainWin = new MainFrame(_("Aorta"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_FRAME_STYLE & ~ (wxRESIZE_BORDER | wxMAXIMIZE_BOX)); 
	MainWin->Show(TRUE); 
	SetTopWindow(MainWin); 

	if (argc > 1)
		MainWin->LoadNormal(argv[1]);

	return TRUE;
} 

#ifdef __WXMAC__
void MainApp::MacOpenFile(const wxString &fileName)
{
	MainWin->LoadNormal(fileName);
}

#endif

MainFrame::MainFrame(const wxString &title, const wxPoint &pos, const wxSize &size, long style) 
  : wxFrame((wxFrame *) NULL, -1, title, pos, size, style) 
{ 
	fileMenu = new wxMenu;
	fileMenu->Append(wxID_ABOUT, _T("&About Aorta..."));
	fileMenu->AppendSeparator();
	fileMenu->Append(MENU_LoadNormal, _T("L&oad image..."));
	fileMenu->Append(MENU_SaveAs, wxT("&Save as..."));
	fileMenu->Append(wxID_EXIT, wxT("&Quit"));

	menuBar = new wxMenuBar;
	menuBar->Append(fileMenu, wxT("&File"));
	SetMenuBar(menuBar);
	
	wxBoxSizer *topsizer = new wxBoxSizer(wxVERTICAL);
	notebook = new wxNotebook(this, -1);
	
	basicPage = new BasicPage(notebook, -1, wxDefaultPosition, wxDefaultSize);
	notebook->AddPage(basicPage, wxT("Compose"), true);

	batchPage = new BatchPage(notebook, -1, wxDefaultPosition, wxDefaultSize);
	notebook->AddPage(batchPage, wxT("Batch Convert"), false);

#ifdef __WIN32__
	topsizer->Add(notebook, 1, wxEXPAND);
#else
	topsizer->Add(notebook, 1, wxEXPAND | wxALL, 10);
#endif

	SetAutoLayout(TRUE);
	SetSizer(topsizer);
	topsizer->Fit(this);
	topsizer->SetSizeHints(this);
}

void MainFrame::OnExit(wxCommandEvent& event)
{
	Close(TRUE);
}

void MainFrame::OnAbout(wxCommandEvent& event)
{
	wxMessageBox(wxT("Aorta 2.1 (the Aleph One Replacement Texture Accessory)\n(C) 2006-2008 Gregory Smith\n\nAorta is licensed under the GPL. See COPYING.txt"), wxT("About Aorta"), wxOK);
}

void MainFrame::OnLoadNormal(wxCommandEvent& event)
{
	basicPage->OnLoadNormal(event);
}

void MainFrame::OnSaveAs(wxCommandEvent& event)
{
	basicPage->OnSaveAs(event);
}

void MainFrame::OnLoadMask(wxCommandEvent& event)
{
	basicPage->OnLoadMask(event);
}

void MainFrame::LoadNormal(const wxString& path)
{
	notebook->SetSelection(0);
	basicPage->LoadNormal(path);
}

BasicPage::BasicPage(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
  : wxPanel(parent, id, pos, size)
{
	wxBoxSizer *pageSizer = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer *column[2];
	column[0] = new wxBoxSizer(wxVERTICAL);

	wxImageExt whiteImage(256, 256);
	whiteImage.White();
	
	normalImageStatic = new wxStaticBitmap(this, -1, wxBitmap(whiteImage));
#ifdef wxUSE_DRAG_AND_DROP
	normalImageStatic->SetDropTarget(new DnDNormalImage(this));
#endif	
	column[0]->Add(normalImageStatic, 0, wxALIGN_CENTER | wxEXPAND | wxALL, 10);
	normalImageFilename = new wxStaticText(this, -1, wxT(""));
	column[0]->Add(normalImageFilename, 0, wxALIGN_CENTER | wxEXPAND | wxLEFT | wxRIGHT, 10);
	normalImageSize = new wxStaticText(this, -1, wxT(""));
	column[0]->Add(normalImageSize, 0, wxALIGN_CENTER | wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
	normalImageButton = new wxButton(this, BUTTON_NormalImage, wxT("Load normal..."));
	column[0]->Add(normalImageButton, 0, wxALIGN_CENTER | wxALL, 10);
	
	unpremultiplyAlpha = new wxButton(this, BUTTON_UnpremultiplyAlpha, wxT("Un-Premultiply Alpha"));
	column[0]->Add(unpremultiplyAlpha, 0, wxALIGN_CENTER | wxALL, 10);

	normalMap = new wxButton(this, BUTTON_MakeNormalMap, wxT("Make Normal Map"));
	column[0]->Add(normalMap, 0, wxALIGN_CENTER | wxALL, 10);
	
	pageSizer->Add(column[0], 0, wxEXPAND | wxALL, 10);
	
	column[1] = new wxBoxSizer(wxVERTICAL);
	
	maskImageStatic = new wxStaticBitmap(this, -1, wxBitmap(whiteImage));
#ifdef wxUSE_DRAG_AND_DROP
	maskImageStatic->SetDropTarget(new DnDMask(this));
#endif
	column[1]->Add(maskImageStatic, 0, wxALIGN_CENTER | wxEXPAND | wxALL, 10);

	maskStatus = new wxStaticText(this, -1, wxT(""));
	column[1]->Add(maskStatus, 0, wxALIGN_CENTER | wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 10);
	column[1]->AddSpacer(maskStatus->GetRect().GetHeight());
	
	wxBoxSizer *loadClearSizer = new wxBoxSizer(wxHORIZONTAL);
	maskImageButton = new wxButton(this, BUTTON_MaskImage, wxT("Load mask..."));
	loadClearSizer->Add(maskImageButton, 1, wxEXPAND | wxRIGHT, 10);
	clearMaskButton = new wxButton(this, BUTTON_ClearMask, wxT("Clear mask"));
	loadClearSizer->Add(clearMaskButton, 1, wxEXPAND | wxLEFT, 10);
	column[1]->Add(loadClearSizer, 0, wxEXPAND | wxALIGN_CENTER | wxALL, 10);
	
	wxBoxSizer *opacTypeSizer = new wxBoxSizer(wxHORIZONTAL);
	opacTypeTwo = new wxButton(this, BUTTON_OpacTypeTwo, wxT("Opac_type=2"));
	opacTypeThree = new wxButton(this, BUTTON_OpacTypeThree, wxT("Opac_type=3"));
	opacTypeSizer->Add(opacTypeTwo, 1, wxEXPAND | wxRIGHT, 10);
	opacTypeSizer->Add(opacTypeThree, 1, wxEXPAND | wxLEFT, 10);
	column[1]->Add(opacTypeSizer, 0, wxEXPAND | wxALIGN_CENTER | wxALL, 10);
	SetMaskButtonEnablement(false);
	
	wxBoxSizer *saveSizer = new wxBoxSizer(wxHORIZONTAL);
	saveAsButton = new wxButton(this, BUTTON_SaveAs, wxT("Save as..."));
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
	wxConfig config;
	wxString Directory;
	config.Read(wxT("Single/DefaultDirectory/Load"), &Directory, wxT(""));
	wxFileDialog *openFileDialog = new wxFileDialog( this,
							 wxT("Choose Image"),
							 Directory,
							 wxT(""),
							 wxT("Image Files ") + wxImage::GetImageExtWildcard() + wxT("|All Files|*.*"),
							 wxFD_OPEN | wxFD_CHANGE_DIR,
							 wxDefaultPosition);
	if (openFileDialog->ShowModal() == wxID_OK)
	{
		LoadNormal(openFileDialog->GetPath());
	}
}


void BasicPage::OnLoadMask(wxCommandEvent &)
{
	
	wxConfig config;
	wxString Directory;
	config.Read(wxT("Single/DefaultDirectory/Load"), &Directory, wxT(""));
	wxFileDialog *openFileDialog = new wxFileDialog(this,
							wxT("Choose Mask"),
							wxT(""),
							wxT(""),
							wxT("Image Files ") + wxImage::GetImageExtWildcard(),
							wxFD_OPEN | wxFD_CHANGE_DIR,
							wxDefaultPosition);
	if (openFileDialog->ShowModal() == wxID_OK)
	{
		LoadMask(openFileDialog->GetPath());
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

void BasicPage::OnUnpremultiplyAlpha(wxCommandEvent &)
{
	normalImage.UnpremultiplyAlpha();
	UpdateNormalDisplay();
}

void BasicPage::OnMakeNormalMap(wxCommandEvent&)
{
	normalImage.MakeOpacTypeTwo();
	maskImage = normalImage;
	normalImage.MakeNormalMap();
	UpdateNormalDisplay();
	UpdateMaskDisplay();
}	

void BasicPage::OnSaveAs(wxCommandEvent &)
{
	if (!normalImage.Ok()) return;

	wxFileDialog *saveFileDialog;
	{
		wxConfig config;
		wxString Directory;
		config.Read(wxT("Single/DefaultDirectory/Save"), &Directory, wxT(""));
		saveFileDialog = new wxFileDialog(this,
								wxT("Save As"),
								Directory,
						  (normalImageFilename->GetLabel().BeforeLast('.') + wxT(".dds")),
								wxT("DDS files (*.dds)|*.dds|PNG files (*.png)|*.png"),
								wxFD_SAVE | wxFD_OVERWRITE_PROMPT | wxFD_CHANGE_DIR,
								wxDefaultPosition);
		if (saveFileDialog->ShowModal() != wxID_OK) return;
		
		Directory = saveFileDialog->GetPath().BeforeLast(wxFileName::GetPathSeparator());
		config.Write(wxT("Single/DefaultDirectory/Save"), Directory);
	}
    
	wxImageExt saveImage = normalImage;
	if (maskImage.Ok())
	{
		maskImage.ToAlpha(saveImage);
//			saveImage.PrepareForMipmaps();
	}
	else
	{
		if (saveImage.HasAlpha())
		{
			exit(0);
		}
	}

	if (saveFileDialog->GetFilename().AfterLast('.').MakeLower() == wxT("png")) {
		saveImage.SaveFile(saveFileDialog->GetPath(), wxBITMAP_TYPE_PNG);
	} else {
		// query for a preset
		DDSOptionsDialog ddsOptions(wxT("Single"), saveImage.HasAlpha());
		if (ddsOptions.ShowModal() != wxID_OK) return;
	
		if (ddsOptions.generateMipmaps->GetValue()) {
			saveImage.SetOption(wxIMAGE_OPTION_DDS_USE_MIPMAPS, 1);

			if (ddsOptions.repeatingTexture->GetValue()) {
			    saveImage.SetOption(wxIMAGE_OPTION_DDS_MIPMAP_WRAP_MODE, wxIMAGE_OPTION_DDS_WRAP_REPEAT);
			} else {
			    saveImage.SetOption(wxIMAGE_OPTION_DDS_MIPMAP_WRAP_MODE, wxIMAGE_OPTION_DDS_WRAP_CLAMP);
			}
			
			saveImage.SetOption(wxIMAGE_OPTION_DDS_MIPMAP_FILTER, ddsOptions.mipmapFilterChoice->GetSelection());
			saveImage.SetOption(wxIMAGE_OPTION_DDS_PREMULTIPLY_ALPHA, 0);

			if (saveImage.HasAlpha() && ddsOptions.colorFillBackground->GetValue()) {
				if (ddsOptions.reconstructColors->GetValue()) {
					saveImage.ReconstructColors(ddsOptions.backgroundColor);
				}

				saveImage.PrepareForMipmaps();
			}
			
		} else {
			saveImage.SetOption(wxIMAGE_OPTION_DDS_USE_MIPMAPS, 0);
		}

		if (ddsOptions.useDXTC->GetValue()) {
			saveImage.SetOption(wxIMAGE_OPTION_DDS_COMPRESS, 1);
		} else {
			saveImage.SetOption(wxIMAGE_OPTION_DDS_COMPRESS, 0);
		}

		saveImage.SaveFile(saveFileDialog->GetPath(), wxDDSHandler::wxBITMAP_TYPE_DDS);
	}
}

void BasicPage::LoadNormal(const wxString& path)
{
	wxConfig config;
	config.Write(wxT("Single/DefaultDirectory/Load"), path.BeforeLast('/'));

	normalImage.LoadFile(path);
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
	
		normalImageFilename->SetLabel(path.AfterLast('/'));
		wxString dimensionsString;
		dimensionsString.Printf(wxT("%ix%i"), normalImage.GetWidth(), normalImage.GetHeight());
		normalImageSize->SetLabel(dimensionsString);
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

void BasicPage::LoadMask(const wxString& path)
{
	wxConfig config;
	config.Write(wxT("Single/DefaultDirectory/Load"), path.BeforeLast('/'));

	maskImage.LoadFile(path);
	if (maskImage.Ok())
	{
		if (maskImage.GetWidth() == normalImage.GetWidth() && maskImage.GetHeight() == normalImage.GetHeight())
		{
			maskImage.MakeOpacTypeTwo();
		}
		else
		{
			wxMessageBox(wxT("The mask must be the same width and height as the image."), wxT("Invalid mask"), wxOK);
			maskImage.Destroy();
		}
	}
	else
	{
		maskImage.Destroy();
	}
	UpdateMaskDisplay();
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
		maskStatus->SetLabel(wxT(""));
	} 
	else if (normalImage.Ok())
	{
		maskImageDisplay.Create(normalImage.GetWidth(), normalImage.GetHeight());
		maskImageDisplay.White();
		maskStatus->SetLabel(wxT("No mask"));
	}
	else
	{
		maskImageDisplay.Create(width, height);
		maskImageDisplay.White();
		maskStatus->SetLabel(wxT(""));
	}
	maskImageDisplay.PropRescale(width, height);
	maskImageStatic->SetBitmap(wxBitmap(maskImageDisplay));
	
}

BatchPage::BatchPage(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size)
	: wxPanel(parent, id, pos, size)
{
	wxConfig config;
	chooseFiles = new wxButton(this, BUTTON_ChooseFiles, wxT("Choose Source..."));
	recurseCheckbox = new wxCheckBox(this, CHECKBOX_Recurse, wxT("Traverse (and recreate) subfolders"));

	config.Read(wxT("Batch/Source"), &source, wxT(""));
	if (source == wxT(""))
		fileStatus = new wxTextCtrl(this, -1, wxT("No files selected"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP);
	else
		fileStatus = new wxTextCtrl(this, -1, source, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP);

	findMasks = new wxCheckBox(this, BUTTON_FindMasks, wxT("Attempt to find masks"));
	bool value;
	config.Read(wxT("Batch/Recurse"), &value, true);
	recurseCheckbox->SetValue(value ? 1 : 0);
	config.Read(wxT("Batch/FindMasks"), &value, false);
	findMasks->SetValue(value ? 1 : 0);
	maskString = new wxTextCtrl(this, TEXT_MaskString);
	wxString mask;
	config.Read(wxT("Batch/MaskString"), &mask, wxT("^mask\\$"));
	mask.Replace(wxT("\\$"), wxT("$"));
	maskString->SetValue(mask);
	
	selectDestination = new wxButton(this, BUTTON_ChooseDestination, wxT("Choose Destination..."));

	config.Read(wxT("Batch/Destination"), &destination, wxT(""));
	if (destination == wxT(""))
		destinationStatus = new wxTextCtrl(this, -1, wxT("No destination chosen"), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP);
	else
		destinationStatus = new wxTextCtrl(this, -1, destination, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY | wxTE_BESTWRAP);
//	destinationStatus->Wrap(300);

	convert = new wxButton(this, BUTTON_Convert, wxT("Batch Convert..."));

	saveAsDDS = new wxRadioButton(this, -1, wxT("Save as DDS"), wxDefaultPosition, wxDefaultSize, wxRB_GROUP);
	saveAsDDS->SetValue(1);
	saveAsPNG = new wxRadioButton(this, -1, wxT("Save as PNG"));

	do_layout();
}

void BatchPage::do_layout()
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	//wxStaticBox *chooseFilesStaticBox = new wxStaticBox(this, -1, wxT("Drop Files to Choose Sources"));
	//chooseFilesStaticBox->SetDropTarget(new DnDBatchFiles(this));
	//wxStaticBoxSizer* chooseFilesBox = new wxStaticBoxSizer(chooseFilesStaticBox, wxHORIZONTAL);
	wxBoxSizer* chooseFilesBox = new wxBoxSizer(wxHORIZONTAL);
	chooseFiles->SetDropTarget(new DnDBatchFiles(this));
	chooseFilesBox->Add(chooseFiles, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxADJUST_MINSIZE, 10);
	fileStatus->SetDropTarget(new DnDBatchFiles(this));
	chooseFilesBox->Add(fileStatus, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxADJUST_MINSIZE, 10);
	topSizer->Add(chooseFilesBox, 0, wxALL | wxEXPAND, 10);
	wxBoxSizer* recurseBoxSizer = new wxBoxSizer(wxHORIZONTAL);
	recurseBoxSizer->AddSpacer(10);
	recurseBoxSizer->Add(recurseCheckbox, 0, wxEXPAND, 10);
	topSizer->Add(recurseBoxSizer, 0, wxALL & ~wxTOP | wxEXPAND, 10);
	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND);

	//wxStaticBoxSizer* chooseMasksBox = new wxStaticBoxSizer(new wxStaticBox(this, -1, wxT("Choose Masks")), wxVERTICAL);
	wxBoxSizer* chooseMasksBox = new wxBoxSizer(wxVERTICAL);
	chooseMasksBox->Add(findMasks, 0, wxALL, 10);
	chooseMasksBox->Add(maskString, 0, wxLEFT | wxRIGHT | wxEXPAND, 10);
	wxStaticText *maskStringHelp = new wxStaticText(this, -1, wxT("Use ^ for file name and $ for extension; e.g. 1.png and ^A$ -> 1A.png"));
	wxFont helpFont = maskStringHelp->GetFont();
	helpFont.SetStyle(wxFONTSTYLE_ITALIC);
	maskStringHelp->SetFont(helpFont);
	chooseMasksBox->Add(maskStringHelp, 0, wxALL, 10);
	topSizer->Add(chooseMasksBox, 0, wxALL | wxEXPAND, 10);
	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND);

	//wxStaticBox *chooseDestinationStaticBox = new wxStaticBox(this, -1, wxT("Drop Folder to Choose Destination"));
	//chooseDestinationStaticBox->SetDropTarget(new DnDBatchDestination(this));
	//wxStaticBoxSizer* chooseDestinationBox = new wxStaticBoxSizer(chooseDestinationStaticBox, wxHORIZONTAL);
	wxBoxSizer *chooseDestinationBox = new wxBoxSizer(wxHORIZONTAL);
	selectDestination->SetDropTarget(new DnDBatchDestination(this));
	chooseDestinationBox->Add(selectDestination, 0, wxALL | wxALIGN_CENTER_VERTICAL | wxADJUST_MINSIZE, 10);
	destinationStatus->SetDropTarget(new DnDBatchDestination(this));
	chooseDestinationBox->Add(destinationStatus, 1, wxALL | wxALIGN_CENTER_VERTICAL | wxADJUST_MINSIZE, 10);
	topSizer->Add(chooseDestinationBox, 0, wxALL | wxEXPAND, 10);
	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND);
	wxBoxSizer *formatSizer = new wxBoxSizer(wxHORIZONTAL);
	formatSizer->Add(saveAsDDS, 1, wxALL, 10);
	formatSizer->AddSpacer(10);
	formatSizer->Add(saveAsPNG, 1, wxALL, 10);
	topSizer->Add(formatSizer, 0, wxALL, 10);
	topSizer->Add(new wxStaticLine(this), 0, wxEXPAND);
	topSizer->Add(convert, 0, wxALL, 20);
	
	SetAutoLayout(true);
	SetSizer(topSizer);
	topSizer->Fit(this);
	topSizer->SetSizeHints(this);
	Layout();
}

void BatchPage::OnChooseSource(wxCommandEvent &)
{
	wxDirDialog *openDirDialog = new wxDirDialog(this,
						     wxT("Choose a source folder"));
	if (openDirDialog->ShowModal() == wxID_OK)
	{
		ChooseSource(openDirDialog->GetPath());
	}
}

void BatchPage::ChooseSource(const wxString& folder)
{
	source = folder;
	wxConfig config;
	config.Write(wxT("Batch/Source"), source);
//	fileStatus->SetLabel(folder);
	fileStatus->SetValue(folder);
//	fileStatus->Wrap(300);
}

void BatchPage::OnChooseDestination(wxCommandEvent &)
{
	wxDirDialog *openDirDialog = new wxDirDialog(this,
						     wxT("Choose a destination folder"));
	if (openDirDialog->ShowModal() == wxID_OK)
	{
		ChooseDestination(openDirDialog->GetPath());
	}
						    
}

void BatchPage::ChooseDestination(const wxString& folder)
{
	destination = folder;
	wxConfig config;
	config.Write(wxT("Batch/Destination"), destination);
	destinationStatus->SetValue(folder);
//	destinationStatus->Wrap(300);
}

void BatchPage::OnConvert(wxCommandEvent &)
{
	if (!wxDir::Exists(source)) return;
	if (!wxDir::Exists(destination)) return;

	wxArrayString filesToConvert;
	// build an array of files to convert
	wxDir::GetAllFiles(source, &filesToConvert, wxT(""), wxDIR_FILES | (recurseCheckbox->GetValue() ? wxDIR_DIRS : 0));

	// remove files beginning with .
	wxArrayString filesWithDots = filesToConvert;
	filesToConvert.Clear();
	for (int i = 0; i < filesWithDots.Count(); i++)
	{
		wxFileName filename = filesWithDots[i];
		if (!filename.IsOk()) continue;
		if (filename.GetName()[0] == '.') continue;
		filesToConvert.Add(filename.GetFullPath());
	}
	

	bool useMasks = findMasks->GetValue();
	bool toPNG = saveAsPNG->GetValue();

	if (useMasks)
	{
		// for each file in this list, we need to remove the corresponding mask
		wxArrayString listIncludingMasks = filesToConvert;
		for (int i = 0; i < listIncludingMasks.Count(); i++)
		{
			wxFileName filename = listIncludingMasks[i];
			wxString mask = maskString->GetValue();
			mask.Replace(wxT("^"), filename.GetPath() + wxFileName::GetPathSeparator() + filename.GetName(), false);
			if (filename.GetExt() != wxT(""))
				mask.Replace(wxT("$"), (wxT(".") + filename.GetExt()), false);
			else
				mask.Replace(wxT("$"), wxT(""));

			int loc = filesToConvert.Index(mask);
			if (loc != wxNOT_FOUND)
			{
				filesToConvert.RemoveAt(loc);
			}
		}
	}

	// make all the files relative to the source
	for (int i = 0; i < filesToConvert.Count(); i++)
	{
		wxFileName filename = filesToConvert[i];
		filename.MakeRelativeTo(source);
		filesToConvert[i] = filename.GetFullPath();
	}

	// get the DDS options
	DDSOptionsDialog ddsOptions(wxT("Batch"), true);
	if (!toPNG)
		if (ddsOptions.ShowModal() != wxID_OK) return;

	wxProgressDialog pd(wxT("Converting"), wxT("Converting"), filesToConvert.Count(), NULL, wxPD_AUTO_HIDE | wxPD_APP_MODAL | wxPD_ELAPSED_TIME | wxPD_CAN_ABORT);

	for (int i = 0; i < filesToConvert.Count(); i++)
	{
		wxString progressString;
		progressString.Printf(wxT("Converting...%i/%i"), i + 1, filesToConvert.Count());
		if (!pd.Update(i, progressString)) break;
		wxImageExt normalImage;
		wxImageExt maskImage;

		wxFileName normalFile = source + wxFileName::GetPathSeparator() + filesToConvert[i];
		wxString mask = maskString->GetValue();
		mask.Replace(wxT("^"), normalFile.GetPath() + wxFileName::GetPathSeparator() + normalFile.GetName(), false);
		mask.Replace(wxT("$"), (wxT(".") + normalFile.GetExt()), false);
		wxFileName maskFile = mask;
		wxFileName saveFile;
		if (recurseCheckbox->GetValue())
		{
			saveFile = destination + wxFileName::GetPathSeparator() + wxFileName(filesToConvert[i]).GetPath() + wxFileName::GetPathSeparator() + wxFileName(filesToConvert[i]).GetName() + (toPNG ? wxT(".png") : wxT(".dds"));
			wxFileName::Mkdir(saveFile.GetPath(), 0777, wxPATH_MKDIR_FULL);
		} 
		else
		{
			saveFile = destination + wxFileName::GetPathSeparator() + wxFileName(filesToConvert[i]).GetName() + (toPNG ? wxT(".png") : wxT(".dds"));
		}
		
		if (normalFile.GetName()[0] == '.') continue;
		if (!normalImage.LoadFile(normalFile.GetFullPath())) continue;
		if (!normalImage.Ok()) continue;
		if (normalImage.HasMask())
			normalImage.MaskToAlpha();
		if (wxFileName::FileExists(maskFile.GetFullPath()))
		{
			maskImage.LoadFile(maskFile.GetFullPath());
			if (maskImage.Ok())
			{
				if (maskImage.GetWidth() == normalImage.GetWidth() && maskImage.GetHeight() == normalImage.GetHeight())
				{
					maskImage.MakeOpacTypeTwo();
					maskImage.ToAlpha(normalImage);
				}
			}
		}

		if (toPNG) {
			normalImage.SaveFile(saveFile.GetFullPath(), wxBITMAP_TYPE_PNG);
		} else {
			// set the image options
			if (ddsOptions.generateMipmaps->GetValue())
			{
				normalImage.SetOption(wxIMAGE_OPTION_DDS_USE_MIPMAPS, 1);
				if (ddsOptions.repeatingTexture->GetValue()) {
					normalImage.SetOption(wxIMAGE_OPTION_DDS_MIPMAP_WRAP_MODE, wxIMAGE_OPTION_DDS_WRAP_REPEAT);
				} else {
					normalImage.SetOption(wxIMAGE_OPTION_DDS_MIPMAP_WRAP_MODE, wxIMAGE_OPTION_DDS_WRAP_CLAMP);
				}

				normalImage.SetOption(wxIMAGE_OPTION_DDS_MIPMAP_FILTER, ddsOptions.mipmapFilterChoice->GetSelection());
				normalImage.SetOption(wxIMAGE_OPTION_DDS_PREMULTIPLY_ALPHA, 0);

				if (normalImage.HasAlpha() && ddsOptions.colorFillBackground->GetValue())
				{
					if (ddsOptions.reconstructColors->GetValue())
						normalImage.ReconstructColors(ddsOptions.backgroundColor);

					normalImage.PrepareForMipmaps();
				}
			}
			else
			{
				normalImage.SetOption(wxIMAGE_OPTION_DDS_USE_MIPMAPS, 0);
			}

			if (ddsOptions.useDXTC->GetValue())
			{
				normalImage.SetOption(wxIMAGE_OPTION_DDS_COMPRESS, 1);
			} 
			else
			{
				normalImage.SetOption(wxIMAGE_OPTION_DDS_COMPRESS, 0);
			}

			normalImage.SaveFile(saveFile.GetFullPath(), wxDDSHandler::wxBITMAP_TYPE_DDS);
		}
	}
}

void BatchPage::SaveFindMaskConfig(wxCommandEvent &)
{
	wxConfig config;
	config.Write(wxT("Batch/FindMasks"), findMasks->GetValue() == 1);
	wxString mask = maskString->GetValue();
	mask.Replace(wxT("$"), wxT("\\$"));
	config.Write(wxT("Batch/MaskString"), mask);
}

void BatchPage::SaveRecurseConfig(wxCommandEvent &)
{
	wxConfig config;
	config.Write(wxT("Batch/Recurse"), recurseCheckbox->GetValue() == 1);
}

#if wxUSE_DRAG_AND_DROP

bool DnDNormalImage::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
    if (!filenames[0].empty())
        m_page->LoadNormal(filenames[0]);
}

bool DnDMask::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
    if (!filenames[0].empty())
        m_page->LoadMask(filenames[0]);
}

bool DnDBatchFiles::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
    if (!filenames[0].empty() && wxFileName(filenames[0]).DirExists())
        m_page->ChooseSource(filenames[0]);
}

bool DnDBatchDestination::OnDropFiles(wxCoord, wxCoord, const wxArrayString& filenames)
{
    if (!filenames[0].empty() && wxFileName(filenames[0]).DirExists())
        m_page->ChooseDestination(filenames[0]);
}
#endif

BEGIN_EVENT_TABLE(MainFrame, wxFrame)
EVT_MENU(wxID_EXIT, MainFrame::OnExit)
EVT_MENU(wxID_ABOUT, MainFrame::OnAbout)
EVT_MENU(MENU_LoadNormal, MainFrame::OnLoadNormal)
EVT_MENU(MENU_SaveAs, MainFrame::OnSaveAs)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(BasicPage, wxPanel)
EVT_BUTTON(BUTTON_NormalImage, BasicPage::OnLoadNormal)
EVT_BUTTON(BUTTON_MaskImage, BasicPage::OnLoadMask)
EVT_BUTTON(BUTTON_ClearMask, BasicPage::OnClearMask)
EVT_BUTTON(BUTTON_OpacTypeTwo, BasicPage::OnOpacTypeTwo)
EVT_BUTTON(BUTTON_OpacTypeThree, BasicPage::OnOpacTypeThree)
EVT_BUTTON(BUTTON_SaveAs, BasicPage::OnSaveAs)
EVT_BUTTON(BUTTON_UnpremultiplyAlpha, BasicPage::OnUnpremultiplyAlpha)
EVT_BUTTON(BUTTON_MakeNormalMap, BasicPage::OnMakeNormalMap)
END_EVENT_TABLE()

BEGIN_EVENT_TABLE(BatchPage, wxPanel)
EVT_BUTTON(BUTTON_ChooseFiles, BatchPage::OnChooseSource)
EVT_BUTTON(BUTTON_ChooseDestination, BatchPage::OnChooseDestination)
EVT_BUTTON(BUTTON_Convert, BatchPage::OnConvert)
EVT_BUTTON(BUTTON_FindMasks, BatchPage::SaveFindMaskConfig)
EVT_TEXT(TEXT_MaskString, BatchPage::SaveFindMaskConfig)
EVT_CHECKBOX(CHECKBOX_Recurse, BatchPage::SaveRecurseConfig)
END_EVENT_TABLE()
