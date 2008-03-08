/*
 
 DDSOptionsDialog.h: part of the Aleph One Replacement Texture Utility
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

#ifndef DDSOPTIONSDIALOG_H
#define DDSOPTIONSDIALOG_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP 
#include <wx/wx.h>
#include <wx/checkbox.h>
#include <wx/choice.h>
#include <wx/colordlg.h>
#include <wx/config.h>
#ifdef wxUSE_DRAG_AND_DROP
#include <wx/dnd.h>
#endif
#include <wx/image.h>
#include <wx/notebook.h>
#include <wx/radiobox.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
#include <wx/statline.h>

#include <wx/bitmap.h>
#include <wx/bmpbuttn.h>
#endif

#include "aorta.h"

class DDSOptionsDialog : public wxDialog
{
public:
	DDSOptionsDialog(const wxString& preferencesPrefix, bool HasAlpha);

	wxStaticBox* mipmapBox_staticbox;
	wxCheckBox* useDXTC;
	wxCheckBox* generateMipmaps;
	wxChoice* mipmapFilterChoice;
	wxCheckBox* repeatingTexture;
	wxCheckBox* colorFillBackground;
	wxCheckBox* reconstructColors;
	wxButton* chooseBackground;

	wxColor backgroundColor;
	
	void OnChooseBackground(wxCommandEvent &);
	void OnUpdateEnablement(wxCommandEvent &);
private:
	wxString m_prefix;
	bool m_hasAlpha;
	void fill_from_prefs();
	void do_layout();
	void update_enablement();
	bool Validate();
	
	DECLARE_EVENT_TABLE()
};

#endif
