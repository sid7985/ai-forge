#include "ai_forge_dock.h"
#include "editor/editor_node.h"
#include "editor/file_system/editor_file_system.h"
#include "core/io/json.h"
#include "scene/gui/separator.h"
#include "scene/gui/label.h"
#include "scene/gui/scroll_container.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/class_db.h"

void AIForgeDock::_bind_methods() {}

AIForgeDock::AIForgeDock() {
    set_name("AI GameForge");
    set_custom_minimum_size(Size2(320, 500));

    tab_container = memnew(TabContainer);
    tab_container->set_v_size_flags(SIZE_EXPAND_FILL);
    add_child(tab_container);

    // =====================================================
    // TAB 1: GENERATE
    // =====================================================
    VBoxContainer *generate_tab = memnew(VBoxContainer);
    generate_tab->set_name("Generate");
    tab_container->add_child(generate_tab);

    // --- Agent Activity Log ---
    Label *log_label = memnew(Label);
    log_label->set_text("Agent Activity:");
    generate_tab->add_child(log_label);

    ScrollContainer *log_scroll = memnew(ScrollContainer);
    log_scroll->set_custom_minimum_size(Size2(0, 100));
    generate_tab->add_child(log_scroll);

    agent_log = memnew(RichTextLabel);
    agent_log->set_use_bbcode(true);
    agent_log->set_h_size_flags(SIZE_EXPAND_FILL);
    agent_log->set_v_size_flags(SIZE_EXPAND_FILL);
    log_scroll->add_child(agent_log);

    generation_progress = memnew(ProgressBar);
    generation_progress->set_min(0);
    generation_progress->set_max(100);
    generation_progress->set_value(0);
    generation_progress->hide();
    generate_tab->add_child(generation_progress);

    generate_tab->add_child(memnew(HSeparator));

    // --- Chat History ---
    ScrollContainer *chat_scroll = memnew(ScrollContainer);
    chat_scroll->set_v_size_flags(SIZE_EXPAND_FILL);
    generate_tab->add_child(chat_scroll);

    chat_history = memnew(RichTextLabel);
    chat_history->set_use_bbcode(true);
    chat_history->set_selection_enabled(true);
    chat_history->set_h_size_flags(SIZE_EXPAND_FILL);
    chat_history->set_v_size_flags(SIZE_EXPAND_FILL);
    chat_scroll->add_child(chat_history);

    generate_tab->add_child(memnew(HSeparator));

    // --- Multi-line Prompt Input ---
    prompt_input = memnew(TextEdit);
    prompt_input->set_placeholder("Describe your game in detail...\ne.g. A 2D platformer with coin collection, moving platforms, and enemy AI");
    prompt_input->set_custom_minimum_size(Size2(0, 60));
    prompt_input->set_h_size_flags(SIZE_EXPAND_FILL);
    prompt_input->connect("text_changed", callable_mp(this, &AIForgeDock::_on_prompt_changed));
    generate_tab->add_child(prompt_input);

    char_count_label = memnew(Label);
    char_count_label->set_text("0 characters");
    char_count_label->set_horizontal_alignment(HORIZONTAL_ALIGNMENT_RIGHT);
    generate_tab->add_child(char_count_label);

    // --- Button Row ---
    HBoxContainer *btn_row = memnew(HBoxContainer);
    generate_tab->add_child(btn_row);

    generate_btn = memnew(Button);
    generate_btn->set_text("Generate");
    generate_btn->set_h_size_flags(SIZE_EXPAND_FILL);
    generate_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_generate_pressed));
    btn_row->add_child(generate_btn);

    stop_btn = memnew(Button);
    stop_btn->set_text("Stop");
    stop_btn->set_disabled(true);
    stop_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_stop_pressed));
    btn_row->add_child(stop_btn);

    refine_btn = memnew(Button);
    refine_btn->set_text("Refine Scene");
    refine_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_refine_pressed));
    btn_row->add_child(refine_btn);

    clear_chat_btn = memnew(Button);
    clear_chat_btn->set_text("Clear");
    clear_chat_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_clear_chat));
    btn_row->add_child(clear_chat_btn);

    // --- Advanced Options (Collapsible) ---
    toggle_advanced_btn = memnew(Button);
    toggle_advanced_btn->set_text("Advanced Options ▸");
    toggle_advanced_btn->set_flat(true);
    toggle_advanced_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_toggle_advanced));
    generate_tab->add_child(toggle_advanced_btn);

    advanced_options_panel = memnew(VBoxContainer);
    advanced_options_panel->hide();
    generate_tab->add_child(advanced_options_panel);

    // Complexity
    HBoxContainer *complexity_row = memnew(HBoxContainer);
    advanced_options_panel->add_child(complexity_row);
    Label *complexity_lbl = memnew(Label);
    complexity_lbl->set_text("Complexity:");
    complexity_lbl->set_custom_minimum_size(Size2(100, 0));
    complexity_row->add_child(complexity_lbl);
    complexity_select = memnew(OptionButton);
    complexity_select->add_item("Simple");
    complexity_select->add_item("Medium");
    complexity_select->add_item("Complex");
    complexity_select->add_item("Expert");
    complexity_select->select(1);
    complexity_select->set_h_size_flags(SIZE_EXPAND_FILL);
    complexity_row->add_child(complexity_select);

    // Target Platform
    HBoxContainer *platform_row = memnew(HBoxContainer);
    advanced_options_panel->add_child(platform_row);
    Label *platform_lbl = memnew(Label);
    platform_lbl->set_text("Platform:");
    platform_lbl->set_custom_minimum_size(Size2(100, 0));
    platform_row->add_child(platform_lbl);
    target_platform = memnew(OptionButton);
    target_platform->add_item("Desktop");
    target_platform->add_item("Mobile");
    target_platform->add_item("Web");
    target_platform->set_h_size_flags(SIZE_EXPAND_FILL);
    platform_row->add_child(target_platform);

    // Checkboxes
    HBoxContainer *checkbox_row = memnew(HBoxContainer);
    advanced_options_panel->add_child(checkbox_row);
    include_audio = memnew(CheckBox);
    include_audio->set_text("Include Audio");
    include_audio->set_pressed(true);
    checkbox_row->add_child(include_audio);
    include_particles = memnew(CheckBox);
    include_particles->set_text("Include Particles");
    include_particles->set_pressed(true);
    checkbox_row->add_child(include_particles);

    // Max Nodes
    HBoxContainer *nodes_row = memnew(HBoxContainer);
    advanced_options_panel->add_child(nodes_row);
    Label *nodes_lbl = memnew(Label);
    nodes_lbl->set_text("Max Nodes:");
    nodes_lbl->set_custom_minimum_size(Size2(100, 0));
    nodes_row->add_child(nodes_lbl);
    max_nodes = memnew(SpinBox);
    max_nodes->set_min(10);
    max_nodes->set_max(500);
    max_nodes->set_value(50);
    max_nodes->set_step(10);
    max_nodes->set_h_size_flags(SIZE_EXPAND_FILL);
    nodes_row->add_child(max_nodes);

    // =====================================================
    // TAB 2: TEMPLATES
    // =====================================================
    VBoxContainer *templates_tab = memnew(VBoxContainer);
    templates_tab->set_name("Templates");
    tab_container->add_child(templates_tab);

    Label *tmpl_header = memnew(Label);
    tmpl_header->set_text("Quick-Start Templates");
    templates_tab->add_child(tmpl_header);

    templates_tab->add_child(memnew(HSeparator));

    auto add_template_btn = [&](Button **btn, const String &text, const String &desc, const String &prompt) {
        VBoxContainer *tmpl_item = memnew(VBoxContainer);
        templates_tab->add_child(tmpl_item);

        *btn = memnew(Button);
        (*btn)->set_text(text);
        (*btn)->connect("pressed", callable_mp(this, &AIForgeDock::_on_template_pressed).bind(prompt));
        tmpl_item->add_child(*btn);

        Label *desc_label = memnew(Label);
        desc_label->set_text(desc);
        desc_label->add_theme_font_size_override("font_size", 11);
        desc_label->set_autowrap_mode(TextServer::AUTOWRAP_WORD_SMART);
        tmpl_item->add_child(desc_label);
    };

    add_template_btn(&btn_template_platformer, "2D Platformer",
        "Classic side-scrolling platformer with coins, moving platforms, and enemies.",
        "A classic 2D platformer with coin collection, a moving platform, and a simple enemy logic.");

    add_template_btn(&btn_template_rpg, "Top-Down RPG",
        "Explore a world with player movement, inventory, and NPC dialogue.",
        "A top-down 2D RPG with a player controller, an inventory system, and an NPC interaction zone.");

    add_template_btn(&btn_template_puzzle, "Puzzle Game",
        "Match-3 puzzle with an 8x8 gem grid and swap mechanics.",
        "A match-3 puzzle game with an 8x8 grid filled with gems and basic swapping logic.");

    add_template_btn(&btn_template_runner, "Endless Runner",
        "Side-scrolling runner with procedural obstacles and parallax layers.",
        "An endless runner side-scroller with procedural obstacle generation and parallax background.");

    add_template_btn(&btn_template_tower, "Tower Defense",
        "Place towers along a path to defend against waves of enemies.",
        "A simple tower defense game with a path for enemies and a placable tower that shoots projectiles.");

    templates_tab->add_child(memnew(HSeparator));

    // Custom Template
    Label *custom_lbl = memnew(Label);
    custom_lbl->set_text("Create Custom Template:");
    templates_tab->add_child(custom_lbl);

    custom_template_input = memnew(TextEdit);
    custom_template_input->set_placeholder("Describe your reusable template...");
    custom_template_input->set_custom_minimum_size(Size2(0, 50));
    templates_tab->add_child(custom_template_input);

    save_template_btn = memnew(Button);
    save_template_btn->set_text("Save as Template");
    save_template_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_save_template));
    templates_tab->add_child(save_template_btn);

    // =====================================================
    // TAB 3: SETTINGS
    // =====================================================
    VBoxContainer *settings_tab = memnew(VBoxContainer);
    settings_tab->set_name("Settings");
    tab_container->add_child(settings_tab);

    // Provider
    Label *provider_lbl = memnew(Label);
    provider_lbl->set_text("AI Provider:");
    settings_tab->add_child(provider_lbl);

    provider_select = memnew(OptionButton);
    provider_select->add_item("Claude 3.5 Sonnet (Anthropic)");
    provider_select->add_item("GPT-4o (OpenAI)");
    provider_select->add_item("Gemini 1.5 Flash (Google)");
    provider_select->add_item("Gemini 2.5 Pro (Google)");
    provider_select->add_item("Local AI (AirLLM)");
    settings_tab->add_child(provider_select);

    settings_tab->add_child(memnew(HSeparator));

    // API Key
    Label *key_lbl = memnew(Label);
    key_lbl->set_text("API Key:");
    settings_tab->add_child(key_lbl);

    api_key_input = memnew(LineEdit);
    api_key_input->set_placeholder("sk-ant-... or sk-... or AIza...");
    api_key_input->set_secret(true);
    settings_tab->add_child(api_key_input);

    settings_tab->add_child(memnew(HSeparator));

    // Backend URL
    Label *url_lbl = memnew(Label);
    url_lbl->set_text("Backend URL:");
    settings_tab->add_child(url_lbl);

    backend_url_input = memnew(LineEdit);
    backend_url_input->set_text("http://127.0.0.1:8742");
    settings_tab->add_child(backend_url_input);

    // Connection Test
    HBoxContainer *conn_row = memnew(HBoxContainer);
    settings_tab->add_child(conn_row);

    test_connection_btn = memnew(Button);
    test_connection_btn->set_text("Test Connection");
    test_connection_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_test_connection));
    conn_row->add_child(test_connection_btn);

    connection_status = memnew(Label);
    connection_status->set_text("  Not tested");
    conn_row->add_child(connection_status);

    settings_tab->add_child(memnew(HSeparator));

    // Temperature
    Label *temp_lbl = memnew(Label);
    temp_lbl->set_text("AI Creativity (Temperature):");
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

    // Save
    save_settings_btn = memnew(Button);
    save_settings_btn->set_text("Save Settings");
    save_settings_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_save_settings));
    settings_tab->add_child(save_settings_btn);

    // =====================================================
    // TAB 4: HISTORY
    // =====================================================
    VBoxContainer *history_tab = memnew(VBoxContainer);
    history_tab->set_name("History");
    tab_container->add_child(history_tab);

    Label *hist_header = memnew(Label);
    hist_header->set_text("Generation History:");
    history_tab->add_child(hist_header);

    generation_history = memnew(ItemList);
    generation_history->set_v_size_flags(SIZE_EXPAND_FILL);
    generation_history->connect("item_selected", callable_mp(this, &AIForgeDock::_on_history_selected));
    history_tab->add_child(generation_history);

    clear_history_btn = memnew(Button);
    clear_history_btn->set_text("Clear History");
    clear_history_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_clear_history));
    history_tab->add_child(clear_history_btn);

    // =====================================================
    // HTTP + SSE CLIENTS
    // =====================================================
    http_client = memnew(AIHttpClient);
    add_child(http_client);
    http_client->connect("generation_complete", callable_mp(this, &AIForgeDock::_on_generation_complete));
    http_client->connect("generation_failed", callable_mp(this, &AIForgeDock::_on_generation_failed));
    http_client->connect("health_check_result", callable_mp(this, &AIForgeDock::_on_health_check_result));

    sse_client = memnew(AISSEClient);
    add_child(sse_client);
    sse_client->connect("agent_step", callable_mp(this, &AIForgeDock::_on_agent_step));

    // Load saved settings
    _load_settings();
}

