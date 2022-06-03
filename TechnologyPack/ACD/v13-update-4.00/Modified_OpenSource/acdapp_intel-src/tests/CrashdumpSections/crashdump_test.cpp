/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2019 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials,
 * and your use of them is governed by the express license under which they
 * were provided to you ("License"). Unless the License provides otherwise,
 * you may not use, modify, copy, publish, distribute, disclose or transmit
 * this software or the related documents without Intel's prior written
 * permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in the
 * License.
 *
 ******************************************************************************/
#include "../mock/test_crashdump.hpp"
#include "crashdump.hpp"

#include <fstream>
#include <iostream>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

extern "C" {
#include "CrashdumpSections/Uncore.h"
}

using namespace ::testing;
using namespace crashdump;

TEST(CrashdumpTest, createCrashdump_SPR)
{
    std::cerr
        << "***** This test should take about 210 seconds to complete *****"
        << std::endl;

    TestCrashdump crashdump(cd_spr, 700);
    std::string crashdumpContents;
    std::string triggerType = "UnitTest";
    std::string timestamp = newTimestamp();
    bool isTelemetry = false;

    crashdump.libPeciMock->DelegateForCrashdumpSetUp();

    uint8_t returnBusValidQuery[4] = {0x3f, 0x0f, 0x00,
                                      0xc0}; // 0xc0000f3f; bit 30 set
    uint8_t returnEnumeratedBus[4] = {0x00, 0x00, 0x7e, 0x7f}; // 0x7f7e0000;

    uint8_t coreMaskHigh[4] = {0x55, 0x8c, 0x47, 0x09}; // 0x09478c55;
    uint8_t coreMaskLow[4] = {0xb5, 0xb0, 0x36, 0x52};  // 0x5236b0b5;
    uint8_t cc = PECI_DEV_CC_SUCCESS;
    uint8_t cc2 = PECI_DEV_CC_FATAL_MCA_DETECTED;

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal)
        .WillOnce(DoAll(
            SetArrayArgument<7>(coreMaskHigh,
                                coreMaskHigh + 4), // core mask higher dword
            SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(
            DoAll(SetArrayArgument<7>(coreMaskLow,
                                      coreMaskLow + 4), // core mask lower dword
                  SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(coreMaskHigh,
                                            coreMaskHigh + 4), // cha mask 0
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(coreMaskLow,
                                            coreMaskLow + 4), // cha mask 1
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnBusValidQuery, returnBusValidQuery + 4),
            SetArgPointee<8>(cc2),
            Return(PECI_CC_SUCCESS))) // enumerated bus query
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnEnumeratedBus, returnEnumeratedBus + 4),
            SetArgPointee<8>(cc2),
            Return(PECI_CC_SUCCESS))); // enumerated bus query

    EXPECT_CALL(*crashdump.libPeciMock, peci_GetCPUID)
        .WillOnce(DoAll(
            SetArgPointee<1>((CPUModel)0x000806F0), SetArgPointee<2>(1),
            SetArgPointee<3>(PECI_DEV_CC_SUCCESS), Return(PECI_CC_SUCCESS)));

    createCrashdump(crashdump.cpusInfo, crashdumpContents, triggerType,
                    timestamp, isTelemetry);

    cJSON* output = cJSON_Parse(crashdumpContents.c_str());
    cJSON* metadata_section = output->child->child;
    cJSON* MCA_section = output->child->child->next->child->next->child;
    cJSON* uncore_section = MCA_section->next;
    cJSON* TOR_section = uncore_section->next;
    cJSON* pm_info_section = TOR_section->next;
    cJSON* big_core_section = pm_info_section->next;

    ASSERT_NE(metadata_section, nullptr);
    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(metadata_section->child, "cha_count");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, "0x1c");

    actual =
        cJSON_GetObjectItemCaseSensitive(metadata_section->child, "core_mask");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, "0x5236b0b509478c55");

    ASSERT_NE(MCA_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(MCA_section, "uncore");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(uncore_section, nullptr);
    actual =
        cJSON_GetObjectItemCaseSensitive(uncore_section, PCI_ABORT_MSG_KEY);
    ASSERT_NE(actual, nullptr);
    EXPECT_THAT(actual->valuestring, HasSubstr("Max time 30 sec exceeded"));

    actual =
        cJSON_GetObjectItemCaseSensitive(uncore_section, "B31_D30_F4_0xfc");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, UNCORE_NA);

    actual =
        cJSON_GetObjectItemCaseSensitive(uncore_section, "B00_D00_F0_0x108");
    ASSERT_NE(actual, nullptr);
    EXPECT_STRNE(actual->valuestring, UNCORE_NA);

    actual =
        cJSON_GetObjectItemCaseSensitive(uncore_section, MMIO_ABORT_MSG_KEY);
    ASSERT_NE(actual, nullptr);
    EXPECT_THAT(actual->valuestring, HasSubstr("Max time 30 sec exceeded"));

    actual =
        cJSON_GetObjectItemCaseSensitive(uncore_section, "B30_D29_F0_0x23460");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, UNCORE_NA);

    actual =
        cJSON_GetObjectItemCaseSensitive(uncore_section, "B30_D26_F0_0x20150");
    ASSERT_NE(actual, nullptr);
    EXPECT_STRNE(actual->valuestring, UNCORE_NA);

    actual =
        cJSON_GetObjectItemCaseSensitive(uncore_section, RDIAMSR_ABORT_MSG_KEY);
    ASSERT_NE(actual, nullptr);
    EXPECT_THAT(actual->valuestring, HasSubstr("Max time 15 sec exceeded"));

    actual = cJSON_GetObjectItemCaseSensitive(uncore_section, "RDIAMSR_0xa01");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, UNCORE_NA);

    actual = cJSON_GetObjectItemCaseSensitive(uncore_section, "RDIAMSR_0x3fa");
    ASSERT_NE(actual, nullptr);
    EXPECT_STRNE(actual->valuestring, UNCORE_NA);

    ASSERT_NE(TOR_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(TOR_section, "cha0");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(pm_info_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(pm_info_section, "pm_0x002");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(big_core_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(big_core_section, "_version");
    ASSERT_NE(actual, nullptr);

    // Create the new crashdump filename
    std::string new_logfile_name =
        crashdumpPrefix + "unit_test" + "-" + timestamp + ".json";

    std::filesystem::path out_file = crashdumpDir / new_logfile_name;

    std::error_code ec;
    std::filesystem::create_directories(crashdumpDir, ec);
    ASSERT_EQ(ec.value(), 0) << "failed to create " << crashdumpDir.c_str()
                             << " : " << ec.message().c_str();

    std::cerr << std::endl
              << "Storing crashlog output file @ " << out_file << std::endl;

    // open the JSON file to write the crashdump contents
    std::fstream fpJson;
    fpJson.open(out_file.c_str(), std::ios::out);
    if (fpJson)
    {
        fpJson << crashdumpContents;
        fpJson.close();
    }
}

TEST(CrashdumpTest, createCrashdump_SPR_2Sockets)
{
    std::cerr
        << "***** This test should take about 420 seconds to complete *****"
        << std::endl;

    TestCrashdump crashdump(cd_spr, 700, 2);
    std::string crashdumpContents;
    std::string triggerType = "UnitTest";
    std::string timestamp = newTimestamp();
    bool isTelemetry = false;

    crashdump.libPeciMock->DelegateForCrashdumpSetUpFor2Sockets();

    uint8_t returnBusValidQuery[4] = {0x3f, 0x0f, 0x00,
                                      0xc0}; // 0xc0000f3f; bit 30 set
    uint8_t returnEnumeratedBus[4] = {0x00, 0x00, 0x7e, 0x7f}; // 0x7f7e0000;

    uint8_t coreMaskHigh[4] = {0x55, 0x8c, 0x47, 0x09}; // 0x09478c55;
    uint8_t coreMaskLow[4] = {0xb5, 0xb0, 0x36, 0x52};  // 0x5236b0b5;
    uint8_t cc = PECI_DEV_CC_SUCCESS;
    uint8_t cc2 = PECI_DEV_CC_FATAL_MCA_DETECTED;

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal)
        .WillOnce(DoAll(
            SetArrayArgument<7>(coreMaskHigh,
                                coreMaskHigh + 4), // core mask higher dword
            SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(
            DoAll(SetArrayArgument<7>(coreMaskLow,
                                      coreMaskLow + 4), // core mask lower dword
                  SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(coreMaskHigh,
                                            coreMaskHigh + 4), // cha mask 0
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(coreMaskLow,
                                            coreMaskLow + 4), // cha mask 1
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(
                            coreMaskHigh,
                            coreMaskHigh + 4), // CPU 2 core mask higher dword
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(
            SetArrayArgument<7>(coreMaskLow,
                                coreMaskLow + 4), // CPU 2 core mask lower dword
            SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(
            DoAll(SetArrayArgument<7>(coreMaskHigh,
                                      coreMaskHigh + 4), // CPU 2 cha mask 0
                  SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(
            DoAll(SetArrayArgument<7>(coreMaskLow,
                                      coreMaskLow + 4), // CPU 2 cha mask 1
                  SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnBusValidQuery, returnBusValidQuery + 4),
            SetArgPointee<8>(cc2),
            Return(PECI_CC_SUCCESS))) //  enumerated bus query
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnEnumeratedBus, returnEnumeratedBus + 4),
            SetArgPointee<8>(cc2),
            Return(PECI_CC_SUCCESS))) // enumerated bus query
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnBusValidQuery, returnBusValidQuery + 4),
            SetArgPointee<8>(cc2),
            Return(PECI_CC_SUCCESS))) // CPU 2 enumerated bus query
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnEnumeratedBus, returnEnumeratedBus + 4),
            SetArgPointee<8>(cc2),
            Return(PECI_CC_SUCCESS))); // CPU 2 enumerated bus query

    EXPECT_CALL(*crashdump.libPeciMock, peci_GetCPUID)
        .WillOnce(DoAll(
            SetArgPointee<1>((CPUModel)0x000806F0), SetArgPointee<2>(1),
            SetArgPointee<3>(PECI_DEV_CC_SUCCESS), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(
            SetArgPointee<1>((CPUModel)0x000806F0), SetArgPointee<2>(1),
            SetArgPointee<3>(PECI_DEV_CC_SUCCESS), Return(PECI_CC_SUCCESS)));

    createCrashdump(crashdump.cpusInfo, crashdumpContents, triggerType,
                    timestamp, isTelemetry);

    cJSON* output = cJSON_Parse(crashdumpContents.c_str());
    cJSON* metadata_section = output->child->child;
    cJSON* cpu0_MCA_section = metadata_section->next->child->next->child;
    cJSON* cpu0_uncore_section = cpu0_MCA_section->next;
    cJSON* cpu0_TOR_section = cpu0_uncore_section->next;
    cJSON* cpu0_pm_info_section = cpu0_TOR_section->next;
    cJSON* cpu0_big_core_section = cpu0_pm_info_section->next;

    cJSON* cpu1_MCA_section = metadata_section->next->child->next->next->child;
    cJSON* cpu1_uncore_section = cpu1_MCA_section->next;
    cJSON* cpu1_TOR_section = cpu1_uncore_section->next;
    cJSON* cpu1_pm_info_section = cpu1_TOR_section->next;
    cJSON* cpu1_big_core_section = cpu1_pm_info_section->next;

    ASSERT_NE(metadata_section, nullptr);
    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(metadata_section->child, "cha_count");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, "0x1c");

    actual =
        cJSON_GetObjectItemCaseSensitive(metadata_section->child, "core_mask");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, "0x5236b0b509478c55");

    ASSERT_NE(cpu0_MCA_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(cpu0_MCA_section, "uncore");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(cpu0_uncore_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(cpu0_uncore_section,
                                              PCI_ABORT_MSG_KEY);
    ASSERT_NE(actual, nullptr);
    EXPECT_THAT(actual->valuestring, HasSubstr("Max time 30 sec exceeded"));

    actual = cJSON_GetObjectItemCaseSensitive(cpu0_uncore_section,
                                              "B31_D30_F4_0xfc");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, UNCORE_NA);

    actual = cJSON_GetObjectItemCaseSensitive(cpu0_uncore_section,
                                              "B00_D00_F0_0x108");
    ASSERT_NE(actual, nullptr);
    EXPECT_STRNE(actual->valuestring, UNCORE_NA);

    actual = cJSON_GetObjectItemCaseSensitive(cpu0_uncore_section,
                                              MMIO_ABORT_MSG_KEY);
    ASSERT_NE(actual, nullptr);
    EXPECT_THAT(actual->valuestring, HasSubstr("Max time 30 sec exceeded"));

    actual = cJSON_GetObjectItemCaseSensitive(cpu0_uncore_section,
                                              "B30_D29_F0_0x23460");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, UNCORE_NA);

    actual = cJSON_GetObjectItemCaseSensitive(cpu0_uncore_section,
                                              "B30_D26_F0_0x20150");
    ASSERT_NE(actual, nullptr);
    EXPECT_STRNE(actual->valuestring, UNCORE_NA);

    actual = cJSON_GetObjectItemCaseSensitive(cpu0_uncore_section,
                                              RDIAMSR_ABORT_MSG_KEY);
    ASSERT_NE(actual, nullptr);
    EXPECT_THAT(actual->valuestring, HasSubstr("Max time 15 sec exceeded"));

    actual =
        cJSON_GetObjectItemCaseSensitive(cpu0_uncore_section, "RDIAMSR_0xa01");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, UNCORE_NA);

    actual =
        cJSON_GetObjectItemCaseSensitive(cpu0_uncore_section, "RDIAMSR_0x3fa");
    ASSERT_NE(actual, nullptr);
    EXPECT_STRNE(actual->valuestring, UNCORE_NA);

    ASSERT_NE(cpu0_TOR_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(cpu0_TOR_section, "cha0");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(cpu0_pm_info_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(cpu0_pm_info_section, "pm_0x002");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(cpu0_big_core_section, nullptr);
    actual =
        cJSON_GetObjectItemCaseSensitive(cpu0_big_core_section, "_version");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(cpu1_MCA_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(cpu1_MCA_section, "uncore");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(cpu1_uncore_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(cpu1_uncore_section,
                                              PCI_ABORT_MSG_KEY);
    ASSERT_NE(actual, nullptr);
    EXPECT_THAT(actual->valuestring, HasSubstr("Max time 30 sec exceeded"));

    actual = cJSON_GetObjectItemCaseSensitive(cpu1_uncore_section,
                                              "B31_D30_F4_0xfc");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, UNCORE_NA);

    actual = cJSON_GetObjectItemCaseSensitive(cpu1_uncore_section,
                                              "B00_D00_F0_0x108");
    ASSERT_NE(actual, nullptr);
    EXPECT_STRNE(actual->valuestring, UNCORE_NA);

    actual = cJSON_GetObjectItemCaseSensitive(cpu1_uncore_section,
                                              MMIO_ABORT_MSG_KEY);
    ASSERT_NE(actual, nullptr);
    EXPECT_THAT(actual->valuestring, HasSubstr("Max time 30 sec exceeded"));

    actual = cJSON_GetObjectItemCaseSensitive(cpu1_uncore_section,
                                              "B30_D29_F0_0x23460");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, UNCORE_NA);

    actual = cJSON_GetObjectItemCaseSensitive(cpu1_uncore_section,
                                              "B30_D26_F0_0x20150");
    ASSERT_NE(actual, nullptr);
    EXPECT_STRNE(actual->valuestring, UNCORE_NA);

    actual = cJSON_GetObjectItemCaseSensitive(cpu1_uncore_section,
                                              RDIAMSR_ABORT_MSG_KEY);
    ASSERT_NE(actual, nullptr);
    EXPECT_THAT(actual->valuestring, HasSubstr("Max time 15 sec exceeded"));

    actual =
        cJSON_GetObjectItemCaseSensitive(cpu1_uncore_section, "RDIAMSR_0xa01");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, UNCORE_NA);

    actual =
        cJSON_GetObjectItemCaseSensitive(cpu1_uncore_section, "RDIAMSR_0x3fa");
    ASSERT_NE(actual, nullptr);
    EXPECT_STRNE(actual->valuestring, UNCORE_NA);

    ASSERT_NE(cpu1_TOR_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(cpu1_TOR_section, "cha0");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(cpu1_pm_info_section, nullptr);
    actual = cJSON_GetObjectItemCaseSensitive(cpu1_pm_info_section, "pm_0x002");
    ASSERT_NE(actual, nullptr);

    ASSERT_NE(cpu1_big_core_section, nullptr);
    actual =
        cJSON_GetObjectItemCaseSensitive(cpu1_big_core_section, "_version");
    ASSERT_NE(actual, nullptr);

    // Create the new crashdump filename
    std::string new_logfile_name =
        crashdumpPrefix + "unit_test" + "-" + timestamp + ".json";

    std::filesystem::path out_file = crashdumpDir / new_logfile_name;

    std::error_code ec;
    std::filesystem::create_directories(crashdumpDir, ec);
    ASSERT_EQ(ec.value(), 0) << "failed to create " << crashdumpDir.c_str()
                             << " : " << ec.message().c_str();

    std::cerr << std::endl
              << "Storing crashlog output file @ " << out_file << std::endl;

    // open the JSON file to write the crashdump contents
    std::fstream fpJson;
    fpJson.open(out_file.c_str(), std::ios::out);
    if (fpJson)
    {
        fpJson << crashdumpContents;
        fpJson.close();
    }
}

TEST(CrashdumpTest, createCrashdump_SPR_no_delay)
{
    TestCrashdump crashdump(cd_spr, 0);
    std::string crashdumpContents;
    std::string triggerType = "UnitTest";
    std::string timestamp = newTimestamp();
    bool isTelemetry = false;

    crashdump.libPeciMock->DelegateForCrashdumpSetUp();

    uint8_t returnBusValidQuery[4] = {0x3f, 0x0f, 0x00,
                                      0xc0}; // 0xc0000f3f; bit 30 set
    uint8_t returnEnumeratedBus[4] = {0x00, 0x00, 0x7e, 0x7f}; // 0x7f7e0000;

    uint8_t coreMaskHigh[4] = {0x55, 0x8c, 0x47, 0x09}; // 0x09478c55;
    uint8_t coreMaskLow[4] = {0xb5, 0xb0, 0x36, 0x52};  // 0x5236b0b5;
    uint8_t cc = PECI_DEV_CC_SUCCESS;
    uint8_t cc2 = PECI_DEV_CC_FATAL_MCA_DETECTED;

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal)
        .WillOnce(DoAll(
            SetArrayArgument<7>(coreMaskHigh,
                                coreMaskHigh + 4), // core mask higher dword
            SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(
            DoAll(SetArrayArgument<7>(coreMaskLow,
                                      coreMaskLow + 4), // core mask lower dword
                  SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(coreMaskHigh,
                                            coreMaskHigh + 4), // cha mask 0
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(coreMaskLow,
                                            coreMaskLow + 4), // cha mask 1
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnBusValidQuery, returnBusValidQuery + 4),
            SetArgPointee<8>(cc2),
            Return(PECI_CC_SUCCESS))) // enumerated bus query
        .WillOnce(DoAll(
            SetArrayArgument<7>(returnEnumeratedBus, returnEnumeratedBus + 4),
            SetArgPointee<8>(cc2),
            Return(PECI_CC_SUCCESS))); // enumerated bus query

    EXPECT_CALL(*crashdump.libPeciMock, peci_GetCPUID)
        .WillOnce(DoAll(
            SetArgPointee<1>((CPUModel)0x000806F0), SetArgPointee<2>(1),
            SetArgPointee<3>(PECI_DEV_CC_SUCCESS), Return(PECI_CC_SUCCESS)));

    createCrashdump(crashdump.cpusInfo, crashdumpContents, triggerType,
                    timestamp, isTelemetry);

    cJSON* output = cJSON_Parse(crashdumpContents.c_str());
    cJSON* metadata_section = output->child->child;

    ASSERT_NE(metadata_section, nullptr);
    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(metadata_section, "_reset_detected");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, "NONE");

    actual =
        cJSON_GetObjectItemCaseSensitive(metadata_section, "_input_file_error");
    ASSERT_EQ(actual, nullptr);

    actual = cJSON_GetObjectItemCaseSensitive(metadata_section, "trigger_type");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, "UnitTest");

    // Create the new crashdump filename
    std::string new_logfile_name =
        crashdumpPrefix + "unit_test" + "-" + timestamp + ".json";

    std::filesystem::path out_file = crashdumpDir / new_logfile_name;

    std::error_code ec;
    std::filesystem::create_directories(crashdumpDir, ec);
    ASSERT_EQ(ec.value(), 0) << "failed to create " << crashdumpDir.c_str()
                             << " : " << ec.message().c_str();

    std::cerr << std::endl
              << "Storing crashlog output file @ " << out_file << std::endl;

    // open the JSON file to write the crashdump contents
    std::fstream fpJson;
    fpJson.open(out_file.c_str(), std::ios::out);
    if (fpJson)
    {
        fpJson << crashdumpContents;
        fpJson.close();
    }
}

