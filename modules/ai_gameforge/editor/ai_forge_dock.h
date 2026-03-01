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
#include "editor/ai_http_client.h"
#include "editor/ai_sse_client.h"

class AIForgeDock : public VBoxContainer {
    GDCLASS(AIForgeDock, VBoxContainer);

private:
    TabContainer *tab_container;

    // Tab 1: Generate
    RichTextLabel *chat_history;
    LineEdit *prompt_input;
    Button *generate_btn;
    Button *refine_btn;
    ProgressBar *generation_progress;
    RichTextLabel *agent_log;

    // Tab 2: Templates
    Button *btn_template_platformer;
    Button *btn_template_rpg;
    Button *btn_template_puzzle;
    Button *btn_template_runner;
    Button *btn_template_tower;

    // Tab 3: Settings
    LineEdit *api_key_input;
    OptionButton *provider_select;
    Button *test_connection_btn;

    AIHttpClient *http_client;
    AISSEClient *sse_client;
    String current_project_path;
    bool is_generating = false;

    void _on_generate_pressed();
    void _on_refine_pressed();
    void _on_generation_complete(const Dictionary &result);
    void _on_generation_failed(const String &error);
    void _on_agent_step(const String &step, const String &message, int status);
    void _add_chat_message(const String &role, const String &message);
    void _on_template_pressed(const String &prompt);

protected:
    static void _bind_methods();

public:
    AIForgeDock();
    void setup(const String &project_path);
};

#endif // AI_FORGE_DOCK_H
