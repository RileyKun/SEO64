#include "widgets.hpp"
#include "binds.hpp"

enum class WidgetType
{
	TOGGLE, SELECT_SINGLE, SLIDER_INT, SLIDER_FLOAT
};

struct WidgetCtx
{
	WidgetType m_type{};

	std::any m_val_off{};
	std::any m_val_on{};

	WidgetCtx(const std::any val_off, const std::any val_on)
	{
		m_type = WidgetType::TOGGLE;
		m_val_off = val_off;
		m_val_on = val_on;
	}
};

struct WidgetCtxSelectSingle final : public WidgetCtx
{
	std::vector<std::pair<std::string, int>> m_vars{};

	WidgetCtxSelectSingle(const std::vector<std::pair<std::string, int>> &vars)
		: WidgetCtx(std::make_any<int>(vars.front().second), std::make_any<int>(vars.back().second))
	{
		m_type = WidgetType::SELECT_SINGLE;
		m_vars = vars;
	}
};

struct WidgetCtxSliderInt final : public WidgetCtx
{
	int m_val_min{};
	int m_val_max{};
	const char *m_fmt{};

	WidgetCtxSliderInt(const int min, const int max, const char *const fmt)
		: WidgetCtx(std::make_any<int>(min), std::make_any<int>(max))
	{
		m_type = WidgetType::SLIDER_INT;
		m_val_min = min;
		m_val_max = max;
		m_fmt = fmt;
	}
};

struct WidgetCtxSliderFloat final : public WidgetCtx
{
	float m_val_min{};
	float m_val_max{};
	const char *m_fmt{};

	WidgetCtxSliderFloat(const float min, const float max, const char *const fmt)
		: WidgetCtx(std::make_any<float>(min), std::make_any<float>(max))
	{
		m_type = WidgetType::SLIDER_FLOAT;
		m_val_min = min;
		m_val_max = max;
		m_fmt = fmt;
	}
};

//==================================================================================================================================

bool inputKey(int *const var)
{
	if (!var) {
		return false;
	}

	bool callback{};

	static bool active{};

	if (!active)
	{
		if (!*var) {
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		}

		std::string str{ "bind key" };

		if (*var) {
			str = utils::keyToStr(*var);
		}

		const ImVec2 anchor{ ImGui::GetCursorPos() };

		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.0f, 0.5f });

		if (ImGui::Button(str.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f })) {
			active = true;
			ImGui::SetCursorPos(anchor);
		}

		ImGui::PopStyleVar();

		if (!*var) {
			ImGui::PopStyleColor();
		}
	}

	if (active)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.0f, 0.5f });

		ImGui::Button("...", ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f });

		ImGui::PopStyleColor();
		ImGui::PopStyleVar();

		for (int n{}; n < 256; n++)
		{
			const bool mouse{ n > 0 && n < 7 };
			const bool letter{ n >= 'A' && n <= 'Z' };
			const bool num{ n >= 48 && n <= 57 };
			const bool esc{ n == VK_ESCAPE };
			const bool numpad{ n >= VK_NUMPAD0 && n <= VK_NUMPAD9 };
			const bool other{ n == VK_LSHIFT || n == VK_RSHIFT || n == VK_SHIFT || n == VK_INSERT || n == VK_MENU || n == VK_CAPITAL || n == VK_SPACE || n == VK_CONTROL };
			const bool allowed{ mouse || letter || num || esc || numpad || other };

			if (input_dx->getKey(n).pressed && allowed) {
				*var = (esc ? 0 : n);
				active = false;
				callback = true;
				break;
			}
		}
	}

	return callback;
}

static int resizeCallback(ImGuiInputTextCallbackData *data)
{
	if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
	{
		std::string *const str{ static_cast<std::string *>(data->UserData) };
		str->resize(data->BufTextLen);
		data->Buf = str->data();
	}

	return 0;
}

