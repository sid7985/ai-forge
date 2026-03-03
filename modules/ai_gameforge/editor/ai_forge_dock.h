#ifndef AI_FORGE_DOCK_H
#define AI_FORGE_DOCK_H

#include "editor/plugins/editor_plugin.h"
#include "editor/editor_interface.h"
#include "scene/gui/box_container.h"
#include "scene/gui/text_edit.h"
#include "scene/gui/rich_text_label.h"
#include "scene/gui/button.h"
#include "scene/gui/line_edit.h"
#include "scene/gui/tab_container.h"
#include "scene/gui/progress_bar.h"
#include "scene/gui/option_button.h"
#include "scene/gui/check_box.h"
#include "scene/gui/spin_box.h"
#include "scene/gui/slider.h"
#include "scene/gui/item_list.h"
#include "scene/gui/separator.h"
#include "scene/gui/label.h"
#include "scene/gui/panel.h"
#include "scene/gui/color_rect.h"
#include "scene/gui/scroll_container.h"
#include "core/io/config_file.h"
#include "editor/ai_http_client.h"
#include "editor/ai_sse_client.h"

class AIForgeDock : public VBoxContainer {
    GDCLASS(AIForgeDock, VBoxContainer);

private:
    TabContainer *tab_container;

    // --- Tab 1: Generate ---
    RichTextLabel *chat_history;
    TextEdit *prompt_input;
    Label *char_count_label;
    Button *generate_btn;
    Button *stop_btn;
    Button *refine_btn;
    Button *clear_chat_btn;
    ProgressBar *generation_progress;

    // Agent Brain Monitor
    VBoxContainer *brain_monitor;
    struct AgentRow {
        Label *name_label;
        Label *status_label;
        ProgressBar *progress;
    };
    AgentRow agent_rows[5];

    // Advanced Options
    VBoxContainer *advanced_options_panel;
    Button *toggle_advanced_btn;
    OptionButton *complexity_select;
    OptionButton *target_platform;
    CheckBox *include_audio;
    CheckBox *include_particles;
    SpinBox *max_nodes;

    // --- Tab 2: Templates ---
    Button *btn_template_platformer;
    Button *btn_template_rpg;
    Button *btn_template_puzzle;
    Button *btn_template_runner;
    Button *btn_template_tower;
    TextEdit *custom_template_input;
    Button *save_template_btn;

    // --- Tab 3: Settings ---
    LineEdit *api_key_input;
    OptionButton *provider_select;
    LineEdit *backend_url_input;
    HSlider *temperature_slider;
    Label *temperature_label;
    Button *test_connection_btn;
    Label *connection_status;
    Button *save_settings_btn;

    // --- Tab 4: History ---
    ItemList *generation_history;
    Button *clear_history_btn;
    Vector<String> history_prompts;

    // Internals
    AIHttpClient *http_client;
    AISSEClient *sse_client;
    String current_project_path;
    bool is_generating = false;
    bool advanced_visible = false;
    int current_agent_index = 0;

    void _on_generate_pressed();
    void _on_stop_pressed();
    void _on_refine_pressed();
    void _on_generation_complete(const Dictionary &result);
    void _on_generation_failed(const String &error);
    void _on_agent_step(const String &step, const String &message, int status);
    void _add_chat_message(const String &role, const String &message);
    void _on_template_pressed(const String &prompt);
    void _on_toggle_advanced();
    void _on_clear_chat();
    void _on_prompt_changed();
    void _on_test_connection();
    void _on_save_settings();
    void _on_save_template();
    void _on_history_selected(int index);
    void _on_clear_history();
    void _on_temperature_changed(double value);
    void _on_health_check_result(bool success, const String &message);
    void _load_settings();
    void _add_history_entry(const String &prompt);
    void _update_agent_monitor(const String &agent, int status, float progress);
    void _reset_agent_monitor();

    // Theme helpers
    Color _deep_void_bg() const { return Color(0.043f, 0.051f, 0.078f); }
    Color _panel_bg() const { return Color(0.078f, 0.086f, 0.133f); }
    Color _panel_header() const { return Color(0.102f, 0.114f, 0.168f); }
    Color _panel_border() const { return Color(0.165f, 0.180f, 0.251f); }
    Color _neon_cyan() const { return Color(0.0f, 0.898f, 1.0f); }
    Color _neon_purple() const { return Color(0.616f, 0.306f, 0.867f); }
    Color _success_green() const { return Color(0.0f, 0.902f, 0.463f); }
    Color _warning_yellow() const { return Color(1.0f, 0.835f, 0.310f); }
    Color _error_red() const { return Color(1.0f, 0.090f, 0.267f); }
    Color _text_primary() const { return Color(0.878f, 0.902f, 0.929f); }
    Color _text_muted() const { return Color(0.388f, 0.416f, 0.502f); }

protected:
    static void _bind_methods();

public:
    AIForgeDock();
    void setup(const String &project_path);
};

#endif // AI_FORGE_DOCK_H
