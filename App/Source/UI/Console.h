#pragma once

#include "imgui.h"
#include "ImguiWidgetBase.h"

namespace App
{
	class Console : public ImguiWidgetBase
	{
	public:
		Console();
		~Console();

	private:
		void RenderContent();
		void OnEvent(Event event);

	};
}

