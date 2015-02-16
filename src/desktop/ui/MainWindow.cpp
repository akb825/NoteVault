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
#include <wx/richtext/richtextctrl.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/editlbox.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

#define VERSION "0.1"

namespace NoteVault
{

static const int cIdNoteList = 0;
static const int cIdNoteText = 1;

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_NEW, MainWindow::OnNew)
	EVT_MENU(wxID_OPEN, MainWindow::OnOpen)
	EVT_MENU(wxID_EXIT, MainWindow::OnExit)

	EVT_MENU(wxID_UNDO, MainWindow::OnUndo)
	EVT_MENU(wxID_REDO, MainWindow::OnRedo)
	EVT_MENU(wxID_CUT, MainWindow::OnCut)
	EVT_MENU(wxID_COPY, MainWindow::OnCopy)
	EVT_MENU(wxID_PASTE, MainWindow::OnPaste)
	EVT_MENU(wxID_DELETE, MainWindow::OnDelete)
	EVT_MENU(wxID_SELECTALL, MainWindow::OnSelectAll)

	EVT_MENU(wxID_ABOUT, MainWindow::OnAbout)

	EVT_IDLE(MainWindow::OnIdle)

	EVT_TEXT(cIdNoteText, MainWindow::OnNoteTextChanged)
wxEND_EVENT_TABLE()

MainWindow::MainWindow(const wxString& title, const wxSize& size)
	: wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size), m_FocusEntry(nullptr)
{
	//Menu
	wxMenuBar* menuBar = new wxMenuBar;

	wxMenu* fileMenu = new wxMenu;
	fileMenu->Append(wxID_NEW, "", "");
	fileMenu->Append(wxID_OPEN, "", "");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_EXIT, "", "");

	wxMenu* editMenu = new wxMenu;
	m_UndoItem = editMenu->Append(wxID_UNDO, "", "");
	m_RedoItem = editMenu->Append(wxID_REDO, "", "");
	editMenu->AppendSeparator();
	m_CutItem = editMenu->Append(wxID_CUT, "", "");
	m_CopyItem = editMenu->Append(wxID_COPY, "", "");
	m_PasteItem = editMenu->Append(wxID_PASTE, "", "");
	m_DeleteItem = editMenu->Append(wxID_DELETE, "", "");
	editMenu->AppendSeparator();
	m_SelectAllItem = editMenu->Append(wxID_SELECTALL, "", "");

	wxMenu* helpMenu = new wxMenu;
	helpMenu->Append(wxID_ABOUT, "", "");

	menuBar->Append(fileMenu, "&File");
	menuBar->Append(editMenu, "&Edit");
	menuBar->Append(helpMenu, "&Help");

	SetMenuBar(menuBar);

	//Window layout
	wxSplitterWindow* splitWindow = new wxSplitterWindow(this);
	splitWindow->SetMinimumPaneSize(50);

	wxPanel* listPanel = new wxPanel(splitWindow);
	wxPanel* notePanel = new wxPanel(splitWindow);
	splitWindow->SplitVertically(listPanel, notePanel, size.GetWidth()/4);

	//List panel
	wxSizer* vertSizer = new wxBoxSizer(wxVERTICAL);

	m_NoteList = new wxEditableListBox(listPanel, cIdNoteList, "Notes");
	m_NoteList->SetMinClientSize(wxSize(100, 100));
	vertSizer->Add(m_NoteList, wxSizerFlags().Expand().Border(wxALL, 10).Proportion(1));

	listPanel->SetSizerAndFit(vertSizer);

	//Note panel
	vertSizer = new wxBoxSizer(wxVERTICAL);

	m_NoteText = new wxRichTextCtrl(notePanel, cIdNoteText);
	vertSizer->Add(m_NoteText, wxSizerFlags().Expand().Border(wxALL, 10).Proportion(1));

	notePanel->SetSizerAndFit(vertSizer);

	UpdateMenuItems();
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

void MainWindow::OnUndo(wxCommandEvent&)
{
	wxCommandProcessor* undoStack = GetUndoStack();
	if (undoStack)
		undoStack->Undo();
}

