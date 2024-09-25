#pragma once

#pragma region aimbot

MAKE_CFGVAR(aimbot_active, true);
MAKE_CFGVAR(aimbot_active_on_attack, false);
MAKE_CFGVAR(aimbot_auto_shoot, true);
MAKE_CFGVAR(aimbot_strafe_pred, false);
MAKE_CFGVAR(aimbot_fakelag_fix, false);

MAKE_CFGVAR(aimbot_ignore_friends, true);
MAKE_CFGVAR(aimbot_ignore_invisible, false);
MAKE_CFGVAR(aimbot_ignore_invulnerable, true);
MAKE_CFGVAR(aimbot_ignore_sentries, false);
MAKE_CFGVAR(aimbot_ignore_dispensers, false);
MAKE_CFGVAR(aimbot_ignore_teleporters, false);
MAKE_CFGVAR(aimbot_ignore_disabled_buildings, false);
MAKE_CFGVAR(aimbot_ignore_sentry_busters, true);

MAKE_CFGVAR(aimbot_hitscan_active, true);
MAKE_CFGVAR(aimbot_hitscan_mode, 0); //(0 = fov) (1 = distance)
MAKE_CFGVAR(aimbot_hitscan_aim_method, 1); //(0 = normal) (1 = smooth) (2 = silent) (3 = psilent)
MAKE_CFGVAR(aimbot_hitscan_aim_pos, 0); //(0 = auto) (1 = body) (2 = head)
MAKE_CFGVAR(aimbot_hitscan_fov, 20.0f);
MAKE_CFGVAR(aimbot_hitscan_smooth, 25.0f);
MAKE_CFGVAR(aimbot_hitscan_hitchance, 0.0f);
MAKE_CFGVAR(aimbot_hitscan_scan_hitboxes, false); 
MAKE_CFGVAR(aimbot_hitscan_wait_for_headshot, true);
MAKE_CFGVAR(aimbot_hitscan_bbox_multipoint, true);
MAKE_CFGVAR(aimbot_hitscan_target_stickies, true);
MAKE_CFGVAR(aimbot_hitscan_target_rockets_minigun_mvm, true);
MAKE_CFGVAR(aimbot_hitscan_auto_rev_minigun, false);

MAKE_CFGVAR(aimbot_melee_active, true);
MAKE_CFGVAR(aimbot_melee_mode, 1); //(0 = fov) (1 = distance)
MAKE_CFGVAR(aimbot_melee_aim_method, 1); //(0 = normal) (1 = smooth) (2 = silent) (3 = psilent)
MAKE_CFGVAR(aimbot_melee_fov, 60.0f);
MAKE_CFGVAR(aimbot_melee_smooth, 25.0f);
MAKE_CFGVAR(aimbot_melee_swing_pred, true);
MAKE_CFGVAR(aimbot_melee_swing_pred_time, 0.2f);
MAKE_CFGVAR(aimbot_melee_walk_to_target, false);

MAKE_CFGVAR(aimbot_projectile_active, true);
MAKE_CFGVAR(aimbot_projectile_mode, 0); //(0 = current fov) (1 = predicted fov) (2 = distance)
MAKE_CFGVAR(aimbot_projectile_aim_method, 1); //(0 = normal) (1 = snap) (2 = silent) (3 = psilent)
MAKE_CFGVAR(aimbot_projectile_aim_pos, 0); //(0 = auto) (1 = feet) (2 = body) (3 = head)
MAKE_CFGVAR(aimbot_projectile_fov, 30.0f);
MAKE_CFGVAR(aimbot_projectile_snap_time, 0.08f);
MAKE_CFGVAR(aimbot_projectile_snap_smooth, false);
MAKE_CFGVAR(aimbot_projectile_snap_smooth_start, 20.0f);
MAKE_CFGVAR(aimbot_projectile_snap_smooth_end, 10.0f);
MAKE_CFGVAR(aimbot_projectile_max_sim_targets, 1);
MAKE_CFGVAR(aimbot_projectile_max_sim_time, 2.0f);
MAKE_CFGVAR(aimbot_projectile_bbox_multipoint, true);
MAKE_CFGVAR(aimbot_projectile_multipoint_scale, 70);
MAKE_CFGVAR(aimbot_projectile_splash_multipoint, 2); //(0 = disabled) (1 = enabled) (2 = preferred)
MAKE_CFGVAR(aimbot_projectile_splash_on_ground_only, true);
MAKE_CFGVAR(aimbot_projectile_smooth_flamethrowers_active, false);
MAKE_CFGVAR(aimbot_projectile_smooth_flamethrowers, 25.0f);

