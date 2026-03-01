#include "register_types.h"

#ifdef TOOLS_ENABLED
#include "editor/ai_gameforge_plugin.h"
#include "backend/launcher.h"
#include "editor/editor_node.h"

static AIGameForgePlugin *ai_plugin = nullptr;

void ai_gameforge_init_callback() {
    EditorNode *editor = EditorNode::get_singleton();
    if (editor) {
        ai_plugin = memnew(AIGameForgePlugin);
        editor->add_editor_plugin(ai_plugin);
    }
}
#endif

void initialize_ai_gameforge_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
#ifdef TOOLS_ENABLED
        EditorNode::add_init_callback(ai_gameforge_init_callback);
        AIBackendLauncher::get_singleton(); // Spawn instance
#endif
    }
}

void uninitialize_ai_gameforge_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
#ifdef TOOLS_ENABLED
        if (AIBackendLauncher::get_singleton()) {
            AIBackendLauncher::get_singleton()->stop();
            memdelete(AIBackendLauncher::get_singleton());
        }
#endif
    }
}
