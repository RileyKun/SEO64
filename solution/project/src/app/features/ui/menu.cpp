#include "menu.hpp"
#include "widgets.hpp"
#include "ui.hpp"

#include "../notifs/notifs.hpp"
#include "../player_list/player_list.hpp"
#include "../entity_visuals/entity_visuals.hpp"
#include "../cfg.hpp"

void Menu::run()
{
	aa_hovered = false;

	if (!ui->isOpen()) {
		return;
	}

	if (ImGui::Begin("seo64", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		ImGui::SetWindowSize({ 600.0f, 460.0f }, ImGuiCond_::ImGuiCond_Once);

		if (ImGui::BeginTabBar("tabbar_main"))
		{
			enum class Tabs {
				AIMBOT, AUTOMATION, EXPLOITS, VISUALS, MISC, PLAYERS, CONFIGS
			};

			static Tabs tab{ Tabs::AIMBOT };

			if (ImGui::BeginTabItem("aimbot")) {
				tab = Tabs::AIMBOT;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("automation")) {
				tab = Tabs::AUTOMATION;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("exploits")) {
				tab = Tabs::EXPLOITS;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("visuals")) {
				tab = Tabs::VISUALS;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("misc")) {
				tab = Tabs::MISC;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("players")) {
				tab = Tabs::PLAYERS;
				ImGui::EndTabItem();
			}

			if (ImGui::BeginTabItem("configs")) {
				tab = Tabs::CONFIGS;
				ImGui::EndTabItem();
			}

			if (tab == Tabs::AIMBOT)
			{
				if (ImGui::BeginTabBar("tabbar_aimbot"))
				{
					enum class AimbotTabs {
						MAIN, VISUALS
					};

					static AimbotTabs tab{ AimbotTabs::MAIN };

					if (ImGui::BeginTabItem("main")) {
						tab = AimbotTabs::MAIN;
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("visuals")) {
						tab = AimbotTabs::VISUALS;
						ImGui::EndTabItem();
					}

					if (tab == AimbotTabs::MAIN)
					{
						ImGui::BeginChild("group_aimbot_main");
						{
							ImGui::BeginChild("group_aimbot_main0", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("aimbot_global", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("global");
									ImGui::Separator();

									toggle("active", &cfg::aimbot_active);
									toggle("active on attack", &cfg::aimbot_active_on_attack);

									ImGui::Separator();

									toggle("auto shoot", &cfg::aimbot_auto_shoot);
									toggle("strafe prediction", &cfg::aimbot_strafe_pred);

									ImGui::Separator();

									toggle("fakelag fix", &cfg::aimbot_fakelag_fix);

									ImGui::Separator();

									selectMulti("ignore",
												{
													{ "friend", &cfg::aimbot_ignore_friends },
													{ "invisible", &cfg::aimbot_ignore_invisible },
													{ "invulnerable", &cfg::aimbot_ignore_invulnerable },
													{ "sentry", &cfg::aimbot_ignore_sentries },
													{ "dispenser", &cfg::aimbot_ignore_dispensers },
													{ "teleporter", &cfg::aimbot_ignore_teleporters },
													{ "sapped building", &cfg::aimbot_ignore_disabled_buildings },
													{ "sentry buster", &cfg::aimbot_ignore_sentry_busters },
												});

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("aimbot_melee", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("melee");
									ImGui::Separator();

									toggle("active", &cfg::aimbot_melee_active);
									toggle("walk to target", &cfg::aimbot_melee_walk_to_target);

									ImGui::Separator();

									selectSingle("mode", &cfg::aimbot_melee_mode, { { "fov", 0 }, { "dist", 1 } });
									selectSingle("aim method", &cfg::aimbot_melee_aim_method, { { "normal", 0 }, { "smooth", 1 }, { "silent", 2 }, { "psilent", 3 } });
									sliderFloat("fov", &cfg::aimbot_melee_fov, 1.0f, 180.0f, "%.0f");
									sliderFloat("smooth", &cfg::aimbot_melee_smooth, 1.0f, 100.0f, "%.0f%%");

									ImGui::Separator();

									toggle("swing prediction", &cfg::aimbot_melee_swing_pred);
									sliderFloat("swing time", &cfg::aimbot_melee_swing_pred_time, 0.1f, 0.2f, "%.2fs");

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_aimbot_main1", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("aimbot_hitscan", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("hitscan");
									ImGui::Separator();

									toggle("active", &cfg::aimbot_hitscan_active);
									toggle("auto rev minigun", &cfg::aimbot_hitscan_auto_rev_minigun);

									ImGui::Separator();

									selectSingle("mode", &cfg::aimbot_hitscan_mode, { { "fov", 0 }, { "distance", 1 } });
									selectSingle("aim method", &cfg::aimbot_hitscan_aim_method, { { "normal", 0 }, { "smooth", 1 }, { "silent", 2 }, { "psilent", 3 } });
									selectSingle("aim position", &cfg::aimbot_hitscan_aim_pos, { { "auto", 0 }, { "body", 1 }, { "head", 2 } });
									sliderFloat("fov", &cfg::aimbot_hitscan_fov, 1.0f, 180.0f, "%.0f");
									sliderFloat("smooth", &cfg::aimbot_hitscan_smooth, 1.0f, 100.0f, "%.0f%%");
									sliderFloat("hitchance", &cfg::aimbot_hitscan_hitchance, 0.0f, 100.0f, "%.0f%%");

									ImGui::Separator();

									toggle("wait for headshot", &cfg::aimbot_hitscan_wait_for_headshot);

									ImGui::Separator();

									toggle("scan hitboxes", &cfg::aimbot_hitscan_scan_hitboxes);
									toggle("scan bounding box", &cfg::aimbot_hitscan_bbox_multipoint);

									ImGui::Separator();

									toggle("target stickies", &cfg::aimbot_hitscan_target_stickies);
									toggle("target rockets (mvm)", &cfg::aimbot_hitscan_target_rockets_minigun_mvm);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_aimbot_main2", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("aimbot_projectile", { 200.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("projectile");
									ImGui::Separator();

									toggle("active", &cfg::aimbot_projectile_active);
									toggle("splash on ground only", &cfg::aimbot_projectile_splash_on_ground_only);

									ImGui::Separator();

									selectSingle("mode", &cfg::aimbot_projectile_mode, { { "fov to current", 0 }, { "fov to predicted", 1 }, { "distance", 2 } });
									selectSingle("aim method", &cfg::aimbot_projectile_aim_method, { { "normal", 0 }, { "snap", 1 }, { "silent", 2 }, { "psilent", 3 } });
									selectSingle("aim position", &cfg::aimbot_projectile_aim_pos, { { "auto", 0 }, { "feet", 1 }, { "body", 2 }, { "head", 3 } });
									sliderFloat("fov", &cfg::aimbot_projectile_fov, 1.0f, 180.0f, "%.0f");

									ImGui::Separator();

									toggle("snap smooth", &cfg::aimbot_projectile_snap_smooth);
									sliderFloat("snap time", &cfg::aimbot_projectile_snap_time, 0.05f, 0.3f, "%.2fs");
									sliderFloat("snap smooth start", &cfg::aimbot_projectile_snap_smooth_start, 1.0f, 100.0f, "%.0f%%");
									sliderFloat("snap smooth end", &cfg::aimbot_projectile_snap_smooth_end, 1.0f, 100.0f, "%.0f%%");

									ImGui::Separator();

									sliderInt("max sim targets", &cfg::aimbot_projectile_max_sim_targets, 1, 6);
									sliderFloat("max sim time", &cfg::aimbot_projectile_max_sim_time, 0.5f, 5.0f, "%.1fs");

									ImGui::Separator();

									toggle("scan bounding box", &cfg::aimbot_projectile_bbox_multipoint);
									sliderInt("multipoint scale", &cfg::aimbot_projectile_multipoint_scale, 50, 100, "%d%%");
									selectSingle("scan splash points", &cfg::aimbot_projectile_splash_multipoint, { { "off", 0 }, { "on", 1 }, { "preferred", 2 } });

									ImGui::Separator();

									toggle("smooth flamethrower", &cfg::aimbot_projectile_smooth_flamethrowers_active);
									sliderFloat("smooth", &cfg::aimbot_projectile_smooth_flamethrowers, 1.0f, 100.0f, "%.0f%%");

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();
					}

					else if (tab == AimbotTabs::VISUALS)
					{
						ImGui::BeginChild("group_aimbot_visuals");
						{
							ImGui::BeginChild("group_aimbot_visuals0", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("prediction_path", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("prediction path");
									ImGui::Separator();

									toggle("active", &cfg::prediction_path_active);
									toggle("no depth", &cfg::prediction_path_no_depth);

									ImGui::Separator();

									sliderFloat("duration", &cfg::prediction_path_duration, 1.0f, 10.0f, "%.1fs");

									ImGui::Separator();

									selectSingle("style", &cfg::prediction_path_style, { { "line", 0 }, { "spaced line", 1 }, { "spaced sphere", 2 }, { "arrows", 3 }, {"spiked", 4} });

									ImGui::Separator();

									colorPicker("color", &cfg::prediction_path_color);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("fov_circle", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("fov circle");
									ImGui::Separator();

									toggle("active", &cfg::fov_circle_active);
									toggle("without aimbot", &cfg::fov_circle_without_aimbot);

									ImGui::Separator();

									selectSingle("style", &cfg::fov_circle_style, { { "static", 0 }, { "spinning spaced", 1 } });

									ImGui::Separator();

									sliderInt("spacing", &cfg::fov_circle_spacing, 2, 10);
									sliderFloat("spin speed", &cfg::fov_circle_spin_speed, 0.1f, 1.0f, "%.2f");

									ImGui::Separator();

									colorPicker("color", &cfg::fov_circle_color);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_aimbot_visuals1", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("hitboxes", { 200.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("hitboxes");
									ImGui::Separator();

									toggle("active", &cfg::hitboxes_active);
									toggle("no depth", &cfg::hitboxes_no_depth);
									toggle("clear on shot", &cfg::hitboxes_clear_on_shot);

									ImGui::Separator();

									sliderFloat("duration", &cfg::hitboxes_duration, 0.5f, 5.0f, "%.1fs");

									ImGui::Separator();

									selectSingle("style", &cfg::hitboxes_style, { { "box", 0 }, { "filled box", 1 }, { "both", 2 } });
									selectSingle("timeout style", &cfg::hitboxes_timeout_style, { { "instant", 0 }, { "fade out", 1 } });
									sliderFloat("fade out duration", &cfg::hitboxes_fade_out_duration, 0.1f, 1.0f, "%.1fs");

									ImGui::Separator();

									toggle("show target hitbox", &cfg::hitboxes_show_target_hitbox);

									ImGui::Separator();

									colorPicker("color", &cfg::hitboxes_color);
									colorPicker("target hitbox color", &cfg::hitboxes_target_hitbox_color);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_aimbot_visuals2", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("lag_comp_visuals", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("lag compensation");
									ImGui::Separator();

									toggle("active", &cfg::lag_comp_visuals_active);
									toggle("last only", &cfg::lag_comp_visuals_last_only);
									toggle("no depth", &cfg::lag_comp_visuals_no_depth);

									ImGui::Separator();

									selectSingle("material", &cfg::lag_comp_visuals_mat, { { "shaded", 0 }, { "flat", 1 }, { "outlined", 2 }, { "hitboxes", 3 } });

									ImGui::Separator();

									colorPicker("color", &cfg::lag_comp_visuals_clr);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();
					}

					ImGui::EndTabBar();
				}
			}

			else if (tab == Tabs::AUTOMATION)
			{
				ImGui::BeginChild("group_automation");
				{
					ImGui::BeginChild("group_automation0", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
					{
						ImGui::BeginChild("auto_backstab", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("auto backstab");
							ImGui::Separator();

							toggle("active", &cfg::auto_backstab_active);
							toggle("legit mode", &cfg::auto_backstab_legit_mode);
							toggle("butterknife if lethal", &cfg::auto_backstab_lethal_active);

							ImGui::Separator();

							selectMulti("ignore",
										{
											{ "friend", &cfg::auto_backstab_ignore_friends },
											{ "invisible", &cfg::auto_backstab_ignore_invisible },
											{ "invulnerable", &cfg::auto_backstab_ignore_invulnerable }
										});

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();

						ImGui::BeginChild("auto_detonate", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("auto detonate");
							ImGui::Separator();

							toggle("active", &cfg::auto_detonate_active);

							ImGui::Separator();

							selectMulti("ignore",
										{
											{ "friend", &cfg::auto_detonate_ignore_friends },
											{ "invisible", &cfg::auto_detonate_ignore_invisible },
											{ "invulnerable", &cfg::auto_detonate_ignore_invulnerable },
											{ "sentries", &cfg::auto_detonate_ignore_sentries },
											{ "dispensers", &cfg::auto_detonate_ignore_dispensers },
											{ "teleporters", &cfg::auto_detonate_ignore_teleporters },
										});

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();

						ImGui::BeginChild("auto_queue", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("auto queue");
							ImGui::Separator();

							toggle("active", &cfg::auto_queue_active);
							selectSingle("mode", &cfg::auto_queue_mode, { { "only in menu", 0 }, { "always", 1 } });

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					ImGui::SameLine();

					ImGui::BeginChild("group_automation1", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
					{
						ImGui::BeginChild("auto_vaccinator", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("auto vaccinator");
							ImGui::Separator();

							toggle("active", &cfg::auto_vacc_active);
							toggle("prioritise self", &cfg::auto_vacc_prioritise_local);

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();

						ImGui::BeginChild("auto_strafe", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("auto strafe");
							ImGui::Separator();

							toggle("active", &cfg::auto_strafe_active);

							ImGui::Separator();

							sliderFloat("smoothness", &cfg::auto_strafe_smooth, 0.0f, 1.0f, "%.2f");

							ImGui::Separator();
							selectSingle("mode", &cfg::auto_strafe_mode, { { "on WASD", 0 }, { "always", 1 } });

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();

						ImGui::BeginChild("auto_accept", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("auto accept");
							ImGui::Separator();

							toggle("active", &cfg::auto_accept_active);
							selectSingle("mode", &cfg::auto_accept_mode, { { "only in menu", 0 }, { "always", 1 } });
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					ImGui::SameLine();

					ImGui::BeginChild("group_automation2", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
					{
						ImGui::BeginChild("auto_sentry_wiggler", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("auto sentry wiggler");
							ImGui::Separator();

							toggle("active", &cfg::auto_sentry_wiggler);
							toggle("self rodeo", &cfg::auto_sentry_wiggler_self_rodeo);

							ImGui::Separator();

							sliderFloat("distance", &cfg::auto_sentry_wiggler_distance, 0.03125f, 0.5f, "%.2f");

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();

						ImGui::BeginChild("auto_other", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("other");
							ImGui::Separator();

							toggle("auto reflect", &cfg::auto_reflect);
							toggle("auto disguise", &cfg::auto_disguise);
							toggle("auto edge jump", &cfg::auto_edge_jump_active);
							toggle("auto charge turn", &cfg::auto_charge_turn);
							toggle("auto mvm ready up", &cfg::auto_mvm_ready_up);

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();
				}
				ImGui::EndChild();
			}

			else if (tab == Tabs::EXPLOITS)
			{
				ImGui::BeginChild("group_exploits");
				{
					ImGui::BeginChild("group_exploits0", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
					{
						ImGui::BeginChild("tickbase", { 200.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("tickbase");
							ImGui::Separator();

							toggle("recharge", &cfg::tb_recharge);

							ImGui::Separator();

							toggle("doubletap", &cfg::tb_doubletap);
							toggle("warp", &cfg::tb_warp);

							ImGui::Separator();

							sliderInt("warp ticks", &cfg::tb_warp_ticks, 1, 21);

							ImGui::Separator();

							selectSingle("anti warp mode", &cfg::tb_antiwarp_mode, { { "off", 0 }, { "necessary", 1 }, { "always", 2 } });
							selectSingle("anti warp style", &cfg::tb_antiwarp_style, { { "minimal", 0 }, { "full", 1 }, {"hide shot", 2} });

							ImGui::Separator();

							toggle("doubletap only", &cfg::tb_dt_lock);
							toggle("aimbot only", &cfg::tb_dt_aimbot_only);

							ImGui::Separator();

							sliderFloat("aimbot hitchance", &cfg::tb_dt_hitchance, 0.0f, 100.0f, "%.0f%%");

							ImGui::Separator();

							selectSingle("indicator", &cfg::tb_indicator, { { "off", 0 }, { "text", 1 }, { "nitro (2020)", 2 } });
							sliderFloat("indicator pos", &cfg::tb_indicator_pos, 0.0f, 1.0f, "%.2f");
							toggle("interp visual ticks", &cfg::tb_indicator_lerp);

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();

						ImGui::BeginChild("crits", { 200.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("crits");
							ImGui::Separator();

							toggle("force crit", &cfg::crits_force_crit);

							ImGui::Separator();

							selectSingle("indicator", &cfg::crits_indicator, { { "off", 0 }, { "text", 1 } });
							sliderFloat("indicator pos", &cfg::crits_indicator_pos, 0.0f, 1.0f, "%.2f");

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					ImGui::SameLine();

					ImGui::BeginChild("group_exploits1", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
					{
						ImGui::BeginChild("fakelag", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("fakelag");
							ImGui::Separator();

							toggle("active", &cfg::fakelag_active);
							sliderInt("ticks", &cfg::fakelag_ticks, 1, 21);
							toggle("break lc", &cfg::fakelag_break_lc);

							ImGui::Separator();

							toggle("render hitboxes", &cfg::fakelag_render_hitboxes);
							toggle("render in firsperson", &cfg::fakelag_render_hitboxes_firstperson);
							colorPicker("color", &cfg::fakelag_clr);

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();

						ImGui::BeginChild("seed_pred", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("seed prediction");
							ImGui::Separator();

							toggle("active", &cfg::seed_pred_active);
							sliderFloat("resync interval", &cfg::seed_pred_resync_interval, 1.0f, 5.0f, "%.1fs");

							ImGui::Separator();

							selectSingle("indicator", &cfg::seed_pred_indicator, { { "off", 0 }, { "text", 1 } });
							sliderFloat("indicator pos", &cfg::seed_pred_indicator_pos, 0.0f, 1.0f, "%.2f");

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					ImGui::SameLine();

					ImGui::BeginChild("group_exploits2", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
					{
						ImGui::BeginChild("anti_aim", { 200.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("anti aim");
							ImGui::Separator();

							toggle("active", &cfg::antiaim_active);

							ImGui::Separator();

							selectSingle("pitch", &cfg::antiaim_pitch,
							{
								{ "off", 0 }, 
								{ "up", 1 }, 
								{ "down", 2 }, 
								{ "center", 3 }, 
								{ "jitter", 4 }
							});

							selectSingle("fake pitch", &cfg::antiaim_pitch_fake,
							{ 
								{ "off", 0 },
								{ "up", 1 },
								{ "down", 2 },
								{ "match", 3 },
								{ "opposite", 4 },
								{ "jitter", 5 }
							});

							ImGui::Separator();

							selectSingle("yaw", &cfg::antiaim_yaw_type,
							{
								{ "direction", 0 },
								{ "spin", 1 },
								{ "jitter", 2 }
							});

							sliderFloat("yaw add", &cfg::antiaim_yaw, -180.0f, 180.0f, "%.0f");
							sliderFloat("yaw spin speed", &cfg::antiaim_yaw_spin_speed, 1.0f, 100.0f, "%.1f");

							ImGui::Separator();

							selectSingle("fake yaw", &cfg::antiaim_fake_type,
							{
								{ "off", 0 },
								{ "forward", 1 },
								{ "opposite", 2 },
								{ "spin", 3 },
								{ "random", 4 },
								{ "jitter", 5 }
							});

							sliderFloat("fake yaw add", &cfg::antiaim_fake_yaw, -180.0f, 180.0f, "%.0f");
							sliderFloat("fake yaw spin speed", &cfg::antiaim_fake_spin_speed, 1.0f, 100.0f, "%.1f");

							ImGui::Separator();

							selectSingle("jitter type", &cfg::antiaim_jitter_type, { { "offset", 0 }, { "center", 1 } });
							sliderFloat("jitter offset", &cfg::antiaim_jitter_offset, -180.0f, 180.0f, "%.0f");

							ImGui::Separator();

							toggle("fakewalk", &cfg::antiaim_fakewalk_active);
							sliderInt("fakewalk ticks", &cfg::antiaim_fakewalk_ticks, 7, 21);

							ImGui::Separator();

							toggle("render yaw", &cfg::antiaim_render_lines);

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();

						if (ImGui::IsItemHovered(ImGuiHoveredFlags_ForTooltip | ImGuiHoveredFlags_Stationary | ImGuiHoveredFlags_AllowWhenBlockedByActiveItem)) {
							aa_hovered = true;
						}
					}
					ImGui::EndChild();
				}
				ImGui::EndChild();
			}

			else if (tab == Tabs::VISUALS)
			{
				if (ImGui::BeginTabBar("tabbar_visuals"))
				{
					enum class VisualTabs {
						ESP, ENTITIES, MAP, OTHER, COLORS
					};

					static VisualTabs tab{ VisualTabs::ESP };

					if (ImGui::BeginTabItem("entities")) {
						tab = VisualTabs::ENTITIES;
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("map")) {
						tab = VisualTabs::MAP;
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("other")) {
						tab = VisualTabs::OTHER;
						ImGui::EndTabItem();
					}

					if (ImGui::BeginTabItem("colors")) {
						tab = VisualTabs::COLORS;
						ImGui::EndTabItem();
					}

					if (tab == VisualTabs::ENTITIES)
					{
						const std::shared_lock<std::shared_mutex> lock(entity_visuals->getMutex());

						ImGui::BeginChild("group_entities");
						{
							static EVGroup *temp_group{};

							ImGui::BeginChild("group_entities_first", { 145.0f, 0.0f });
							{
								ImGui::BeginChild("group_entities_groups", { 145.0f, ImGui::GetContentRegionAvail().y * 0.51f }, ImGuiChildFlags_Border);
								{
									ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

									ImGui::Text("groups");
									ImGui::Separator();

									static std::string new_group_name{};

									if (inputStringWithHint("##new_group", &new_group_name, "new group", "%s", ImGuiInputTextFlags_EnterReturnsTrue))
									{
										if (new_group_name.size())
										{

											EVGroup group{};
											group.name = new_group_name;

											entity_visuals->pushGroup(group);
											entity_visuals->save(utils::getFilePath() + "entity_visuals.json");

											temp_group = &entity_visuals->getGroups().back();
											new_group_name.clear();
										}
									}

									ImGui::Separator();

									std::vector<EVGroup> &groups{ entity_visuals->getGroups() };

									static size_t dragged_group_index{};

									bool any_group_hovered{};

									for (size_t i{}; i < groups.size(); ++i)
									{
										EVGroup &group{ groups[i] };

										const bool is_active{ temp_group == &group };

										ImGui::PushID(&group);

										if (is_active) {
											ImGui::PushStyleColor(ImGuiCol_Button, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
											ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
											ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImGui::GetStyleColorVec4(ImGuiCol_ButtonActive));
										}

										ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.0f, 0.5f });

										if (ImGui::Button(group.name.c_str(), ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f })) {
											temp_group = &group;
										}

										ImGui::PopStyleVar();

										if (is_active) {
											ImGui::PopStyleColor(3);
										}

										if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
										{
											dragged_group_index = i;

											ImGui::SetDragDropPayload("GROUP_REORDER", &i, sizeof(size_t));
											ImGui::Text("moving %s", group.name.c_str());
											ImGui::EndDragDropSource();
										}

										if (ImGui::BeginDragDropTarget())
										{
											if (const ImGuiPayload *const payload{ ImGui::AcceptDragDropPayload("GROUP_REORDER") })
											{
												const size_t src_index{ *(size_t *)payload->Data };

												if (src_index != i)
												{
													std::swap(groups[src_index], groups[i]);

													if (temp_group == &groups[src_index]) {
														temp_group = &groups[i];
													}

													else if (temp_group == &groups[i]) {
														temp_group = &groups[src_index];
													}

													entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
												}
											}

											ImGui::EndDragDropTarget();
										}

										if (ImGui::IsItemHovered()) {
											any_group_hovered = true;
										}

										if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
											ImGui::OpenPopup("group_context_menu");
										}

										if (ImGui::BeginPopup("group_context_menu"))
										{
											if (ImGui::MenuItem("delete")) {
												entity_visuals->deleteGroup(group.name);
												temp_group = nullptr;
											}

											ImGui::EndPopup();
										}

										ImGui::PopID();
									}

									if (!any_group_hovered && ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
										temp_group = nullptr;
									}

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("esp_settings", { 0.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

									ImGui::Text("esp settings");
									ImGui::Separator();

									selectSingle("name pos", &cfg::esp_name_pos, { { "name on top", 0 }, { "name on right", 1 } });
									selectSingle("text color", &cfg::esp_text_color, { { "default text color", 0 }, { "custom text color", 1 } });
									selectSingle("hp color", &cfg::esp_health_color, { { "default hp color", 0 }, { "custom hp color", 1 } });
									sliderInt("hp bar divisions", &cfg::esp_healthbar_divisions, 1, 10, "%d hp bar divisions");

									ImGui::Separator();
									ImGui::Text("outline settings");
									ImGui::Separator();

									selectSingle("outlines style", &cfg::outlines_style, { { "blur", 0 }, { "sharp", 1 }, { "thick", 2 } });
									sliderInt("bloom", &cfg::outlines_bloom_amount, 1, 10, "%d bloom");
									toggle("fill occluded", &cfg::outlines_fill_occluded);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_entities_classes", { 145.0f, 0.0f }, ImGuiChildFlags_Border);
							{
								ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

								ImGui::Text("classes");
								ImGui::Separator();

								auto getClassName = [](const EVClass c) -> const char *const
								{
									switch (c)
									{
										case EVClass::PLAYER: {
											return "player";
										}

										case EVClass::SENTRYGUN: {
											return "sentrygun";
										}

										case EVClass::DISPENSER: {
											return "dispenser";
										}

										case EVClass::TELEPORTER: {
											return "teleporter";
										}

										case EVClass::PROJECTILE: {
											return "projectile";
										}

										case EVClass::HEALTHPACK: {
											return "healthpack";
										}

										case EVClass::AMMOPACK: {
											return "ammopack";
										}

										case EVClass::HALLOWEEN_GIFTS: {
											return "halloween gifts";
										}

										case EVClass::MVM_MONEY: {
											return "mvm money";
										}

										case EVClass::CTF_FLAGS: {
											return "ctf flags";
										}

										default: {
											return "unknown";
										}
									}
								};

								auto classCheckbox = [&](const EVClass cur_class)
								{
									const auto as_int{ static_cast<int>(cur_class) };

									bool has_class{};

									if (temp_group) {
										has_class = temp_group->classes & as_int;
									}

									if (ImGui::Checkbox(getClassName(cur_class), &has_class))
									{
										if (temp_group)
										{
											if (has_class) {
                                                temp_group->classes |= as_int;
											}

											else {
                                                temp_group->classes &= ~as_int;
											}

											entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
										}
									}
								};

								if (temp_group)
								{
									classCheckbox(EVClass::PLAYER);
									classCheckbox(EVClass::SENTRYGUN);
									classCheckbox(EVClass::DISPENSER);
									classCheckbox(EVClass::TELEPORTER);
									classCheckbox(EVClass::PROJECTILE);
									classCheckbox(EVClass::HEALTHPACK);
									classCheckbox(EVClass::AMMOPACK);
									classCheckbox(EVClass::HALLOWEEN_GIFTS);
									classCheckbox(EVClass::MVM_MONEY);
									classCheckbox(EVClass::CTF_FLAGS);
								}

								ImGui::PopItemWidth();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_entities_conditions", { 145.0f, 0.0f }, ImGuiChildFlags_Border);
							{
								ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

								ImGui::Text("conditions");
								ImGui::Separator();

								auto getCondName = [](const EVCond c) -> const char *const
								{
									switch (c)
									{
										case EVCond::NONE: {
											return "none";
										}

										case EVCond::LOCAL: {
											return "local";
										}

										case EVCond::TEAMMATE: {
											return "teammate";
										}

										case EVCond::ENEMY: {
											return "enemy";
										}
										
										case EVCond::FRIEND: {
											return "friend";
										}

										case EVCond::INVISIBLE: {
											return "invisible";
										}

										case EVCond::INVULNERABLE: {
											return "invulnerable";
										}

										case EVCond::AIMBOT_TARGET: {
											return "aimbot target";
										}

										case EVCond::CLASS_SCOUT: {
											return "class scout";
										}

										case EVCond::CLASS_SOLDIER: {
											return "class soldier";
										}

										case EVCond::CLASS_PYRO: {
											return "class pyro";
										}

										case EVCond::CLASS_DEMOMAN: {
											return "class demoman";
										}

										case EVCond::CLASS_HEAVY: {
											return "class heavy";
										}

										case EVCond::CLASS_ENGINEER: {
											return "class engineer";
										}

										case EVCond::CLASS_MEDIC: {
											return "class medic";
										}

										case EVCond::CLASS_SNIPER: {
											return "class sniper";
										}

										case EVCond::CLASS_SPY: {
											return "class spy";
										}

										default: {
											return "unknown";
										}
									}
								};

								auto condCheckbox = [&](const EVCond c)
								{
									const auto as_int{ static_cast<int>(c) };

									bool has_cond{};

									if (temp_group) {
										has_cond = temp_group->conds & as_int;
									}

									if (ImGui::Checkbox(getCondName(c), &has_cond))
									{
										if (temp_group)
										{
											if (has_cond) {
                                                temp_group->conds |= as_int;
											}

											else {
                                                temp_group->conds &= ~as_int;
											}

											entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
										}
									}
								};

								if (temp_group)
								{
                                    ImGui::Separator();

									condCheckbox(EVCond::LOCAL);
									condCheckbox(EVCond::TEAMMATE);
									condCheckbox(EVCond::ENEMY);
									condCheckbox(EVCond::FRIEND);

									ImGui::Separator();

									condCheckbox(EVCond::INVISIBLE);
									condCheckbox(EVCond::INVULNERABLE);

									ImGui::Separator();

									condCheckbox(EVCond::AIMBOT_TARGET);

									ImGui::Separator();

									condCheckbox(EVCond::CLASS_SCOUT);
									condCheckbox(EVCond::CLASS_SOLDIER);
									condCheckbox(EVCond::CLASS_PYRO);
									condCheckbox(EVCond::CLASS_DEMOMAN);
									condCheckbox(EVCond::CLASS_HEAVY);
									condCheckbox(EVCond::CLASS_ENGINEER);
									condCheckbox(EVCond::CLASS_MEDIC);
									condCheckbox(EVCond::CLASS_SNIPER);
									condCheckbox(EVCond::CLASS_SPY);
								}

								ImGui::PopItemWidth();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_entities_effects", { 145.0f, 0.0f }, ImGuiChildFlags_Border);
							{
								ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

								ImGui::Text("effects");
								ImGui::Separator();

								auto getEffectName = [](const EVEffect e) -> const char *const
								{
									switch (e)
									{
										case EVEffect::NAME: {
											return "name";
										}

										case EVEffect::BOX: {
											return "box";
										}

										case EVEffect::HP: {
											return "health";
										}

										case EVEffect::HPBAR: {
											return "health bar";
										}

										case EVEffect::MATERIAL: {
											return "material";
										}

										case EVEffect::OUTLINE: {
											return "outline";
										}

										case EVEffect::CLASS: {
											return "class";
										}

										case EVEffect::CLASSICON: {
											return "class icon";
										}

										case EVEffect::TAGS: {
											return "tags";
										}

										case EVEffect::UBER: {
											return "uber";
										}

										case EVEffect::UBERBAR: {
											return "uber bar";
										}

										case EVEffect::STEAMID: {
											return "steam id";
										}

										case EVEffect::CONDITIONS: {
											return "conditions";
										}
										
										case EVEffect::WEAPONS: {
											return "weapons";
										}

										default: {
											return "unknown";
										}
									}
								};

								auto effectCheckbox = [&](const EVEffect e)
								{
									const auto as_int{ static_cast<int>(e) };

									bool has_effect{};

									if (temp_group) {
										has_effect = temp_group->effects & as_int;
									}

									if (ImGui::Checkbox(getEffectName(e), &has_effect))
									{
										if (temp_group)
										{
											if (has_effect) {
                                                temp_group->effects |= as_int;
                                            }

                                            else {
                                                temp_group->effects &= ~as_int;
											}

											entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
										}
									}
								};

								if (temp_group)
								{
									ImGui::BeginChild("group_visuals_entities_effects", { ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y * 0.55f }, ImGuiChildFlags_Border);
									{
										effectCheckbox(EVEffect::NAME);
										effectCheckbox(EVEffect::BOX);
										effectCheckbox(EVEffect::HP);
										effectCheckbox(EVEffect::HPBAR);
										effectCheckbox(EVEffect::CLASS);
										effectCheckbox(EVEffect::CLASSICON);
										effectCheckbox(EVEffect::TAGS);
										effectCheckbox(EVEffect::UBER);
										effectCheckbox(EVEffect::UBERBAR);
										effectCheckbox(EVEffect::STEAMID);
										effectCheckbox(EVEffect::WEAPONS);

										if (temp_group->effects & static_cast<int>(EVEffect::WEAPONS)) {
											auto getWeaponType = [](const EVWeaponType t) -> const char *const
											{
												switch (t)
												{
													case EVWeaponType::ACTIVE_NAME: {
														return "active name";
													}

													case EVWeaponType::ACTIVE_ICON: {
														return "active icon";
													}

													case EVWeaponType::ALL_NAMES: {
														return "all names";
													}

													default: {
														return "unknown";
													}
												}
											};

											ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

											ImGui::Separator();

											if (ImGui::BeginCombo("##wep_type", temp_group ? getWeaponType(temp_group->weapon_type) : "select weapon type"))
											{
												for (int i{}; i < static_cast<int>(EVWeaponType::COUNT); i++)
												{
													const EVWeaponType t{ static_cast<EVWeaponType>(i) };

													if (ImGui::Selectable(getWeaponType(t), temp_group && temp_group->weapon_type == t))
													{
														if (temp_group) {
															temp_group->weapon_type = t;

															entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
														}
													}
												}

												ImGui::EndCombo();
											}

											ImGui::PopItemWidth();
										}

										ImGui::Separator();

										ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

										if (sliderFloat("##esp_alpha", &temp_group->esp_alpha, 0.01f, 1.0f, "%.2f")) {
											entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
										}

										ImGui::PopItemWidth();
									}
									ImGui::EndChild();

									ImGui::Separator();

									ImGui::BeginChild("group_visuals_entities_materials", { ImGui::GetContentRegionAvail().x, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
									{
										ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

										auto getMatType = [](const EVMaterialType t) -> const char *const
										{
											switch (t)
											{
												case EVMaterialType::ORIGINAL: {
													return "original";
												}

												case EVMaterialType::FLAT: {
													return "flat";
												}

												case EVMaterialType::SHADED: {
													return "shaded";
												}

												case EVMaterialType::GLOSSY: {
													return "glossy";
												}

												case EVMaterialType::GLOW: {
													return "glow";
												}

												case EVMaterialType::PLASTIC: {
													return "plastic";
												}

												default: {
													return "unknown";
												}
											}
										};

										effectCheckbox(EVEffect::MATERIAL);

										ImGui::Separator();

										if (toggle("depth", &temp_group->mat_depth)) {
											entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
										}

										ImGui::Separator();

										if (ImGui::BeginCombo("##mat_type", temp_group ? getMatType(temp_group->mat_type) : "select material type"))
										{
											for (int i{}; i < static_cast<int>(EVMaterialType::COUNT); i++)
											{
												const EVMaterialType t{ static_cast<EVMaterialType>(i) };

												if (ImGui::Selectable(getMatType(t), temp_group && temp_group->mat_type == t))
												{
													if (temp_group) {
														temp_group->mat_type = t;

														entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
													}
												}
											}

											ImGui::EndCombo();
										}

										if (sliderFloat("##mat_alpha", &temp_group->mat_alpha, 0.01f, 1.0f, "%.2f")) {
											entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
										}

										ImGui::PopItemWidth();
									}
									ImGui::EndChild();

									ImGui::Separator();

									ImGui::BeginChild("group_visuals_entities_effects_outline", { ImGui::GetContentRegionAvail().x, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
									{
										ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

										effectCheckbox(EVEffect::OUTLINE);

										ImGui::Separator();

										if (sliderFloat("##outline_alpha", &temp_group->outline_alpha, 0.01f, 1.0f, "%.2f")) {
											entity_visuals->save(utils::getFilePath() + "entity_visuals.json");
										}

										ImGui::PopItemWidth();
									}
									ImGui::EndChild();
								}

								ImGui::PopItemWidth();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();
					}

					else if (tab == VisualTabs::MAP)
					{
						ImGui::BeginChild("group_map");
						{
							ImGui::BeginChild("group_map0", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("map_global", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("global");
									ImGui::Separator();

									toggle("active", &cfg::map_active);

									ImGui::Separator();

									toggle("draw nav", &cfg::map_draw_nav);
									toggle("highlight height", &cfg::map_nav_highlight_height);
									colorPicker("nav color", &cfg::map_nav_color);

									ImGui::Separator();

									sliderFloat("width", &cfg::map_width, 100.0f, 1000.0f, "%.0f");
									sliderFloat("height", &cfg::map_height, 100.0f, 1000.0f, "%.0f");

									ImGui::Separator();

									sliderFloat("zoom", &cfg::map_zoom, 0.1f, 1.0f, "%.2f");
									sliderFloat("origin", &cfg::map_origin, 0.5f, 0.95f, "%.2f");
									sliderFloat("perspective", &cfg::map_perspective, 0.0f, 2.0f, "%.2f");

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::SameLine();

								ImGui::BeginChild("map_players", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("players");

									ImGui::Separator();

									toggle("active", &cfg::map_players_active);

									selectSingle("style", &cfg::map_players_style,
									{
										{ "circle", 0 },
										{ "arrow", 1 }
									});

									ImGui::Separator();

									selectMulti("ignore",
									{
										{ "friend", &cfg::map_players_ignore_friends },
										{ "local", &cfg::map_players_ignore_local },
										{ "enemies", &cfg::map_players_ignore_enemies },
										{ "teammates", &cfg::map_players_ignore_teammates }
									});

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();
					}

					else if (tab == VisualTabs::OTHER)
					{
						ImGui::BeginChild("group_visuals_other");
						{
							ImGui::BeginChild("group_visuals_other0", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("specator_list", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("specator list");
									ImGui::Separator();

									toggle("active", &cfg::spectator_list_active);
									toggle("show respawn time", &cfg::spectator_list_show_respawn_time);

									ImGui::Separator();

									toggle("override firstperson color", &cfg::spectator_list_override_firstperson);
									colorPicker("override color", &cfg::spectator_list_firstperson_clr);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("fov_override", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("fov override");
									ImGui::Separator();

									toggle("active", &cfg::fov_override_active);

									ImGui::Separator();

									sliderInt("value", &cfg::fov_override_value, 1, 180);
									sliderInt("value zoomed", &cfg::fov_override_value_zoomed, 1, 180);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("viewmodel_override", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("viewmodel override");
									ImGui::Separator();

									toggle("active", &cfg::vm_override_active);

									ImGui::Separator();

									sliderFloat("offset x", &cfg::vm_override_offset_x, -50.0f, 50.0f, "%.1f");
									sliderFloat("offset y", &cfg::vm_override_offset_y, -50.0f, 50.0f, "%.1f");
									sliderFloat("offset z", &cfg::vm_override_offset_z, -50.0f, 50.0f, "%.1f");

									ImGui::Separator();

									sliderFloat("sway", &cfg::vm_override_sway, 0.0f, 100.0f, "%.0f%%");

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("removals", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("removals");
									ImGui::Separator();

									toggle("remove scope texture", &cfg::remove_scope);
									toggle("remove view punch", &cfg::remove_punch);
									toggle("remove view shake", &cfg::remove_shake);
									toggle("remove view fade", &cfg::remove_fade);
									toggle("remove post processing", &cfg::remove_post_processing);
									toggle("remove fog", &cfg::remove_fog);
									toggle("remove sky fog", &cfg::remove_sky_fog);
									toggle("remove hands", &cfg::remove_hands);
									toggle("remove visual interp", &cfg::remove_interp);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_visuals_other1", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("aspect_ratio", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("aspect ratio");
									ImGui::Separator();

									toggle("active", &cfg::aspect_ratio_active);

									ImGui::Separator();

									sliderFloat("value", &cfg::aspect_ratio_value, 1.0f, 2.0f, "%.2f");

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("thirdperson_override", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("thirdperson");
									ImGui::Separator();

									toggle("active", &cfg::tp_override_active);
									toggle("force", &cfg::tp_override_force);
									toggle("collision", &cfg::tp_override_collision);

									ImGui::Separator();

									sliderFloat("dist forward", &cfg::tp_override_dist_forward, 50.0f, 300.0f, "%.0f");
									sliderFloat("dist right", &cfg::tp_override_dist_right, -50.0f, 50.0f, "%.0f");
									sliderFloat("dist up", &cfg::tp_override_dist_up, -50.0f, 50.0f, "%.0f");

									ImGui::Separator();

									sliderFloat("lerp time", &cfg::tp_override_lerp_time, 0.0f, 0.5f, "%.2fs");

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("particle_modulation", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("particle modulation");
									ImGui::Separator();

									toggle("active", &cfg::particle_mod_active);

									ImGui::Separator();

									selectSingle("mode", &cfg::particle_mod_mode, { { "color", 0 }, { "rainbow", 1 } });
									sliderFloat("rainbow speed", &cfg::particle_mod_rainbow_speed, 0.1f, 5.0f, "%.2f");

									ImGui::Separator();

									colorPicker("color", &cfg::particle_mod_color);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("proj_sim_visuals", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("draw projectile arc");
									ImGui::Separator();

									toggle("active", &cfg::proj_sim_visual_active);

									ImGui::Separator();

									selectSingle("mode", &cfg::proj_sim_visual_mode, { { "color", 0 }, { "rainbow", 1 } });
									sliderFloat("rainbow speed", &cfg::proj_sim_visual_rainbow_speed, 0.1f, 5.0f, "%.2f");

									ImGui::Separator();

									colorPicker("color", &cfg::proj_sim_visual_color);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("notifications", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("notifications");
									ImGui::Separator();

									toggle("active", &cfg::notifs_active);
									toggle("print to console", &cfg::notifs_print_to_con);

									ImGui::Separator();

									sliderFloat("duration", &cfg::notifs_duration, 1.0f, 10.0f, "%.1fs");
									sliderFloat("fade in time", &cfg::notifs_fade_in_time, 0.0f, 0.1f, "%.2fs");
									sliderFloat("fade out time", &cfg::notifs_fade_out_time, 0.0f, 0.1f, "%.2fs");

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_visuals_other2", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("material_modulation", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("material modulation");
									ImGui::Separator();

									toggle("active", &cfg::mat_mod_active);

									ImGui::Separator();

									selectSingle("mode", &cfg::mat_mod_mode, { { "night", 0 }, { "color", 1 } });
									sliderFloat("night scale", &cfg::mat_mod_night_scale, 0.0f, 1.0f, "%.2f");

									ImGui::Separator();

									colorPicker("world", &cfg::mat_mod_color_world);
									colorPicker("props", &cfg::mat_mod_color_props);
									colorPicker("sky", &cfg::mat_mod_color_sky);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("tracers", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("tracers");
									ImGui::Separator();

									toggle("active", &cfg::tracers_active);
									toggle("beam fade", &cfg::tracers_fade);

									ImGui::Separator();

									selectSingle("type", &cfg::tracers_type, { { "beam", 0 }, { "white machina", 1 }, { "team machina", 2}, {"team capper", 3} });

									ImGui::Separator();

									selectSingle("color mode", &cfg::tracers_mode, { { "color", 0 }, { "rainbow", 1 } });
									sliderFloat("rainbow speed", &cfg::tracers_rainbow_speed, 0.1f, 5.0f, "%.2f");
									colorPicker("color", &cfg::tracers_color);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("other", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("other");
									ImGui::Separator();

									toggle("reveal scoreboard", &cfg::reveal_scoreboard);
									toggle("color scoreboard", &cfg::color_scoreboard);
									toggle("static player model lighting", &cfg::static_player_model_lighting);
									toggle("spritecard texture hack", &cfg::spritecard_texture_hack);
									toggle("allow mvm inspect", &cfg::allow_mvm_inspect);
									toggle("flat textures", &cfg::flat_textures);
									toggle("force low lod models", &cfg::force_low_lod_models);
									toggle("crosshair follows aimbot", &cfg::crosshair_follows_aimbot);

									ImGui::Separator();

									selectSingle("sniper lines", &cfg::sniper_sight_lines, { { "off", 0 }, { "laser", 1 }, { "line", 2 } });

									ImGui::Separator();

									inputString("skybox override", &cfg::skybox_override);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();
					}

					else if (tab == VisualTabs::COLORS)
					{
						ImGui::BeginChild("group_visuals_colors");
						{
							ImGui::BeginChild("group_visuals_colors0", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("colors_global", { 120.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("global");
									ImGui::Separator();

									colorPicker("local", &cfg::color_local);
									colorPicker("friend", &cfg::color_friend);
									colorPicker("enemy", &cfg::color_enemy);
									colorPicker("teammate", &cfg::color_teammate);
									colorPicker("target", &cfg::color_target);
									colorPicker("invuln", &cfg::color_invuln);
									colorPicker("invis", &cfg::color_invis);
									colorPicker("overheal", &cfg::color_overheal);
									colorPicker("uber", &cfg::color_uber);
									colorPicker("conds", &cfg::color_conds);
									colorPicker("health pack", &cfg::color_health_pack);
									colorPicker("ammo pack", &cfg::color_ammo_pack);
									colorPicker("halloween gift", &cfg::color_halloween_gift);
									colorPicker("mvm money", &cfg::color_mvm_money);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();

							ImGui::SameLine();

							ImGui::BeginChild("group_visuals_colors1", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::BeginChild("colors_esp", { 120.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("esp");
									ImGui::Separator();

									colorPicker("text color", &cfg::color_esp_text);
									colorPicker("low health color", &cfg::color_esp_low_health);
									colorPicker("high health color", &cfg::color_esp_high_health);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("colors_notifications", { 120.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("notifications");
									ImGui::Separator();

									colorPicker("message", &cfg::color_notifs_style_message);
									colorPicker("warning", &cfg::color_notifs_style_warning);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("colors_indicators", { 120.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("indicators");
									ImGui::Separator();

									colorPicker("extra good", &cfg::color_indicators_extra_good);
									colorPicker("good", &cfg::color_indicators_good);
									colorPicker("mid", &cfg::color_indicators_mid);
									colorPicker("bad", &cfg::color_indicators_bad);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();

								ImGui::BeginChild("colors_ui", { 120.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
								{
									ImGui::PushItemWidth(100.0f);

									ImGui::Text("ui");
									ImGui::Separator();

									colorPicker("base", &cfg::ui_color_base);

									ImGui::PopItemWidth();
								}
								ImGui::EndChild();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();
					}

					ImGui::EndTabBar();
				}
			}

			else if (tab == Tabs::MISC)
			{
				ImGui::BeginChild("group_misc");
				{
					ImGui::BeginChild("group_misc0", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
					{
						ImGui::BeginChild("misc", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::PushItemWidth(100.0f);

							ImGui::Text("misc");
							ImGui::Separator();

							toggle("bhop", &cfg::misc_bhop);
							toggle("fast stop", &cfg::misc_faststop);
							toggle("sv_pure bypass", &cfg::misc_sv_pure_bypass);
							toggle("noisemaker spam", &cfg::misc_noisemaker_spam);
							toggle("mvm instant respawn", &cfg::misc_mvm_instant_respawn);
							toggle("rainbow party", &cfg::misc_rainbow_party);
							toggle("giant weapon sounds", &cfg::misc_giant_weapon_sounds);
							toggle("expanded inventory", &cfg::misc_expanded_inventory);
							toggle("bypass f2p chat", &cfg::misc_bypass_f2p_chat);

							ImGui::Separator();

							toggle("medieval chat", &cfg::misc_medieval_chat);
							selectSingle("owo chat", &cfg::misc_owo_chat, { { "off", 0 }, { "owo", 1 }, { "uwu", 2 }, {"uvu", 3 } });

							ImGui::PopItemWidth();
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();

					ImGui::SameLine();

					ImGui::BeginChild("group_misc1", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
					{
						ImGui::BeginChild("group_misc2", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::BeginChild("taunt_slide", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::PushItemWidth(100.0f);

								ImGui::Text("taunt slide");
								ImGui::Separator();

								toggle("active", &cfg::misc_taunts_slide);
								toggle("control", &cfg::misc_taunts_control);
								toggle("spin", &cfg::misc_taunts_spin);
								toggle("follow camera", &cfg::misc_taunts_follow_camera);

								ImGui::Separator();

								sliderFloat("spin speed", &cfg::misc_taunts_spin_speed, -20.0f, 20.0f, "%.1f");
								selectSingle("spin mode", &cfg::misc_taunts_spin_mode, { { "normal", 0 }, { "random", 1 }, { "sine", 2 }, { "triangle", 3 } });

								ImGui::PopItemWidth();
							}
							ImGui::EndChild();

							ImGui::BeginChild("damage_feedback", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::PushItemWidth(100.0f);

								ImGui::Text("damage feedback");
								ImGui::Separator();

								toggle("active", &cfg::misc_damage_feedback_active);

								ImGui::Separator();

								toggle("show invisible spies", &cfg::misc_damage_feedback_show_invisible_spies);
								toggle("show through walls", &cfg::misc_damage_feedback_show_through_walls);

								ImGui::PopItemWidth();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();

						ImGui::SameLine();

						ImGui::BeginChild("group_misc3", { 0.0f, 0.0f }, ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
						{
							ImGui::BeginChild("removals", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::PushItemWidth(100.0f);

								ImGui::Text("removals");
								ImGui::Separator();

								toggle("remove end of match surveys", &cfg::misc_remove_end_of_match_surveys);
								toggle("remove autojoin countdown", &cfg::misc_remove_autojoin_countdown);
								toggle("remove equip regions", &cfg::misc_remove_equip_regions);
								toggle("remove teammate pushaway", &cfg::misc_remove_teammate_pushaway);
								toggle("remove item found notification", &cfg::misc_remove_item_found_notification);
								toggle("remove fix angle", &cfg::misc_remove_fix_angle);

								ImGui::PopItemWidth();
							}
							ImGui::EndChild();

							ImGui::BeginChild("region_selector", { 180.0f, 0.0f }, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
							{
								ImGui::PushItemWidth(100.0f);

								ImGui::Text("region selector");
								ImGui::Separator();

								toggle("active", &cfg::region_selector_active);

								ImGui::Separator();

								toggle("disable europe", &cfg::region_selector_disable_europe);
								toggle("disable na", &cfg::region_selector_disable_na);
								toggle("disable south america", &cfg::region_selector_disable_sa);
								toggle("disable asia", &cfg::region_selector_disable_asia);
								toggle("disable australia", &cfg::region_selector_disable_australia);
								toggle("disable africa", &cfg::region_selector_disable_africa);

								ImGui::PopItemWidth();
							}
							ImGui::EndChild();
						}
						ImGui::EndChild();
					}
					ImGui::EndChild();
				}
				ImGui::EndChild();
			}

			else if (tab == Tabs::PLAYERS)
			{
				ImGui::BeginChild("group_player_list");
				{
					ImGui::BeginChild("group_player_list_tags", { 200.0f, 0.0f }, ImGuiChildFlags_Border);
					{
						ImGui::PushItemWidth(100.0f);

						ImGui::Text("tags");
						ImGui::Separator();

						static std::string new_tag_name{};

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

						if (inputStringWithHint("##new_tag", &new_tag_name, "create new", "%s", ImGuiInputTextFlags_EnterReturnsTrue)) {
							PlayerListTag tag{ new_tag_name, Color{ 255,255,255,255 }, 0 };
							player_list->add(tag);
							new_tag_name.clear();
						}

						ImGui::PopItemWidth();

						ImGui::Separator();

						ImGui::BeginChild("group_player_list_existing_tags", { 0.0f, 0.0f });
						{
							const float frame_padding{ ImGui::GetStyle().FramePadding.x };

							for (const auto &[tag_name, tag_hash] : player_list->getTags())
							{
								ImGui::PushID(tag_hash);

								PlayerListTag *const tag{ player_list->findTagByHash(tag_hash) };

								// how?
								if (!tag) {
									ImGui::PopID();
									continue;
								}

								ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.0f, 0.5f });
								ImGui::Button(tag_name.c_str(), { ImGui::GetContentRegionAvail().x, 0.0f });
								ImGui::PopStyleVar();

								if (ImGui::IsItemHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) {
									ImGui::OpenPopup("tag_popup_context");
								}

								bool edit_tag{};

								if (ImGui::BeginPopup("tag_popup_context"))
								{
									ImGui::Text("tag options");

									ImGui::Separator();

									if (ImGui::MenuItem("edit")) {
										edit_tag = true;
									}

									if (ImGui::MenuItem("delete")) {
										player_list->remove(*tag);
									}

									ImGui::EndPopup();
								}

								if (edit_tag) {
									ImGui::OpenPopup("tag_popup_edit");
								}

								if (ImGui::BeginPopup("tag_popup_edit"))
								{
									ImGui::PushItemWidth(100.0f);
									ImGui::Text("edit tag");

									ImGui::Separator();

									bool save{};

									if (inputString("name", &tag->name, "%s", ImGuiInputTextFlags_EnterReturnsTrue)) {
										save = true;
									}

									if (ImGui::SliderInt("priority", &tag->priority, -1, 10, tag->priority == -1 ? "ignored" : "%d")) {
										save = true;
									}

									if (colorPicker("color", &tag->color)) {
										save = true;
									}

									if (save) {
										player_list->save(utils::getFilePath() + "player_list.json");
									}

									ImGui::PopItemWidth();
									ImGui::EndPopup();
								}


								ImGui::PopID();
							}
						}
						ImGui::EndChild();

						ImGui::PopItemWidth();
					}
					ImGui::EndChild();

					ImGui::SameLine();

					ImGui::BeginChild("group_player_list_players", { 0.0f, 0.0f }, ImGuiChildFlags_Border);
					{
						ImGui::PushItemWidth(100.0f);

						ImGui::Text("players");
						ImGui::Separator();

						static char searched_player[32]{ '\0' };

						ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);
						ImGui::InputTextWithHint(" ", "search for player", searched_player, sizeof(searched_player), ImGuiInputTextFlags_AutoSelectAll);
						ImGui::PopItemWidth();

						ImGui::Separator();

						for (int i{}; i < i::ent_list->GetHighestEntityIndex(); i++)
						{
							IClientEntity *const entity{ i::ent_list->GetClientEntity(i) };

							if (!entity) {
								continue;
							}

							if (entity->GetClassId() != class_ids::CTFPlayer) {
								continue;
							}

							C_TFPlayer *player{ static_cast<C_TFPlayer *>(entity) };

							player_info_t player_info{};

							if (!i::engine->GetPlayerInfo(player->entindex(), &player_info) || player_info.fakeplayer) {
								continue;
							}

							const std::string player_name{ utils::wideToUtf8(utils::utf8ToWide(player_info.name)) };

							if (std::string cmp{ searched_player }; cmp.size())
							{
								std::string transform{ player_name };

                                std::transform(transform.begin(), transform.end(), transform.begin(), ::tolower);
								std::transform(cmp.begin(), cmp.end(), cmp.begin(), ::tolower);

								if (transform.find(cmp) == std::string::npos) {
									continue;
								}
							}

							ImGui::PushID(player_info.friendsID);

							ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.0f, 0.5f });
							ImGui::Button(player_name.c_str(), { ImGui::GetContentRegionAvail().x, 0.0f });
							ImGui::PopStyleVar();

							if (ImGui::IsItemHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right))) {
								ImGui::OpenPopup("player_popup_context");
							}

							bool edit_player{};

							if (ImGui::BeginPopup("player_popup_context"))
							{
								ImGui::Text("player options");

								ImGui::Separator();

								if (ImGui::MenuItem("add to tag")) {
									edit_player = true;
								}

								if (ImGui::MenuItem("remove from tag")) {
									player_list->setPlayerTag(player_info.friendsID, 0);
								}

								if (ImGui::MenuItem("votekick")) {
									i::engine->ClientCmd_Unrestricted(std::format("callvote kick {}", player_info.userID).c_str());
								}

								if (ImGui::MenuItem("steam profile"))
								{
									i::steam_api_context->SteamFriends()->ActivateGameOverlayToToUser("steamid", CSteamID(
										player_info.friendsID,
										k_EUniversePublic,
										k_EAccountTypeIndividual
									));
								}

								if (ImGui::MenuItem("steam history")) {
									CSteamID id(player_info.friendsID, k_EUniversePublic, k_EAccountTypeIndividual);
									i::steam_api_context->SteamFriends()->ActivateGameOverlayToWebPage(std::format("https://steamhistory.net/id/{}", id.ConvertToUint64()).c_str());
								}

								ImGui::EndPopup();
							}

							if (edit_player) {
								ImGui::OpenPopup("player_popup_add_to_tag");
							}

							if (ImGui::BeginPopup("player_popup_add_to_tag"))
							{
								ImGui::Text("add to tag");

								ImGui::Separator();

								PlayerListTag *existing_tag{ player_list->findPlayer(player) };

								for (const auto &[tag_name, tag_hash] : player_list->getTags())
								{
									PlayerListTag *const tag{ player_list->findTagByHash(tag_hash) };

									if (!tag || tag == existing_tag) {
										continue;
									}

									if (ImGui::MenuItem(tag_name.c_str())) {
										player_list->setPlayerTag(player_info.friendsID, tag_hash);
									}
								}

								ImGui::EndPopup();
							}

							ImGui::PopID();
						}

						ImGui::PopItemWidth();
					}
					ImGui::EndChild();
				}
				ImGui::EndChild();
			}

			else if (tab == Tabs::CONFIGS)
			{
				static const std::string path{ utils::getFilePath() + "cfg\\" };

				if (!std::filesystem::exists(path)) {
					std::filesystem::create_directories(path);
				}

				static char new_cfg_name[50]{ '\0' };

				ImGui::PushItemWidth(ImGui::GetContentRegionAvail().x);

				if (ImGui::InputTextWithHint(" ", "create new", new_cfg_name, sizeof(new_cfg_name),
					ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsNoBlank)) {
					cfg::save(std::format("{}{}.json", path, new_cfg_name));
					notifs->message(std::format("created '{}'", new_cfg_name));
					memset(new_cfg_name, 0, sizeof(new_cfg_name));
				}

				ImGui::PopItemWidth();

				if (!ImGui::IsItemActivated()) {
					memset(new_cfg_name, 0, sizeof(new_cfg_name));
				}

				ImGui::BeginChild("configs", ImVec2{ ImGui::GetContentRegionAvail().x, 0.0f },
								  ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeX | ImGuiChildFlags_AutoResizeY);
				{
					ImGui::Text("existing");

					ImGui::Separator();

					std::vector<std::filesystem::path> to_remove{};

					for (const std::filesystem::directory_entry &file : std::filesystem::directory_iterator{ path })
					{
						if (!file.path().extension().string().ends_with(".json")) {
							continue;
						}

						ImGui::PushID(file.path().string().c_str());

						ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.0f, 0.5f });
						ImGui::Button(file.path().stem().string().c_str(), { floorf(ImGui::GetContentRegionAvail().x * 0.55f), 0.0f });
						ImGui::PopStyleVar();

						ImGui::PushStyleVar(ImGuiStyleVar_ButtonTextAlign, ImVec2{ 0.5f, 0.5f });

						ImGui::SameLine();

						const float w{ floorf(ImGui::GetContentRegionAvail().x * 0.3225f) };

						if (ImGui::Button("load", { w, 0.0f })) {
							cfg::load(file.path().string());
							notifs->message(std::string{ "loaded " } + std::format("'{}'", file.path().stem().string()));
						}

						ImGui::SameLine();

						if (ImGui::Button("save", { w, 0.0f })) {
							cfg::save(file.path().string());
							notifs->message(std::string{ "saved " } + std::format("'{}'", file.path().stem().string()));
						}

						ImGui::SameLine();

						if (ImGui::Button("delete", { w, 0.0f })) {
							to_remove.push_back(file.path());
							notifs->message(std::string{ "deleted " } + std::format("'{}'", file.path().stem().string()));
						}

						ImGui::PopStyleVar();

						ImGui::PopID();
					}

					for (const std::filesystem::path p : to_remove) {
						std::filesystem::remove(p);
					}
				}
				ImGui::EndChild();
			}

			ImGui::EndTabBar();
		}

		if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
			ImGui::SetWindowPos({ cfg::ui_pos_menu_x, cfg::ui_pos_menu_y });
		}

		cfg::ui_pos_menu_x = ImGui::GetWindowPos().x;
		cfg::ui_pos_menu_y = ImGui::GetWindowPos().y;

		ImGui::End();
	}
}