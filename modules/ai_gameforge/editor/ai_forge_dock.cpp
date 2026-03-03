#include "ai_forge_dock.h"
#include "editor/editor_node.h"
#include "editor/file_system/editor_file_system.h"
#include "core/io/json.h"
#include "scene/gui/separator.h"
#include "scene/gui/label.h"
#include "scene/gui/scroll_container.h"
#include "scene/resources/style_box_flat.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/class_db.h"

void AIForgeDock::_bind_methods() {}

// ══════════════════════════════════════════════════════════════
// Deep Void Style Helpers
// ══════════════════════════════════════════════════════════════

static Ref<StyleBoxFlat> make_panel_style(const Color &bg, const Color &border, int radius = 4, int border_width = 1) {
    Ref<StyleBoxFlat> sb;
    sb.instantiate();
    sb->set_bg_color(bg);
    sb->set_border_color(border);
    sb->set_border_width_all(border_width);
    sb->set_corner_radius_all(radius);
    sb->set_content_margin_all(6);
    return sb;
}

static Ref<StyleBoxFlat> make_btn_style(const Color &bg, int radius = 6) {
    Ref<StyleBoxFlat> sb;
    sb.instantiate();
    sb->set_bg_color(bg);
    sb->set_corner_radius_all(radius);
    sb->set_content_margin_all(8);
    return sb;
}

// ══════════════════════════════════════════════════════════════
// CONSTRUCTOR — Deep Void Themed UI
// ══════════════════════════════════════════════════════════════

