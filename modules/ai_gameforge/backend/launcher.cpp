#include "launcher.h"
#include "core/os/os.h"
#include "core/config/project_settings.h"

AIBackendLauncher *AIBackendLauncher::singleton = nullptr;

AIBackendLauncher *AIBackendLauncher::get_singleton() {
    if (!singleton) {
        singleton = memnew(AIBackendLauncher);
    }
    return singleton;
}

AIBackendLauncher::AIBackendLauncher() {}

AIBackendLauncher::~AIBackendLauncher() {
    stop();
}

void AIBackendLauncher::start() {
    if (is_running || server_pid != 0) return;

    String exe = "python3";
#ifdef WINDOWS_ENABLED
    exe = "python.exe";
#endif

    // The backend is expected to be adjacent to the godot binary or in the source tree locally
    // For MVP compilation: we assume the source tree structure
    String backend_path = ProjectSettings::get_singleton()->globalize_path("res://ai_backend/main.py");
    List<String> args;
    args.push_back(backend_path);

    // Fork off process detatched
    Error err = OS::get_singleton()->create_process(exe, args, &server_pid, false);
    if (err == OK) {
        is_running = true;
        print_line("AI GameForge Backend started running on localhost:8742 (PID: " + itos(server_pid) + ")");
    } else {
        print_error("Failed to start AI GameForge Python Backend.");
    }
}

void AIBackendLauncher::stop() {
    if (is_running && server_pid != 0) {
        OS::get_singleton()->kill(server_pid);
        server_pid = 0;
        is_running = false;
        print_line("AI GameForge Backend stopped.");
    }
}
