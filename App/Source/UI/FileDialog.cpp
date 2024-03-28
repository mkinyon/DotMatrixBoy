#include "FileDialog.h"
#include "EventManager.h"

namespace App
{
	FileDialog::FileDialog(Core::GameBoy* gb, sAppState& appState) : ImguiWidgetBase("File Browser"), m_GameBoy(gb), m_AppState(appState)
	{
		m_DisableTitleWrapper = true;
		m_FileDialog.SetTitle("Open Rom");
		m_FileDialog.SetTypeFilters({ ".gb" });

		EventManager::Instance().Subscribe(Event::OPEN_FILE_DIALOG, this);
	}

	FileDialog::~FileDialog() {}

	void FileDialog::RenderContent()
	{
		m_FileDialog.Display();

		if (m_FileDialog.HasSelected())
		{
			// clean up and create new gameboy
			m_GameBoy->LoadRom(m_FileDialog.GetSelected().string().c_str());

			m_AppState.AddRecentRomEntry(m_FileDialog.GetSelected().string());
			m_FileDialog.ClearSelected();
		}
	}

	void FileDialog::OnEvent(Event event)
	{
		if (event == Event::OPEN_FILE_DIALOG)
		{
			m_FileDialog.Open();
			m_ShowWindow = true;
		}
	}
}