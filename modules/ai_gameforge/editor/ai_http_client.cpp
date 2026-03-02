#include "ai_http_client.h"
#include "core/io/json.h"
#include "core/object/callable_method_pointer.h"
#include "core/object/class_db.h"

void AIHttpClient::_bind_methods() {
    ADD_SIGNAL(MethodInfo("generation_complete", PropertyInfo(Variant::DICTIONARY, "result")));
    ADD_SIGNAL(MethodInfo("generation_failed", PropertyInfo(Variant::STRING, "error")));
    ADD_SIGNAL(MethodInfo("agent_step", PropertyInfo(Variant::STRING, "step"), PropertyInfo(Variant::STRING, "message"), PropertyInfo(Variant::INT, "status")));
    ADD_SIGNAL(MethodInfo("health_check_result", PropertyInfo(Variant::BOOL, "success"), PropertyInfo(Variant::STRING, "message")));
}

AIHttpClient::AIHttpClient() {
    backend_url = "http://127.0.0.1:8742";

    http_req = memnew(HTTPRequest);
    add_child(http_req);
    http_req->connect("request_completed", callable_mp(this, &AIHttpClient::_on_request_completed));

    health_req = memnew(HTTPRequest);
    add_child(health_req);
    health_req->connect("request_completed", callable_mp(this, &AIHttpClient::_on_health_completed));
}

void AIHttpClient::set_backend_url(const String &url) {
    backend_url = url;
}

String AIHttpClient::get_backend_url() const {
    return backend_url;
}

void AIHttpClient::send_generate_request(const Dictionary &payload) {
    String json = JSON::stringify(payload);
    Vector<String> headers;
    headers.push_back("Content-Type: application/json");

    http_req->request(backend_url + "/generate", headers, HTTPClient::METHOD_POST, json);
}

void AIHttpClient::send_health_check(const String &url) {
    Vector<String> headers;
    health_req->request(url, headers, HTTPClient::METHOD_GET);
}

void AIHttpClient::_on_request_completed(int p_result, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data) {
    if (p_result != HTTPRequest::RESULT_SUCCESS || p_code != 200) {
        emit_signal("generation_failed", "Failed to contact AI Server (HTTP " + itos(p_code) + ")");
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

void AIHttpClient::_on_health_completed(int p_result, int p_code, const PackedStringArray &headers, const PackedByteArray &p_data) {
    if (p_result == HTTPRequest::RESULT_SUCCESS && p_code == 200) {
        emit_signal("health_check_result", true, "Server is running");
    } else if (p_result == HTTPRequest::RESULT_CANT_CONNECT) {
        emit_signal("health_check_result", false, "Cannot connect");
    } else {
        emit_signal("health_check_result", false, "HTTP " + itos(p_code));
    }
}
