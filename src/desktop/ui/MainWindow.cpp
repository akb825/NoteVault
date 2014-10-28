/*
 * Copyright 2014 Aaron Barany
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

#include "ui/MainWindow.h"
#include <wx/app.h>
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
	wxMessageBox("NoteVault version " VERSION "\nCopyright 2014 Aaron Barany", "About NoteVault",
		wxOK, this);
}

class App : public wxApp
{
public:
	App()
		: m_Window(nullptr) {}

	virtual ~App()
	{
	}

	virtual bool OnInit()
	{
		m_Window = new MainWindow("Note Vault", wxSize(800, 600));
		m_Window->Show(true);
		return true;
	}

private:
	MainWindow* m_Window;
};

} // namespace NoteVault

wxIMPLEMENT_APP_NO_MAIN(NoteVault::App);

extern "C"
{

int NoteVault_RunApp()
{
	int argc = 0;
	wxChar** argv = nullptr;
	return wxEntry(argc, argv);
}

} // extern "C"
