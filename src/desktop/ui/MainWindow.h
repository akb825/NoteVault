#pragma once
/*
 * Copyright 2015 Aaron Barany
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QtWidgets/QMainWindow>
#include <memory>

namespace Ui
{
	class MainWindow;
}

class QListWidgetItem;
class QUndoStack;

namespace NoteVault
{

class Note;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:
	MainWindow();
	~MainWindow();

	bool open(const std::string& fileName);

private Q_SLOTS:
	void onNew();
	void onOpen();
	void onSave();
	void onSaveAs();
	void onUndo();
	void onRedo();
	void onCut();
	void onCopy();
	void onPaste();
	void onDelete();
	void onSelectAll();
	void onAddNote();
	void onRemoveNote();
	void onPasswordGenerator();
	void onAbout();

	void onNoteRenamed(QListWidgetItem* item);
	void onNoteSelectionChanged();

	void onNoteTextChanged();

	void updateMenuItems();

private:
	MainWindow(const MainWindow&) = delete;
	MainWindow& operator=(const MainWindow&) = delete;

	void closeEvent(QCloseEvent* event) override;

	bool canClose();
	void clear();
	void updateUi();
	void updateTitle();
	void markDirty();
	void sortNotes();
	void updateForSelection(ptrdiff_t item);
	void updateForDeselection();
	void updateCommands(const Note& note);

	QObject* getCurrentUndoStack();
	QObject* getCurrentTextEdit();

	bool hasSelection() const;
	bool hasText() const;
	bool canUndo() const;
	bool canRedo() const;
	QString getUndoText() const;
	QString getRedoText() const;

	bool save();
	bool saveAs();

	struct ChildItems;
	struct NoteContext;
	class NoteCommand;
	class AddCommand;
	class RemoveCommand;
	class RenameCommand;

	std::unique_ptr<Ui::MainWindow> m_impl;
	std::unique_ptr<ChildItems> m_children;
	std::unique_ptr<NoteContext> m_notes;

	bool m_ignoreSelectionChanges;
};

} // namespace NoteVault
