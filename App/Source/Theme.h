#pragma once

#include "imgui.h"
#include "Roboto-Regular.embed"

namespace App
{
	ImVec4 RGBToImVec4(int r, int g, int b)
	{
		ImVec4 color;
		color.x = static_cast<float>(r) / 255.0f;
		color.y = static_cast<float>(g) / 255.0f;
		color.z = static_cast<float>(b) / 255.0f;
		color.w = 1.0f;
		return color;
	}

	const ImVec4 white = RGBToImVec4(255, 255, 255);

	const ImVec4 consoleDarkColor = RGBToImVec4(39, 41, 41);
	const ImVec4 consoleLightColor = RGBToImVec4(196, 190, 187);
	const ImVec4 consoleBlueColor = RGBToImVec4(73, 71, 134);
	const ImVec4 consoleRedColor = RGBToImVec4(154, 34, 87);

	void setTheme()
	{
		ImGuiStyle* style = &ImGui::GetStyle();

		style->WindowPadding = ImVec2(5, 5);
		style->WindowRounding = 5.0f;
		style->FramePadding = ImVec2(5, 5);
		style->FrameRounding = 4.0f;
		style->ItemSpacing = ImVec2(12, 8);
		style->ItemInnerSpacing = ImVec2(8, 6);
		style->IndentSpacing = 25.0f;
		style->ScrollbarSize = 15.0f;
		style->ScrollbarRounding = 9.0f;
		style->GrabMinSize = 5.0f;
		style->GrabRounding = 3.0f;


		// text
		style->Colors[ImGuiCol_Text] = consoleDarkColor;
		style->Colors[ImGuiCol_TextDisabled] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);

		// tabs
		style->Colors[ImGuiCol_Tab] = consoleBlueColor;
		style->Colors[ImGuiCol_TabHovered] = consoleRedColor;
		style->Colors[ImGuiCol_TabActive] = consoleRedColor;
		
		style->Colors[ImGuiCol_MenuBarBg] = consoleDarkColor;
		style->Colors[ImGuiCol_WindowBg] = consoleLightColor;

		style->Colors[ImGuiCol_PopupBg] = consoleDarkColor;
		style->Colors[ImGuiCol_Border] = ImVec4(0.80f, 0.80f, 0.83f, 0.88f);
		style->Colors[ImGuiCol_BorderShadow] = ImVec4(0.92f, 0.91f, 0.88f, 0.00f);

		style->Colors[ImGuiCol_FrameBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.24f, 0.23f, 0.29f, 1.00f);
		style->Colors[ImGuiCol_FrameBgActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);

		style->Colors[ImGuiCol_TitleBg] = consoleLightColor;
		style->Colors[ImGuiCol_TitleBgCollapsed] = consoleLightColor;
		style->Colors[ImGuiCol_TitleBgActive] = consoleDarkColor;
		
		style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrab] = consoleRedColor;
		style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

		style->Colors[ImGuiCol_CheckMark] = consoleRedColor;

		style->Colors[ImGuiCol_SliderGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
		style->Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);

		style->Colors[ImGuiCol_Button] = consoleBlueColor;
		style->Colors[ImGuiCol_ButtonHovered] = consoleRedColor;
		style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);

		style->Colors[ImGuiCol_Header] = consoleDarkColor;
		style->Colors[ImGuiCol_HeaderHovered] = consoleDarkColor;
		style->Colors[ImGuiCol_HeaderActive] = consoleDarkColor;

		style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
		style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
		style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
		style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
		style->Colors[ImGuiCol_TextSelectedBg] = consoleRedColor;

		// set font
		ImGuiIO& io = ImGui::GetIO();
		ImFontConfig fontConfig;
		fontConfig.FontDataOwnedByAtlas = false;
		ImFont* robotoFont = io.Fonts->AddFontFromMemoryTTF((void*)g_RobotoRegular, sizeof(g_RobotoRegular), 14.0f, &fontConfig);
		io.FontDefault = robotoFont;
	}	
}