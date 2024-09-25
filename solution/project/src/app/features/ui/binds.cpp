#include "binds.hpp"
#include "ui.hpp"

#include "../cfg.hpp"

void Binds::save(const std::string &path)
{
	nlohmann::json j{};

	for (const Bind &bind : m_binds)
	{
		if (!bind.var) {
			continue;
		}

		nlohmann::json b{};

		b["name"] = bind.name;
		b["key"] = bind.key;
		b["type"] = bind.type == BindType::TOGGLE ? "toggle" : "hold";
		b["cfg_var_name"] = bind.cfg_var_name;
		b["fmt"] = bind.fmt;

		if (bind.var->hash() == typeid(bool).hash_code()) {
			b["val_off"] = std::any_cast<bool>(bind.val_off);
			b["val_on"] = std::any_cast<bool>(bind.val_on);
		}
		else if (bind.var->hash() == typeid(int).hash_code()) {
			b["val_off"] = std::any_cast<int>(bind.val_off);
			b["val_on"] = std::any_cast<int>(bind.val_on);
		}

		else if (bind.var->hash() == typeid(float).hash_code()) {
			b["val_off"] = std::any_cast<float>(bind.val_off);
			b["val_on"] = std::any_cast<float>(bind.val_on);
		}

        b["appear_in_bindslist"] = bind.appear_in_bindslist;

		if (!bind.selects.empty())
		{
			nlohmann::json selects{};

			for (const std::pair<std::string, int> &select : bind.selects)
			{
				nlohmann::json s{};

				s["name"] = select.first;
				s["value"] = select.second;

				selects.push_back(s);
			}

			b["selects"] = selects;
		}

		j.push_back(b);
	}

	std::ofstream file{ path.data() };

	file << std::setw(4) << j;

	file.close();
}

void Binds::load(const std::string &path)
{
	nlohmann::json j{};

	std::ifstream file{ path.data() };

	if (!file.is_open()) {
		return;
	}

	file >> j;

	file.close();

	auto findCfgVarByName = [&](const std::string &name) -> ConfigVar *const
	{
		for (ConfigVar *const var : getInsts<ConfigVar>())
		{
			if (!var || var->name() != name) {
				continue;
			}

			return var;
		}

		return nullptr;
	};
	
	if (!j.empty()) {
		m_binds.clear();
	}

	for (const nlohmann::json &b : j)
	{
		Bind bind{};

		bind.name = b["name"].get<std::string>();
		bind.key = b["key"].get<int>();
		bind.type = b["type"].get<std::string>() == "toggle" ? BindType::TOGGLE : BindType::HOLD;
		bind.cfg_var_name = b["cfg_var_name"].get<std::string>();
		bind.fmt = b["fmt"].get<std::string>();

		if (b.contains("selects"))
		{
			for (const nlohmann::json &s : b["selects"]) {
				bind.selects.push_back({ s["name"].get<std::string>(), s["value"].get<int>() });
			}
		}

		if (bind.cfg_var_name.empty()) {
			continue;
		}

		bind.var = findCfgVarByName(bind.cfg_var_name);

		if (!bind.var) {
			continue;
		}

		if (bind.var->hash() == typeid(bool).hash_code()) {
			bind.val_off = b["val_off"].get<bool>();
			bind.val_on = b["val_on"].get<bool>();
		}

		else if (bind.var->hash() == typeid(int).hash_code()) {
			bind.val_off = b["val_off"].get<int>();
			bind.val_on = b["val_on"].get<int>();
		}

		else if (bind.var->hash() == typeid(float).hash_code()) {
			bind.val_off = b["val_off"].get<float>();
			bind.val_on = b["val_on"].get<float>();
		}
		
        bind.appear_in_bindslist = b["appear_in_bindslist"].get<bool>();

		m_binds.push_back(bind);
	}
}

bool Binds::onLoad()
{
	load(utils::getFilePath() + "binds.json");

	return true;
}

void Binds::add(const Bind bind)
{
	m_binds.push_back(bind);

	save(utils::getFilePath() + "binds.json");
}

