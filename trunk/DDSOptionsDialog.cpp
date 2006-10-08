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

extern bool HasS3TC(); // from imagdds

DDSOptionsDialog::DDSOptionsDialog(const wxString& prefix)
	: wxDialog(NULL, -1, _T("DDS Options"), wxDefaultPosition, wxDefaultSize), m_prefix(prefix)
{

	mipmapBox_staticbox = new wxStaticBox(this, -1, wxT("Mipmap Halo Removal Strategy"));
	useDXTC = new wxCheckBox(this, -1, wxT("Use DXTC (Texture Compression)"));
	generateMipmaps = new wxCheckBox(this, BUTTON_GenerateMipmaps, wxT("Generate Mipmaps"));
	wxArrayString filterChoices;
	filterChoices.Add("Triangle");
	filterChoices.Add("Mitchell (Bicubic)");
	filterChoices.Add("Lanczos");
	mipmapFilterChoice = new wxChoice(this, -1, wxDefaultPosition, wxDefaultSize, filterChoices);
	noHaloRemoval = new wxRadioButton(this, BUTTON_NoHaloRemoval, wxT("None"));
	premultiplyAlpha = new wxRadioButton(this, BUTTON_PremultiplyAlpha, wxT("Premultiply Alpha"));
	colorFillBackground = new wxRadioButton(this, BUTTON_ColorFillBackground, wxT("Color Fill Background (Very Slow)"));
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
	if (HasS3TC())
	{
		config.Read("UseDXTC", &value, true);
		useDXTC->SetValue(value ? 1 : 0);
	}
	else
	{
		useDXTC->SetValue(0);
	}
	
	config.Read("GenerateMipmaps", &value, true);
	generateMipmaps->SetValue(value ? 1 : 0);

	long filter;
	config.Read("MipmapFilter", &filter, 1);
	mipmapFilterChoice->SetSelection(filter);

	wxString haloRemovalStrategy;
	config.Read("HaloRemovalStrategy", &haloRemovalStrategy, "None");
	if (haloRemovalStrategy == "PremultiplyAlpha")
	{
		premultiplyAlpha->SetValue(1);
	}
	else if (haloRemovalStrategy == "ColorFillBackground")
	{
		colorFillBackground->SetValue(1);
	}
	else
	{
		noHaloRemoval->SetValue(1);
	}

	config.Read("ReconstructEdgeColors", &value, true);
	reconstructColors->SetValue(value ? 1 : 0);

	long r, g, b;
	config.Read("BackgroundColor/R", &r, 0xff);
	config.Read("BackgroundColor/G", &g, 0xff);
	config.Read("BackgroundColor/B", &b, 0xff);
	backgroundColor.Set((unsigned char) r, (unsigned char) g, (unsigned char) b);
}

void DDSOptionsDialog::update_enablement()
{
	useDXTC->Enable(HasS3TC());

	noHaloRemoval->Enable(generateMipmaps->GetValue());
	premultiplyAlpha->Enable(generateMipmaps->GetValue());
	mipmapFilterChoice->Enable(generateMipmaps->GetValue());
	colorFillBackground->Enable(generateMipmaps->GetValue());
	reconstructColors->Enable(generateMipmaps->GetValue() && colorFillBackground->GetValue());
	chooseBackground->Enable(generateMipmaps->GetValue() && colorFillBackground->GetValue() && reconstructColors->GetValue());
}

bool DDSOptionsDialog::Validate()
{
	wxConfig config;
	config.SetPath(m_prefix);
	config.Write("UseDXTC", useDXTC->GetValue());
	config.Write("GenerateMipmaps", generateMipmaps->GetValue());
	if (generateMipmaps->GetValue())
	{
		config.Write("MipmapFilter", mipmapFilterChoice->GetSelection());
		if (noHaloRemoval->GetValue())
		{
			config.Write("HaloRemovalStrategy", "None");
		}
		else if (premultiplyAlpha->GetValue())
		{
			config.Write("HaloRemovalStrategy", "PremultiplyAlpha");
		} 
		else if (colorFillBackground->GetValue())
		{
			config.Write("HaloRemovalStrategy", "ColorFillBackground");
		}

		if (colorFillBackground->GetValue())
		{
			config.Write("ReconstructColors", reconstructColors->GetValue() == 1);
			
			config.Write("BackgroundColor/R", (long) backgroundColor.Red());
			config.Write("BackgroundColor/G", (long) backgroundColor.Green());
			config.Write("BackgroundColor/B", (long) backgroundColor.Blue());
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
	mipmapFilterIndenter->Add(new wxStaticText(this, -1, "Filter:"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 10);
	mipmapFilterIndenter->Add(mipmapFilterChoice, 1, wxALIGN_CENTER_VERTICAL | wxEXPAND);
	topSizer->Add(useDXTC, 0, wxALL |wxADJUST_MINSIZE, 10);
	topSizer->Add(generateMipmaps, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE, 10);
	topSizer->Add(mipmapFilterIndenter, 0, wxALL | wxADJUST_MINSIZE | wxEXPAND, 10);
	mipmapIndenter->Add(20, 20, 0, wxADJUST_MINSIZE, 1);
	mipmapBox->Add(noHaloRemoval, 0, wxALL|wxADJUST_MINSIZE, 10);
	mipmapBox->Add(premultiplyAlpha, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE, 10);
	mipmapBox->Add(colorFillBackground, 0, wxLEFT|wxRIGHT|wxBOTTOM|wxADJUST_MINSIZE, 10);
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
EVT_RADIOBUTTON(BUTTON_NoHaloRemoval, DDSOptionsDialog::OnUpdateEnablement)
EVT_RADIOBUTTON(BUTTON_PremultiplyAlpha, DDSOptionsDialog::OnUpdateEnablement)
EVT_RADIOBUTTON(BUTTON_ColorFillBackground, DDSOptionsDialog::OnUpdateEnablement)
EVT_CHECKBOX(BUTTON_ReconstructColors, DDSOptionsDialog::OnUpdateEnablement)
EVT_CHECKBOX(BUTTON_GenerateMipmaps, DDSOptionsDialog::OnUpdateEnablement)

END_EVENT_TABLE()


