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
#include <wx/menu.h>
#include <wx/msgdlg.h>

#include "../io/NoteFile.h"
#include "../io/Crypto.h"
#include "../io/FileIStream.h"
#include "../io/FileOStream.h"
#include "../notes/NoteSet.h"

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

	std::vector<uint8_t> salt = Crypto::Random(Crypto::cSaltLenBytes);
	printf("calculating key...\n");
	std::vector<uint8_t> key = Crypto::GenerateKey("asdf", salt, Crypto::cDefaultKeyIterations);
	if (key.empty())
	{
		printf("couldn't calculate key\n");
		abort();
	}
	printf("done\n");
	{
		NoteSet testNotes;
		NoteSet::iterator newNote = testNotes.insert(testNotes.end());
		newNote->SetTitle("Test 1");
		newNote->SetMessage("Test Note 1");
		newNote = testNotes.insert(testNotes.end());
		newNote->SetTitle("Test 2");
		newNote->SetMessage("Test Note 2");
		newNote = testNotes.insert(testNotes.end());
		newNote->SetTitle("Test 3");
		newNote->SetMessage("Test Note 3");

		FileOStream stream;
		if (!stream.Open("asdf.notes"))
		{
			printf("couldn't open notes\n");
			abort();
		}

		NoteFile::Result result = NoteFile::SaveNotes(testNotes, stream, salt, key);
		switch (result)
		{
			case NoteFile::Result::Success:
				printf("succesfully saved notes\n");
				break;
			case NoteFile::Result::InvalidFile:
				printf("invalid file\n");
				abort();
				break;
			case NoteFile::Result::IoError:
				printf("io error\n");
				abort();
				break;
			case NoteFile::Result::EncryptionError:
				printf("encryption error\n");
				abort();
				break;
			default:
				printf("unknown error\n");
				abort();
				break;
		}
	}

	{
		FileIStream stream;
		if (!stream.Open("asdf.notes"))
		{
			printf("couldn't open notes\n");
			abort();
		}

		NoteSet testNotes;
		std::vector<uint8_t> loadSalt;
		std::vector<uint8_t> loadKey;
		NoteFile::Result result = NoteFile::LoadNotes(testNotes, stream, "bla", loadSalt, loadKey);
		switch (result)
		{
			case NoteFile::Result::Success:
				printf("succesfully loaded notes\n");
				abort();
				break;
			case NoteFile::Result::InvalidFile:
				printf("invalid file\n");
				abort();
				break;
			case NoteFile::Result::IoError:
				printf("io error\n");
				abort();
				break;
			case NoteFile::Result::EncryptionError:
				printf("encryption error\n");
				break;
			default:
				printf("unknown error\n");
				abort();
				break;
		}
	}

	{
		FileIStream stream;
		if (!stream.Open("asdf.notes"))
		{
			printf("couldn't open notes\n");
			abort();
		}

		NoteSet testNotes;
		std::vector<uint8_t> loadSalt;
		std::vector<uint8_t> loadKey;
		NoteFile::Result result = NoteFile::LoadNotes(testNotes, stream, "asdf", loadSalt, loadKey);
		switch (result)
		{
			case NoteFile::Result::Success:
				printf("succesfully loaded notes\n");
				break;
			case NoteFile::Result::InvalidFile:
				printf("invalid file\n");
				abort();
				break;
			case NoteFile::Result::IoError:
				printf("io error\n");
				abort();
				break;
			case NoteFile::Result::EncryptionError:
				printf("encryption error\n");
				abort();
				break;
			default:
				printf("unknown error\n");
				abort();
				break;
		}

		if (loadSalt.size() != salt.size() || memcmp(loadSalt.data(), salt.data(), salt.size()) != 0)
		{
			printf("salt doesn't match\n");
			abort();
		}

		if (loadKey.size() != key.size() || memcmp(loadKey.data(), key.data(), key.size()) != 0)
		{
			printf("salt doesn't match\n");
			abort();
		}

		if (testNotes.size() != 3)
		{
			printf("bad note count\n");
			abort();
		}

		NoteSet::const_iterator iter = testNotes.begin();
		printf("%s: %s\n", iter->GetTitle().c_str(), iter->GetMessage().c_str());
		if (iter->GetTitle() != "Test 1" || iter->GetMessage() != "Test Note 1")
		{
			printf("bad note contents\n");
			abort();
		}
		++iter;
		printf("%s: %s\n", iter->GetTitle().c_str(), iter->GetMessage().c_str());
		if (iter->GetTitle() != "Test 2" || iter->GetMessage() != "Test Note 2")
		{
			printf("bad note contents\n");
			abort();
		}
		++iter;
		printf("%s: %s\n", iter->GetTitle().c_str(), iter->GetMessage().c_str());
		if (iter->GetTitle() != "Test 3" || iter->GetMessage() != "Test Note 3")
		{
			printf("bad note contents\n");
			abort();
		}
		++iter;
		if (iter != testNotes.end())
		{
			printf("bad note iterator\n");
			abort();
		}

		printf("success!\n");
	}
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

} // namespace NoteVault
