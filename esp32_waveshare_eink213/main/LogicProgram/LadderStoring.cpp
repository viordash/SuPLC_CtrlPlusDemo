#include "LogicProgram/Ladder.h"
#include "LogicProgram/LogicProgram.h"
#include "LogicProgram/Serializer/Record.h"
#include "Storage/redundant_storage.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void Ladder::Load() {
    DeleteStorage();
    RemoveAll();
    redundant_storage storage = redundant_storage_load(storage_0_partition,
                                                       storage_0_path,
                                                       storage_1_partition,
                                                       storage_1_path,
                                                       ladder_storage_name);

    log_i(TAG_Ladder,
          "Load ver: 0x%X, size:%u",
          (unsigned int)storage.version,
          (unsigned int)storage.size);

    if (storage.version != LADDER_VERSION //
        || Deserialize(storage.data, storage.size) == 0) {
        log_i(TAG_Ladder, "Nothing to load");
        RemoveAll();
        InitialLoad();
    }
    delete[] storage.data;
}

void Ladder::InitialLoad() {
    // return;
    auto incomeRail0 = new Network(LogicItemState::lisActive);
    incomeRail0->Append(new WiFiStaBinding(MapIO::V4));
    // incomeRail0->Append(
    //     new SettingsElement(SettingsElement::Discriminator::t_wifi_station_settings_ssid));
    // incomeRail0->Append(
    //     new SettingsElement(SettingsElement::Discriminator::t_wifi_station_settings_password));
    Append(incomeRail0);

    // auto incomeRail1 = new Network(LogicItemState::lisActive);
    // incomeRail1->Append(new InputNC(MapIO::D0));
    // incomeRail1->Append(new InputNO(MapIO::V1));
    // incomeRail1->Append(new TimerMSecs(1000));
    // incomeRail1->Append(new ResetOutput(MapIO::V1));
    // Append(incomeRail1);

    // auto incomeRail2 = new Network(LogicItemState::lisActive);
    // // incomeRail2->Append(new ComparatorNe(33, MapIO::V3));
    // incomeRail2->Append(new InputNO(MapIO::V1));
    // // incomeRail2->Append(new DateTimeBinding(MapIO::V2));
    // incomeRail2->Append(new DirectOutput(MapIO::O0));
    // Append(incomeRail2);

    auto incomeRail2_0 = new Network(LogicItemState::lisActive);
    incomeRail2_0->Append(new Indicator(MapIO::A0, 255, 0, 0));
    Append(incomeRail2_0);

    auto incomeRail2_1 = new Network(LogicItemState::lisActive);
    incomeRail2_1->Append(new ComparatorLs(5, MapIO::A0));
    incomeRail2_1->Append(new ResetOutput(MapIO::O1));
    Append(incomeRail2_1);

    uint32_t period0_ms = 90;
    uint32_t period1_ms = 10;
    for (size_t i = 5; i < (255 - 25); i += 25) {
        auto network0 = new Network(LogicItemState::lisActive);
        network0->Append(new ComparatorGE(i, MapIO::A0));
        network0->Append(new ComparatorLs(i + 25, MapIO::A0));
        network0->Append(new ContinuationIn());
        Append(network0);

        auto network1 = new Network(LogicItemState::lisActive);
        network1->Append(new ContinuationOut());
        network1->Append(new SquareWaveGenerator(period0_ms, period1_ms));
        network1->Append(new DirectOutput(MapIO::O1));
        Append(network1);

        period0_ms -= 10;
        period1_ms += 10;
    }

    {
        auto network = new Network(LogicItemState::lisActive);
        network->Append(new ComparatorGE((255 - 25), MapIO::A0));
        network->Append(new SetOutput(MapIO::O1));
        Append(network);
    }

    // auto incomeRail2_1 = new Network(LogicItemState::lisActive);
    // incomeRail2_1->Append(new ComparatorLE(5, MapIO::A0));
    // incomeRail2_1->Append(new ResetOutput(MapIO::O1));
    // Append(incomeRail2_1);

    // auto incomeRail2_2 = new Network(LogicItemState::lisActive);
    // incomeRail2_2->Append(new ComparatorGr(5, MapIO::A0));
    // incomeRail2_2->Append(new ComparatorLE(10, MapIO::A0));
    // incomeRail2_2->Append(new SquareWaveGenerator(80, 20));
    // incomeRail2_2->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail2_2);

    // auto incomeRail2_3 = new Network(LogicItemState::lisActive);
    // incomeRail2_3->Append(new ComparatorGr(10, MapIO::A0));
    // incomeRail2_3->Append(new ComparatorLE(15, MapIO::A0));
    // incomeRail2_3->Append(new SquareWaveGenerator(60, 20));
    // incomeRail2_3->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail2_3);

    // auto incomeRail2_4 = new Network(LogicItemState::lisActive);
    // incomeRail2_4->Append(new ComparatorGr(15, MapIO::A0));
    // incomeRail2_4->Append(new ComparatorLE(20, MapIO::A0));
    // incomeRail2_4->Append(new SquareWaveGenerator(40, 60));
    // incomeRail2_4->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail2_4);

    // auto incomeRail2_5 = new Network(LogicItemState::lisActive);
    // incomeRail2_5->Append(new ComparatorGr(25, MapIO::A0));
    // incomeRail2_5->Append(new ComparatorLE(30, MapIO::A0));
    // incomeRail2_5->Append(new SquareWaveGenerator(20, 80));
    // incomeRail2_5->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail2_5);

    // auto incomeRail2_6 = new Network(LogicItemState::lisActive);
    // incomeRail2_6->Append(new ComparatorGr(30, MapIO::A0));
    // incomeRail2_6->Append(new SetOutput(MapIO::O1));
    // Append(incomeRail2_6);

    // auto incomeRail3 = new Network(LogicItemState::lisActive);
    // incomeRail3->Append(new InputNO(MapIO::D0));
    // incomeRail3->Append(new SettingsElement(SettingsElement::Discriminator::t_current_time));
    // incomeRail3->Append(new TimerSecs(3));
    // incomeRail3->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail3);

    // auto incomeRail4 = new Network(LogicItemState::lisActive);
    // incomeRail4->Append(new InputNO(MapIO::V1));
    // incomeRail4->Append(new IncOutput(MapIO::V3));
    // Append(incomeRail4);

    // auto incomeRail6 = new Network(LogicItemState::lisActive);
    // incomeRail6->Append(new WiFiStaBinding(MapIO::V4));
    // Append(incomeRail6);

    // auto incomeRail7 = new Network(LogicItemState::lisActive);
    // incomeRail7->Append(new WiFiApBinding(MapIO::V4, "ssid1234567890abcd", "passwordyqDQ", "*"));
    // Append(incomeRail7);

    // auto incomeRail8 = new Network(LogicItemState::lisActive);
    // incomeRail8->Append(new WiFiBinding(MapIO::V4, "ssiyQd1234567890abcd"));
    // Append(incomeRail8);

    // // auto incomeRail9 = new Network(LogicItemState::lisActive);
    // // incomeRail9->Append(new WiFiBinding(MapIO::V4, "ssid123"));
    // // Append(incomeRail9);

    // auto incomeRail5 = new Network(LogicItemState::lisActive);
    // incomeRail5->Append(new ComparatorGE(254, MapIO::V3));
    // incomeRail5->Append(new IncOutput(MapIO::V4));
    // incomeRail5->Append(new ResetOutput(MapIO::V3));
    // Append(incomeRail5);

    // auto incomeRail33 = new Network(LogicItemState::lisActive);
    // incomeRail33->Append(
    //     new SettingsElement(SettingsElement::Discriminator::t_wifi_station_settings_ssid));
    // Append(incomeRail33);

    // auto incomeRail44 = new Network(LogicItemState::lisActive);
    // incomeRail44->Append(new DateTimeBinding(MapIO::V2));
    // Append(incomeRail44);

    // auto incomeRail45 = new Network(LogicItemState::lisActive);
    // incomeRail45->Append(new Indicator(MapIO::V2, 255, 0, 0));
    // Append(incomeRail45);

    // auto incomeRail314 = new Network(LogicItemState::lisActive);
    // incomeRail314->Append(new TimerSecs(9));
    // incomeRail314->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail314);

    // auto incomeRail315 = new Network(LogicItemState::lisActive);
    // incomeRail315->Append(new TimerSecs(91));
    // incomeRail315->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail315);

    // auto incomeRail316 = new Network(LogicItemState::lisActive);
    // incomeRail316->Append(new TimerSecs(234));
    // incomeRail316->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail316);

    // auto incomeRail317 = new Network(LogicItemState::lisActive);
    // incomeRail317->Append(new TimerMSecs(2345));
    // incomeRail317->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail317);

    // auto incomeRail318 = new Network(LogicItemState::lisActive);
    // incomeRail318->Append(new TimerMSecs(23456));
    // incomeRail318->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail318);

    // auto incomeRail34 = new Network(LogicItemState::lisActive);
    // incomeRail34->Append(new ComparatorLE(1, MapIO::A0));
    // incomeRail34->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail34);

    // auto incomeRail35 = new Network(LogicItemState::lisActive);
    // incomeRail35->Append(new ComparatorGE(254, MapIO::A0));
    // incomeRail35->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail35);

    // auto incomeRail36 = new Network(LogicItemState::lisActive);
    // incomeRail36->Append(new ComparatorGr(76, MapIO::A0));
    // incomeRail36->Append(new DirectOutput(MapIO::O1));
    // Append(incomeRail36);

    // // auto incomeRail37 = new Network(LogicItemState::lisActive);
    // // incomeRail37->Append(new ComparatorGE(254, MapIO::V4));
    // // incomeRail37->Append(new ResetOutput(MapIO::V4));
    // // Append(incomeRail37);

    // // for (size_t i = 1; i <= 40; i++) {
    // //     auto incomeRail = new Network(LogicItemState::lisActive);
    // //     incomeRail->Append(new InputNC(MapIO::D0));
    // //     incomeRail->Append(new TimerSecs(i));
    // //     incomeRail->Append(new IncOutput(MapIO::V2));
    // //     Append(incomeRail);
    // // }

    // // auto incomeRail7 = new Network(LogicItemState::lisActive);
    // // incomeRail7->Append(new ComparatorGE(125, MapIO::V4));
    // // incomeRail7->Append(new DirectOutput(MapIO::O3));
    // // Append(incomeRail7);

    // // auto incomeRail8 = new Network(LogicItemState::lisActive);
    // // incomeRail8->Append(new WiFiBinding(MapIO::V2, "iPhone_ViorXV"));
    // // Append(incomeRail8);

    // // auto incomeRail9 = new Network(LogicItemState::lisActive);
    // // incomeRail9->Append(new InputNO(MapIO::V2));
    // // incomeRail9->Append(new DirectOutput(MapIO::O0));
    // // Append(incomeRail9);

    // // auto incomeRail10 = new Network(LogicItemState::lisActive);
    // // incomeRail10->Append(new InputNC(MapIO::D0));
    // // incomeRail10->Append(new DirectOutput(MapIO::V2));
    // // Append(incomeRail10);
}

