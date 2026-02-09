#include "DeviceIO.h"
#include "LogicProgram/LogicProgram.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const AllowedIO InputNC::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::D0, MapIO::D1, MapIO::D2, MapIO::D3, MapIO::A0, MapIO::A1,
                                 MapIO::A2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO InputNO::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::D0, MapIO::D1, MapIO::D2, MapIO::D3, MapIO::A0, MapIO::A1,
                                 MapIO::A2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO ComparatorLs::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::D0, MapIO::D1, MapIO::D2, MapIO::D3, MapIO::A0, MapIO::A1,
                                 MapIO::A2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO ComparatorLE::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::D0, MapIO::D1, MapIO::D2, MapIO::D3, MapIO::A0, MapIO::A1,
                                 MapIO::A2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO ComparatorGr::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::D0, MapIO::D1, MapIO::D2, MapIO::D3, MapIO::A0, MapIO::A1,
                                 MapIO::A2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO ComparatorGE::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::D0, MapIO::D1, MapIO::D2, MapIO::D3, MapIO::A0, MapIO::A1,
                                 MapIO::A2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO ComparatorEq::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::D0, MapIO::D1, MapIO::D2, MapIO::D3, MapIO::A0, MapIO::A1,
                                 MapIO::A2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO ComparatorNe::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::D0, MapIO::D1, MapIO::D2, MapIO::D3, MapIO::A0, MapIO::A1,
                                 MapIO::A2, MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO Indicator::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::D0, MapIO::D1, MapIO::D2, MapIO::D3, MapIO::A0,
                                 MapIO::A1, MapIO::A2, MapIO::V1, MapIO::V2, MapIO::V3,
                                 MapIO::V4, MapIO::O0, MapIO::O1, MapIO::O2, MapIO::O3 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO WiFiStaBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO WiFiBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO WiFiApBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO DateTimeBinding::GetAllowedInputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO DecOutput::GetAllowedOutputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO DirectOutput::GetAllowedOutputs() {
    static MapIO allowedIO[] = { MapIO::O0, MapIO::O1, MapIO::O2, MapIO::O3,
                                 MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO IncOutput::GetAllowedOutputs() {
    static MapIO allowedIO[] = { MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO ResetOutput::GetAllowedOutputs() {
    static MapIO allowedIO[] = { MapIO::O0, MapIO::O1, MapIO::O2, MapIO::O3,
                                 MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}

const AllowedIO SetOutput::GetAllowedOutputs() {
    static MapIO allowedIO[] = { MapIO::O0, MapIO::O1, MapIO::O2, MapIO::O3,
                                 MapIO::V1, MapIO::V2, MapIO::V3, MapIO::V4 };
    return { allowedIO, sizeof(allowedIO) / sizeof(allowedIO[0]) };
}