#include "ai_http_client.h"
#include "core/io/json.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/class_db.h"

void AIHttpClient::_bind_methods() {
    ADD_SIGNAL(MethodInfo("generation_complete", PropertyInfo(Variant::DICTIONARY, "result")));
    ADD_SIGNAL(MethodInfo("generation_failed", PropertyInfo(Variant::STRING, "error")));
    ADD_SIGNAL(MethodInfo("agent_step", PropertyInfo(Variant::STRING, "step"), PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::INT, "status")));
}

AIHttpClient::AIHttpClient() {
    http_req = memnew(HTTPRequest);
    add_child(http_req);
    http_req->connect("request_completed", callable_mp(this, &AIHttpClient::_on_request_completed));
}

void AIHttpClient::send_generate_request(const Dictionary &payload) {
    String json = JSON::stringify(payload);
    Vector<String> headers;
    headers.push_back("Content-Type: application/json");
    
    // In Phase 1, the port is hardcoded to the local Python FastAPI server
    http_req->request("http://127.0.0.1:8742/generate", headers, HTTPClient::METHOD_POST, json);
}

void AIHttpClient::_on_request_completed(int p_result, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data) {
    if (p_result != HTTPRequest::RESULT_SUCCESS || p_code != 200) {
        emit_signal("generation_failed", "Failed to contact local AI Server (HTTP " + itos(p_code) + ")");
        return;
    }

    String body = String::utf8((const char *)p_data.ptr(), p_data.size());

    JSON json;
    Error err = json.parse(body);
    if (err == OK && json.get_data().get_type() == Variant::DICTIONARY) {
        emit_signal("generation_complete", json.get_data());
    } else {
        emit_signal("generation_failed", "Invalid JSON response from AI server.");
    }
}
