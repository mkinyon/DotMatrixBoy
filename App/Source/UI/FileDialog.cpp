#include "FileDialog.h"
#include "EventManager.h"

namespace App
{
	FileDialog::FileDialog(Core::GameBoy* gb) : ImguiWidgetBase("File Browser"), gameboy(gb)
	{
		disableTitleWrapper = true;
		fileDialog.SetTitle("Open Rom");
		fileDialog.SetTypeFilters({ ".gb" });

		EventManager::Instance().Subscribe(Event::OPEN_FILE_DIALOG, this);
	}

	FileDialog::~FileDialog() {}

	void FileDialog::RenderContent()
	{
		fileDialog.Display();

		if (fileDialog.HasSelected())
		{
			Core::Cartridge* newCart = new Core::Cartridge(fileDialog.GetSelected().string(), false);
			delete gameboy;
			gameboy = new Core::GameBoy(*newCart);

			gameboy->Run(false);
			fileDialog.ClearSelected();
		}
	}

	void FileDialog::OnEvent(Event event)
	{
		if (event == Event::OPEN_FILE_DIALOG)
		{
			fileDialog.Open();
			ShowWindow = true;
		}
	}
}