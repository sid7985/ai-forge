#ifndef AI_SSE_CLIENT_H
#define AI_SSE_CLIENT_H

#include "scene/main/node.h"
#include "scene/main/http_request.h"

class AISSEClient : public Node {
    GDCLASS(AISSEClient, Node);

    HTTPRequest *http_req;
    String buffer;

    void _on_request_completed(int p_result, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data);

protected:
    static void _bind_methods();
    void _process(double delta);

public:
    AISSEClient();
    void connect_to_stream(const String &url);
    void cancel();
};

#endif // AI_SSE_CLIENT_H
