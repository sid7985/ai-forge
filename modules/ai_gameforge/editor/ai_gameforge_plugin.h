#ifndef AI_GAMEFORGE_PLUGIN_H
#define AI_GAMEFORGE_PLUGIN_H

#include "editor/plugins/editor_plugin.h"

class AIForgeDock;

class AIGameForgePlugin : public EditorPlugin {
    GDCLASS(AIGameForgePlugin, EditorPlugin);

    AIForgeDock *dock;

public:
    virtual String get_plugin_name() const override { return "AIGameForge"; }
    bool has_main_screen() const override { return false; }
    
    AIGameForgePlugin();
    ~AIGameForgePlugin();
};

#endif // AI_GAMEFORGE_PLUGIN_H
