#include "FileDialog.h"
#include "EventManager.h"

namespace App
{
	FileDialog::FileDialog(Core::GameBoy* gb) : ImguiWidgetBase("File Browser"), m_GameBoy(gb)
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
			Core::Cartridge* newCart = new Core::Cartridge(m_FileDialog.GetSelected().string(), false);
			delete m_GameBoy;
			m_GameBoy = new Core::GameBoy(*newCart);

			m_GameBoy->Run(false);
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