void Binds::remove(Bind *const bind_ptr)
{
	if (!bind_ptr) {
		return;
	}

	m_binds.erase(std::remove_if(m_binds.begin(), m_binds.end(), [&bind_ptr](const Bind &bind) { return &bind == bind_ptr; }), m_binds.end());

	save(utils::getFilePath() + "binds.json");
}

void Binds::think()
{
	const bool want_reset{ i::surface->IsCursorVisible() || i::vgui->IsGameUIVisible() };

	for (Bind &bind : m_binds)
	{
		if (want_reset && bind.type == BindType::HOLD)
		{
			if (bind.var->hash() == typeid(bool).hash_code()) {
				*static_cast<bool *>(bind.var->ptr()) = std::any_cast<bool>(bind.val_off);
			}

			else if (bind.var->hash() == typeid(int).hash_code()) {
				*static_cast<int *>(bind.var->ptr()) = std::any_cast<int>(bind.val_off);
			}

			else if (bind.var->hash() == typeid(float).hash_code()) {
				*static_cast<float *>(bind.var->ptr()) = std::any_cast<float>(bind.val_off);
			}

			continue;
		}

		if (bind.type == BindType::TOGGLE)
		{
			if (!input_dx->getKeyInGame(bind.key).pressed) {
				continue;
			}

			if (bind.var->hash() == typeid(bool).hash_code()) {
				bool &var_val{ *static_cast<bool *>(bind.var->ptr()) };
				var_val = !var_val;
			}

			else if (bind.var->hash() == typeid(int).hash_code())
			{
				int &var_val{ *static_cast<int *>(bind.var->ptr()) };

				if (var_val == std::any_cast<int>(bind.val_off)) {
					var_val = std::any_cast<int>(bind.val_on);
				}

				else {
					var_val = std::any_cast<int>(bind.val_off);
				}
			}

			else if (bind.var->hash() == typeid(float).hash_code())
			{
				float &var_val{ *static_cast<float *>(bind.var->ptr()) };

				if (var_val == std::any_cast<float>(bind.val_off)) {
					var_val = std::any_cast<float>(bind.val_on);
				}

				else {
					var_val = std::any_cast<float>(bind.val_off);
				}
			}
		}

		else if (bind.type == BindType::HOLD)
		{
			const bool pressed{ input_dx->getKeyInGame(bind.key).held };
			const bool released{ input_dx->getKeyInGame(bind.key).released };

			if (bind.var->hash() == typeid(bool).hash_code())
			{
				if (pressed) {
					*static_cast<bool *>(bind.var->ptr()) = std::any_cast<bool>(bind.val_on);
				}

				if (released) {
					*static_cast<bool *>(bind.var->ptr()) = std::any_cast<bool>(bind.val_off);
				}
			}

			else if (bind.var->hash() == typeid(int).hash_code())
			{
				if (pressed) {
					*static_cast<int *>(bind.var->ptr()) = std::any_cast<int>(bind.val_on);
				}

				if (released) {
					*static_cast<int *>(bind.var->ptr()) = std::any_cast<int>(bind.val_off);
				}
			}

			else if (bind.var->hash() == typeid(float).hash_code())
			{
				if (pressed) {
					*static_cast<float *>(bind.var->ptr()) = std::any_cast<float>(bind.val_on);
				}

				if (released) {
					*static_cast<float *>(bind.var->ptr()) = std::any_cast<float>(bind.val_off);
				}
			}
		}
	}
}

