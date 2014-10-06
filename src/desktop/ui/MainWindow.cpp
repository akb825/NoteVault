/*
 * Copyright 2014 Aaron Barany
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

#include "ui/MainWindow.h"
#include <wx/app.h>

namespace NoteVault
{

MainWindow::MainWindow(const wxString& title, const wxSize& size)
	: wxFrame(nullptr, wxID_ANY, title, wxDefaultPosition, size)
{
}

class App : public wxApp
{
public:
	App()
		: m_Window(nullptr) {}

	virtual ~App()
	{
	}

	virtual bool OnInit()
	{
		m_Window = new MainWindow("Note Vault", wxSize(800, 600));
		m_Window->Show(true);
		return true;
	}

private:
	MainWindow* m_Window;
};

} // namespace NoteVault

wxIMPLEMENT_APP_NO_MAIN(NoteVault::App);

extern "C"
{

int NoteVault_RunApp()
{
	int argc = 0;
	wxChar** argv = nullptr;
	return wxEntry(argc, argv);
}

} // extern "C"
