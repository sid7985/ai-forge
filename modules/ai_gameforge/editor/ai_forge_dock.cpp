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
    set_custom_minimum_size(Size2(300, 400));

    tab_container = memnew(TabContainer);
    tab_container->set_v_size_flags(SIZE_EXPAND_FILL);
    add_child(tab_container);

    // --- TAB 1: GENERATE ---
    VBoxContainer *generate_tab = memnew(VBoxContainer);
    generate_tab->set_name("Generate");
    tab_container->add_child(generate_tab);

    // Agent live log
    Label *log_label = memnew(Label);
    log_label->set_text("Agent Activity:");
    generate_tab->add_child(log_label);

    ScrollContainer *log_scroll = memnew(ScrollContainer);
    log_scroll->set_custom_minimum_size(Size2(0, 120));
    generate_tab->add_child(log_scroll);

    agent_log = memnew(RichTextLabel);
    agent_log->set_use_bbcode(true);
    agent_log->set_h_size_flags(SIZE_EXPAND_FILL);
    agent_log->set_v_size_flags(SIZE_EXPAND_FILL);
    log_scroll->add_child(agent_log);
    
    generate_tab->add_child(memnew(HSeparator));

    // Chat History
    ScrollContainer *chat_scroll = memnew(ScrollContainer);
    chat_scroll->set_v_size_flags(SIZE_EXPAND_FILL);
    generate_tab->add_child(chat_scroll);

    chat_history = memnew(RichTextLabel);
    chat_history->set_use_bbcode(true);
    chat_history->set_selection_enabled(true);
    chat_history->set_h_size_flags(SIZE_EXPAND_FILL);
    chat_history->set_v_size_flags(SIZE_EXPAND_FILL);
    chat_scroll->add_child(chat_history);

    // Progress Bar
    generation_progress = memnew(ProgressBar);
    generation_progress->set_min(0);
    generation_progress->set_max(100);
    generation_progress->set_value(0);
    generation_progress->hide();
    generate_tab->add_child(generation_progress);

    // Input Control
    HBoxContainer *input_row = memnew(HBoxContainer);
    generate_tab->add_child(input_row);

    prompt_input = memnew(LineEdit);
    prompt_input->set_placeholder("Describe your game (e.g. 2D platformer)...");
    prompt_input->set_h_size_flags(SIZE_EXPAND_FILL);
    input_row->add_child(prompt_input);

    generate_btn = memnew(Button);
    generate_btn->set_text("Generate");
    generate_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_generate_pressed));
    input_row->add_child(generate_btn);

    refine_btn = memnew(Button);
    refine_btn->set_text("Refine Current Scene");
    refine_btn->connect("pressed", callable_mp(this, &AIForgeDock::_on_refine_pressed));
    generate_tab->add_child(refine_btn);

    // --- TAB 2: TEMPLATES ---
    VBoxContainer *templates_tab = memnew(VBoxContainer);
    templates_tab->set_name("Templates");
    tab_container->add_child(templates_tab);

    templates_tab->add_child([&]() { auto *l = memnew(Label); l->set_text("Select a starting template:"); return l; }());

    auto add_template_btn = [&](Button **btn, const String &text, const String &prompt) {
        *btn = memnew(Button);
        (*btn)->set_text(text);
        (*btn)->connect("pressed", callable_mp(this, &AIForgeDock::_on_template_pressed).bind(prompt));
        templates_tab->add_child(*btn);
    };

    add_template_btn(&btn_template_platformer, "2D Platformer", "A classic 2D platformer with coin collection, a moving platform, and a simple enemy logic.");
    add_template_btn(&btn_template_rpg, "Top-Down RPG", "A top-down 2D RPG with a player controller, an inventory system, and an NPC interaction zone.");
    add_template_btn(&btn_template_puzzle, "Puzzle Game", "A match-3 puzzle game with an 8x8 grid filled with gems and basic swapping logic.");
    add_template_btn(&btn_template_runner, "Endless Runner", "An endless runner side-scroller with procedural obstacle generation and parallax background.");
    add_template_btn(&btn_template_tower, "Tower Defense", "A simple tower defense game with a path for enemies and a placable tower that shoots projectiles.");

    // --- TAB 3: SETTINGS ---
    VBoxContainer *settings_tab = memnew(VBoxContainer);
    settings_tab->set_name("Settings");
    tab_container->add_child(settings_tab);

    settings_tab->add_child([&]() { auto *l = memnew(Label); l->set_text("AI Provider:"); return l; }());
    provider_select = memnew(OptionButton);
    provider_select->add_item("Claude (Anthropic)");
    provider_select->add_item("GPT-4o (OpenAI)");
    provider_select->add_item("Gemini Flash (Google)");
    provider_select->add_item("Local (AirLLM)");
    settings_tab->add_child(provider_select);

    settings_tab->add_child([&]() { auto *l = memnew(Label); l->set_text("API Key:"); return l; }());
    api_key_input = memnew(LineEdit);
    api_key_input->set_placeholder("sk-ant-...");
    api_key_input->set_secret(true);
    settings_tab->add_child(api_key_input);

    // --- HTTP CLIENT ---
    http_client = memnew(AIHttpClient);
    add_child(http_client);
    http_client->connect("generation_complete", callable_mp(this, &AIForgeDock::_on_generation_complete));
    http_client->connect("generation_failed", callable_mp(this, &AIForgeDock::_on_generation_failed));
    
    // --- SSE CLIENT ---
    sse_client = memnew(AISSEClient);
    add_child(sse_client);
    sse_client->connect("agent_step", callable_mp(this, &AIForgeDock::_on_agent_step));
}