void bindWidget(void *const var, WidgetCtx *const ctx)
{
	if (!var || !ctx) {
		return;
	}

	auto findCfgVar = [var]() -> ConfigVar *const
	{
		for (ConfigVar *const cfg_var : getInsts<ConfigVar>())
		{
			if (cfg_var->ptr() != var) {
				continue;
			}

			return cfg_var;
		}

		return nullptr;
	};

	ConfigVar *const cfg_var{ findCfgVar() };

	if (!cfg_var) {
		return;
	}
	
	ImGui::PushID(var);

	if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
		ImGui::OpenPopup("bind_popup_context");
	}

	bool open_bind{};

	if (ImGui::BeginPopup("bind_popup_context"))
	{
		ImGui::Text(cfg_var->name().c_str());

		ImGui::Separator();

		if (ImGui::MenuItem("bind")) {
			open_bind = true;
		}

		ImGui::EndPopup();
	}

	static Bind bind_data{};

	if (open_bind)
	{
		ImGui::OpenPopup("bind_popup_bind");

		bind_data = Bind{};

		bind_data.val_off = ctx->m_val_off;
		bind_data.val_on = ctx->m_val_on;
		bind_data.var = cfg_var;
		bind_data.cfg_var_name = cfg_var->name();
		bind_data.appear_in_bindslist = true;
	}

	if (ImGui::BeginPopup("bind_popup_bind"))
	{
		ImGui::Text(std::format("binding {}", cfg_var->name()).c_str());

		ImGui::Separator();

		ImGui::InputTextWithHint("##bind_name", "name", bind_data.name.data(), bind_data.name.capacity() + 1, ImGuiInputTextFlags_CallbackResize, resizeCallback, &bind_data.name);

		inputKey(&bind_data.key);

		if (ImGui::BeginCombo("##bind_type", bind_data.type == BindType::TOGGLE ? "toggle" : "hold"))
		{
			if (ImGui::Selectable("toggle", bind_data.type == BindType::TOGGLE)) {
				bind_data.type = BindType::TOGGLE;
			}

			if (ImGui::Selectable("hold", bind_data.type == BindType::HOLD)) {
				bind_data.type = BindType::HOLD;
			}

			ImGui::EndCombo();
		}

		if (ctx->m_type == WidgetType::TOGGLE)
		{
			if (ImGui::BeginCombo("##val_off", std::any_cast<bool>(bind_data.val_off) ? "true" : "false"))
			{
				if (ImGui::Selectable("false", std::any_cast<bool>(bind_data.val_off) == false)) {
					bind_data.val_off = std::make_any<bool>(false);
				}

				if (ImGui::Selectable("true", std::any_cast<bool>(bind_data.val_off) == true)) {
					bind_data.val_off = std::make_any<bool>(true);
				}

				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("##val_on", std::any_cast<bool>(bind_data.val_on) ? "true" : "false"))
			{
				if (ImGui::Selectable("false", std::any_cast<bool>(bind_data.val_on) == false)) {
					bind_data.val_on = std::make_any<bool>(false);
				}

				if (ImGui::Selectable("true", std::any_cast<bool>(bind_data.val_on) == true)) {
					bind_data.val_on = std::make_any<bool>(true);
				}

				ImGui::EndCombo();
			}
		}

		else if (ctx->m_type == WidgetType::SELECT_SINGLE)
		{
			WidgetCtxSelectSingle *const select{ static_cast<WidgetCtxSelectSingle *>(ctx) };

			auto valToSelect = [&](const int val)
			{
				for (const std::pair<std::string, int> &v : select->m_vars)
				{
					if (v.second != val) {
						continue;
					}

					return v;
				}

				return std::pair<std::string, int>{};
			};

			const std::pair<std::string, int> cur_off{ valToSelect(std::any_cast<int>(bind_data.val_off)) };
			const std::pair<std::string, int> cur_on{ valToSelect(std::any_cast<int>(bind_data.val_on)) };

			if (ImGui::BeginCombo("##val_off", cur_off.first.c_str()))
			{
				for (const std::pair<std::string, int> &v : select->m_vars)
				{
					if (ImGui::Selectable(v.first.c_str(), std::any_cast<int>(bind_data.val_off) == v.second)) {
						bind_data.val_off = std::make_any<int>(v.second);
					}
				}

				ImGui::EndCombo();
			}

			if (ImGui::BeginCombo("##val_on", cur_on.first.c_str()))
			{
				for (const std::pair<std::string, int> &v : select->m_vars)
				{
					if (ImGui::Selectable(v.first.c_str(), std::any_cast<int>(bind_data.val_on) == v.second)) {
						bind_data.val_on = std::make_any<int>(v.second);
					}
				}

				ImGui::EndCombo();
			}
		}

		else if (ctx->m_type == WidgetType::SLIDER_INT)
		{
			WidgetCtxSliderInt *const slider{ static_cast<WidgetCtxSliderInt *>(ctx) };

			static int slider_off{};
			static int slider_on{};

			slider_off = std::any_cast<int>(bind_data.val_off);
			slider_on = std::any_cast<int>(bind_data.val_on);

			if (ImGui::SliderInt("##val_off", &slider_off, slider->m_val_min, slider->m_val_max, slider->m_fmt, ImGuiSliderFlags_AlwaysClamp)) {
				bind_data.val_off = std::make_any<int>(slider_off);
			}

			if (ImGui::SliderInt("##val_on", &slider_on, slider->m_val_min, slider->m_val_max, slider->m_fmt, ImGuiSliderFlags_AlwaysClamp)) {
				bind_data.val_on = std::make_any<int>(slider_on);
			}
		}

		else if (ctx->m_type == WidgetType::SLIDER_FLOAT)
		{
			WidgetCtxSliderFloat *const slider{ static_cast<WidgetCtxSliderFloat *>(ctx) };

			static float slider_off{};
			static float slider_on{};

			slider_off = std::any_cast<float>(bind_data.val_off);
			slider_on = std::any_cast<float>(bind_data.val_on);

			if (ImGui::SliderFloat("##val_off", &slider_off, slider->m_val_min, slider->m_val_max, slider->m_fmt, ImGuiSliderFlags_AlwaysClamp)) {
				bind_data.val_off = std::make_any<float>(slider_off);
			}

			if (ImGui::SliderFloat("##val_on", &slider_on, slider->m_val_min, slider->m_val_max, slider->m_fmt, ImGuiSliderFlags_AlwaysClamp)) {
				bind_data.val_on = std::make_any<float>(slider_on);
			}
		}

		ImGui::Separator();

		ImGui::Checkbox("show in binds", &bind_data.appear_in_bindslist);

		ImGui::Separator();

		ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.0f, 0.5f });

		if (!std::string_view{ bind_data.name.data() }.empty() && bind_data.key != 0)
		{
			if (ImGui::Button("save", ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f }))
			{
				if (ctx->m_type == WidgetType::TOGGLE) {
					*static_cast<bool *>(var) = std::any_cast<bool>(bind_data.val_off);
				}

				else if (ctx->m_type == WidgetType::SELECT_SINGLE) {
					*static_cast<int *>(var) = std::any_cast<int>(bind_data.val_off);
					bind_data.selects = static_cast<WidgetCtxSelectSingle *>(ctx)->m_vars;
				}

				else if (ctx->m_type == WidgetType::SLIDER_INT) {
					*static_cast<int *>(var) = std::any_cast<int>(bind_data.val_off);
					bind_data.fmt = static_cast<WidgetCtxSliderInt *>(ctx)->m_fmt;
				}

				else if (ctx->m_type == WidgetType::SLIDER_FLOAT) {
					*static_cast<float *>(var) = std::any_cast<float>(bind_data.val_off);
					bind_data.fmt = static_cast<WidgetCtxSliderFloat *>(ctx)->m_fmt;
				}

				binds->add(bind_data);

				ImGui::CloseCurrentPopup();
			}
		}

		else
		{
			ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			ImGui::Button("save", ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f });
			ImGui::PopStyleColor();
		}

		ImGui::PopStyleVar();

		ImGui::EndPopup();
	}

	ImGui::PopID();
}

