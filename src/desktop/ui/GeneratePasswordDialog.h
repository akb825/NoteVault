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

#include <QMainWindow>

namespace Ui {
class GeneratePasswordDialog;
}

namespace NoteVault
{

class GeneratePasswordDialog : public QMainWindow
{
	Q_OBJECT

public:
	explicit GeneratePasswordDialog(QWidget *parent = 0);
	~GeneratePasswordDialog();

public Q_SLOTS:
	void generatePassword();

private:
	Ui::GeneratePasswordDialog *m_impl;
};

} // namespace NoteVault
