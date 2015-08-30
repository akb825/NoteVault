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

#include "ConfirmCloseDialog.h"

#include "ui_ConfirmCloseDialog.h"
#include "ConfirmCloseDialog.moc"

namespace NoteVault
{

ConfirmCloseDialog::ConfirmCloseDialog(QWidget* parent)
	: QDialog(parent), m_impl(new Ui::ConfirmCloseDialog)
{
	m_impl->setupUi(this);

	QObject::connect(m_impl->saveButton, SIGNAL(clicked()), this, SLOT(onSave()));
	QObject::connect(m_impl->cancelButton, SIGNAL(clicked()), this, SLOT(onCancel()));
	QObject::connect(m_impl->dontSaveButton, SIGNAL(clicked()), this, SLOT(onDontSave()));
}

ConfirmCloseDialog::~ConfirmCloseDialog()
{
}

ConfirmCloseDialog::Result ConfirmCloseDialog::show()
{
	m_impl->saveButton->setFocus();
	return static_cast<Result>(exec());
}

void ConfirmCloseDialog::onSave()
{
	done(static_cast<int>(Result::Save));
}

void ConfirmCloseDialog::onCancel()
{
	done(static_cast<int>(Result::Cancel));
}

void ConfirmCloseDialog::onDontSave()
{
	done(static_cast<int>(Result::DontSave));
}

} // namespace NoteVault
