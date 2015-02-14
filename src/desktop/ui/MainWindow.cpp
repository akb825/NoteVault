/*
 * Copyright 2015 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0

 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MainWindow.h"
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/listbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

#define VERSION "0.1"

namespace NoteVault
{

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_NEW, MainWindow::OnNew)
	EVT_MENU(wxID_OPEN, MainWindow::OnOpen)
	EVT_MENU(wxID_EXIT, MainWindow::OnExit)
	EVT_MENU(wxID_ABOUT, MainWindow::OnAbout)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxString& title, const wxSize& size)
	: wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size)
{
	//Menu
	wxMenuBar* menuBar = new wxMenuBar;

	wxMenu* fileMenu = new wxMenu;
	fileMenu->Append(wxID_NEW, "", "");
	fileMenu->Append(wxID_OPEN, "", "");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "", "");

	wxMenu* editMenu = new wxMenu;
	editMenu->Append(wxID_UNDO, "", "");
	editMenu->Append(wxID_REDO, "", "");
	editMenu->AppendSeparator();
	editMenu->Append(wxID_CUT, "", "");
	editMenu->Append(wxID_COPY, "", "");
	editMenu->Append(wxID_PASTE, "", "");
	editMenu->Append(wxID_DELETE, "", "");
	editMenu->AppendSeparator();
	editMenu->Append(wxID_SELECTALL, "", "");

	wxMenu* helpMenu = new wxMenu;
	helpMenu->Append(wxID_ABOUT, "", "");

	menuBar->Append(fileMenu, "&File");
	menuBar->Append(editMenu, "&Edit");
	menuBar->Append(helpMenu, "&Help");

	SetMenuBar(menuBar);

	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnNew, this, wxID_NEW);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnOpen, this, wxID_OPEN);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnExit, this, wxID_EXIT);
	Bind(wxEVT_COMMAND_MENU_SELECTED, &MainWindow::OnAbout, this, wxID_ABOUT);

	//Window layout
	wxSplitterWindow* splitWindow = new wxSplitterWindow(this);
	splitWindow->SetMinimumPaneSize(50);

	wxPanel* listPanel = new wxPanel(splitWindow);
	wxPanel* notePanel = new wxPanel(splitWindow);
	splitWindow->SplitVertically(listPanel, notePanel, size.GetWidth()/4);

	//List panel
	wxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);

	vertSizer->Add(new wxStaticText(listPanel, wxID_ANY, "Notes"),
		wxSizerFlags().Center().Border(wxTOP, 10));
	m_NoteList = new wxListBox(listPanel, wxID_ANY);
	vertSizer->Add(m_NoteList, wxSizerFlags().Expand().Border(wxALL, 10).Proportion(1));

	wxSizer* horizSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton* addNoteButton = new wxButton(listPanel, wxID_ANY, "Add");
	horizSizer->Add(addNoteButton, wxSizerFlags().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

	wxButton* removeNoteButton = new wxButton(listPanel, wxID_ANY, "Remove");
	horizSizer->Add(removeNoteButton, wxSizerFlags().Border(wxBOTTOM | wxRIGHT, 10));

	vertSizer->Add(horizSizer, 0, wxALIGN_LEFT, 0);

	listPanel->SetSizerAndFit(vertSizer);

	//Note panel
	vertSizer = new wxBoxSizer(wxVERTICAL);

	m_NoteText = new wxTextCtrl(notePanel, wxID_ANY, wxEmptyString, wxDefaultPosition,
		wxDefaultSize, wxTE_MULTILINE | wxTE_PROCESS_TAB | wxTE_WORDWRAP);
	vertSizer->Add(m_NoteText, wxSizerFlags().Expand().Border(wxALL, 10).Proportion(1));

	notePanel->SetSizerAndFit(vertSizer);
}

void MainWindow::OnExit(wxCommandEvent&)
{
	Close(true);
}

void MainWindow::OnNew(wxCommandEvent&)
{
	printf("new\n");
}

void MainWindow::OnOpen(wxCommandEvent&)
{
	printf("open\n");
}

void MainWindow::OnAbout(wxCommandEvent&)
{
	wxMessageBox(L"NoteVault version " VERSION "\nCopyright \u00A9 2015 Aaron Barany",
		"About NoteVault", wxOK, this);
}

} // namespace NoteVault