MAKE_CFGVAR(fov_circle_active, false);
MAKE_CFGVAR(fov_circle_without_aimbot, false);
MAKE_CFGVAR(fov_circle_color, Color({ 255, 255, 255, 255 }));
MAKE_CFGVAR(fov_circle_style, 1); //(0 = static) (1 = spinning spaced)
MAKE_CFGVAR(fov_circle_spacing, 2);
MAKE_CFGVAR(fov_circle_spin_speed, 0.1f);

MAKE_CFGVAR(prediction_path_active, true);
MAKE_CFGVAR(prediction_path_no_depth, false);
MAKE_CFGVAR(prediction_path_duration, 5.0f);
MAKE_CFGVAR(prediction_path_color, Color({ 255, 255, 255, 255 }));
MAKE_CFGVAR(prediction_path_style, 1); //(0 = line) (1 = spaced line) (2 = spaced sphere) (3 = arrows)

MAKE_CFGVAR(hitboxes_active, true);
MAKE_CFGVAR(hitboxes_no_depth, true);
MAKE_CFGVAR(hitboxes_duration, 1.0f);
MAKE_CFGVAR(hitboxes_style, 2); //(0 = box) (1 = filled box) (2 = both)
MAKE_CFGVAR(hitboxes_timeout_style, 1); //(0 = instant) (1 = fade out)
MAKE_CFGVAR(hitboxes_fade_out_duration, 0.5f);
MAKE_CFGVAR(hitboxes_color, Color({ 255, 100, 100, 100 }));
MAKE_CFGVAR(hitboxes_show_target_hitbox, false);
MAKE_CFGVAR(hitboxes_target_hitbox_color, Color({ 255, 255, 255, 200 }));
MAKE_CFGVAR(hitboxes_clear_on_shot, false);

MAKE_CFGVAR(lag_comp_visuals_active, true);
MAKE_CFGVAR(lag_comp_visuals_last_only, true);
MAKE_CFGVAR(lag_comp_visuals_no_depth, false);
MAKE_CFGVAR(lag_comp_visuals_mat, 0); // (0 = shaded) (1 = flat) (2 = counter clockwise) (3 = hitboxes)
MAKE_CFGVAR(lag_comp_visuals_clr, Color({ 200, 200, 200, 200 }));

#pragma endregion

#pragma region automation

MAKE_CFGVAR(auto_backstab_active, false);
MAKE_CFGVAR(auto_backstab_legit_mode, false);
MAKE_CFGVAR(auto_backstab_aim_method, 1); //(0 = normal) (1 = silent) (2 = psilent)
MAKE_CFGVAR(auto_backstab_lethal_active, false);
MAKE_CFGVAR(auto_backstab_ignore_friends, true);
MAKE_CFGVAR(auto_backstab_ignore_invisible, false);
MAKE_CFGVAR(auto_backstab_ignore_invulnerable, true);

MAKE_CFGVAR(auto_detonate_active, false);
MAKE_CFGVAR(auto_detonate_ignore_friends, true);
MAKE_CFGVAR(auto_detonate_ignore_invisible, false);
MAKE_CFGVAR(auto_detonate_ignore_invulnerable, true);
MAKE_CFGVAR(auto_detonate_ignore_sentries, false);
MAKE_CFGVAR(auto_detonate_ignore_dispensers, false);
MAKE_CFGVAR(auto_detonate_ignore_teleporters, false);


MAKE_CFGVAR(auto_strafe_active, false);
MAKE_CFGVAR(auto_strafe_smooth, 0.5f);
MAKE_CFGVAR(auto_strafe_mode, 0); // (0 = on WASD) (1 = always)

