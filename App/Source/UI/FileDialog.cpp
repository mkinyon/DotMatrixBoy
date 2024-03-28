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
			// pause the emulator so we don't risk deleting it
			// while in the middle of a clock cycle
			m_GameBoy->Pause();

			// create new cart
			Core::Cartridge* newCart = new Core::Cartridge(m_FileDialog.GetSelected().string(), m_AppState.IsBootRomEnabled);
			
			// clean up and create new gameboy
			delete m_GameBoy;
			m_GameBoy = new Core::GameBoy(*newCart);
			m_GameBoy->Run(m_AppState.IsBootRomEnabled);

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