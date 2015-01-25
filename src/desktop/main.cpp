#include "ui/MainWindow.h"
#include <wx/app.h>

namespace
{

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
		m_Window = new NoteVault::MainWindow("Note Vault", wxSize(800, 600));
		m_Window->Show(true);
		return true;
	}

private:
	NoteVault::MainWindow* m_Window;
};

} // namespace

wxIMPLEMENT_APP(App);