MAKE_CFGVAR(auto_sentry_wiggler, false);
MAKE_CFGVAR(auto_sentry_wiggler_self_rodeo, false);
MAKE_CFGVAR(auto_sentry_wiggler_distance, 0.1f);

MAKE_CFGVAR(auto_vacc_active, true);
MAKE_CFGVAR(auto_vacc_prioritise_local, false);

MAKE_CFGVAR(auto_disguise, false);
MAKE_CFGVAR(auto_edge_jump_active, false);
MAKE_CFGVAR(auto_charge_turn, true);
MAKE_CFGVAR(auto_mvm_ready_up, false);
MAKE_CFGVAR(auto_reflect, false);

MAKE_CFGVAR(auto_queue_active, false);
MAKE_CFGVAR(auto_queue_mode, 0); // (0 = only in menu) (1 = always)
MAKE_CFGVAR(auto_accept_active, false);
MAKE_CFGVAR(auto_accept_mode, 0); // (0 = only in menu) (1 = always)

#pragma endregion

#pragma region crits

MAKE_CFGVAR(crits_force_crit, false);
MAKE_CFGVAR(crits_indicator, 1); // (0 = off) (1 = text)
MAKE_CFGVAR(crits_indicator_pos, 0.55f);

#pragma endregion

#pragma region anti_aim

MAKE_CFGVAR(antiaim_active, false);
MAKE_CFGVAR(antiaim_pitch, 0); // (0 = off) (1 = up) (2 = down) (3 = center) (4 = jitter)
MAKE_CFGVAR(antiaim_pitch_fake, 0); // (0 = off) (1 = up) (2 = down) (3 = match) (4 = opposite) (5 = jitter)
MAKE_CFGVAR(antiaim_yaw, 0.f);
MAKE_CFGVAR(antiaim_yaw_type, 0); // (0 = direction) (1 = spin) (2 = jitter)
MAKE_CFGVAR(antiaim_yaw_spin_speed, 10.f);
MAKE_CFGVAR(antiaim_fake_yaw, 0.f);
MAKE_CFGVAR(antiaim_fake_type, 1); // (0 = off) (1 = forward) (2 = opposite) (3 = spin) (4 = random) (5 = jitter)
MAKE_CFGVAR(antiaim_fake_spin_speed, 10.f);
MAKE_CFGVAR(antiaim_jitter_type, 0) // (0 = offset) (1 = center)
MAKE_CFGVAR(antiaim_jitter_offset, 0.f);
MAKE_CFGVAR(antiaim_render_lines, false);
MAKE_CFGVAR(antiaim_fakewalk_active, false);
MAKE_CFGVAR(antiaim_fakewalk_ticks, 21);

#pragma endregion

#pragma region fake_lag

MAKE_CFGVAR(fakelag_active, false);
MAKE_CFGVAR(fakelag_ticks, 15);
MAKE_CFGVAR(fakelag_break_lc, true);
MAKE_CFGVAR(fakelag_render_hitboxes, false);
MAKE_CFGVAR(fakelag_render_hitboxes_firstperson, true);
MAKE_CFGVAR(fakelag_clr, Color({ 200, 200, 200, 200 }));

#pragma endregion

#pragma region tick_base

MAKE_CFGVAR(tb_recharge, false);
MAKE_CFGVAR(tb_doubletap, false);
MAKE_CFGVAR(tb_warp, false);
MAKE_CFGVAR(tb_warp_ticks, 21);
MAKE_CFGVAR(tb_antiwarp_mode, 0);
MAKE_CFGVAR(tb_antiwarp_style, 0); // (0 = minimal) (1 = full) (2 = hide shot)
MAKE_CFGVAR(tb_dt_lock, false);
MAKE_CFGVAR(tb_dt_hitchance, 0.f);
MAKE_CFGVAR(tb_dt_aimbot_only, false);
MAKE_CFGVAR(tb_indicator, 1); // (0 = off) (1 = text) (2 = 2020 nitro)
MAKE_CFGVAR(tb_indicator_pos, 0.7f);
MAKE_CFGVAR(tb_indicator_lerp, false);

#pragma endregion

#pragma region seedpred

