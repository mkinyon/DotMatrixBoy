#pragma once

#include "ImguiWidgetBase.h"
#include "Core/GameBoy.h"
#include <imfilebrowser.h>

namespace App
{
	class FileDialog : public ImguiWidgetBase
	{
	public:
		FileDialog(Core::GameBoy* gb);
		~FileDialog();

	private:
		void RenderContent();
		void OnEvent(Event event);

	private:
		ImGui::FileBrowser m_FileDialog;
		Core::GameBoy* m_GameBoy;
	};
}
