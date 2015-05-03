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

#include "AboutDialog.h"
#include "ConfirmCloseDialog.h"
#include "OpenPasswordDialog.h"
#include "SavePasswordDialog.h"
#include "io/Crypto.h"
#include "io/FileIStream.h"
#include "io/FileOStream.h"
#include "io/NoteFile.h"
#include "notes/NoteSet.h"
#include <QtCore/QDir>
#include <QtCore/QTimer>
#include <QtGui/QKeyEvent>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QUndoStack>
#include <assert.h>

#include "ui_MainWindow.h"
#include "MainWindow.moc"

namespace NoteVault
{

struct MainWindow::ChildItems
{
	ChildItems(QWidget* parent)
		: aboutDialog(parent), confirmCloseDialog(parent), openPasswordDialog(parent),
		savePasswordDialog(parent), fileDialog(parent)
	{
	}

	AboutDialog aboutDialog;
	ConfirmCloseDialog confirmCloseDialog;
	OpenPasswordDialog openPasswordDialog;
	SavePasswordDialog savePasswordDialog;
	QFileDialog fileDialog;
	QUndoStack undoStack;
	QTimer timer;
};

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


class MainWindow::NoteCommand : public QUndoCommand
{
public:
	NoteCommand(const QString& text, MainWindow& parent, const Note& note)
		: QUndoCommand(text), m_parent(&parent), m_note(note)
	{
	}

	uint64_t getNoteId() const	{return m_note.getId();}
	void updateNote(const Note& note) const
	{
		if (note.getId() != m_note.getId())
			return;
		m_note = note;
	}

protected:
	MainWindow* m_parent;
	mutable Note m_note;
};

class MainWindow::AddCommand : public NoteCommand
{
public:
	AddCommand(MainWindow& parent, const Note& note)
		: NoteCommand("add note", parent, note)
	{
	}

	void undo() override
	{
		NoteSet& noteSet = m_parent->m_notes->noteSet;
		NoteSet::iterator foundIter = noteSet.find(m_note.getId());
		ptrdiff_t index = m_parent->m_notes->selectedNote - m_parent->m_notes->noteSet.begin();
		delete m_parent->m_impl->noteList->takeItem(static_cast<int>(index));
		noteSet.erase(foundIter);

		QList<QListWidgetItem*> selectedItems = m_parent->m_impl->noteList->selectedItems();
		if (selectedItems.empty())
			m_parent->updateForDeselection();
		else
		{
			index = m_parent->m_impl->noteList->row(selectedItems[0]);
			m_parent->updateForSelection(index);
		}
		m_parent->markDirty();
	}

	void redo() override
	{
		NoteSet& noteSet = m_parent->m_notes->noteSet;
		noteSet.insert(noteSet.end(), m_note);
		m_parent->sortNotes();
		m_parent->updateUi();

		ptrdiff_t index = noteSet.find(m_note.getId()) - noteSet.begin();
		m_parent->m_impl->noteList->setCurrentRow(static_cast<int>(index));
		m_parent->updateForSelection(index);
		m_parent->markDirty();
	}

protected:
	AddCommand(const QString& text, MainWindow& parent, const Note& note)
		: NoteCommand(text, parent, note)
	{
	}
};

class MainWindow::RemoveCommand : public AddCommand
{
public:
	RemoveCommand(MainWindow& parent, const Note& note)
		: AddCommand("remove note", parent, note)
	{
	}

	void undo() override
	{
		AddCommand::redo();
	}

	void redo() override
	{
		AddCommand::undo();
	}
};

class MainWindow::RenameCommand : public QUndoCommand
{
public:
	RenameCommand(MainWindow& parent, const Note& note, const std::string& oldName,
		const std::string& newName)
		: QUndoCommand("rename note"), m_parent(&parent), m_noteId(note.getId()),
		  m_oldName(oldName), m_newName(newName)
	{
	}

