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

#include "AboutDialog.h"
#include "Version.h"

#include "ui_AboutDialog.h"

namespace NoteVault
{

AboutDialog::AboutDialog(QWidget* parent)
	: QDialog(parent), m_impl(new Ui::AboutDialog)
{
	m_impl->setupUi(this);
	m_impl->appName->setText(m_impl->appName->text() + " " VERSION_STRING);
}

AboutDialog::~AboutDialog()
{
}

} // namespace NoteVault