MAKE_CFGVAR(seed_pred_active, true);
MAKE_CFGVAR(seed_pred_resync_interval, 5.0f);
MAKE_CFGVAR(seed_pred_indicator, 1); // (0 = off) (1 = text)
MAKE_CFGVAR(seed_pred_indicator_pos, 0.85f);

#pragma endregion

#pragma region entity_visuals

MAKE_CFGVAR(esp_name_pos, 0); // (0 = top) (1 = right)
MAKE_CFGVAR(esp_healthbar_divisions, 1);
MAKE_CFGVAR(esp_text_color, 1); // (0 = default) (1 = custom)
MAKE_CFGVAR(esp_health_color, 0) // (0 = default) (1 = custom)

MAKE_CFGVAR(color_esp_text, Color({ 200, 200, 200, 255 }));
MAKE_CFGVAR(color_esp_outline, Color({ 10, 10, 10, 255 }));
MAKE_CFGVAR(color_esp_low_health, Color({ 255, 110, 110, 255 }));
MAKE_CFGVAR(color_esp_high_health, Color({ 137, 255, 110, 255 }));

MAKE_CFGVAR(esp_spacing_x, 2.0f);
MAKE_CFGVAR(esp_spacing_y, 2.0f);

MAKE_CFGVAR(outlines_style, 1); // (0 = blur) (1 = sharp) (2 = fat)
MAKE_CFGVAR(outlines_bloom_amount, 3);
MAKE_CFGVAR(outlines_fill_occluded, false);

#pragma endregion

#pragma region map

MAKE_CFGVAR(map_active, true)

MAKE_CFGVAR(map_draw_nav, true);
MAKE_CFGVAR(map_nav_highlight_height, true);
MAKE_CFGVAR(map_nav_color, Color({ 60, 60, 60, 255 }));

MAKE_CFGVAR(map_width, 350.0f);
MAKE_CFGVAR(map_height, 250.0f);

MAKE_CFGVAR(map_zoom, 0.2f);
MAKE_CFGVAR(map_origin, 0.8f);
MAKE_CFGVAR(map_perspective, 1.0f);

MAKE_CFGVAR(map_players_active, true);
MAKE_CFGVAR(map_players_style, 0); // (0 = circle) (1 = arrow)
MAKE_CFGVAR(map_players_ignore_local, false);
MAKE_CFGVAR(map_players_ignore_friends, false);
MAKE_CFGVAR(map_players_ignore_enemies, false);
MAKE_CFGVAR(map_players_ignore_teammates, false);

#pragma endregion

#pragma region visuals

MAKE_CFGVAR(fov_override_active, false);
MAKE_CFGVAR(fov_override_value, 120);
MAKE_CFGVAR(fov_override_value_zoomed, 120);

MAKE_CFGVAR(vm_override_active, false);
MAKE_CFGVAR(vm_override_offset_x, 0.0f);
MAKE_CFGVAR(vm_override_offset_y, 0.0f);
MAKE_CFGVAR(vm_override_offset_z, 0.0f);
MAKE_CFGVAR(vm_override_sway, 0.0f);

MAKE_CFGVAR(aspect_ratio_active, false);
MAKE_CFGVAR(aspect_ratio_value, 1.33f);

MAKE_CFGVAR(tp_override_active, true);
MAKE_CFGVAR(tp_override_force, false);
MAKE_CFGVAR(tp_override_collision, true);
MAKE_CFGVAR(tp_override_lerp_time, 0.0f);
MAKE_CFGVAR(tp_override_dist_forward, 150.0f);
MAKE_CFGVAR(tp_override_dist_right, 0.0f);
MAKE_CFGVAR(tp_override_dist_up, 0.0f);

MAKE_CFGVAR(mat_mod_active, true);
MAKE_CFGVAR(mat_mod_mode, 0); // (0 = night) (1 = color)
MAKE_CFGVAR(mat_mod_night_scale, 0.5f);
MAKE_CFGVAR(mat_mod_color_world, Color({ 150, 150, 180, 255 }));
MAKE_CFGVAR(mat_mod_color_props, Color({ 150, 150, 180, 255 }));
MAKE_CFGVAR(mat_mod_color_sky, Color({ 150, 150, 180, 255 }));

MAKE_CFGVAR(skybox_override, std::string(""));