	void undo() override
	{
		NoteSet& noteSet = m_parent->m_notes->noteSet;
		Note& note = *noteSet.find(m_noteId);
		note.setTitle(m_oldName);
		m_parent->updateCommands(note);
		m_parent->sortNotes();

		ptrdiff_t selectedNote = noteSet.find(m_noteId) - noteSet.begin();
		m_parent->m_impl->noteList->setCurrentRow(static_cast<int>(selectedNote));
		m_parent->markDirty();
	}

	void redo() override
	{
		NoteSet& noteSet = m_parent->m_notes->noteSet;
		Note& note = *noteSet.find(m_noteId);
		note.setTitle(m_newName);
		m_parent->updateCommands(note);
		m_parent->sortNotes();

		ptrdiff_t selectedNote = noteSet.find(m_noteId) - noteSet.begin();
		m_parent->m_impl->noteList->setCurrentRow(static_cast<int>(selectedNote));
		m_parent->markDirty();
	}

private:
	MainWindow* m_parent;
	uint64_t m_noteId;
	std::string m_oldName;
	std::string m_newName;
};

MainWindow::MainWindow()
	: m_impl(new Ui::MainWindow), m_children(new ChildItems(this)), m_notes(new NoteContext),
	m_ignoreSelectionChanges(false)
{
	m_impl->setupUi(this);
	m_impl->splitter->setStretchFactor(0, 0);
	m_impl->splitter->setStretchFactor(1, 1);

	QStringList filter;
	filter.append("Secure note files (*.secnote)");

	m_children->fileDialog.setNameFilters(filter);
	m_children->fileDialog.setDefaultSuffix(".secnote");
	m_children->fileDialog.setDirectory(QDir::home());

	updateForDeselection();
	updateMenuItems();

	// Menu items
	QObject::connect(m_impl->actionNew, SIGNAL(triggered()), this, SLOT(onNew()));
	QObject::connect(m_impl->actionOpen, SIGNAL(triggered()), this, SLOT(onOpen()));
	QObject::connect(m_impl->actionSave, SIGNAL(triggered()), this, SLOT(onSave()));
	QObject::connect(m_impl->actionSaveAs, SIGNAL(triggered()), this, SLOT(onSaveAs()));
	QObject::connect(m_impl->actionUndo, SIGNAL(triggered()), this, SLOT(onUndo()));
	QObject::connect(m_impl->actionRedo, SIGNAL(triggered()), this, SLOT(onRedo()));
	QObject::connect(m_impl->actionCut, SIGNAL(triggered()), this, SLOT(onCut()));
	QObject::connect(m_impl->actionCopy, SIGNAL(triggered()), this, SLOT(onCopy()));
	QObject::connect(m_impl->actionPaste, SIGNAL(triggered()), this, SLOT(onPaste()));
	QObject::connect(m_impl->actionDelete, SIGNAL(triggered()), this, SLOT(onDelete()));
	QObject::connect(m_impl->actionSelectAll, SIGNAL(triggered()), this, SLOT(onSelectAll()));
	QObject::connect(m_impl->actionAddNote, SIGNAL(triggered()), this, SLOT(onAddNote()));
	QObject::connect(m_impl->actionRemoveNote, SIGNAL(triggered()), this, SLOT(onRemoveNote()));
	QObject::connect(m_impl->actionAbout, SIGNAL(triggered()), this, SLOT(onAbout()));

	// Buttons
	QObject::connect(m_impl->addButton, SIGNAL(clicked()), this, SLOT(onAddNote()));
	QObject::connect(m_impl->removeButton, SIGNAL(clicked()), this, SLOT(onRemoveNote()));

	// Note list
	QObject::connect(m_impl->noteList, SIGNAL(itemChanged(QListWidgetItem*)), this,
		SLOT(onNoteRenamed(QListWidgetItem*)));
	QObject::connect(m_impl->noteList, SIGNAL(itemSelectionChanged()), this,
		SLOT(onNoteSelectionChanged()));

	// Note text
	QObject::connect(m_impl->noteText, SIGNAL(textChanged()), this, SLOT(onNoteTextChanged()));

	// Timer
	QObject::connect(&m_children->timer, SIGNAL(timeout()), this, SLOT(updateMenuItems()));
	m_children->timer.start(0);
}

MainWindow::~MainWindow()
{
}

bool MainWindow::open(const std::string& filePath)
{
	FileIStream stream;
	if (!stream.open(filePath))
	{
		std::string message = "Couldn't open file '" + filePath + "'";
		QMessageBox::warning(this, "Couldn't Open", message.c_str());
		return false;
	}

	if (!m_children->openPasswordDialog.exec())
		return false;

	std::string password = m_children->openPasswordDialog.getPassword();
	assert(!password.empty());

	QFileInfo fileInfo(filePath.c_str());
	std::string fileName = fileInfo.fileName().toStdString();
	size_t extensionPos = fileName.find_last_of('.');
	if (extensionPos != std::string::npos)
		fileName = fileName.substr(0, extensionPos);

	std::vector<uint8_t> salt, key;
	NoteSet noteSet;
	NoteFile::Result result = NoteFile::loadNotes(noteSet, stream, password, salt, key);
	switch (result)
	{
		case NoteFile::Result::Success:
			clear();
			m_notes->noteSet = noteSet;
			m_notes->savePath = filePath;
			m_notes->fileName = fileName;
			m_notes->salt = salt;
			m_notes->key = key;

			updateTitle();
			updateUi();
			return true;
		case NoteFile::Result::InvalidFile:
			QMessageBox::warning(this, "Couldn't Open", "Invalid file format");
			break;
		case NoteFile::Result::InvalidVersion:
			QMessageBox::warning(this, "Couldn't Open",
				"File version is too new. Please update Note Vault.");
			break;
		case NoteFile::Result::IoError:
			QMessageBox::warning(this, "Couldn't Open", "Error reading file");
			break;
		case NoteFile::Result::EncryptionError:
			QMessageBox::warning(this, "Couldn't Open", "Incorrect password");
			break;
		default:
			assert(false);
			break;
	}

	return false;
}

void MainWindow::onNew()
{
	if (!canClose())
		return;

	clear();
}

void MainWindow::onOpen()
{
	if (!canClose())
		return;

	m_children->fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	m_children->fileDialog.setFileMode(QFileDialog::ExistingFile);
	m_children->fileDialog.setWindowTitle("Open Notes");

	if (!m_children->fileDialog.exec())
		return;

	QStringList selectedFile = m_children->fileDialog.selectedFiles();
	open(selectedFile[0].toStdString());
}

void MainWindow::onSave()
{
	save();
}

void MainWindow::onSaveAs()
{
	saveAs();
}

void MainWindow::onUndo()
{
	QMetaObject::invokeMethod(getCurrentUndoStack(), "undo");
}

void MainWindow::onRedo()
{
	QMetaObject::invokeMethod(getCurrentUndoStack(), "redo");
}

void MainWindow::onCut()
{
	QObject* textEdit = getCurrentTextEdit();
	if (textEdit)
		QMetaObject::invokeMethod(textEdit, "cut");
}

void MainWindow::onCopy()
{
	QObject* textEdit = getCurrentTextEdit();
	if (textEdit)
		QMetaObject::invokeMethod(textEdit, "copy");
}

void MainWindow::onPaste()
{
	QObject* textEdit = getCurrentTextEdit();
	if (textEdit)
		QMetaObject::invokeMethod(textEdit, "paste");
}

void MainWindow::onDelete()
{
	if (!hasSelection())
		return;

	QObject* textEdit = getCurrentTextEdit();
	if (textEdit)
	{
		QKeyEvent keyEvent(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier);
		QApplication::sendEvent(textEdit, &keyEvent);
	}
}

void MainWindow::onSelectAll()
{
	QObject* textEdit = getCurrentTextEdit();
	if (textEdit)
		QMetaObject::invokeMethod(textEdit, "selectAll");
}

void MainWindow::onAddNote()
{
	NoteSet::iterator newNoteIter = m_notes->noteSet.insert(m_notes->noteSet.end());
	Note newNote = *newNoteIter;
	m_notes->noteSet.erase(newNoteIter);

	newNote.setTitle("New note");
	m_children->undoStack.push(new AddCommand(*this, newNote));

	ptrdiff_t newItemIndex = m_notes->selectedNote - m_notes->noteSet.begin();
	QListWidgetItem* newItem = m_impl->noteList->item(static_cast<int>(newItemIndex));
	m_impl->noteList->editItem(newItem);
}

void MainWindow::onRemoveNote()
{
	if (m_notes->selectedNote == NoteSet::iterator())
		return;

	m_children->undoStack.push(new RemoveCommand(*this, *m_notes->selectedNote));
}

void MainWindow::onAbout()
{
	m_children->aboutDialog.show();
	m_children->aboutDialog.raise();
	m_children->aboutDialog.activateWindow();
}

void MainWindow::onNoteRenamed(QListWidgetItem* item)
{
	Note& note = m_notes->noteSet[m_impl->noteList->row(item)];

	std::string oldName = note.getTitle();
	std::string newName = item->text().toStdString();
	m_children->undoStack.push(new RenameCommand(*this, note, oldName, newName));
}

void MainWindow::onNoteSelectionChanged()
{
	QList<QListWidgetItem*> selectedItems = m_impl->noteList->selectedItems();
	if (selectedItems.empty())
		updateForDeselection();
	else
		updateForSelection(m_impl->noteList->row(selectedItems[0]));
}

void MainWindow::onNoteTextChanged()
{
	if (m_ignoreSelectionChanges || m_notes->selectedNote == NoteSet::iterator())
		return;

	m_notes->selectedNote->setMessage(m_impl->noteText->toPlainText().toStdString());
	updateCommands(*m_notes->selectedNote);
	markDirty();
}

void MainWindow::updateMenuItems()
{
	QObject* textEdit = getCurrentTextEdit();
	bool hasSelect = hasSelection();

	m_impl->actionUndo->setEnabled(canUndo());
	m_impl->actionUndo->setText(getUndoText());
	m_impl->actionRedo->setEnabled(canRedo());
	m_impl->actionRedo->setText(getRedoText());
	m_impl->actionCut->setEnabled(hasSelect);
	m_impl->actionCopy->setEnabled(hasSelect);
	m_impl->actionPaste->setEnabled(textEdit != nullptr);
	m_impl->actionDelete->setEnabled(hasSelect);
	m_impl->actionSelectAll->setEnabled(hasText());
	m_impl->actionRemoveNote->setEnabled(m_notes->selectedNote != NoteSet::iterator());
}

void MainWindow::closeEvent(QCloseEvent* event)
{
	if (canClose())
		event->accept();
	else
		event->ignore();
}

bool MainWindow::canClose()
{
	if (!m_notes->dirty)
		return true;

	switch (m_children->confirmCloseDialog.show())
	{
		case ConfirmCloseDialog::Result::Save:
			return save();
		case ConfirmCloseDialog::Result::Cancel:
			return false;
		case ConfirmCloseDialog::Result::DontSave:
			return true;
	}

	assert(false);
	return false;
}

void MainWindow::clear()
{
	m_notes.reset(new NoteContext);
	m_children->undoStack.clear();
	updateUi();
	updateTitle();
}

void MainWindow::updateUi()
{
	m_impl->noteList->clear();
	for (const Note& note : m_notes->noteSet)
	{
		QListWidgetItem* newItem = new QListWidgetItem(note.getTitle().c_str());
		newItem->setFlags(newItem->flags() | Qt::ItemIsEditable);
		m_impl->noteList->insertItem(m_impl->noteList->count(), newItem);
	}

	updateForDeselection();
}

void MainWindow::updateTitle()
{
	std::string title = "Note Vault";
	if (!m_notes->fileName.empty())
	{
		title += " - ";
		title += m_notes->fileName;
	}

	title += "[*]";

	setWindowTitle(title.c_str());
	setWindowModified(m_notes->dirty);
}

void MainWindow::markDirty()
{
	m_notes->dirty = true;
	updateTitle();
}

void MainWindow::sortNotes()
{
	m_ignoreSelectionChanges = true;

	const uint64_t cNotSelected = (uint64_t)-1;
	uint64_t selectedNoteId = cNotSelected;
	if (m_notes->selectedNote != NoteSet::iterator())
		selectedNoteId = m_notes->selectedNote->getId();
	m_notes->noteSet.sort([] (const Note& left, const Note& right) -> bool
		{
			return strcasecmp(left.getTitle().c_str(), right.getTitle().c_str()) < 0;
		});

	if (selectedNoteId != cNotSelected)
		m_notes->selectedNote = m_notes->noteSet.find(selectedNoteId);

	updateUi();

	if (selectedNoteId != cNotSelected)
	{
		m_impl->noteList->setCurrentRow(
			static_cast<int>(m_notes->selectedNote - m_notes->noteSet.begin()));
	}

	m_ignoreSelectionChanges = false;
}

void MainWindow::updateForSelection(ptrdiff_t item)
{
	if (m_ignoreSelectionChanges || (size_t)item >= m_notes->noteSet.size())
		return;

	m_ignoreSelectionChanges = true;
	m_notes->selectedNote = m_notes->noteSet.begin() + item;
	m_impl->removeButton->setEnabled(true);
	m_impl->actionRemoveNote->setEnabled(true);
	m_impl->noteText->setEnabled(true);
	m_impl->noteText->setPlainText(m_notes->selectedNote->getMessage().c_str());
	m_impl->noteText->document()->clearUndoRedoStacks();
	m_ignoreSelectionChanges = false;
}

void MainWindow::updateForDeselection()
{
	if (m_ignoreSelectionChanges)
		return;

	m_ignoreSelectionChanges = true;
	m_notes->selectedNote = NoteSet::iterator();
	m_impl->removeButton->setEnabled(false);
	m_impl->actionRemoveNote->setEnabled(false);
	m_impl->noteText->setEnabled(false);
	m_impl->noteText->clear();
	m_ignoreSelectionChanges = false;
}

void MainWindow::updateCommands(const Note& note)
{
	for (int i = 0; i < m_children->undoStack.count(); ++i)
	{
		const NoteCommand* noteCommand =
				dynamic_cast<const NoteCommand*>(m_children->undoStack.command(i));
		if (noteCommand)
			noteCommand->updateNote(note);
	}
}

QObject* MainWindow::getCurrentUndoStack()
{
	QWidget* focus = focusWidget();
	if (QPlainTextEdit* plainText = dynamic_cast<QPlainTextEdit*>(focus))
		return plainText->document();
	else if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(focus))
		return lineEdit;
	else
		return &m_children->undoStack;
}

