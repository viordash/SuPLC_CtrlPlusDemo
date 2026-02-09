#pragma once

#include <stdint.h>
#include <unistd.h>

enum SmartconfigStatus {
    scs_Start,
    scs_Started,
    scs_Disconnected,
    scs_GotIP,
    scs_ScanDone,
    scs_FoundChannel,
    scs_GotCreds,
    scs_Completed,
    scs_Error
};

void start_smartconfig();
void finish_smartconfig();
enum SmartconfigStatus smartconfig_status();
