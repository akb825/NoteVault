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
#include "../io/Crypto.h"
#include "../io/NoteFile.h"
#include "../io/FileIStream.h"
#include "../io/FileOStream.h"
#include <wx/richtext/richtextctrl.h>
#include <wx/splitter.h>
#include <wx/panel.h>
#include <wx/stattext.h>
#include <wx/listctrl.h>
#include <wx/textctrl.h>
#include <wx/button.h>
#include <wx/sizer.h>
#include <wx/menu.h>
#include <wx/filedlg.h>
#include <wx/aboutdlg.h>
#include <wx/textdlg.h>
#include <wx/msgdlg.h>

#define VERSION "0.2"

namespace NoteVault
{

static const int cIdNoteList = wxWindow::NewControlId();
static const int cIdNoteText = wxWindow::NewControlId();
static const int cIdAddButton = wxWindow::NewControlId();
static const int cIdRemoveButton = wxWindow::NewControlId();

wxBEGIN_EVENT_TABLE(MainWindow, wxFrame)
	EVT_MENU(wxID_NEW, MainWindow::OnNew)
	EVT_MENU(wxID_OPEN, MainWindow::OnOpen)
	EVT_MENU(wxID_SAVE, MainWindow::OnSave)
	EVT_MENU(wxID_SAVEAS, MainWindow::OnSaveAs)
	EVT_MENU(wxID_EXIT, MainWindow::OnExit)

	EVT_MENU(wxID_UNDO, MainWindow::OnUndo)
	EVT_MENU(wxID_REDO, MainWindow::OnRedo)
	EVT_MENU(wxID_CUT, MainWindow::OnCut)
	EVT_MENU(wxID_COPY, MainWindow::OnCopy)
	EVT_MENU(wxID_PASTE, MainWindow::OnPaste)
	EVT_MENU(wxID_DELETE, MainWindow::OnDelete)
	EVT_MENU(wxID_SELECTALL, MainWindow::OnSelectAll)
	EVT_MENU(wxID_ADD, MainWindow::OnAddNote)
	EVT_MENU(wxID_REMOVE, MainWindow::OnRemoveNote)

	EVT_MENU(wxID_ABOUT, MainWindow::OnAbout)

	EVT_CLOSE(MainWindow::OnClose)

	EVT_TEXT(cIdNoteText, MainWindow::OnNoteTextChanged)

	EVT_BUTTON(cIdAddButton, MainWindow::OnAddNote)
	EVT_BUTTON(cIdRemoveButton, MainWindow::OnRemoveNote)

	EVT_LIST_END_LABEL_EDIT(cIdNoteList, MainWindow::OnTitleEdit)
	EVT_LIST_KEY_DOWN(cIdNoteList, MainWindow::OnTitleKeyDown)
	EVT_LIST_ITEM_SELECTED(cIdNoteList, MainWindow::OnSelectNote)
	EVT_LIST_ITEM_DESELECTED(cIdNoteList, MainWindow::OnDeselectNote)
wxEND_EVENT_TABLE()

struct MainWindow::NoteContext
{
	NoteContext()
		: dirty(false)
	{
	}

	NoteSet noteSet;
	std::vector<uint8_t> key;
	std::vector<uint8_t> salt;
	std::string savePath;

	bool dirty;
	std::string fileName;

	NoteSet::iterator selectedNote;
};

class MainWindow::NoteCommand : public wxCommand
{
public:
	NoteCommand(MainWindow& parent, const Note& note)
		: m_Parent(&parent), m_Note(note)
	{
	}

	uint64_t GetNoteId() const	{return m_Note.GetId();}
	void UpdateNote(const Note& note)
	{
		if (note.GetId() != m_Note.GetId())
			return;
		m_Note = note;
	}

	bool CanUndo() const override	{return true;}

protected:
	MainWindow* m_Parent;
	Note m_Note;
};

class MainWindow::AddCommand : public NoteCommand
{
public:
	AddCommand(MainWindow& parent, const Note& note)
		: NoteCommand(parent, note)
	{
	}