AIForgeDock::AIForgeDock() {
    set_name("AI GameForge");
    set_custom_minimum_size(Size2(320, 500));

    tab_container = memnew(TabContainer);
    tab_container->set_v_size_flags(SIZE_EXPAND_FILL);
    add_child(tab_container);

    // ─────────────────────────────────────────────────────────
    // TAB 1: GENERATE
    // ─────────────────────────────────────────────────────────
    VBoxContainer *generate_tab = memnew(VBoxContainer);
    generate_tab->set_name("Generate");
    tab_container->add_child(generate_tab);

    // ── Agent Brain Monitor ──────────────────────────────────
    Label *brain_title = memnew(Label);
    brain_title->set_text("AGENT BRAIN MONITOR");
    brain_title->add_theme_color_override("font_color", _neon_cyan());
    brain_title->add_theme_font_size_override("font_size", 11);
    generate_tab->add_child(brain_title);

    brain_monitor = memnew(VBoxContainer);
    generate_tab->add_child(brain_monitor);

    String agent_names[5] = {"Architect", "Scene Builder", "Coder", "Validator", "Finalizer"};
    Color agent_colors[5] = {
        Color(0.0f, 0.898f, 1.0f),   // Cyan
        Color(0.616f, 0.306f, 0.867f), // Purple
        Color(0.0f, 0.902f, 0.463f),   // Green
        Color(1.0f, 0.835f, 0.310f),   // Yellow
        Color(0.314f, 0.686f, 1.0f)    // Blue
    };

    for (int i = 0; i < 5; i++) {
        HBoxContainer *row = memnew(HBoxContainer);
        brain_monitor->add_child(row);

        agent_rows[i].name_label = memnew(Label);
        agent_rows[i].name_label->set_text(agent_names[i]);
        agent_rows[i].name_label->set_custom_minimum_size(Size2(80, 0));
        agent_rows[i].name_label->add_theme_color_override("font_color", agent_colors[i]);
        agent_rows[i].name_label->add_theme_font_size_override("font_size", 11);
        row->add_child(agent_rows[i].name_label);

        agent_rows[i].progress = memnew(ProgressBar);
        agent_rows[i].progress->set_min(0);
        agent_rows[i].progress->set_max(100);
        agent_rows[i].progress->set_value(0);
        agent_rows[i].progress->set_h_size_flags(SIZE_EXPAND_FILL);
        agent_rows[i].progress->set_custom_minimum_size(Size2(0, 14));
        agent_rows[i].progress->set_show_percentage(false);
        row->add_child(agent_rows[i].progress);

        agent_rows[i].status_label = memnew(Label);
        agent_rows[i].status_label->set_text("IDLE");
        agent_rows[i].status_label->set_custom_minimum_size(Size2(60, 0));
        agent_rows[i].status_label->add_theme_color_override("font_color", _text_muted());
        agent_rows[i].status_label->add_theme_font_size_override("font_size", 10);
        agent_rows[i].status_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_RIGHT);
        row->add_child(agent_rows[i].status_label);
    }

    generation_progress = memnew(ProgressBar);
    generation_progress->set_min(0);
    generation_progress->set_max(100);
    generation_progress->set_value(0);
    generation_progress->hide();
    generate_tab->add_child(generation_progress);

    generate_tab->add_child(memnew(HSeparator));

    // ── Chat History (BBCode Bubble Style) ────────────────────
    Label *chat_title = memnew(Label);
    chat_title->set_text("AI CO-PILOT CHAT");
    chat_title->add_theme_color_override("font_color", _neon_purple());
    chat_title->add_theme_font_size_override("font_size", 11);
    generate_tab->add_child(chat_title);

    ScrollContainer *chat_scroll = memnew(ScrollContainer);
    chat_scroll->set_v_size_flags(SIZE_EXPAND_FILL);
    generate_tab->add_child(chat_scroll);

    chat_history = memnew(RichTextLabel);
    chat_history->set_use_bbcode(true);
    chat_history->set_selection_enabled(true);
    chat_history->set_scroll_follow(true);
    chat_history->set_h_size_flags(SIZE_EXPAND_FILL);
    chat_history->set_v_size_flags(SIZE_EXPAND_FILL);
    chat_scroll->add_child(chat_history);

    generate_tab->add_child(memnew(HSeparator));

    // ── Multi-line Prompt Input ──────────────────────────────
    prompt_input = memnew(TextEdit);
    prompt_input->set_placeholder("Describe your dream game...\ne.g. \"A 2D platformer where a robot escapes a factory\"");
    prompt_input->set_custom_minimum_size(Size2(0, 60));
    prompt_input->set_h_size_flags(SIZE_EXPAND_FILL);
    prompt_input->connect("text_changed", callable_mp(this, &AIForgeDock::_on_prompt_changed));
    generate_tab->add_child(prompt_input);

    char_count_label = memnew(Label);
    char_count_label->set_text("0 chars");
    char_count_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_RIGHT);
    char_count_label->add_theme_color_override("font_color", _text_muted());
    char_count_label->add_theme_font_size_override("font_size", 10);
    generate_tab->add_child(char_count_label);

    // ── Button Row ───────────────────────────────────────────
    HBoxContainer *btn_row = memnew(HBoxContainer);
    generate_tab->add_child(btn_row);

    generate_btn = memnew(Button);
    generate_btn->set_text("⚡ Generate");
    generate_btn->set_h_size_flags(SIZE_EXPAND_FILL);
    generate_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_generate_pressed));
    btn_row->add_child(generate_btn);

    stop_btn = memnew(Button);
    stop_btn->set_text("■ Stop");
    stop_btn->set_disabled(true);
    stop_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_stop_pressed));
    btn_row->add_child(stop_btn);

    refine_btn = memnew(Button);
    refine_btn->set_text("✦ Refine");
    refine_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_refine_pressed));
    btn_row->add_child(refine_btn);

    clear_chat_btn = memnew(Button);
    clear_chat_btn->set_text("✕");
    clear_chat_btn->set_tooltip_text("Clear Chat");
    clear_chat_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_clear_chat));
    btn_row->add_child(clear_chat_btn);

    // ── Advanced Options (Collapsible) ───────────────────────
    toggle_advanced_btn = memnew(Button);
    toggle_advanced_btn->set_text("▸ Advanced Options");
    toggle_advanced_btn->set_flat(true);
    toggle_advanced_btn->add_theme_color_override("font_color", _text_muted());
    toggle_advanced_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_toggle_advanced));
    generate_tab->add_child(toggle_advanced_btn);

    advanced_options_panel = memnew(VBoxContainer);
    advanced_options_panel->hide();
    generate_tab->add_child(advanced_options_panel);

    // Complexity
    HBoxContainer *complexity_row = memnew(HBoxContainer);
    advanced_options_panel->add_child(complexity_row);
    Label *c_lbl = memnew(Label);
    c_lbl->set_text("Complexity:");
    c_lbl->set_custom_minimum_size(Size2(90, 0));
    complexity_row->add_child(c_lbl);
    complexity_select = memnew(OptionButton);
    complexity_select->add_item("Simple");
    complexity_select->add_item("Medium");
    complexity_select->add_item("Complex");
    complexity_select->add_item("Expert");
    complexity_select->select(1);
    complexity_select->set_h_size_flags(SIZE_EXPAND_FILL);
    complexity_row->add_child(complexity_select);

    // Target Platform
    HBoxContainer *plat_row = memnew(HBoxContainer);
    advanced_options_panel->add_child(plat_row);
    Label *p_lbl = memnew(Label);
    p_lbl->set_text("Platform:");
    p_lbl->set_custom_minimum_size(Size2(90, 0));
    plat_row->add_child(p_lbl);
    target_platform = memnew(OptionButton);
    target_platform->add_item("Desktop");
    target_platform->add_item("Mobile");
    target_platform->add_item("Web");
    target_platform->set_h_size_flags(SIZE_EXPAND_FILL);
    plat_row->add_child(target_platform);

    // Checkboxes
    HBoxContainer *cb_row = memnew(HBoxContainer);
    advanced_options_panel->add_child(cb_row);
    include_audio = memnew(CheckBox);
    include_audio->set_text("Audio");
    include_audio->set_pressed(true);
    cb_row->add_child(include_audio);
    include_particles = memnew(CheckBox);
    include_particles->set_text("Particles");
    include_particles->set_pressed(true);
    cb_row->add_child(include_particles);

    // Max Nodes
    HBoxContainer *nodes_row = memnew(HBoxContainer);
    advanced_options_panel->add_child(nodes_row);
    Label *n_lbl = memnew(Label);
    n_lbl->set_text("Max Nodes:");
    n_lbl->set_custom_minimum_size(Size2(90, 0));
    nodes_row->add_child(n_lbl);
    max_nodes = memnew(SpinBox);
    max_nodes->set_min(10);
    max_nodes->set_max(500);
    max_nodes->set_value(50);
    max_nodes->set_step(10);
    max_nodes->set_h_size_flags(SIZE_EXPAND_FILL);
    nodes_row->add_child(max_nodes);

    // ─────────────────────────────────────────────────────────
    // TAB 2: TEMPLATES
    // ─────────────────────────────────────────────────────────
    VBoxContainer *templates_tab = memnew(VBoxContainer);
    templates_tab->set_name("Templates");
    tab_container->add_child(templates_tab);

    Label *tmpl_header = memnew(Label);
    tmpl_header->set_text("QUICK-START TEMPLATES");
    tmpl_header->add_theme_color_override("font_color", _neon_cyan());
    tmpl_header->add_theme_font_size_override("font_size", 11);
    templates_tab->add_child(tmpl_header);

    templates_tab->add_child(memnew(HSeparator));

    auto add_tmpl = [&](Button **btn, const String &name, const String &desc, const String &prompt) {
        VBoxContainer *item = memnew(VBoxContainer);
        templates_tab->add_child(item);

        *btn = memnew(Button);
        (*btn)->set_text(name);
        (*btn)->connect("pressed", callable_mp(this, &AIForgeDock::_on_template_pressed).bind(prompt));
        item->add_child(*btn);

        Label *d = memnew(Label);
        d->set_text(desc);
        d->add_theme_font_size_override("font_size", 11);
        d->add_theme_color_override("font_color", _text_muted());
        d->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
        item->add_child(d);
    };

    add_tmpl(&btn_template_platformer, "🏃 2D Platformer",
        "Side-scrolling platformer with coins, enemies, and moving platforms.",
        "A classic 2D platformer with coin collection, a moving platform, and a simple enemy logic.");
    add_tmpl(&btn_template_rpg, "⚔️ Top-Down RPG",
        "Explore a world with player movement, inventory, and NPC dialogue.",
        "A top-down 2D RPG with a player controller, an inventory system, and an NPC interaction zone.");
    add_tmpl(&btn_template_puzzle, "🧩 Puzzle Game",
        "Match-3 puzzle with an 8x8 gem grid and swap mechanics.",
        "A match-3 puzzle game with an 8x8 grid filled with gems and basic swapping logic.");
    add_tmpl(&btn_template_runner, "🏃‍♂️ Endless Runner",
        "Side-scrolling runner with procedural obstacles and parallax layers.",
        "An endless runner side-scroller with procedural obstacle generation and parallax background.");
    add_tmpl(&btn_template_tower, "🏰 Tower Defense",
        "Place towers along a path to defend against waves of enemies.",
        "A simple tower defense game with a path for enemies and a placable tower that shoots projectiles.");

    templates_tab->add_child(memnew(HSeparator));

    Label *custom_lbl = memnew(Label);
    custom_lbl->set_text("CREATE CUSTOM TEMPLATE:");
    custom_lbl->add_theme_color_override("font_color", _neon_purple());
    custom_lbl->add_theme_font_size_override("font_size", 11);
    templates_tab->add_child(custom_lbl);

    custom_template_input = memnew(TextEdit);
    custom_template_input->set_placeholder("Describe your reusable template...");
    custom_template_input->set_custom_minimum_size(Size2(0, 50));
    templates_tab->add_child(custom_template_input);

    save_template_btn = memnew(Button);
    save_template_btn->set_text("💾 Save Template");
    save_template_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_save_template));
    templates_tab->add_child(save_template_btn);

    // ─────────────────────────────────────────────────────────
    // TAB 3: SETTINGS
    // ─────────────────────────────────────────────────────────
    VBoxContainer *settings_tab = memnew(VBoxContainer);
    settings_tab->set_name("Settings");
    tab_container->add_child(settings_tab);

    Label *prov_lbl = memnew(Label);
    prov_lbl->set_text("AI PROVIDER");
    prov_lbl->add_theme_color_override("font_color", _neon_cyan());
    prov_lbl->add_theme_font_size_override("font_size", 11);
    settings_tab->add_child(prov_lbl);

    provider_select = memnew(OptionButton);
    provider_select->add_item("Claude 3.5 Sonnet (Anthropic)");
    provider_select->add_item("GPT-4o (OpenAI)");
    provider_select->add_item("Gemini 1.5 Flash (Google)");
    provider_select->add_item("Gemini 2.5 Pro (Google)");
    provider_select->add_item("Local AI (AirLLM)");
    settings_tab->add_child(provider_select);

    settings_tab->add_child(memnew(HSeparator));

    Label *key_lbl = memnew(Label);
    key_lbl->set_text("API KEY");
    key_lbl->add_theme_color_override("font_color", _neon_cyan());
    key_lbl->add_theme_font_size_override("font_size", 11);
    settings_tab->add_child(key_lbl);

    api_key_input = memnew(LineEdit);
    api_key_input->set_placeholder("sk-ant-... or sk-... or AIza...");
    api_key_input->set_secret(true);
    settings_tab->add_child(api_key_input);

    settings_tab->add_child(memnew(HSeparator));

    Label *url_lbl = memnew(Label);
    url_lbl->set_text("BACKEND URL");
    url_lbl->add_theme_color_override("font_color", _neon_cyan());
    url_lbl->add_theme_font_size_override("font_size", 11);
    settings_tab->add_child(url_lbl);

    backend_url_input = memnew(LineEdit);
    backend_url_input->set_text("http://127.0.0.1:8742");
    settings_tab->add_child(backend_url_input);

    HBoxContainer *conn_row = memnew(HBoxContainer);
    settings_tab->add_child(conn_row);
    test_connection_btn = memnew(Button);
    test_connection_btn->set_text("🔌 Test Connection");
    test_connection_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_test_connection));
    conn_row->add_child(test_connection_btn);
    connection_status = memnew(Label);
    connection_status->set_text("  Not tested");
    connection_status->add_theme_color_override("font_color", _text_muted());
    conn_row->add_child(connection_status);

    settings_tab->add_child(memnew(HSeparator));

    Label *temp_lbl = memnew(Label);
    temp_lbl->set_text("AI CREATIVITY (TEMPERATURE)");
    temp_lbl->add_theme_color_override("font_color", _neon_cyan());
    temp_lbl->add_theme_font_size_override("font_size", 11);
    settings_tab->add_child(temp_lbl);

    HBoxContainer *temp_row = memnew(HBoxContainer);
    settings_tab->add_child(temp_row);
    temperature_slider = memnew(HSlider);
    temperature_slider->set_min(0.0);
    temperature_slider->set_max(1.0);
    temperature_slider->set_step(0.05);
    temperature_slider->set_value(0.7);
    temperature_slider->set_h_size_flags(SIZE_EXPAND_FILL);
    temperature_slider->connect("value_changed", callable_mp(this, &AIForgeDock::_on_temperature_changed));
    temp_row->add_child(temperature_slider);
    temperature_label = memnew(Label);
    temperature_label->set_text("0.70");
    temperature_label->set_custom_minimum_size(Size2(40, 0));
    temp_row->add_child(temperature_label);

    settings_tab->add_child(memnew(HSeparator));

    save_settings_btn = memnew(Button);
    save_settings_btn->set_text("💾 Save Settings");
    save_settings_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_save_settings));
    settings_tab->add_child(save_settings_btn);

    // ─────────────────────────────────────────────────────────
    // TAB 4: HISTORY
    // ─────────────────────────────────────────────────────────
    VBoxContainer *history_tab = memnew(VBoxContainer);
    history_tab->set_name("History");
    tab_container->add_child(history_tab);

    Label *hist_lbl = memnew(Label);
    hist_lbl->set_text("GENERATION HISTORY");
    hist_lbl->add_theme_color_override("font_color", _neon_cyan());
    hist_lbl->add_theme_font_size_override("font_size", 11);
    history_tab->add_child(hist_lbl);

    generation_history = memnew(ItemList);
    generation_history->set_v_size_flags(SIZE_EXPAND_FILL);
    generation_history->connect("item_selected", callable_mp(this, &AIForgeDock::_on_history_selected));
    history_tab->add_child(generation_history);

    clear_history_btn = memnew(Button);
    clear_history_btn->set_text("🗑 Clear History");
    clear_history_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_clear_history));
    history_tab->add_child(clear_history_btn);

    // ─────────────────────────────────────────────────────────
    // HTTP + SSE
    // ─────────────────────────────────────────────────────────
    http_client = memnew(AIHttpClient);
    add_child(http_client);
    http_client->connect("generation_complete", callable_mp(this, &AIForgeDock::_on_generation_complete));
    http_client->connect("generation_failed", callable_mp(this, &AIForgeDock::_on_generation_failed));
    http_client->connect("health_check_result", callable_mp(this, &AIForgeDock::_on_health_check_result));

    sse_client = memnew(AISSEClient);
    add_child(sse_client);
    sse_client->connect("agent_step", callable_mp(this, &AIForgeDock::_on_agent_step));

    _load_settings();
}