MAKE_CFGVAR(particle_mod_active, false);
MAKE_CFGVAR(particle_mod_mode, 0); // (0 = color) (1 = rainbow)
MAKE_CFGVAR(particle_mod_color, Color({ 255, 255, 255, 255 }));
MAKE_CFGVAR(particle_mod_rainbow_speed, 0.5f);

MAKE_CFGVAR(proj_sim_visual_active, false);
MAKE_CFGVAR(proj_sim_visual_mode, 0); // (0 = color) (1 = rainbow)
MAKE_CFGVAR(proj_sim_visual_color, Color({ 255, 255, 255, 255 }));
MAKE_CFGVAR(proj_sim_visual_rainbow_speed, 0.5f);

MAKE_CFGVAR(remove_scope, true);
MAKE_CFGVAR(remove_punch, true);
MAKE_CFGVAR(remove_shake, true);
MAKE_CFGVAR(remove_fade, true);
MAKE_CFGVAR(remove_post_processing, true);
MAKE_CFGVAR(remove_fog, true);
MAKE_CFGVAR(remove_sky_fog, true);
MAKE_CFGVAR(remove_hands, true);
MAKE_CFGVAR(remove_interp, false);

MAKE_CFGVAR(reveal_scoreboard, true);
MAKE_CFGVAR(color_scoreboard, true);
MAKE_CFGVAR(static_player_model_lighting, true);
MAKE_CFGVAR(spritecard_texture_hack, false);
MAKE_CFGVAR(sniper_sight_lines, 0); // (0 = off), (1 = lasers), (2 = lines)
MAKE_CFGVAR(allow_mvm_inspect, true);
MAKE_CFGVAR(flat_textures, false);
MAKE_CFGVAR(force_low_lod_models, false);
MAKE_CFGVAR(crosshair_follows_aimbot, false);

MAKE_CFGVAR(spectator_list_active, true);
MAKE_CFGVAR(spectator_list_x, 400.0f);
MAKE_CFGVAR(spectator_list_y, 400.0f);
MAKE_CFGVAR(spectator_list_show_respawn_time, true);
MAKE_CFGVAR(spectator_list_override_firstperson, true);
MAKE_CFGVAR(spectator_list_firstperson_clr, Color(255, 255, 0, 255));

MAKE_CFGVAR(tracers_active, true);
MAKE_CFGVAR(tracers_type, 0); // 0 = beams, 1+ = particle type
MAKE_CFGVAR(tracers_mode, 0); // 0 = use color, 1 = rainbow
MAKE_CFGVAR(tracers_rainbow_speed, 0.5f);
MAKE_CFGVAR(tracers_fade, true);
MAKE_CFGVAR(tracers_color, Color({ 255, 255, 255, 255 }));

#pragma endregion

#pragma region misc

MAKE_CFGVAR(misc_bhop, true);
MAKE_CFGVAR(misc_faststop, false);
MAKE_CFGVAR(misc_sv_pure_bypass, true);
MAKE_CFGVAR(misc_noisemaker_spam, false);
MAKE_CFGVAR(misc_mvm_instant_respawn, false);
MAKE_CFGVAR(misc_rainbow_party, true);
MAKE_CFGVAR(misc_giant_weapon_sounds, false);
MAKE_CFGVAR(misc_medieval_chat, false);
MAKE_CFGVAR(misc_owo_chat, 0); // (0 = off) (1 = owo vanilla) (2 = uwu moderate) (3 = uvu unreadable)
MAKE_CFGVAR(misc_expanded_inventory, false);
MAKE_CFGVAR(misc_bypass_f2p_chat, false);

MAKE_CFGVAR(misc_damage_feedback_active, true);
MAKE_CFGVAR(misc_damage_feedback_show_invisible_spies, true);
MAKE_CFGVAR(misc_damage_feedback_show_through_walls, true);

MAKE_CFGVAR(misc_taunts_slide, true);
MAKE_CFGVAR(misc_taunts_control, true);
MAKE_CFGVAR(misc_taunts_follow_camera, false);
MAKE_CFGVAR(misc_taunts_spin, true);
MAKE_CFGVAR(misc_taunts_spin_speed, 5.0f);
MAKE_CFGVAR(misc_taunts_spin_mode, 0); // normal, random, sine, triangle

