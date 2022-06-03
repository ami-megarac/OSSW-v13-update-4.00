/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2021 Intel Corporation.
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

extern "C" {
#include "CrashdumpSections/PowerManagement.h"
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(PowerManagementTestFixture, logPowerManagement_Invalid_Model)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_numberOfModels, // invalid model#
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};
    cJSON* root = cJSON_CreateObject();
    ret = logPowerManagement(&cpuInfo, root);
    EXPECT_EQ(ret, ACD_FAILURE);
}

TEST(PowerManagementTestFixture, logPowerManagement_NULL_pJsonChild)
{
    int ret;

    TestCrashdump crashdump(cd_spr);
    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logPowerManagement(&cpuinfo, NULL);
        EXPECT_EQ(ret, ACD_INVALID_OBJECT);
    }
}

TEST(PowerManagementTestFixture, logPowerManagement_fullFlow)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x90, 0x80};
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(DoAll(SetArrayArgument<4>(returnValue, returnValue + 2),
                              SetArgPointee<5>(PECI_DEV_CC_SUCCESS),
                              Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logPowerManagement(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x001");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x002");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x003");
    EXPECT_STREQ(actual->valuestring, "0x8090");

    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x407");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x507");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x607");
    EXPECT_STREQ(actual->valuestring, "0x8090");

    cJSON* pm_subindex_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_index0");
    ASSERT_NE(pm_subindex_node, nullptr);

    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x010b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x110b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x210b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x310b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x410b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x510b");
    EXPECT_STREQ(actual->valuestring, "0x8090");

    cJSON* dispatcher_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "dispatcher");
    ASSERT_NE(dispatcher_node, nullptr);

    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "shared_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "shared_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "shared_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "sequencer_idx0");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "sequencer_idx1");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "sequencer_idx2");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(dispatcher_node, "cfc_pma_idx0");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(dispatcher_node, "cfc_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(dispatcher_node, "cfc_pma_idx2");
    EXPECT_STREQ(actual->valuestring, "0x8090");
}

TEST(PowerManagementTestFixture, logPowerManagement_ICX_fullFlow)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x90, 0x80};
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(DoAll(SetArrayArgument<4>(returnValue, returnValue + 2),
                              SetArgPointee<5>(PECI_DEV_CC_SUCCESS),
                              Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logPowerManagement(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x001");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x002");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x003");
    EXPECT_STREQ(actual->valuestring, "0x8090");

    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x407");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x507");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x607");
    EXPECT_STREQ(actual->valuestring, "0x8090");

    cJSON* pm_subindex_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_index0");
    ASSERT_NE(pm_subindex_node, nullptr);

    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x010b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x110b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x210b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x310b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x410b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x510b");
    EXPECT_STREQ(actual->valuestring, "0x8090");
}

TEST(PowerManagementTestFixture, logPowerManagement_error_return)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x90, 0x80};
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(DoAll(SetArrayArgument<4>(returnValue, returnValue + 2),
                              SetArgPointee<5>(PECI_DEV_CC_FATAL_MCA_DETECTED),
                              Return(PECI_CC_TIMEOUT)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logPowerManagement(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x001");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x002");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x003");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");

    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x407");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x507");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x607");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");

    cJSON* pm_subindex_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_index0");
    ASSERT_NE(pm_subindex_node, nullptr);

    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x010b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x110b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x210b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x310b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x410b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x510b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");

    cJSON* dispatcher_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "dispatcher");
    ASSERT_NE(dispatcher_node, nullptr);

    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "shared_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "shared_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "shared_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "sequencer_idx0");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "sequencer_idx1");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "sequencer_idx2");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(dispatcher_node, "cfc_pma_idx0");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(dispatcher_node, "cfc_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(dispatcher_node, "cfc_pma_idx2");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
}

TEST(PowerManagementTestFixture, logPowerManagement_ICX_error_return)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x90, 0x80};
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(DoAll(SetArrayArgument<4>(returnValue, returnValue + 2),
                              SetArgPointee<5>(PECI_DEV_CC_FATAL_MCA_DETECTED),
                              Return(PECI_CC_TIMEOUT)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logPowerManagement(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x001");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x002");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x003");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");

    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x407");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x507");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x607");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");

    cJSON* pm_subindex_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_index0");
    ASSERT_NE(pm_subindex_node, nullptr);

    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x010b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x110b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x210b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x310b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x410b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x510b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x94,RC:0x6");

    cJSON* dispatcher_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "dispatcher");
    ASSERT_EQ(dispatcher_node, nullptr);
}

TEST(PowerManagementTestFixture, logPowerManagement_error_return_skip_section)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x90, 0x80};
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(
            DoAll(SetArrayArgument<4>(returnValue, returnValue + 2),
                  SetArgPointee<5>(PECI_DEV_CC_CATASTROPHIC_MCA_ERROR),
                  Return(PECI_CC_TIMEOUT)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logPowerManagement(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x001");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x93,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x002");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x003");
    EXPECT_STREQ(actual->valuestring, "N/A");

    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x407");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x507");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x607");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* pm_subindex_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_index0");
    ASSERT_NE(pm_subindex_node, nullptr);

    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x010b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x93,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x110b");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x210b");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x310b");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x410b");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x510b");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* dispatcher_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "dispatcher");
    ASSERT_NE(dispatcher_node, nullptr);

    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "shared_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "shared_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "shared_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "sequencer_idx0");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "sequencer_idx1");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual =
        cJSON_GetObjectItemCaseSensitive(dispatcher_node, "sequencer_idx2");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(dispatcher_node, "cfc_pma_idx0");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(dispatcher_node, "cfc_pma_idx1");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(dispatcher_node, "cfc_pma_idx2");
    EXPECT_STREQ(actual->valuestring, "N/A");
}

TEST(PowerManagementTestFixture,
     logPowerManagement_ICX_error_return_skip_section)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 0,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    EXPECT_CALL(*crashdump.libPeciMock, peci_Lock)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    uint8_t returnValue[2] = {0x90, 0x80};
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(
            DoAll(SetArrayArgument<4>(returnValue, returnValue + 2),
                  SetArgPointee<5>(PECI_DEV_CC_CATASTROPHIC_MCA_ERROR),
                  Return(PECI_CC_TIMEOUT)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logPowerManagement(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    cJSON* actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x001");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x93,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x002");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x003");
    EXPECT_STREQ(actual->valuestring, "N/A");

    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x407");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x507");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_0x607");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* pm_subindex_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "pm_index0");
    ASSERT_NE(pm_subindex_node, nullptr);

    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x010b");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x93,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x110b");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x210b");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x310b");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x410b");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(pm_subindex_node,
                                              "pm_subindex_0x510b");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* dispatcher_node =
        cJSON_GetObjectItemCaseSensitive(crashdump.root, "dispatcher");
    ASSERT_EQ(dispatcher_node, nullptr);
}