void AIForgeDock::setup(const String &project_path) {
    current_project_path = project_path;
}

// ══════════════════════════════════════════════════════════════
// AGENT BRAIN MONITOR
// ══════════════════════════════════════════════════════════════

void AIForgeDock::_reset_agent_monitor() {
    for (int i = 0; i < 5; i++) {
        agent_rows[i].progress->set_value(0);
        agent_rows[i].status_label->set_text("IDLE");
        agent_rows[i].status_label->add_theme_color_override("font_color", _text_muted());
    }
    current_agent_index = 0;
}

void AIForgeDock::_update_agent_monitor(const String &agent, int status, float progress) {
    // Map agent name to index
    int idx = -1;
    if (agent.containsn("architect")) { idx = 0; }
    else if (agent.containsn("scene") || agent.containsn("builder")) { idx = 1; }
    else if (agent.containsn("coder") || agent.containsn("code")) { idx = 2; }
    else if (agent.containsn("valid")) { idx = 3; }
    else if (agent.containsn("final") || agent.containsn("complete")) { idx = 4; }
    else { idx = MIN(current_agent_index, 4); }

    if (idx < 0 || idx >= 5) { return; }

    agent_rows[idx].progress->set_value(progress);

    String status_text;
    Color status_color;
    switch (status) {
        case 0: status_text = "PENDING"; status_color = _text_muted(); break;
        case 1: status_text = "ACTIVE"; status_color = _warning_yellow(); break;
        case 2: status_text = "DONE"; status_color = _success_green(); break;
        case 3: status_text = "ERROR"; status_color = _error_red(); break;
        default: status_text = "IDLE"; status_color = _text_muted(); break;
    }

    agent_rows[idx].status_label->set_text(status_text);
    agent_rows[idx].status_label->add_theme_color_override("font_color", status_color);

    if (status == 2 && idx == current_agent_index) {
        current_agent_index++;
    }
}

