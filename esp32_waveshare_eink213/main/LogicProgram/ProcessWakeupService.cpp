
#include "LogicProgram/ProcessWakeupService.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define microseconds_in_millisecond 1000
#define round_us (microseconds_in_millisecond / 2)

static const char *TAG_ProcessWakeupService = "ProcessWakeupService";

bool ProcessWakeupService::Request(const void *id,
                                   uint32_t delay_ms,
                                   ProcessWakeupRequestPriority priority) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    bool request_already_in = ids.find(id) != ids.end();
    if (request_already_in) {
        log_d(TAG_ProcessWakeupService,
              "Request already in:%u, %p, size:%u",
              (unsigned int)delay_ms,
              id,
              (unsigned int)requests.size());
        return false;
    }

    auto current_time = timer_get_time_us();
    auto next_time = current_time + ((uint64_t)delay_ms * microseconds_in_millisecond);

    ProcessWakeupRequestData request = { id, next_time, priority };
    auto upper = requests.upper_bound(request);
    if (upper != requests.end()) {
        auto &upper_req = *upper;
        bool request_can_be_joined = request.priority == ProcessWakeupRequestPriority::pwrp_Idle
                                  && (upper_req.next_time - next_time) < idle_dead_band_us;
        if (request_can_be_joined) {
            log_d(TAG_ProcessWakeupService,
                  "Request is joined in:%u, %p, diff:%d, next:%u",
                  (unsigned int)delay_ms,
                  id,
                  (int)(upper_req.next_time - next_time),
                  (unsigned int)upper_req.next_time);
            request.next_time = upper_req.next_time;
        }
    }

    requests.insert(std::move(request));
    ids.insert(id);

    log_d(TAG_ProcessWakeupService,
          "Request:%u, %p, size:%u, time:%u",
          (unsigned int)delay_ms,
          id,
          (unsigned int)requests.size(),
          (unsigned int)(current_time / microseconds_in_millisecond));
    return true;
}

void ProcessWakeupService::RemoveRequest(const void *id) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto id_it = ids.find(id);
    bool request_exists = id_it != ids.end();
    if (!request_exists) {
        return;
    }
    ids.erase(id_it);

    for (auto it = requests.begin(); it != requests.end(); it++) {
        auto &req = *it;

        if (req.id == id) {
            requests.erase(it);
            break;
        }
    }

    log_d(TAG_ProcessWakeupService,
          "RemoveRequest: %p, size:%u, systick:%u",
          id,
          (unsigned int)requests.size(),
          (unsigned int)timer_get_time_us());
}

static char *
println(const std::set<ProcessWakeupRequestData, ProcessWakeupRequestDataCmp> &requests) {
    static char buffer[512];
    int pos = sprintf(buffer, "[");
    bool first{ true };
    for (auto &x : requests) {
        pos += sprintf(&buffer[pos], "%s", (first ? first = false, "" : ", "));
        pos += sprintf(&buffer[pos],
                       "%p|%u",
                       x.id,
                       (unsigned int)(x.next_time / microseconds_in_millisecond));
    }
    printf(&buffer[pos], "]");
    return buffer;
}

bool ProcessWakeupService::Get(uint32_t *delay_ms) {
    std::lock_guard<std::mutex> lock(lock_mutex);
    if (requests.empty()) {
        log_d(TAG_ProcessWakeupService, "empty");
        return false;
    }

    auto current_time = timer_get_time_us();
    auto req_it = requests.begin();
    auto &req = *req_it;
    int64_t timespan = req.next_time - current_time;
    uint32_t wait_ms = ((timespan + round_us) / microseconds_in_millisecond);

    log_d(TAG_ProcessWakeupService,
          "Get:%u, %p, size:%u, time:%u, %s",
          (unsigned int)wait_ms,
          req.id,
          (unsigned int)requests.size(),
          (unsigned int)(current_time / microseconds_in_millisecond),
          println(requests));
    if (timespan < 0) {
        wait_ms = 0;
    }
    *delay_ms = wait_ms;
    return true;
}

void ProcessWakeupService::RemoveExpired() {
    std::lock_guard<std::mutex> lock(lock_mutex);
    auto current_time = timer_get_time_us();
    while (!requests.empty()) {
        auto req_it = requests.begin();
        auto &req = *req_it;
        int64_t timespan = req.next_time - current_time;
        bool expired = timespan < round_us;
        if (expired) {
            ids.erase(req.id);
            requests.erase(req_it);
            log_d(TAG_ProcessWakeupService,
                  "RemoveExpired: %p, size:%u, systick:%u",
                  req.id,
                  (unsigned int)requests.size(),
                  (unsigned int)(current_time / microseconds_in_millisecond));
        } else {
            break;
        }
    }
}