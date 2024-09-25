#include "ui.hpp"

#include "../cfg.hpp"

bool UI::onLoad()
{
	if (!ImGui::CreateContext()) {
		return false;
	}

	if (!ImGui_ImplWin32_Init(FindWindowA("Valve001", nullptr))) {
		return false;
	}

	if (!ImGui_ImplDX9_Init(i::dxdev)) {
		return false;
	}

	if (ImGuiIO *const io{ &ImGui::GetIO() })
	{
		io->IniFilename = nullptr;
		io->LogFilename = nullptr;

		io->MouseDrawCursor = false;

		io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

		ImFontConfig cfg{};
		{
			cfg.OversampleH = 1;
			cfg.OversampleV = 1;
			cfg.PixelSnapH = true;
			cfg.FontBuilderFlags |= ImGuiFreeTypeBuilderFlags_Monochrome | ImGuiFreeTypeBuilderFlags_MonoHinting;
		}

		io->FontDefault = io->Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\TAHOMA.TTF", 13.0f, &cfg);
	}

	if (ImGuiStyle *const style{ &ImGui::GetStyle() })
	{
		style->WindowPadding = { 4.0f, 4.0f };
		style->FramePadding = { 2.0f, 2.0f };
		style->ItemSpacing = { 4.0f, 4.0f };
		style->ItemInnerSpacing = { 4.0f, 4.0f };
		style->TouchExtraPadding = { 0.0f, 0.0f };

		style->IndentSpacing = 20.0f;
		style->ScrollbarSize = 10.0f;
		style->GrabMinSize = 6.0f;

		style->WindowBorderSize = 1.0f;
		style->ChildBorderSize = 1.0f;
		style->FrameBorderSize = 1.0f;
		style->PopupBorderSize = 1.0f;
		style->TabBorderSize = 1.0f;
		style->TabBarBorderSize = 1.0f;
		style->TabBarOverlineSize = 1.0f;

		style->WindowRounding = 0.0f;
		style->ChildRounding = 0.0f;
		style->FrameRounding = 0.0f;
		style->PopupRounding = 0.0f;
		style->ScrollbarRounding = 0.0f;
		style->GrabRounding = 0.0f;
		style->TabRounding = 0.0f;

		style->AntiAliasedFill = false;
		style->AntiAliasedLines = false;
		style->AntiAliasedLinesUseTex = false;
	}

	return true;
}

bool UI::onUnload()
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();

	ImGui::DestroyContext();

	return true;
}

