#include "ai_sse_client.h"
#include "core/io/json.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/class_db.h"

void AISSEClient::_bind_methods() {
    ADD_SIGNAL(MethodInfo("agent_step", PropertyInfo(Variant::STRING, "step"), PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::INT, "status"), PropertyInfo(Variant::DICTIONARY, "extra")));
    ADD_SIGNAL(MethodInfo("stream_closed"));
}

AISSEClient::AISSEClient() {
    http_req = memnew(HTTPRequest);
    http_req->set_use_threads(true); // Needed for streaming chunk by chunk
    http_req->set_download_chunk_size(4096);
    add_child(http_req);
    
    // Connect to request_completed to know when it fully ends
    http_req->connect("request_completed", callable_mp(this, &AISSEClient::_on_request_completed));
    set_process(false);
}

void AISSEClient::connect_to_stream(const String &url) {
    buffer = "";
    Vector<String> headers;
    headers.push_back("Accept: text/event-stream");
    
    Error err = http_req->request(url, headers, HTTPClient::METHOD_GET);
    if (err == OK) {
        set_process(true);
    }
}

void AISSEClient::_process(double delta) {
    if (http_req->get_http_client_status() == HTTPClient::STATUS_BODY) {
        // For MVP Godot 4 standard HTTPRequest: It buffers body. 
        // In a real implementation we'd use the StreamPeer/HTTPClient directly to read byte-by-byte SSE.
        // For this scaffold, we simulate via string parsing when data becomes ready at the end via _on_request_completed.
    }
}

void AISSEClient::_on_request_completed(int p_result, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data) {
    set_process(false);
    
    // Fallback if full stream buffered (MVP workaround for Godot HTTPRequest limitations)
    String body = String::utf8((const char *)p_data.ptr(), p_data.size());
    
    Vector<String> lines = body.split("\n");
    for (int i = 0; i < lines.size(); i++) {
        if (lines[i].begins_with("data: ")) {
            String json_str = lines[i].substr(6).strip_edges();
            JSON json;
            if (json.parse(json_str) == OK) {
                Dictionary payload = json.get_data();
                emit_signal("agent_step", payload.get("step", ""), payload.get("message", ""), payload.get("status", 0), payload);
            }
        }
    }
    
    emit_signal("stream_closed");
}

void AISSEClient::cancel() {
    http_req->cancel_request();
    set_process(false);
}