void AIForgeDock::setup(const String &project_path) {
    current_project_path = project_path;
}

// ==========================================================
// GENERATE TAB HANDLERS
// ==========================================================

void AIForgeDock::_on_generate_pressed() {
    String prompt = prompt_input->get_text().strip_edges();
    if (prompt.is_empty()) {
        return;
    }

    is_generating = true;
    generate_btn->set_disabled(true);
    stop_btn->set_disabled(false);
    generation_progress->show();
    generation_progress->set_value(0);
    agent_log->clear();

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
    char_count_label->set_text("0 characters");
}

void AIForgeDock::_on_stop_pressed() {
    if (is_generating) {
        sse_client->cancel();
        is_generating = false;
        generate_btn->set_disabled(false);
        stop_btn->set_disabled(true);
        generation_progress->hide();
        _add_chat_message("ai", "[color=orange]Generation stopped by user.[/color]");
    }
}

void AIForgeDock::_on_refine_pressed() {
    String prompt = prompt_input->get_text().strip_edges();
    if (prompt.is_empty()) {
        return;
    }

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
    char_count_label->set_text("0 characters");
}

void AIForgeDock::_on_prompt_changed() {
    int len = prompt_input->get_text().length();
    char_count_label->set_text(itos(len) + " characters");
}

void AIForgeDock::_on_clear_chat() {
    chat_history->clear();
    agent_log->clear();
}

