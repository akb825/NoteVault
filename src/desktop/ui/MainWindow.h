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

#include <wx/frame.h>
#include <memory>

class wxListCtrl;
class wxListEvent;
class wxRichTextCtrl;
class wxCommandProcessor;
class wxTextEntry;
class wxMenuItem;
class wxButton;

namespace NoteVault
{

class UndoStack;

class MainWindow : public wxFrame
{
public:
	MainWindow(const wxString& title, const wxSize& size);

private:
	struct NoteContext;

	void OnNew(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnExit(wxCommandEvent& event);

	void OnUndo(wxCommandEvent& event);
	void OnRedo(wxCommandEvent& event);
	void OnCut(wxCommandEvent& event);
	void OnCopy(wxCommandEvent& event);
	void OnPaste(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);
	void OnSelectAll(wxCommandEvent& event);

	void OnAbout(wxCommandEvent& event);

	void OnIdle(wxIdleEvent& event);

	void OnNoteTextChanged(wxCommandEvent& event);
	void OnAddNote(wxCommandEvent& event);
	void OnRemoveNote(wxCommandEvent& event);

	void OnTitleEdit(wxListEvent& event);
	void OnSelectNote(wxListEvent& event);
	void OnDeselectNote(wxListEvent& event);

	static wxCommandProcessor* GetUndoStack();
	static wxTextEntry* GetTextEntry();
	static wxRichTextCtrl* GetRichTextCtrl();
	void UpdateMenuItems();
	void UpdateUi();
	void UpdateForSelection(long item);
	void UpdateForDeselection();
	void Sort();

	wxMenuItem* m_UndoItem;
	wxMenuItem* m_RedoItem;
	wxMenuItem* m_CutItem;
	wxMenuItem* m_CopyItem;
	wxMenuItem* m_PasteItem;
	wxMenuItem* m_DeleteItem;
	wxMenuItem* m_SelectAllItem;

	wxListCtrl* m_NoteList;
	wxRichTextCtrl* m_NoteText;
	wxButton* m_RemoveButton;

	std::unique_ptr<NoteContext> m_Notes;
	bool m_IgnoreSelectionChanges;
	bool m_SortNextUpdate;

	wxDECLARE_EVENT_TABLE();
};

} // namespace NoteVault