// ══════════════════════════════════════════════════════════════
// GENERATE TAB HANDLERS
// ══════════════════════════════════════════════════════════════

void AIForgeDock::_on_generate_pressed() {
    String prompt = prompt_input->get_text().strip_edges();
    if (prompt.is_empty()) { return; }

    is_generating = true;
    generate_btn->set_disabled(true);
    stop_btn->set_disabled(false);
    generation_progress->show();
    generation_progress->set_value(0);
    _reset_agent_monitor();

    _add_chat_message("user", prompt);
    _add_history_entry(prompt);

    Dictionary request;
    request["prompt"] = prompt;
    request["project_root"] = current_project_path;
    request["provider"] = provider_select->get_item_text(provider_select->get_selected());
    request["api_key"] = api_key_input->get_text();
    request["mode"] = "generate";
    request["complexity"] = complexity_select->get_item_text(complexity_select->get_selected());
    request["target_platform"] = target_platform->get_item_text(target_platform->get_selected());
    request["include_audio"] = include_audio->is_pressed();
    request["include_particles"] = include_particles->is_pressed();
    request["max_nodes"] = (int)max_nodes->get_value();
    request["temperature"] = temperature_slider->get_value();

    http_client->send_generate_request(request);
    prompt_input->set_text("");
    char_count_label->set_text("0 chars");
}