void UI::setColors(const float hue, const float sat, const float val)
{
	if (ImVec4 *const colors{ ImGui::GetStyle().Colors })
	{
		auto getColor = [&](float h, float s, float v)
		{
			h = math::remap(h, 0.0f, 360.0f, 0.0f, 1.0f);
			s = math::remap(s * sat, 0.0f, 100.0f, 0.0f, 1.0f);
			v = math::remap(v * val, 0.0f, 100.0f, 0.0f, 1.0f);

			if (s == 0.0f) {
				return ImVec4(v, v, v, 1.0f);
			}

			int i{ static_cast<int>(h * 6.0f) };

			float f{ (h * 6.0f) - i };
			float p{ v * (1.0f - s) };
			float q{ v * (1.0f - s * f) };
			float t{ v * (1.0f - s * (1.0f - f)) };

			i %= 6;

			if (i == 0) {
				return ImVec4(v, t, p, 1.0f);
			}

			else if (i == 1) {
				return ImVec4(q, v, p, 1.0f);
			}

			else if (i == 2) {
				return ImVec4(p, v, t, 1.0f);
			}

			else if (i == 3) {
				return ImVec4(p, q, v, 1.0f);
			}

			else if (i == 4) {
				return ImVec4(t, p, v, 1.0f);
			}

			else {
				return ImVec4(v, p, q, 1.0f);
			}
		};

		colors[ImGuiCol_Text] = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);

		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.00f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);

		colors[ImGuiCol_NavHighlight] = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.7f, 0.7f, 0.7f, 1.00f);

		colors[ImGuiCol_WindowBg] = getColor(hue, 0.75f, 0.08f);
		colors[ImGuiCol_PopupBg] = getColor(hue, 0.75f, 0.08f);
		colors[ImGuiCol_Border] = getColor(hue, 0.81f, 0.31f);
		colors[ImGuiCol_FrameBg] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_FrameBgHovered] = getColor(hue, 0.80f, 0.20f);
		colors[ImGuiCol_FrameBgActive] = getColor(hue, 0.80f, 0.20f);
		colors[ImGuiCol_TitleBg] = getColor(hue, 0.75f, 0.08f);
		colors[ImGuiCol_TitleBgActive] = getColor(hue, 0.75f, 0.08f);
		colors[ImGuiCol_TitleBgCollapsed] = getColor(hue, 0.75f, 0.08f);
		colors[ImGuiCol_CheckMark] = getColor(hue, 0.91f, 0.36f);
		colors[ImGuiCol_SliderGrab] = getColor(hue, 0.80f, 0.39f);
		colors[ImGuiCol_SliderGrabActive] = getColor(hue, 0.80f, 0.39f);
		colors[ImGuiCol_Button] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_ButtonHovered] = getColor(hue, 0.80f, 0.20f);
		colors[ImGuiCol_ButtonActive] = getColor(hue, 0.80f, 0.20f);
		colors[ImGuiCol_Header] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_HeaderHovered] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_HeaderActive] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_Separator] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_SeparatorHovered] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_SeparatorActive] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_ResizeGrip] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_ResizeGripHovered] = getColor(hue, 0.80f, 0.20f);
		colors[ImGuiCol_ResizeGripActive] = getColor(hue, 0.80f, 0.20f);
		colors[ImGuiCol_TabHovered] = getColor(hue, 0.75f, 0.16f);
		colors[ImGuiCol_Tab] = getColor(hue, 0.75f, 0.08f);
		colors[ImGuiCol_TabSelected] = getColor(hue, 0.80f, 0.20f);
		colors[ImGuiCol_TableHeaderBg] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_TableRowBg] = getColor(hue, 0.80f, 0.16f);
		colors[ImGuiCol_TableRowBgAlt] = getColor(hue, 0.80f, 0.16f);
	}
}

void UI::start()
{
	if (input_dx->getKey(VK_INSERT).pressed) {
		m_open = !m_open;
		i::input_system->ResetInputState();
	}

	float hue{};
	float sat{};
	float val{};

	utils::rgbToHSV(cfg::ui_color_base, hue, sat, val);

	setColors(hue, sat, val);

	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();

	ImGui::NewFrame();
}

void UI::end()
{
	DWORD color_write_enable{};
	DWORD srgb_write_enable{};
	DWORD color_op_stage_state{};
	DWORD alpha_op_stage_state{};

	i::dxdev->GetRenderState(D3DRS_COLORWRITEENABLE, &color_write_enable);
	i::dxdev->GetRenderState(D3DRS_SRGBWRITEENABLE, &srgb_write_enable);
	i::dxdev->GetTextureStageState(0, D3DTSS_COLOROP, &color_op_stage_state);
	i::dxdev->GetTextureStageState(0, D3DTSS_ALPHAOP, &alpha_op_stage_state);

	i::dxdev->SetRenderState(D3DRS_COLORWRITEENABLE, 0xFFFFFFFF);
	i::dxdev->SetRenderState(D3DRS_SRGBWRITEENABLE, false);
	i::dxdev->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	i::dxdev->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

	ImGui::EndFrame();
	ImGui::Render();

	ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

	i::dxdev->SetRenderState(D3DRS_COLORWRITEENABLE, color_write_enable);
	i::dxdev->SetRenderState(D3DRS_SRGBWRITEENABLE, srgb_write_enable);
	i::dxdev->SetTextureStageState(0, D3DTSS_COLOROP, color_op_stage_state);
	i::dxdev->SetTextureStageState(0, D3DTSS_ALPHAOP, alpha_op_stage_state);
}