void MainWindow::OnRedo(wxCommandEvent&)
{
	wxCommandProcessor* undoStack = GetUndoStack();
	if (undoStack)
		undoStack->Redo();
}

void MainWindow::OnCut(wxCommandEvent&)
{
	wxTextEntry* textEntry = GetTextEntry();
	wxRichTextCtrl* richText = GetRichTextCtrl();
	if (textEntry)
		textEntry->Cut();
	else if (richText)
		richText->Cut();
}

void MainWindow::OnCopy(wxCommandEvent&)
{
	wxTextEntry* textEntry = GetTextEntry();
	wxRichTextCtrl* richText = GetRichTextCtrl();
	if (textEntry)
		textEntry->Copy();
	else if (richText)
		richText->Copy();
}

void MainWindow::OnPaste(wxCommandEvent&)
{
	wxTextEntry* textEntry = GetTextEntry();
	wxRichTextCtrl* richText = GetRichTextCtrl();
	if (textEntry)
		textEntry->Paste();
	else if (richText)
		richText->Paste();
}

void MainWindow::OnDelete(wxCommandEvent&)
{
	wxTextEntry* textEntry = GetTextEntry();
	wxRichTextCtrl* richText = GetRichTextCtrl();
	if (textEntry)
	{
		long from, to;
		textEntry->GetSelection(&from, &to);
		textEntry->Remove(from, to);
	}
	else if (richText)
	{
		long from, to;
		richText->GetSelection(&from, &to);
		richText->Remove(from, to);
	}
}

void MainWindow::OnSelectAll(wxCommandEvent&)
{
	wxTextEntry* textEntry = GetTextEntry();
	if (textEntry)
		textEntry->SelectAll();
}

void MainWindow::OnAbout(wxCommandEvent&)
{
	wxMessageBox(L"NoteVault version " VERSION "\nCopyright \u00A9 2015 Aaron Barany",
		"About NoteVault", wxOK, this);
}

void MainWindow::OnIdle(wxIdleEvent&)
{
	UpdateMenuItems();
}

void MainWindow::OnNoteTextChanged(wxCommandEvent&)
{
}

wxCommandProcessor* MainWindow::GetUndoStack()
{
	wxRichTextCtrl* textControl = dynamic_cast<wxRichTextCtrl*>(FindFocus());
	if (textControl)
		return textControl->GetCommandProcessor();

	return nullptr;
}

wxTextEntry* MainWindow::GetTextEntry()
{
	return dynamic_cast<wxTextEntry*>(FindFocus());
}

wxRichTextCtrl* MainWindow::GetRichTextCtrl()
{
	return dynamic_cast<wxRichTextCtrl*>(FindFocus());
}

void MainWindow::UpdateMenuItems()
{
	wxCommandProcessor* undoStack = GetUndoStack();
	wxTextEntry* textEntry = GetTextEntry();
	wxRichTextCtrl* richText = GetRichTextCtrl();

	if (undoStack)
	{
		m_UndoItem->Enable(undoStack->CanUndo());
		m_RedoItem->Enable(undoStack->CanRedo());
	}
	else
	{
		m_UndoItem->Enable(false);
		m_RedoItem->Enable(false);
	}

	if (textEntry)
	{
		m_CutItem->Enable(textEntry->CanCut());
		m_CopyItem->Enable(textEntry->CanCopy());
		m_PasteItem->Enable(textEntry->CanPaste());

		long from, to;
		textEntry->GetSelection(&from, &to);
		m_DeleteItem->Enable(from != to);

		m_SelectAllItem->Enable(!textEntry->IsEmpty());
	}
	else if (richText)
	{
		m_CutItem->Enable(richText->CanCut());
		m_CopyItem->Enable(richText->CanCopy());
		m_PasteItem->Enable(richText->CanPaste());

		long from, to;
		richText->GetSelection(&from, &to);
		m_DeleteItem->Enable(from != to);

		m_SelectAllItem->Enable(!richText->IsEmpty());
	}
	else
	{
		m_CutItem->Enable(false);
		m_CopyItem->Enable(false);
		m_PasteItem->Enable(false);
		m_DeleteItem->Enable(false);
		m_SelectAllItem->Enable(false);
	}
}

} // namespace NoteVault