void AIForgeDock::_on_stop_pressed() {
    if (is_generating) {
        sse_client->cancel();
        is_generating = false;
        generate_btn->set_disabled(false);
        stop_btn->set_disabled(true);
        generation_progress->hide();
        _add_chat_message("ai", "[color=#FFD54F]■ Generation stopped by user.[/color]");
    }
}

void AIForgeDock::_on_refine_pressed() {
    String prompt = prompt_input->get_text().strip_edges();
    if (prompt.is_empty()) { return; }

    _add_chat_message("user", "[Refinement] " + prompt);
    _add_history_entry("[Refine] " + prompt);

    Dictionary request;
    request["prompt"] = prompt;
    request["project_root"] = current_project_path;
    request["provider"] = provider_select->get_item_text(provider_select->get_selected());
    request["api_key"] = api_key_input->get_text();
    request["mode"] = "refine";
    request["temperature"] = temperature_slider->get_value();

    http_client->send_generate_request(request);
    prompt_input->set_text("");
    char_count_label->set_text("0 chars");
}

void AIForgeDock::_on_prompt_changed() {
    int len = prompt_input->get_text().length();
    char_count_label->set_text(itos(len) + " chars");
}

void AIForgeDock::_on_clear_chat() {
    chat_history->clear();
    _reset_agent_monitor();
}

