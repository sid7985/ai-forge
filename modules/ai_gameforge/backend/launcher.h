#ifndef AI_BACKEND_LAUNCHER_H
#define AI_BACKEND_LAUNCHER_H

#include "core/object/object.h"
#include "core/os/os.h"

class AIBackendLauncher : public Object {
    GDCLASS(AIBackendLauncher, Object);

    static AIBackendLauncher *singleton;
    OS::ProcessID server_pid = 0;
    bool is_running = false;

public:
    static AIBackendLauncher *get_singleton();

    AIBackendLauncher();
    ~AIBackendLauncher();

    void start();
    void stop();
    bool get_is_running() const { return is_running; }
};

#endif // AI_BACKEND_LAUNCHER_H