void AIForgeDock::_on_toggle_advanced() {
    advanced_visible = !advanced_visible;
    if (advanced_visible) {
        advanced_options_panel->show();
        toggle_advanced_btn->set_text("Advanced Options ▾");
    } else {
        advanced_options_panel->hide();
        toggle_advanced_btn->set_text("Advanced Options ▸");
    }
}

// ==========================================================
// GENERATION RESULT HANDLERS
// ==========================================================

void AIForgeDock::_on_generation_complete(const Dictionary &result) {
    String session_id = result.get("session_id", "");
    String backend_url = backend_url_input->get_text().strip_edges();
    if (!session_id.is_empty()) {
        sse_client->connect_to_stream(backend_url + "/stream/" + session_id);
    }
}

void AIForgeDock::_on_generation_failed(const String &error) {
    is_generating = false;
    generate_btn->set_disabled(false);
    stop_btn->set_disabled(true);
    generation_progress->hide();
    _add_chat_message("ai", "[color=red]Error: " + error + "[/color]");
}

void AIForgeDock::_on_agent_step(const String &step, const String &message, int status) {
    String color;
    String icon;
    switch (status) {
        case 0: color = "gray"; icon = "○"; break;
        case 1: color = "#FFB86C"; icon = "◉"; break;
        case 2: color = "#50FA7B"; icon = "●"; break;
        case 3: color = "#FF5555"; icon = "✗"; break;
        default: color = "gray"; icon = "?"; break;
    }

    agent_log->append_text("[color=" + color + "]" + icon + " " + step + "[/color]\n" +
                           "[color=gray]  " + message + "[/color]\n");

    if (status == 2) {
        generation_progress->set_value(generation_progress->get_value() + 20.0f);
    }

    if (step == "Complete" && status == 2) {
        is_generating = false;
        generate_btn->set_disabled(false);
        stop_btn->set_disabled(true);
        generation_progress->hide();
        _add_chat_message("ai", "[color=#50FA7B]Game generation complete![/color]");

        EditorFileSystem::get_singleton()->scan();
        EditorInterface::get_singleton()->open_scene_from_path("res://scenes/main.tscn");
    } else if (status == 3) {
        is_generating = false;
        generate_btn->set_disabled(false);
        stop_btn->set_disabled(true);
        generation_progress->hide();
        _add_chat_message("ai", "[color=#FF5555]Generation Failed: " + message + "[/color]");
    }
}