void AIForgeDock::_on_toggle_advanced() {
    advanced_visible = !advanced_visible;
    if (advanced_visible) {
        advanced_options_panel->show();
        toggle_advanced_btn->set_text("▾ Advanced Options");
    } else {
        advanced_options_panel->hide();
        toggle_advanced_btn->set_text("▸ Advanced Options");
    }
}

// ══════════════════════════════════════════════════════════════
// GENERATION RESULT HANDLERS
// ══════════════════════════════════════════════════════════════

void AIForgeDock::_on_generation_complete(const Dictionary &result) {
    String session_id = result.get("session_id", "");
    String url = backend_url_input->get_text().strip_edges();
    if (!session_id.is_empty()) {
        sse_client->connect_to_stream(url + "/stream/" + session_id);
    }
}

void AIForgeDock::_on_generation_failed(const String &error) {
    is_generating = false;
    generate_btn->set_disabled(false);
    stop_btn->set_disabled(true);
    generation_progress->hide();
    _add_chat_message("ai", "[color=#FF1744]✗ Error: " + error + "[/color]");
}

void AIForgeDock::_on_agent_step(const String &step, const String &message, int status) {
    // Update Brain Monitor
    float prog = (status == 2) ? 100.0f : (status == 1 ? 50.0f : 0.0f);
    _update_agent_monitor(step, status, prog);

    // Update overall progress
    if (status == 2) {
        generation_progress->set_value(generation_progress->get_value() + 20.0f);
    }

    // Update chat with styled agent step
    String color_hex;
    String icon;
    switch (status) {
        case 0: color_hex = "#636A80"; icon = "○"; break;
        case 1: color_hex = "#FFD54F"; icon = "◉"; break;
        case 2: color_hex = "#00E676"; icon = "●"; break;
        case 3: color_hex = "#FF1744"; icon = "✗"; break;
        default: color_hex = "#636A80"; icon = "?"; break;
    }

    chat_history->append_text("[color=" + color_hex + "]" + icon + " " + step + "[/color]\n" +
                              "[color=#636A80]  " + message + "[/color]\n");

    // Final completion
    if (step == "Complete" && status == 2) {
        is_generating = false;
        generate_btn->set_disabled(false);
        stop_btn->set_disabled(true);
        generation_progress->hide();
        _add_chat_message("ai", "[color=#00E676]✓ Game generation complete![/color]");

        EditorFileSystem::get_singleton()->scan();
        EditorInterface::get_singleton()->open_scene_from_path("res://scenes/main.tscn");
    } else if (status == 3) {
        is_generating = false;
        generate_btn->set_disabled(false);
        stop_btn->set_disabled(true);
        generation_progress->hide();
        _add_chat_message("ai", "[color=#FF1744]✗ Generation Failed: " + message + "[/color]");
    }
}