void Binds::paint()
{
	if (m_binds.empty()) {
		return;
	}

	if (ImGui::Begin("binds", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize))
	{
		std::vector<Bind *> to_remove{};

		for (Bind &bind : m_binds)
		{
			bool active{};
			{
				if (bind.var->hash() == typeid(bool).hash_code()) {
					active = *static_cast<bool *>(bind.var->ptr()) == std::any_cast<bool>(bind.val_on);
				}

				else if (bind.var->hash() == typeid(int).hash_code()) {
					active = *static_cast<int *>(bind.var->ptr()) == std::any_cast<int>(bind.val_on);
				}

				else if (bind.var->hash() == typeid(float).hash_code()) {
					active = *static_cast<float *>(bind.var->ptr()) == std::any_cast<float>(bind.val_on);
				}
			}

            if (!ui->isOpen() && !bind.appear_in_bindslist) {
                continue;
            }

			const ImVec4 text_clr_backup{ ImGui::GetStyleColorVec4(ImGuiCol_Text) };

			if (!active) {
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
			}

			ImGui::BeginChild("name", ImVec2{ 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
			{
				const ImVec2 cursor_bak{ ImGui::GetCursorPos() };

				ImGui::InvisibleButton(bind.name.c_str(), ImGui::CalcTextSize(bind.name.c_str()));

				const bool hovered{ ui->isOpen() && ImGui::IsItemHovered() };

				ImGui::SetCursorPos(cursor_bak);

				if (hovered) {
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{});
					ImGui::Button(bind.name.c_str(), ImGui::CalcTextSize(bind.name.c_str()));
					ImGui::PopStyleVar();
				}

				else {
					ImGui::Text(bind.name.c_str());
				}

				if (ui->isOpen())
				{
					ImGui::PushID(&bind);

					if (ImGui::IsItemHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) {
						ImGui::OpenPopup("bind_popup_context");
					}

					if (ImGui::BeginPopup("bind_popup_context"))
					{
						ImGui::PushStyleColor(ImGuiCol_Text, text_clr_backup);

						ImGui::Text(bind.name.c_str());

						ImGui::Separator();

						if (ImGui::MenuItem("remove")) {
							to_remove.push_back(&bind);
							ImGui::CloseCurrentPopup();
						}

						ImGui::PopStyleColor();

						ImGui::EndPopup();
					}

					ImGui::PopID();
				}
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("key", ImVec2{ 30.0f, 0.0f }, ImGuiChildFlags_AutoResizeY);
			{
				ImGui::Text(utils::keyToStr(bind.key).c_str());
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("type", ImVec2{ 40.0f, 0.0f }, ImGuiChildFlags_AutoResizeY);
			{
				ImGui::Text(bind.type == BindType::TOGGLE ? "toggle" : "hold");
			}
			ImGui::EndChild();

			ImGui::SameLine();

			ImGui::BeginChild("value", ImVec2{ ui->isOpen() ? 35.0f : 25.0f, 0.0f }, ImGuiChildFlags_AutoResizeY);
			{
				if (!bind.selects.empty())
				{
					auto valToSelect = [&](const int val)
					{
						for (const std::pair<std::string, int> &v : bind.selects)
						{
							if (v.second != val) {
								continue;
							}

							return v;
						}

						return std::pair<std::string, int>{};
					};

					ImGui::Text(valToSelect(*static_cast<int *>(bind.var->ptr())).first.c_str());
				}

				else
				{
					if (bind.var->hash() == typeid(bool).hash_code()) {
						ImGui::Text(*static_cast<bool *>(bind.var->ptr()) ? "true" : "false");
					}

					else if (bind.var->hash() == typeid(int).hash_code()) {
						ImGui::Text(bind.fmt.c_str(), *static_cast<int *>(bind.var->ptr()));
					}

					else if (bind.var->hash() == typeid(float).hash_code()) {
						ImGui::Text(bind.fmt.c_str(), *static_cast<float *>(bind.var->ptr()));
					}
				}
			}
			ImGui::EndChild();

			if (ui->isOpen())
			{
				ImGui::SameLine();

				ImGui::BeginChild("appear_in_bindslist", ImVec2{ 25.0f, 0.0f }, ImGuiChildFlags_AutoResizeY);
				{
					ImGui::Text(bind.appear_in_bindslist ? "true" : "false");
				}
				ImGui::EndChild();
			}

			if (!active) {
				ImGui::PopStyleColor();
			}
		}

		for (Bind *const p : to_remove) {
			remove(p);
		}

		if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
			ImGui::SetWindowPos({ cfg::ui_pos_binds_x, cfg::ui_pos_binds_y });
		}

		cfg::ui_pos_binds_x = ImGui::GetWindowPos().x;
		cfg::ui_pos_binds_y = ImGui::GetWindowPos().y;

		ImGui::End();
	}
}

void Binds::run()
{
	think();
	paint();
}