	bool Do() override
	{
		NoteSet& noteSet = m_Parent->m_Notes->noteSet;
		noteSet.insert(noteSet.end(), m_Note);
		m_Parent->UpdateForDeselection();
		m_Parent->Sort();
		m_Parent->m_NoteList->SetItemState(noteSet.find(m_Note.GetId()) - noteSet.begin(),
			wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		m_Parent->MarkDirty();
		return true;
	}

	bool Undo() override
	{
		NoteSet& noteSet = m_Parent->m_Notes->noteSet;
		NoteSet::iterator foundIter = noteSet.find(m_Note.GetId());
		m_Parent->m_NoteList->DeleteItem(foundIter - noteSet.begin());
		noteSet.erase(foundIter);
		m_Parent->UpdateForDeselection();
		m_Parent->MarkDirty();
		return true;
	}
};

class MainWindow::RemoveCommand : public AddCommand
{
public:
	RemoveCommand(MainWindow& parent, const Note& note)
		: AddCommand(parent, note)
	{
	}

	bool Do() override
	{
		return AddCommand::Undo();
	}

	bool Undo() override
	{
		return AddCommand::Do();
	}
};

class MainWindow::RenameCommand : public wxCommand
{
public:
	RenameCommand(MainWindow& parent, const Note& note, const std::string& oldName,
		const std::string& newName)
		: m_Parent(&parent), m_NoteId(note.GetId()), m_OldName(oldName), m_NewName(newName)
	{
	}

	bool CanUndo() const override	{return true;}