// ══════════════════════════════════════════════════════════════
// CHAT BUBBLE MESSAGES
// ══════════════════════════════════════════════════════════════

void AIForgeDock::_add_chat_message(const String &role, const String &message) {
    if (role == "user") {
        // User bubble — neon cyan accent
        chat_history->append_text(
            "[color=#00E5FF][b]You ▸[/b][/color] " + message + "\n\n"
        );
    } else {
        // AI bubble — purple accent with agent label
        chat_history->append_text(
            "[color=#9D4EDD][b]AI Co-Pilot ▸[/b][/color] " + message + "\n\n"
        );
    }
}

// ══════════════════════════════════════════════════════════════
// TEMPLATES
// ══════════════════════════════════════════════════════════════

void AIForgeDock::_on_template_pressed(const String &prompt) {
    prompt_input->set_text(prompt);
    _on_prompt_changed();
    tab_container->set_current_tab(0);
}

void AIForgeDock::_on_save_template() {
    String text = custom_template_input->get_text().strip_edges();
    if (text.is_empty()) { return; }
    Ref<ConfigFile> config;
    config.instantiate();
    config->load("user://ai_gameforge_settings.cfg");
    int idx = config->get_value("templates", "count", 0);
    config->set_value("templates", "template_" + itos(idx), text);
    config->set_value("templates", "count", idx + 1);
    config->save("user://ai_gameforge_settings.cfg");
    custom_template_input->set_text("");
}