void AIForgeDock::setup(const String &project_path) {
    current_project_path = project_path;
}

void AIForgeDock::_on_generate_pressed() {
    String prompt = prompt_input->get_text().strip_edges();
    if (prompt.is_empty()) return;

    is_generating = true;
    generate_btn->set_disabled(true);
    generation_progress->show();
    generation_progress->set_value(0);
    agent_log->clear();

    _add_chat_message("user", prompt);

    Dictionary request;
    request["prompt"] = prompt;
    request["project_root"] = current_project_path;
    request["provider"] = provider_select->get_item_text(provider_select->get_selected());
    request["api_key"] = api_key_input->get_text();
    request["mode"] = "generate";

    http_client->send_generate_request(request);
    prompt_input->clear();
}

void AIForgeDock::_on_refine_pressed() {
	String prompt = prompt_input->get_text().strip_edges();
    if (prompt.is_empty()) return;

    _add_chat_message("user", "[Refinement] " + prompt);
    prompt_input->clear();
    
    // In full implementation: HTTP dispatch for /refine
}

void AIForgeDock::_on_generation_complete(const Dictionary &result) {
    String session_id = result.get("session_id", "");
    if (!session_id.is_empty()) {
        sse_client->connect_to_stream("http://127.0.0.1:8742/stream/" + session_id);
    }
}

void AIForgeDock::_on_generation_failed(const String &error) {
    is_generating = false;
    generate_btn->set_disabled(false);
    generation_progress->hide();
    _add_chat_message("ai", "[color=red]Error: " + error + "[/color]");
}

void AIForgeDock::_on_agent_step(const String &step, const String &message, int status) {
    String color;
    String icon;
    switch (status) {
        case 0: color = "gray"; icon = "○"; break; // PENDING
        case 1: color = "orange"; icon = "◉"; break; // RUNNING
        case 2: color = "green"; icon = "●"; break; // DONE
        case 3: color = "red"; icon = "✗"; break; // ERROR
    }

    agent_log->append_text("[color=" + color + "]" + icon + " " + step + "[/color]\n" +
                           "[color=gray]  " + message + "[/color]\n");
                           
    if (status == 2) {
        generation_progress->set_value(generation_progress->get_value() + 20.0f);
    }
    
    // If the entire pipeline is done (Completion step emitted status 2)
    if (step == "Complete" && status == 2) {
        is_generating = false;
        generate_btn->set_disabled(false);
        generation_progress->hide();
        _add_chat_message("ai", "Game generation complete!");
        
        // KEY FORK FEATURE: Auto-Refresh file system and open scene.
        EditorFileSystem::get_singleton()->scan();
        // Since main.tscn path is known
        EditorInterface::get_singleton()->open_scene_from_path("res://scenes/main.tscn");
    } else if (status == 3) {
        is_generating = false;
        generate_btn->set_disabled(false);
        generation_progress->hide();
        _add_chat_message("ai", "[color=red]Generation Failed: " + message + "[/color]");
    }
}

void AIForgeDock::_add_chat_message(const String &role, const String &message) {
    if (role == "user") {
        chat_history->append_text("[color=#58A6FF][b]You:[/b][/color] " + message + "\n\n");
    } else {
        chat_history->append_text("[color=#3FB950][b]AI GameForge:[/b][/color] " + message + "\n\n");
    }
}

void AIForgeDock::_on_template_pressed(const String &prompt) {
    prompt_input->set_text(prompt);
    // Switch back to Generate Tab (Index 0)
    tab_container->set_current_tab(0);
}
