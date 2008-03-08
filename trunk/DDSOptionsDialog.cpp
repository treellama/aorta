/*

  DDSOptionsDialog.cpp: part of the Aleph One Replacement Texture Utility
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

#include "DDSOptionsDialog.h"

DDSOptionsDialog::DDSOptionsDialog(const wxString& prefix, bool HasAlpha)
	: wxDialog(NULL, -1, wxT("DDS Options"), wxDefaultPosition, wxDefaultSize), m_prefix(prefix), m_hasAlpha(HasAlpha)
{

	mipmapBox_staticbox = new wxStaticBox(this, -1, wxT("Mipmap Halo Removal"));
	useDXTC = new wxCheckBox(this, -1, wxT("Use DXTC (Texture Compression)"));
	generateMipmaps = new wxCheckBox(this, BUTTON_GenerateMipmaps, wxT("Generate Mipmaps"));
	wxArrayString filterChoices;
	filterChoices.Add(wxT("Box"));
	filterChoices.Add(wxT("Triangle"));
	filterChoices.Add(wxT("Kaiser"));
	mipmapFilterChoice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, filterChoices);
	colorFillBackground = new wxCheckBox(this, BUTTON_ColorFillBackground, wxT("Fast Halo Removal"));
	reconstructColors = new wxCheckBox(this, BUTTON_ReconstructColors, wxT("Reconstruct Edge Colors"));
	chooseBackground = new wxButton(this, BUTTON_ChooseBackground, wxT("Choose Background Color..."));

	fill_from_prefs();
	update_enablement();
	do_layout();
}

void DDSOptionsDialog::fill_from_prefs()
{
	wxConfig config;
	config.SetPath(m_prefix);

	bool value;
	config.Read(wxT("UseDXTC"), &value, true);
	useDXTC->SetValue(value ? 1 : 0);
	
	config.Read(wxT("GenerateMipmaps"), &value, true);
	generateMipmaps->SetValue(value ? 1 : 0);

	long filter;
	config.Read(wxT("MipmapFilter"), &filter, 1);
	mipmapFilterChoice->SetSelection(filter);

	wxString haloRemovalStrategy;
	config.Read(wxT("FastHaloRemoval"), &value, true);
	colorFillBackground->SetValue(value ? 1 : 0);

	config.Read(wxT("ReconstructEdgeColors"), &value, true);
	reconstructColors->SetValue(value ? 1 : 0);

	long r, g, b;
	config.Read(wxT("BackgroundColor/R"), &r, 0xff);
	config.Read(wxT("BackgroundColor/G"), &g, 0xff);
	config.Read(wxT("BackgroundColor/B"), &b, 0xff);
	backgroundColor.Set((unsigned char) r, (unsigned char) g, (unsigned char) b);
}

void DDSOptionsDialog::update_enablement()
{
	mipmapFilterChoice->Enable(generateMipmaps->GetValue());
	colorFillBackground->Enable(generateMipmaps->GetValue() && m_hasAlpha);
	reconstructColors->Enable(generateMipmaps->GetValue() && m_hasAlpha && colorFillBackground->GetValue());
	chooseBackground->Enable(generateMipmaps->GetValue() && m_hasAlpha && colorFillBackground->GetValue() && reconstructColors->GetValue());
}

bool DDSOptionsDialog::Validate()
{
	wxConfig config;
	config.SetPath(m_prefix);
	config.Write(wxT("UseDXTC"), useDXTC->GetValue());
	config.Write(wxT("GenerateMipmaps"), generateMipmaps->GetValue());
	if (generateMipmaps->GetValue())
	{
		config.Write(wxT("MipmapFilter"), mipmapFilterChoice->GetSelection());
		if (colorFillBackground->GetValue())
		{
			config.Write(wxT("FastHaloRemoval"), true);
			config.Write(wxT("ReconstructEdgeColors"), reconstructColors->GetValue() == 1);
			
			config.Write(wxT("BackgroundColor/R"), (long) backgroundColor.Red());
			config.Write(wxT("BackgroundColor/G"), (long) backgroundColor.Green());
			config.Write(wxT("BackgroundColor/B"), (long) backgroundColor.Blue());
		}
		else
		{
			config.Write(wxT("FastHaloRemoval"), false);
		}
	}

	return TRUE;
}

void DDSOptionsDialog::do_layout()
{
	// begin wxGlade: MyFrame::do_layout
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* mipmapIndenter = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* mipmapFilterIndenter = new wxBoxSizer(wxHORIZONTAL);
	wxStaticBoxSizer* mipmapBox = new wxStaticBoxSizer(mipmapBox_staticbox, wxVERTICAL);
	wxBoxSizer* colorFillIndenter = new wxBoxSizer(wxHORIZONTAL);
	wxBoxSizer* colorFillBox = new wxBoxSizer(wxVERTICAL);
	wxBoxSizer* chooseBackgroundIndenter = new wxBoxSizer(wxHORIZONTAL);
	mipmapFilterIndenter->Add(20, 10, 0, wxADJUST_MINSIZE, 0);
	mipmapFilterIndenter->Add(new wxStaticText(this, -1, wxT("Filter:")), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
	mipmapFilterIndenter->Add(mipmapFilterChoice, 1, wxALIGN_CENTER_VERTICAL | wxEXPAND);
	topSizer->Add(useDXTC, 0, wxALL |wxADJUST_MINSIZE, 10);
	topSizer->Add(generateMipmaps, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE, 10);
	topSizer->Add(mipmapFilterIndenter, 0, wxALL | wxADJUST_MINSIZE | wxEXPAND, 10);
	mipmapIndenter->Add(20, 20, 0, wxADJUST_MINSIZE, 1);
	mipmapBox->Add(colorFillBackground, 0, wxALL|wxADJUST_MINSIZE, 10);
	colorFillIndenter->Add(20, 20, 0, wxADJUST_MINSIZE, 0);
	colorFillBox->Add(reconstructColors, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE, 10);
	chooseBackgroundIndenter->Add(20, 20, 0, wxADJUST_MINSIZE, 0);
	chooseBackgroundIndenter->Add(chooseBackground, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE, 10);
	colorFillBox->Add(chooseBackgroundIndenter, 1, wxEXPAND, 0);
	colorFillIndenter->Add(colorFillBox, 1, wxEXPAND, 0);
	mipmapBox->Add(colorFillIndenter, 1, wxEXPAND, 0);
	mipmapIndenter->Add(mipmapBox, 1, wxLEFT|wxRIGHT|wxBOTTOM|wxEXPAND, 10);
	topSizer->Add(mipmapIndenter, 1, wxEXPAND, 0);
	wxBoxSizer *buttonSizer = new wxBoxSizer(wxHORIZONTAL);
	wxButton *cancelButton = new wxButton(this, wxID_CANCEL);
	buttonSizer->Add(cancelButton, 1, wxLEFT|wxRIGHT, 10);
	wxButton *okButton = new wxButton(this, wxID_OK);
	buttonSizer->Add(okButton, 1, wxLEFT|wxRIGHT, 10);
	topSizer->Add(buttonSizer, 0, wxEXPAND|wxBOTTOM, 10);
	SetAutoLayout(true);
	SetSizer(topSizer);
	topSizer->Fit(this);
	topSizer->SetSizeHints(this);
	Layout();
}

void DDSOptionsDialog::OnChooseBackground(wxCommandEvent &)
{
	wxColourData data;
	data.SetColour(backgroundColor);
	wxColourDialog dialog(this, &data);
	if (dialog.ShowModal() == wxID_OK)
	{
		wxColourData redData = dialog.GetColourData();
		backgroundColor = redData.GetColour();
	}
}

void DDSOptionsDialog::OnUpdateEnablement(wxCommandEvent &)
{
	update_enablement();
}

BEGIN_EVENT_TABLE(DDSOptionsDialog, wxDialog)
EVT_BUTTON(BUTTON_ChooseBackground, DDSOptionsDialog::OnChooseBackground)
EVT_CHECKBOX(BUTTON_ColorFillBackground, DDSOptionsDialog::OnUpdateEnablement)
EVT_CHECKBOX(BUTTON_ReconstructColors, DDSOptionsDialog::OnUpdateEnablement)
EVT_CHECKBOX(BUTTON_GenerateMipmaps, DDSOptionsDialog::OnUpdateEnablement)

END_EVENT_TABLE()