QObject* MainWindow::getCurrentTextEdit()
{
	QWidget* focus = focusWidget();
	if (QPlainTextEdit* plainText = dynamic_cast<QPlainTextEdit*>(focus))
		return plainText;
	else if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(focus))
		return lineEdit;
	else
		return nullptr;
}

bool MainWindow::hasSelection() const
{
	QWidget* focus = focusWidget();
	if (QPlainTextEdit* plainText = dynamic_cast<QPlainTextEdit*>(focus))
		return plainText->textCursor().hasSelection();
	else if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(focus))
		return lineEdit->hasSelectedText();
	else
		return false;
}

bool MainWindow::hasText() const
{
	QWidget* focus = focusWidget();
	if (QPlainTextEdit* plainText = dynamic_cast<QPlainTextEdit*>(focus))
		return !plainText->toPlainText().isEmpty();
	else if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(focus))
		return !lineEdit->text().isEmpty();
	else
		return false;
}

bool MainWindow::canUndo() const
{
	QWidget* focus = focusWidget();
	if (QPlainTextEdit* plainText = dynamic_cast<QPlainTextEdit*>(focus))
		return plainText->document()->isUndoAvailable();
	else if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(focus))
		return lineEdit->isUndoAvailable();
	else
		return m_children->undoStack.canUndo();
}