void Ladder::Store() {
    redundant_storage storage;
    storage.size = Serialize(NULL, 0);
    if (storage.size == 0) {
        log_e(TAG_Ladder, "serialization setup error");
        return;
    }
    storage.data = new uint8_t[storage.size];
    storage.version = LADDER_VERSION;

    if (Serialize(storage.data, storage.size) != storage.size) {
        log_e(TAG_Ladder, "serialize error");
        delete[] storage.data;
        return;
    }

    redundant_storage_store(storage_0_partition,
                            storage_0_path,
                            storage_1_partition,
                            storage_1_path,
                            ladder_storage_name,
                            &storage);
    delete[] storage.data;
}

size_t Ladder::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;

    uint16_t networks_count;
    if (!Record::Read(&networks_count, sizeof(networks_count), buffer, buffer_size, &readed)) {
        log_e(TAG_Ladder, "Deserialize, count read error");
        return 0;
    }
    if (networks_count < Ladder::MinNetworksCount) {
        log_e(TAG_Ladder, "Deserialize, count min limit");
        return 0;
    }
    if (networks_count > Ladder::MaxNetworksCount) {
        log_e(TAG_Ladder, "Deserialize, count max limit");
        return 0;
    }

    reserve(networks_count);
    for (size_t i = 0; i < networks_count; i++) {
        auto network = new Network();
        size_t network_readed = network->Deserialize(&buffer[readed], buffer_size - readed);
        if (network_readed == 0) {
            delete network;
            log_e(TAG_Ladder, "Deserialize, network read error");
            return 0;
        }
        readed += network_readed;
        Append(network);
    }
    return readed;
}

size_t Ladder::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;

    uint16_t networks_count = size();
    if (networks_count < Ladder::MinNetworksCount) {
        return 0;
    }
    if (networks_count > Ladder::MaxNetworksCount) {
        return 0;
    }

    if (!Record::Write(&networks_count, sizeof(networks_count), buffer, buffer_size, &writed)) {
        return 0;
    }

    for (auto it = begin(); it != end(); ++it) {
        auto *network = *it;
        uint8_t *p;
        bool just_obtain_size = buffer == NULL;
        if (!just_obtain_size) {
            p = &buffer[writed];
        } else {
            p = NULL;
        }

        size_t network_writed = network->Serialize(p, buffer_size - writed);
        bool network_serialize_error = network_writed == 0;
        if (network_serialize_error) {
            return 0;
        }
        writed += network_writed;
    }

    return writed;
}

void Ladder::DeleteStorage() {
    log_i(TAG_Ladder, "Delete storage");
    redundant_storage_delete(storage_0_partition,
                             storage_0_path,
                             storage_1_partition,
                             storage_1_path,
                             ladder_storage_name);
}