//==================================================================================================================================

bool toggle(const char *const label, bool *const var)
{
	const bool callback{ ImGui::Checkbox(label, var) };

	bindWidget(var, std::make_shared<WidgetCtx>(std::make_any<bool>(false), std::make_any<bool>(true)).get());

	return callback;
}

bool selectSingle(const char *const label, int *const var, std::vector<std::pair<std::string, int>> vars)
{
	bool callback{};

	if (ImGui::BeginCombo(label, vars[*var].first.c_str()))
	{
		for (int i{}; i < static_cast<int>(vars.size()); i++)
		{
			const bool selected{ i == *var };

			if (ImGui::Selectable(vars[i].first.c_str(), selected)) {
				*var = i;
				callback = true;
			}

			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}

		ImGui::EndCombo();
	}

	bindWidget(var, std::make_shared<WidgetCtxSelectSingle>(vars).get());

	return callback;
}

bool selectMulti(const char *const label, const std::vector<std::pair<std::string, bool *>> vars)
{
	bool callback{};

	std::string preview{};

	for (const std::pair<std::string, bool *> &var : vars)
	{
		if (*var.second)
		{
			if (!preview.empty()) {
				preview += ", ";
			}

			preview += var.first;
		}
	}
	
	if (ImGui::BeginCombo(label, preview.c_str()))
	{
		for (const std::pair<std::string, bool *> &var : vars)
		{
			if (ImGui::SelectableWithValueOnRight(var.first.c_str(), false, std::format("{}", *var.second).c_str(), ImGuiSelectableFlags_NoAutoClosePopups)) {
				*var.second = !*var.second;
			}

			bindWidget(var.second, std::make_shared<WidgetCtx>(std::make_any<bool>(false), std::make_any<bool>(true)).get());
		}

		ImGui::EndCombo();
	}

	return callback;
}

