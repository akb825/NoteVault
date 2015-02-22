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
#include "../notes/NoteSet.h"
#include "../io/NoteFile.h"
#include <wx/richtext/richtextctrl.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/msgdlg.h>

#define VERSION "0.1"

namespace NoteVault
{

static const int cIdNoteList = wxWindow::NewControlId();
static const int cIdNoteText = wxWindow::NewControlId();
static const int cIdAddButton = wxWindow::NewControlId();
static const int cIdRemoveButton = wxWindow::NewControlId();

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

	EVT_BUTTON(cIdAddButton, MainWindow::OnAddNote)
	EVT_BUTTON(cIdRemoveButton, MainWindow::OnRemoveNote)

	EVT_LIST_END_LABEL_EDIT(cIdNoteList, MainWindow::OnTitleEdit)
	EVT_LIST_ITEM_SELECTED(cIdNoteList, MainWindow::OnSelectNote)
	EVT_LIST_ITEM_DESELECTED(cIdNoteList, MainWindow::OnDeselectNote)
wxEND_EVENT_TABLE()

struct MainWindow::NoteContext
{
	NoteContext()
		: selectedNote(noteSet.end())
	{
	}

	NoteSet noteSet;
	std::vector<uint8_t> key;
	std::string saveLocation;

	NoteSet::iterator selectedNote;
};

MainWindow::MainWindow(const wxString& title, const wxSize& size)
	: wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size), m_Notes(new NoteContext),
	m_IgnoreSelectionChanges(false), m_SortNextUpdate(false)
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

	vertSizer->Add(new wxStaticText(listPanel, wxID_ANY, "Notes"),
		wxSizerFlags().Center().Border(wxTOP, 10));

	m_NoteList = new wxListCtrl(listPanel, cIdNoteList, wxDefaultPosition, wxDefaultSize,
		wxLC_REPORT | wxLC_NO_HEADER | wxLC_EDIT_LABELS | wxLC_SINGLE_SEL);
	m_NoteList->InsertColumn(0, wxT("Notes"));
	m_NoteList->SetMinClientSize(wxSize(100, 100));
	vertSizer->Add(m_NoteList, wxSizerFlags().Expand().Border(wxALL, 10).Proportion(1));

	wxSizer* horizSizer = new wxBoxSizer(wxHORIZONTAL);

	wxButton* addButton = new wxButton(listPanel, cIdAddButton, "Add");
	horizSizer->Add(addButton, wxSizerFlags().Border(wxBOTTOM | wxLEFT | wxRIGHT, 10));

	m_RemoveButton = new wxButton(listPanel, cIdRemoveButton, "Remove");
	horizSizer->Add(m_RemoveButton, wxSizerFlags().Border(wxBOTTOM | wxRIGHT, 10));

	vertSizer->Add(horizSizer, 0, wxALIGN_LEFT, 0);

	listPanel->SetSizerAndFit(vertSizer);

	//Note panel
	vertSizer = new wxBoxSizer(wxVERTICAL);

	m_NoteText = new wxRichTextCtrl(notePanel, cIdNoteText);
	vertSizer->Add(m_NoteText, wxSizerFlags().Expand().Border(wxALL, 10).Proportion(1));

	notePanel->SetSizerAndFit(vertSizer);

	UpdateMenuItems();
	UpdateUi();
}

void MainWindow::OnExit(wxCommandEvent&)
{
	Close(true);
}

void MainWindow::OnNew(wxCommandEvent&)
{
	m_Notes.reset(new NoteContext);
	UpdateUi();
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
	if (m_SortNextUpdate)
	{
		Sort();
		m_SortNextUpdate = false;
	}
	UpdateMenuItems();
}

void MainWindow::OnNoteTextChanged(wxCommandEvent&)
{
}

void MainWindow::OnAddNote(wxCommandEvent&)
{
	long index = m_NoteList->GetItemCount();
	m_NoteList->InsertItem(index, wxEmptyString);
	m_NoteList->SetItemState(index, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	m_NoteList->EditLabel(index);
}

void MainWindow::OnRemoveNote(wxCommandEvent&)
{
}

void MainWindow::OnTitleEdit(wxListEvent& event)
{
	bool newNote = (size_t)m_NoteList->GetItemCount() > m_Notes->noteSet.size();
	if (event.IsEditCancelled())
	{
		if (newNote)
			m_NoteList->DeleteItem(event.GetIndex());
		return;
	}

	Note* note;
	if (newNote)
	{
		m_Notes->selectedNote = m_Notes->noteSet.insert(m_Notes->noteSet.end());
		note = &*m_Notes->selectedNote;
	}
	else
		note = &m_Notes->noteSet[event.GetIndex()];

	note->SetTitle(event.GetLabel().ToUTF8().data());
	m_SortNextUpdate = true;
}

void MainWindow::OnSelectNote(wxListEvent& event)
{
	if (!m_IgnoreSelectionChanges)
		UpdateForSelection(event.GetIndex());
}

void MainWindow::OnDeselectNote(wxListEvent&)
{
	if (!m_IgnoreSelectionChanges)
		UpdateForDeselection();
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

void MainWindow::UpdateUi()
{
	m_NoteList->DeleteAllItems();
	for (const Note& note : m_Notes->noteSet)
	{
		m_NoteList->InsertItem(m_NoteList->GetItemCount(),
			wxString::FromUTF8(note.GetTitle().c_str()));
	}
	m_NoteList->SetColumnWidth(0, wxLIST_AUTOSIZE);

	if (!m_IgnoreSelectionChanges)
		UpdateForDeselection();
}

void MainWindow::UpdateForSelection(long item)
{
	m_Notes->selectedNote = m_Notes->noteSet.begin() + item;
}

void MainWindow::UpdateForDeselection()
{
	m_Notes->selectedNote = m_Notes->noteSet.end();
}

void MainWindow::Sort()
{
	m_IgnoreSelectionChanges = true;

	const uint64_t cNotSelected = (uint64_t)-1;
	uint64_t selectedNoteId = cNotSelected;
	if (m_Notes->selectedNote != m_Notes->noteSet.end())
		selectedNoteId = m_Notes->selectedNote->GetId();
	m_Notes->noteSet.sort([] (const Note& left, const Note& right) -> bool
		{
			return strcasecmp(left.GetTitle().c_str(), right.GetTitle().c_str()) < 0;
		});

	if (selectedNoteId != cNotSelected)
		m_Notes->selectedNote = m_Notes->noteSet.find(selectedNoteId);

	UpdateUi();

	if (selectedNoteId != cNotSelected)
	{
		m_NoteList->SetItemState(m_Notes->selectedNote - m_Notes->noteSet.begin(),
			wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
	}

	m_NoteList->SetColumnWidth(0, wxLIST_AUTOSIZE);
	m_IgnoreSelectionChanges = false;
}

} // namespace NoteVault