void AIForgeDock::_add_chat_message(const String &role, const String &message) {
    if (role == "user") {
        chat_history->append_text("[color=#58A6FF][b]You:[/b][/color] " + message + "\n\n");
    } else {
        chat_history->append_text("[color=#3FB950][b]AI GameForge:[/b][/color] " + message + "\n\n");
    }
}

// ==========================================================
// TEMPLATES
// ==========================================================

void AIForgeDock::_on_template_pressed(const String &prompt) {
    prompt_input->set_text(prompt);
    _on_prompt_changed();
    tab_container->set_current_tab(0);
}

void AIForgeDock::_on_save_template() {
    String text = custom_template_input->get_text().strip_edges();
    if (text.is_empty()) {
        return;
    }
    // Save template prompt to config
    Ref<ConfigFile> config;
    config.instantiate();
    config->load("user://ai_gameforge_settings.cfg");

    int idx = config->get_value("templates", "count", 0);
    config->set_value("templates", "template_" + itos(idx), text);
    config->set_value("templates", "count", idx + 1);
    config->save("user://ai_gameforge_settings.cfg");

    custom_template_input->set_text("");
}

// ==========================================================
// SETTINGS
// ==========================================================

void AIForgeDock::_on_test_connection() {
    connection_status->set_text("  Testing...");
    String url = backend_url_input->get_text().strip_edges();
    http_client->set_backend_url(url);
    http_client->send_health_check(url + "/health");
}

void AIForgeDock::_on_health_check_result(bool success, const String &message) {
    if (success) {
        connection_status->set_text("  ✅ Connected");
    } else {
        connection_status->set_text("  ❌ " + message);
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
    Error err = config->load("user://ai_gameforge_settings.cfg");
    if (err != OK) {
        return;
    }

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

// ==========================================================
// HISTORY
// ==========================================================

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
