#include "ai_gameforge_plugin.h"
#include "ai_forge_dock.h"
#include "backend/launcher.h"
#include "editor/editor_node.h"
#include "editor/file_system/editor_paths.h"
#include "core/object/class_db.h"

AIGameForgePlugin::AIGameForgePlugin() {
    // Create the dock
    dock = memnew(AIForgeDock);

    // Add it to the right panel of the editor
    add_control_to_dock(DOCK_SLOT_RIGHT_BL, dock);

    // Auto-start Python AI backend
    if (AIBackendLauncher::get_singleton()) {
        AIBackendLauncher::get_singleton()->start();
    }
    
    // Pass the actual project path to the dock
    dock->setup(EditorPaths::get_singleton()->get_project_settings_dir());
}

AIGameForgePlugin::~AIGameForgePlugin() {
    remove_control_from_docks(dock);
    memdelete(dock);
}