// ══════════════════════════════════════════════════════════════
// SETTINGS
// ══════════════════════════════════════════════════════════════

void AIForgeDock::_on_test_connection() {
    connection_status->set_text("  Testing...");
    connection_status->add_theme_color_override("font_color", _warning_yellow());
    String url = backend_url_input->get_text().strip_edges();
    http_client->set_backend_url(url);
    http_client->send_health_check(url + "/health");
}

void AIForgeDock::_on_health_check_result(bool success, const String &message) {
    if (success) {
        connection_status->set_text("  ✅ Connected");
        connection_status->add_theme_color_override("font_color", _success_green());
    } else {
        connection_status->set_text("  ❌ " + message);
        connection_status->add_theme_color_override("font_color", _error_red());
    }
}

void AIForgeDock::_on_temperature_changed(double value) {
    temperature_label->set_text(String::num(value, 2));
}

void AIForgeDock::_on_save_settings() {
    Ref<ConfigFile> config;
    config.instantiate();
    config->set_value("ai", "provider", provider_select->get_selected());
    config->set_value("ai", "api_key", api_key_input->get_text());
    config->set_value("ai", "backend_url", backend_url_input->get_text());
    config->set_value("ai", "temperature", temperature_slider->get_value());
    config->set_value("generation", "complexity", complexity_select->get_selected());
    config->set_value("generation", "platform", target_platform->get_selected());
    config->set_value("generation", "include_audio", include_audio->is_pressed());
    config->set_value("generation", "include_particles", include_particles->is_pressed());
    config->set_value("generation", "max_nodes", (int)max_nodes->get_value());
    config->save("user://ai_gameforge_settings.cfg");
}

void AIForgeDock::_load_settings() {
    Ref<ConfigFile> config;
    config.instantiate();
    if (config->load("user://ai_gameforge_settings.cfg") != OK) { return; }
    provider_select->select(config->get_value("ai", "provider", 0));
    api_key_input->set_text(config->get_value("ai", "api_key", ""));
    backend_url_input->set_text(config->get_value("ai", "backend_url", "http://127.0.0.1:8742"));
    temperature_slider->set_value(config->get_value("ai", "temperature", 0.7));
    _on_temperature_changed(temperature_slider->get_value());
    complexity_select->select(config->get_value("generation", "complexity", 1));
    target_platform->select(config->get_value("generation", "platform", 0));
    include_audio->set_pressed(config->get_value("generation", "include_audio", true));
    include_particles->set_pressed(config->get_value("generation", "include_particles", true));
    max_nodes->set_value(config->get_value("generation", "max_nodes", 50));
}

// ══════════════════════════════════════════════════════════════
// HISTORY
// ══════════════════════════════════════════════════════════════

void AIForgeDock::_add_history_entry(const String &prompt) {
    String truncated = prompt.length() > 60 ? prompt.substr(0, 60) + "..." : prompt;
    generation_history->add_item(truncated);
    history_prompts.push_back(prompt);
}

void AIForgeDock::_on_history_selected(int index) {
    if (index >= 0 && index < history_prompts.size()) {
        prompt_input->set_text(history_prompts[index]);
        _on_prompt_changed();
        tab_container->set_current_tab(0);
    }
}

void AIForgeDock::_on_clear_history() {
    generation_history->clear();
    history_prompts.clear();
}
