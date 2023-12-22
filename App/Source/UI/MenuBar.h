#pragma once

#include "ImguiWidgetBase.h"

namespace App
{
	class MenuBar : public ImguiWidgetBase
	{
	public:
		MenuBar();
		~MenuBar();

	private:
		void RenderContent();
	};
}