MAKE_CFGVAR(misc_remove_end_of_match_surveys, true);
MAKE_CFGVAR(misc_remove_autojoin_countdown, true);
MAKE_CFGVAR(misc_remove_equip_regions, true);
MAKE_CFGVAR(misc_remove_teammate_pushaway, false);
MAKE_CFGVAR(misc_remove_item_found_notification, true);
MAKE_CFGVAR(misc_remove_fix_angle, true);

#pragma endregion

#pragma region region_selector

MAKE_CFGVAR(region_selector_active, true);
MAKE_CFGVAR(region_selector_disable_europe, false);
MAKE_CFGVAR(region_selector_disable_na, true);
MAKE_CFGVAR(region_selector_disable_sa, false);
MAKE_CFGVAR(region_selector_disable_asia, false);
MAKE_CFGVAR(region_selector_disable_australia, false);
MAKE_CFGVAR(region_selector_disable_africa, false);

#pragma endregion

#pragma region colors

MAKE_CFGVAR(color_local, Color({ 43, 203, 186, 255 }));
MAKE_CFGVAR(color_friend, Color({ 38, 222, 129, 255 }));
MAKE_CFGVAR(color_enemy, Color({ 250, 170, 10, 255 }));
MAKE_CFGVAR(color_teammate, Color({ 252, 92, 101, 255 }));
MAKE_CFGVAR(color_target, Color({ 224, 86, 253, 255 }));
MAKE_CFGVAR(color_invuln, Color({ 165, 94, 234, 255 }));
MAKE_CFGVAR(color_invis, Color({ 209, 216, 224, 255 }));

MAKE_CFGVAR(color_overheal, Color({ 69, 170, 242, 255 }));
MAKE_CFGVAR(color_uber, Color({ 224, 86, 253, 255 }));
MAKE_CFGVAR(color_conds, Color({ 249, 202, 36, 255 }));

MAKE_CFGVAR(color_health_pack, Color({ 46, 204, 113, 255 }));
MAKE_CFGVAR(color_ammo_pack, Color({ 200, 200, 200, 255 }));
MAKE_CFGVAR(color_halloween_gift, Color({ 255, 255, 255, 255 }));
MAKE_CFGVAR(color_mvm_money, Color({ 0, 200, 20, 255 }));

MAKE_CFGVAR(color_notifs_style_message, Color({ 180, 180, 180, 255 }));
MAKE_CFGVAR(color_notifs_style_warning, Color({ 180, 180, 0, 255 }));

MAKE_CFGVAR(color_indicators_extra_good, Color({ 39, 125, 174, 255 }));
MAKE_CFGVAR(color_indicators_good, Color({ 39, 174, 96, 255 }));
MAKE_CFGVAR(color_indicators_bad, Color({ 252, 92, 101, 255 }));
MAKE_CFGVAR(color_indicators_mid, Color({ 230, 126, 34, 255 }));

#pragma endregion

#pragma region notifs

MAKE_CFGVAR(notifs_active, true);
MAKE_CFGVAR(notifs_print_to_con, true);
MAKE_CFGVAR(notifs_duration, 3.0f);
MAKE_CFGVAR(notifs_fade_in_time, 0.15f);
MAKE_CFGVAR(notifs_fade_out_time, 0.5f);

#pragma endregion

#pragma region ui	

MAKE_CFGVAR(ui_color_base, Color({ 100, 100, 100, 255 }));

MAKE_CFGVAR(ui_pos_menu_x, 400.0f);
MAKE_CFGVAR(ui_pos_menu_y, 200.0f);

MAKE_CFGVAR(ui_pos_binds_x, 20.0f);
MAKE_CFGVAR(ui_pos_binds_y, 300.0f);

MAKE_CFGVAR(ui_pos_minimap_x, 100.0f);
MAKE_CFGVAR(ui_pos_minimap_y, 100.0f);

//legacy vars for indicators
MAKE_CFGVAR(color_ui_background, Color({ 20, 20, 20, 255 }));
MAKE_CFGVAR(color_ui_outline, Color({ 40, 40, 40, 255 }));

#pragma endregion