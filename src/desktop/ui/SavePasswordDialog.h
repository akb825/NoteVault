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

#include <QtWidgets/QDialog>
#include <memory>
#include <string>

namespace Ui
{
	class SavePasswordDialog;
}

namespace NoteVault
{

class SavePasswordDialog : public QDialog
{
	Q_OBJECT
public:
	explicit SavePasswordDialog(QWidget* parent);
	~SavePasswordDialog();

	int exec() override;
	std::string getPassword() const;

private Q_SLOTS:
	void onOk();

private:
	SavePasswordDialog(const SavePasswordDialog&) = delete;
	SavePasswordDialog& operator=(const SavePasswordDialog&) = delete;

	std::unique_ptr<Ui::SavePasswordDialog> m_impl;
};

} // namespace NoteVault