	bool Do() override
	{
		NoteSet& noteSet = m_Parent->m_Notes->noteSet;
		Note& note = *noteSet.find(m_NoteId);
		note.SetTitle(m_NewName);
		m_Parent->UpdateCommands(note);
		m_Parent->Sort();
		m_Parent->m_NoteList->SetItemState(noteSet.find(m_NoteId) - noteSet.begin(),
			wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		m_Parent->MarkDirty();
		return true;
	}

	bool Undo() override
	{
		NoteSet& noteSet = m_Parent->m_Notes->noteSet;
		Note& note = *noteSet.find(m_NoteId);
		note.SetTitle(m_OldName);
		m_Parent->UpdateCommands(note);
		m_Parent->Sort();
		m_Parent->m_NoteList->SetItemState(noteSet.find(m_NoteId) - noteSet.begin(),
			wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
		m_Parent->MarkDirty();
		return true;
	}

private:
	MainWindow* m_Parent;
	uint64_t m_NoteId;
	std::string m_OldName;
	std::string m_NewName;
};

MainWindow::MainWindow(const wxString& title, const wxSize& size, const std::string& initialFile)
	: wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size), m_Notes(new NoteContext),
	m_IgnoreSelectionChanges(false), m_SortNextUpdate(false)
{
	//Menu
	wxMenuBar* menuBar = new wxMenuBar;

	wxMenu* fileMenu = new wxMenu;
	fileMenu->Append(wxID_NEW, "", "");
	fileMenu->Append(wxID_OPEN, "", "");
	fileMenu->AppendSeparator();
	fileMenu->Append(wxID_SAVE, "", "");
	fileMenu->Append(wxID_SAVEAS, "Save &As...\tCTRL+SHIFT+S", "");
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
	editMenu->AppendSeparator();
	editMenu->Append(wxID_ADD, "Add &Note\tCTRL+SHIFT+A", "");
	m_RemoveNoteItem = editMenu->Append(wxID_REMOVE, "&Remove Note\tCTRL+SHIFT+R", "");

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

	m_UndoStack = new wxCommandProcessor;

	const char* cFileFilter = "Secure note files (*.secnote)|*.secnote";
	std::string homeDir;
#if _WIN32
	homeDir = std::getenv("HOMEDRIVE");
	homeDir += std::getenv("HOMEPATH");
#else
	homeDir = std::getenv("HOME");
#endif
	m_OpenDialog = new wxFileDialog(this, "Open Notes", wxString::FromUTF8(homeDir.c_str()),
		wxEmptyString, cFileFilter, wxFD_OPEN | wxFD_CHANGE_DIR | wxFD_FILE_MUST_EXIST);
	m_SaveDialog = new wxFileDialog(this, "Save Notes", wxString::FromUTF8(homeDir.c_str()),
		wxEmptyString, cFileFilter, wxFD_SAVE | wxFD_CHANGE_DIR | wxFD_OVERWRITE_PROMPT);

	UpdateMenuItems();
	UpdateUi();
	UpdateTitle();

	if (!initialFile.empty())
		Open(initialFile);
}

void MainWindow::OnExit(wxCommandEvent&)
{
	Close(true);
}

void MainWindow::OnNew(wxCommandEvent&)
{
	Clear();
}

void MainWindow::OnOpen(wxCommandEvent&)
{
	if (m_Notes->dirty)
	{
		int result = wxMessageBox("Save changes before closing?", "Save changes?",
			wxICON_QUESTION | wxYES_NO | wxCANCEL, this);
		if (result == wxCANCEL)
			return;
		else if (result == wxYES)
		{
			if (!Save())
				return;
		}
	}

	if (m_OpenDialog->ShowModal() != wxID_OK)
		return;

	Open(m_OpenDialog->GetPath().ToUTF8().data());
}

void MainWindow::OnSave(wxCommandEvent&)
{
	Save();
}

void MainWindow::OnSaveAs(wxCommandEvent&)
{
	SaveAs();
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
	wxAboutDialogInfo info;
	info.SetName("Note Vault");
	info.SetVersion(VERSION);
	info.SetDescription("Store your notes securely without relying on an online service.");
	info.SetCopyright(wxT("Copyright \u00A9 2015 Aaron Barany"));
	wxAboutBox(info, this);
}

void MainWindow::OnClose(wxCloseEvent& event)
{
	if (event.CanVeto() && m_Notes->dirty)
	{
		int result = wxMessageBox("Save changes before closing?", "Save changes?",
			wxICON_QUESTION | wxYES_NO | wxCANCEL, this);
		if (result == wxCANCEL)
		{
			event.Veto();
			return;
		}
		else if (result == wxYES)
		{
			if (!Save())
			{
				event.Veto();
				return;
			}
		}
	}

	event.Skip();
}

void MainWindow::OnNoteTextChanged(wxCommandEvent&)
{
	if (m_IgnoreSelectionChanges || m_Notes->selectedNote == NoteSet::iterator())
		return;

	m_Notes->selectedNote->SetMessage(m_NoteText->GetValue().ToUTF8().data());
	UpdateCommands(*m_Notes->selectedNote);
	MarkDirty();
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
	if (m_Notes->selectedNote == NoteSet::iterator())
		return;

	m_UndoStack->Store(new RemoveCommand(*this, *m_Notes->selectedNote));
	long index = m_Notes->selectedNote - m_Notes->noteSet.begin();
	m_Notes->noteSet.erase(m_Notes->selectedNote);
	m_Notes->selectedNote = NoteSet::iterator();
	m_NoteList->DeleteItem(index);
	UpdateForDeselection();
	MarkDirty();
}

void MainWindow::OnTitleEdit(wxListEvent& event)
{
	bool newNote = (size_t)m_NoteList->GetItemCount() > m_Notes->noteSet.size();
	if (event.IsEditCancelled())
	{
		if (newNote)
		{
			m_NoteList->DeleteItem(event.GetIndex());
			UpdateForDeselection();
		}
		return;
	}

	Note* note;
	if (newNote)
	{
		m_Notes->selectedNote = m_Notes->noteSet.insert(m_Notes->noteSet.end());
		note = &*m_Notes->selectedNote;
		UpdateForSelection(m_Notes->selectedNote - m_Notes->noteSet.begin());
	}
	else
		note = &m_Notes->noteSet[event.GetIndex()];

	std::string oldName = note->GetTitle();
	std::string newName = event.GetLabel().ToUTF8().data();
	note->SetTitle(newName);

	if (newNote)
		m_UndoStack->Store(new AddCommand(*this, *note));
	else
	{
		m_UndoStack->Store(new RenameCommand(*this, *note, oldName, newName));
		UpdateCommands(*note);
	}
	m_SortNextUpdate = true;

	MarkDirty();
}

void MainWindow::OnTitleKeyDown(wxListEvent& event)
{
	if (event.GetKeyCode() == WXK_F2)
		m_NoteList->EditLabel(event.GetIndex());
	else
		event.Skip();
}

void MainWindow::OnSelectNote(wxListEvent& event)
{
	UpdateForSelection(event.GetIndex());
}

void MainWindow::OnDeselectNote(wxListEvent&)
{
	UpdateForDeselection();
}

wxCommandProcessor* MainWindow::GetUndoStack()
{
	wxRichTextCtrl* textControl = dynamic_cast<wxRichTextCtrl*>(FindFocus());
	if (textControl)
		return textControl->GetCommandProcessor();
	else if (GetTextEntry())
		return nullptr;
	else
		return m_UndoStack;
}

wxTextEntry* MainWindow::GetTextEntry()
{
	return dynamic_cast<wxTextEntry*>(FindFocus());
}

wxRichTextCtrl* MainWindow::GetRichTextCtrl()
{
	return dynamic_cast<wxRichTextCtrl*>(FindFocus());
}

void MainWindow::UpdateWindowUI(long flags)
{
	if (m_SortNextUpdate)
	{
		Sort();
		m_SortNextUpdate = false;
	}
	UpdateMenuItems();
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

	UpdateForDeselection();
}

void MainWindow::UpdateForSelection(long item)
{
	if (m_IgnoreSelectionChanges || (size_t)item >= m_Notes->noteSet.size())
		return;

	m_IgnoreSelectionChanges = true;
	m_Notes->selectedNote = m_Notes->noteSet.begin() + item;
	m_RemoveButton->Enable();
	m_RemoveNoteItem->Enable();
	m_NoteText->Enable();
	m_NoteText->SetValue(wxString::FromUTF8(m_Notes->selectedNote->GetMessage().c_str()));
	m_NoteText->GetCommandProcessor()->ClearCommands();
	m_IgnoreSelectionChanges = false;
}

void MainWindow::UpdateForDeselection()
{
	if (m_IgnoreSelectionChanges)
		return;

	m_IgnoreSelectionChanges = true;
	m_Notes->selectedNote = NoteSet::iterator();
	m_RemoveButton->Disable();
	m_RemoveNoteItem->Enable(false);
	m_NoteText->Disable();
	m_NoteText->Clear();
	m_NoteText->GetCommandProcessor()->ClearCommands();
	m_IgnoreSelectionChanges = false;
}

void MainWindow::UpdateCommands(const Note& note)
{
	for (wxObject* command : m_UndoStack->GetCommands())
	{
		if (NoteCommand* noteCommand = dynamic_cast<NoteCommand*>(command))
			noteCommand->UpdateNote(note);
	}
}

void MainWindow::UpdateTitle()
{
	std::string title = "Note Vault";
	if (!m_Notes->fileName.empty())
	{
		title += " - ";
		title += m_Notes->fileName;
	}

	if (m_Notes->dirty)
		title += " *";

	SetTitle(wxString::FromUTF8(title.c_str()));
}

void MainWindow::Sort()
{
	m_IgnoreSelectionChanges = true;

	const uint64_t cNotSelected = (uint64_t)-1;
	uint64_t selectedNoteId = cNotSelected;
	if (m_Notes->selectedNote != NoteSet::iterator())
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

void MainWindow::Clear()
{
	m_Notes.reset(new NoteContext);
	m_UndoStack->ClearCommands();
	UpdateUi();
	UpdateTitle();
}

bool MainWindow::Open(const std::string& savePath)
{
	FileIStream stream;
	if (!stream.Open(savePath))
	{
		std::string message = "Couldn't open file '" + savePath + "'";
		wxMessageBox(wxString::FromUTF8(message.c_str()), "Couldn't load",
			wxICON_ERROR | wxOK_DEFAULT, this);
		return false;
	}

	std::string password = wxGetPasswordFromUser("Password:", wxGetPasswordFromUserPromptStr,
		wxEmptyString, this).ToUTF8().data();

	if (password.empty())
		return false;

	std::string fileName = savePath;
#if _WIN32
	size_t lastSeparator = fileName.find_last_of("/\\");
#else
	size_t lastSeparator = fileName.find_last_of('/');
#endif
	if (lastSeparator != std::string::npos)
		fileName = fileName.substr(lastSeparator + 1);

	size_t extensionPos = fileName.find_last_of('.');
	if (extensionPos != std::string::npos)
		fileName = fileName.substr(0, extensionPos);

	std::vector<uint8_t> salt, key;
	NoteSet noteSet;
	NoteFile::Result result = NoteFile::LoadNotes(noteSet, stream, password, salt, key);
	switch (result)
	{
		case NoteFile::Result::Success:
			Clear();
			m_Notes->noteSet = noteSet;
			m_Notes->savePath = savePath;
			m_Notes->fileName = fileName;
			m_Notes->salt = salt;
			m_Notes->key = key;

			UpdateTitle();
			UpdateUi();
			return true;
		case NoteFile::Result::InvalidFile:
			wxMessageBox("Invalid file format", "Invalid file", wxICON_EXCLAMATION | wxOK_DEFAULT,
				this);
			break;
		case NoteFile::Result::InvalidVersion:
			wxMessageBox("File version is too new", "Invalid file",
				wxICON_EXCLAMATION | wxOK_DEFAULT, this);
			break;
		case NoteFile::Result::IoError:
			wxMessageBox("Error loading file", "Couldn't load", wxICON_ERROR | wxOK_DEFAULT,
				this);
			break;
		case NoteFile::Result::EncryptionError:
			wxMessageBox("Incorrect password", "Incorrect password",
				wxICON_EXCLAMATION | wxOK_DEFAULT, this);
			break;
	}

	return false;
}

bool MainWindow::Save()
{
	if (m_Notes->savePath.empty())
		return SaveAs();

	m_Notes->dirty = false;
	UpdateTitle();
	FileOStream stream;
	if (!stream.Open(m_Notes->savePath) || NoteFile::SaveNotes(m_Notes->noteSet, stream,
		m_Notes->salt, m_Notes->key) != NoteFile::Result::Success)
	{
		wxMessageBox("Error saving file.", "Couldn't save", wxICON_ERROR | wxOK_DEFAULT, this);
		return false;
	}
	return true;
}

bool MainWindow::SaveAs()
{
	if (m_SaveDialog->ShowModal() != wxID_OK)
		return false;

	std::string password = wxGetPasswordFromUser("Password:", wxGetPasswordFromUserPromptStr,
		wxEmptyString, this).ToUTF8().data();

	if (password.empty())
		return false;

	m_Notes->savePath = m_SaveDialog->GetPath().ToUTF8().data();
	size_t extensionPos = m_Notes->savePath.find_last_of('.');
	if (extensionPos == std::string::npos)
		m_Notes->savePath += ".secnote";

	m_Notes->fileName = m_SaveDialog->GetFilename().ToUTF8().data();
	extensionPos = m_Notes->fileName.find_last_of('.');
	if (extensionPos != std::string::npos)
		m_Notes->fileName = m_Notes->fileName.substr(0, extensionPos);

	m_Notes->salt = Crypto::Random(Crypto::cSaltLenBytes);
	m_Notes->key = Crypto::GenerateKey(password, m_Notes->salt, Crypto::cDefaultKeyIterations);
	return Save();
}

void MainWindow::MarkDirty()
{
	m_Notes->dirty = true;
	UpdateTitle();
}

} // namespace NoteVault
