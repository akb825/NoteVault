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

#include "GeneratePasswordDialog.h"
#include "io/Crypto.h"

#include "ui_GeneratePasswordDialog.h"
#include "GeneratePasswordDialog.moc"

namespace NoteVault
{

GeneratePasswordDialog::GeneratePasswordDialog(QWidget *parent)
	: QMainWindow(parent), m_impl(new Ui::GeneratePasswordDialog)
{
	m_impl->setupUi(this);

	QObject::connect(m_impl->generateButton, SIGNAL(clicked()), this, SLOT(generatePassword()));
}

GeneratePasswordDialog::~GeneratePasswordDialog()
{
	delete m_impl;
}

void GeneratePasswordDialog::generatePassword()
{
	int numChars = m_impl->numberCharacters->text().toInt();
	const char cFirstChar = '!';
	const char cLastChar = '~';
	const unsigned int cNumCharCodes = cLastChar - cFirstChar;

	std::vector<uint8_t> randomNumbers = Crypto::random(numChars);
	std::string randomString(numChars, ' ');
	for (int i = 0; i < numChars; ++i)
		randomString[i] = static_cast<char>((randomNumbers[i] % cNumCharCodes) + cFirstChar);

	m_impl->passwordField->setText(QString::fromStdString(randomString));
}

} // namespace NoteVault
