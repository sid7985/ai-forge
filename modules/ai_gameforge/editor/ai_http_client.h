#ifndef AI_HTTP_CLIENT_H
#define AI_HTTP_CLIENT_H

#include "scene/main/node.h"
#include "scene/main/http_request.h"

class AIHttpClient : public Node {
    GDCLASS(AIHttpClient, Node);

    HTTPRequest *http_req;

    void _on_request_completed(int p_result, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data);

protected:
    static void _bind_methods();

public:
    AIHttpClient();
    void send_generate_request(const Dictionary &payload);
};

#endif // AI_HTTP_CLIENT_H