bool MainWindow::canRedo() const
{
	QWidget* focus = focusWidget();
	if (QPlainTextEdit* plainText = dynamic_cast<QPlainTextEdit*>(focus))
		return plainText->document()->isRedoAvailable();
	else if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(focus))
		return lineEdit->isRedoAvailable();
	else
		return m_children->undoStack.canRedo();
}

QString MainWindow::getUndoText() const
{
	QWidget* focus = focusWidget();
	if (QPlainTextEdit* plainText = dynamic_cast<QPlainTextEdit*>(focus))
	{
		if (plainText->document()->isUndoAvailable())
			return "&Undo text";
		else
			return "&Undo";
	}
	else if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(focus))
	{
		if (lineEdit->isUndoAvailable())
			return "&Undo text";
		else
			return "&Undo";
	}
	else
	{
		if (m_children->undoStack.canUndo())
			return "&Undo " + m_children->undoStack.undoText();
		else
			return "&Undo";
	}
}

QString MainWindow::getRedoText() const
{
	QWidget* focus = focusWidget();
	if (QPlainTextEdit* plainText = dynamic_cast<QPlainTextEdit*>(focus))
	{
		if (plainText->document()->isRedoAvailable())
			return "&Redo text";
		else
			return "&Redo";
	}
	else if (QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(focus))
	{
		if (lineEdit->isRedoAvailable())
			return "&Redo text";
		else
			return "&Redo";
	}
	else
	{
		if (m_children->undoStack.canRedo())
			return "&Redo " + m_children->undoStack.redoText();
		else
			return "&Redo";
	}
}