TEST(CrashdumpTest, createCrashdump_SPR_no_delay_input_file_error)
{
    TestCrashdump crashdump(cd_spr, 0);
    std::string crashdumpContents;
    std::string triggerType = "UnitTest";
    std::string timestamp = newTimestamp();
    bool isTelemetry = false;

    {
        // EDIT THE INPUT FILE TO DELETE FIRST LINE
        std::string fileName =
            "crashdump_input_" +
            crashdump.cpuModelMap[crashdump.cpusInfo[0].model] + ".json";
        std::filesystem::path input_file = crashdump.targetPath / fileName;

        std::ifstream fpJson;
        fpJson.open(input_file.c_str(), std::ios::out | std::ios::in);
        std::stringstream inputFile;
        inputFile << fpJson.rdbuf();
        fpJson.close();
        std::string inputFileString = inputFile.str();

        // delete the first character
        inputFileString.erase(inputFileString.begin());

        // write back to the file after clearing it
        std::ofstream outFileJson;
        outFileJson.open(input_file.c_str(), std::ios::out | std::ios::trunc);
        outFileJson << inputFileString;
        outFileJson.close();
    }

    uint8_t coreMaskHigh[4] = {0x55, 0x8c, 0x47, 0x09}; // 0x09478c55;
    uint8_t coreMaskLow[4] = {0xb5, 0xb0, 0x36, 0x52};  // 0x5236b0b5;
    uint8_t cc = PECI_DEV_CC_SUCCESS;

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal)
        .WillOnce(DoAll(
            SetArrayArgument<7>(coreMaskHigh,
                                coreMaskHigh + 4), // core mask higher dword
            SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(
            DoAll(SetArrayArgument<7>(coreMaskLow,
                                      coreMaskLow + 4), // core mask lower dword
                  SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(coreMaskHigh,
                                            coreMaskHigh + 4), // cha mask 0
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<7>(coreMaskLow,
                                            coreMaskLow + 4), // cha mask 1
                        SetArgPointee<8>(cc), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*crashdump.libPeciMock, peci_GetCPUID)
        .WillOnce(DoAll(
            SetArgPointee<1>((CPUModel)0x000806F0), SetArgPointee<2>(1),
            SetArgPointee<3>(PECI_DEV_CC_SUCCESS), Return(PECI_CC_SUCCESS)));

    createCrashdump(crashdump.cpusInfo, crashdumpContents, triggerType,
                    timestamp, isTelemetry);

    cJSON* output = cJSON_Parse(crashdumpContents.c_str());
    cJSON* metadata_section = output->child->child;

    ASSERT_NE(metadata_section, nullptr);
    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(metadata_section, "_reset_detected");
    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, "NONE");

    actual =
        cJSON_GetObjectItemCaseSensitive(metadata_section, "_input_file_error");
    EXPECT_STREQ(actual->valuestring, "Error while reading input file");

    actual = cJSON_GetObjectItemCaseSensitive(metadata_section, "trigger_type");
    ASSERT_EQ(actual, nullptr);

    // Create the new crashdump filename
    std::string new_logfile_name =
        crashdumpPrefix + "unit_test" + "-" + timestamp + ".json";

    std::filesystem::path out_file = crashdumpDir / new_logfile_name;

    std::error_code ec;
    std::filesystem::create_directories(crashdumpDir, ec);
    ASSERT_EQ(ec.value(), 0) << "failed to create " << crashdumpDir.c_str()
                             << " : " << ec.message().c_str();

    std::cerr << std::endl
              << "Storing crashlog output file @ " << out_file << std::endl;

    // open the JSON file to write the crashdump contents
    std::fstream fpJson;
    fpJson.open(out_file.c_str(), std::ios::out);
    if (fpJson)
    {
        fpJson << crashdumpContents;
        fpJson.close();
    }
}