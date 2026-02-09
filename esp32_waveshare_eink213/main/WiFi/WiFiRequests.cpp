#include "WiFiRequests.h"
#include "os.h"
#include <cassert>

static const char *TAG_WiFiRequests = "WiFiRequests";

bool WiFiRequests::Equals(const RequestItem *a, const RequestItem *b) const {
    if (a->Type != b->Type) {
        return false;
    }
    switch (a->Type) {
        case wqi_Station:
            return true;

        case wqi_Scanner:
            return a->Payload.Scanner.ssid == b->Payload.Scanner.ssid;

        case wqi_AccessPoint:
            return a->Payload.AccessPoint.ssid == b->Payload.AccessPoint.ssid;
    }
    return true;
}

std::list<RequestItem>::iterator WiFiRequests::Find(RequestItem *request) {
    for (auto it = begin(); it != end(); it++) {
        const auto &req = *it;
        if (Equals(&req, request)) {
            return it;
        }
    }
    return end();
}

bool WiFiRequests::Contains(RequestItem *request) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(request);
    return item != end();
}

bool WiFiRequests::OneMoreInQueue() {
    std::lock_guard<std::mutex> lock(lock_mutex);
    return size() > 1;
}

bool WiFiRequests::Scan(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool new_req = item == end();
    if (new_req) {
        push_front(std::move(request));
    }
    log_d(TAG_WiFiRequests, "Scan, ssid:%s, new_req:%u", ssid, new_req);
    return new_req;
}

bool WiFiRequests::RemoveScanner(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_Scanner, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != end();
    if (exists) {
        erase(item);
    }
    log_d(TAG_WiFiRequests, "RemoveScanner, ssid:%s, exists:%u", ssid, exists);
    return exists;
}

bool WiFiRequests::AccessPoint(const char *ssid, const char *password, const char *mac) {
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid } };
    request.Payload.AccessPoint.password = password;
    request.Payload.AccessPoint.mac = mac;
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool new_req = item == end();
    if (new_req) {
        push_front(std::move(request));
    }
    log_d(TAG_WiFiRequests, "AccessPoint, ssid:%s, new_req:%u", ssid, new_req);
    return new_req;
}

bool WiFiRequests::RemoveAccessPoint(const char *ssid) {
    RequestItem request = { RequestItemType::wqi_AccessPoint, { ssid } };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != end();
    if (exists) {
        erase(item);
    }
    log_d(TAG_WiFiRequests, "RemoveAccessPoint, ssid:%s, exists:%u", ssid, exists);
    return exists;
}

bool WiFiRequests::Station() {
    RequestItem request = { RequestItemType::wqi_Station, {} };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool new_req = item == end();
    if (new_req) {
        push_front(std::move(request));
    }
    log_d(TAG_WiFiRequests, "Station, is new req:%u", new_req);
    return new_req;
}

bool WiFiRequests::RemoveStation() {
    RequestItem request = { RequestItemType::wqi_Station, {} };
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto item = Find(&request);
    bool exists = item != end();
    if (exists) {
        erase(item);
    }
    log_d(TAG_WiFiRequests, "RemoveStation, exists:%u", exists);
    return exists;
}

bool WiFiRequests::Pop(RequestItem *request) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    if (empty()) {
        return false;
    }
    auto &req = back();
    request->Payload = req.Payload;
    request->Type = req.Type;
    return true;
}