bool sliderInt(const char *const label, int *const var, int min, int max, const char *const fmt)
{
	const bool callback{ ImGui::SliderInt(label, var, min, max, fmt, ImGuiSliderFlags_AlwaysClamp) };

	bindWidget(var, std::make_shared<WidgetCtxSliderInt>(min, max, fmt).get());

	return callback;
}

bool sliderFloat(const char *const label, float *const var, float min, float max, const char *const fmt)
{
	const bool callback{ ImGui::SliderFloat(label, var, min, max, fmt, ImGuiSliderFlags_AlwaysClamp) };

	bindWidget(var, std::make_shared<WidgetCtxSliderFloat>(min, max, fmt).get());

	return callback;
}

bool colorPicker(const char *const label, Color *const var)
{
	ImVec4 clr{};

	clr.x = static_cast<float>(var->r) / 255.0f;
	clr.y = static_cast<float>(var->g) / 255.0f;
	clr.z = static_cast<float>(var->b) / 255.0f;
	clr.w = static_cast<float>(var->a) / 255.0f;

	const bool callback{ ImGui::ColorEdit4(label, &clr.x, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaBar) };

	var->r = static_cast<uint8_t>(clr.x * 255.0f);
	var->g = static_cast<uint8_t>(clr.y * 255.0f);
	var->b = static_cast<uint8_t>(clr.z * 255.0f);
	var->a = static_cast<uint8_t>(clr.w * 255.0f);

	return callback;
}

bool inputString(const char *const label, std::string *const var, const char *const fmt, int flags)
{
	return ImGui::InputText(label, var->data(), var->capacity() + 1, ImGuiInputTextFlags_CallbackResize | flags, resizeCallback, var);
}

bool inputStringWithHint(const char *const label, std::string *const var, const char *const hint, const char *const fmt, int flags)
{
	return ImGui::InputTextWithHint(label, hint, var->data(), var->capacity() + 1, ImGuiInputTextFlags_CallbackResize | flags, resizeCallback, var);
}