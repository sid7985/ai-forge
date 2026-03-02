#ifndef AI_HTTP_CLIENT_H
#define AI_HTTP_CLIENT_H

#include "scene/main/node.h"
#include "scene/main/http_request.h"

class AIHttpClient : public Node {
    GDCLASS(AIHttpClient, Node);

    HTTPRequest *http_req;
    HTTPRequest *health_req;
    String backend_url;

    void _on_request_completed(int p_result, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data);
    void _on_health_completed(int p_result, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data);

protected:
    static void _bind_methods();

public:
    AIHttpClient();
    void send_generate_request(const Dictionary &payload);
    void send_health_check(const String &url);
    void set_backend_url(const String &url);
    String get_backend_url() const;
};

#endif // AI_HTTP_CLIENT_H