bool MainWindow::save()
{
	if (m_notes->savePath.empty())
		return saveAs();

	FileOStream stream;
	if (!stream.open(m_notes->savePath) || NoteFile::saveNotes(m_notes->noteSet, stream,
		m_notes->salt, m_notes->key) != NoteFile::Result::Success)
	{
		QMessageBox::warning(this, "Couldn't Save", "Error saving file");
		return false;
	}

	m_notes->dirty = false;
	updateTitle();
	return true;
}

bool MainWindow::saveAs()
{
	m_children->fileDialog.setAcceptMode(QFileDialog::AcceptSave);
	m_children->fileDialog.setFileMode(QFileDialog::AnyFile);
	m_children->fileDialog.setWindowTitle("Save Notes");

	if (!m_children->fileDialog.exec())
		return false;

	if (!m_children->savePasswordDialog.exec())
		return false;

	std::string password = m_children->savePasswordDialog.getPassword();
	assert(!password.empty());

	QStringList selectedFile = m_children->fileDialog.selectedFiles();
	QFileInfo fileInfo(selectedFile[0]);

	m_notes->savePath = fileInfo.absoluteFilePath().toStdString();
	m_notes->fileName = fileInfo.fileName().toStdString();
	size_t extensionPos = m_notes->fileName.find_last_of('.');
	if (extensionPos != std::string::npos)
		m_notes->fileName = m_notes->fileName.substr(0, extensionPos);

	m_notes->salt = Crypto::random(Crypto::cSaltLenBytes);
	m_notes->key = Crypto::generateKey(password, m_notes->salt, Crypto::cDefaultKeyIterations);
	return save();
}

} // namespace NoteVault
