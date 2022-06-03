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
#include "CrashdumpSections/UncoreMca.h"
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"

TEST(UncoreMcaTestFixture, logUncoreMca_Invalid_Model)
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
    ret = logUncoreMca(&cpuInfo, root);
    EXPECT_EQ(ret, ACD_FAILURE);
}

TEST(UncoreMcaTestFixture, logUncoreMca_NULL_pJsonChild)
{
    int ret;

    TestCrashdump crashdump(cd_spr);
    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logUncoreMca(&cpuinfo, NULL);
        EXPECT_EQ(ret, ACD_INVALID_OBJECT);
    }
}

TEST(UncoreMcaTestFixture, logUncoreMca_fullFlow)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 2,
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

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdIAMSR)
        .WillRepeatedly(DoAll(SetArgPointee<3>(0xAABBCCDDEEFFABAC),
                              SetArgPointee<4>(PECI_DEV_CC_SUCCESS),
                              Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logUncoreMca(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    cJSON* actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "uncore");
    EXPECT_EQ(cJSON_IsObject(actual), true);

    cJSON* mc4_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "MC4");
    EXPECT_EQ(cJSON_IsObject(mc4_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_ctl");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_addr");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_misc");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_ctl2");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");

    cJSON* upi0_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "UPI0");
    EXPECT_EQ(cJSON_IsObject(upi0_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_addr");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_misc");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_ctl");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_ctl2");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");

    cJSON* M2MEM0_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "M2MEM0");
    EXPECT_EQ(cJSON_IsObject(M2MEM0_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_addr");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_misc");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_ctl");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_ctl2");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");

    cJSON* MDF1_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "MDF1");
    EXPECT_EQ(cJSON_IsObject(MDF1_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_addr");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_misc");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_ctl");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_ctl2");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");

    cJSON* CBO1_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "CBO1");
    EXPECT_EQ(cJSON_IsObject(CBO1_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_addr");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_misc");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_misc3");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_ctl");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_ctl2");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac");
}

TEST(UncoreMcaTestFixture, logUncoreMca_fullFlow_cc_0x80_ret_0x00)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 2,
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

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdIAMSR)
        .WillRepeatedly(DoAll(SetArgPointee<3>(0xAABBCCDDEEFFABAC),
                              SetArgPointee<4>(PECI_DEV_CC_NEED_RETRY),
                              Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logUncoreMca(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    cJSON* actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "uncore");
    EXPECT_EQ(cJSON_IsObject(actual), true);

    cJSON* mc4_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "MC4");
    EXPECT_EQ(cJSON_IsObject(mc4_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac,CC:0x80,RC:0x0");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* upi0_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "UPI0");
    EXPECT_EQ(cJSON_IsObject(upi0_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac,CC:0x80,RC:0x0");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* M2MEM0_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "M2MEM0");
    EXPECT_EQ(cJSON_IsObject(M2MEM0_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac,CC:0x80,RC:0x0");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* MDF1_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "MDF1");
    EXPECT_EQ(cJSON_IsObject(MDF1_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac,CC:0x80,RC:0x0");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* CBO1_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "CBO1");
    EXPECT_EQ(cJSON_IsObject(CBO1_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_status");
    EXPECT_STREQ(actual->valuestring, "0xaabbccddeeffabac,CC:0x80,RC:0x0");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_misc3");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");
}

TEST(UncoreMcaTestFixture, logUncoreMca_fullFlow_cc_0x80_ret_0x06)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0x0,
                       .sectionMask = 0,
                       .chaCount = 2,
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

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdIAMSR)
        .WillRepeatedly(DoAll(SetArgPointee<3>(0xAABBCCDDEEFFABAC),
                              SetArgPointee<4>(PECI_DEV_CC_NEED_RETRY),
                              Return(PECI_CC_TIMEOUT)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logUncoreMca(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    cJSON* actual = cJSON_GetObjectItemCaseSensitive(crashdump.root, "uncore");
    EXPECT_EQ(cJSON_IsObject(actual), true);

    cJSON* mc4_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "MC4");
    EXPECT_EQ(cJSON_IsObject(mc4_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_status");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x80,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(mc4_actual, "mc4_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* upi0_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "UPI0");
    EXPECT_EQ(cJSON_IsObject(upi0_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_status");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x80,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(upi0_actual, "upi0_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* M2MEM0_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "M2MEM0");
    EXPECT_EQ(cJSON_IsObject(M2MEM0_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_status");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x80,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(M2MEM0_actual, "m2mem0_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* MDF1_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "MDF1");
    EXPECT_EQ(cJSON_IsObject(MDF1_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_status");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x80,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(MDF1_actual, "mdf1_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");

    cJSON* CBO1_actual =
        cJSON_GetObjectItemCaseSensitive(crashdump.root->child, "CBO1");
    EXPECT_EQ(cJSON_IsObject(CBO1_actual), true);
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_status");
    EXPECT_STREQ(actual->valuestring, "0x0,CC:0x80,RC:0x6");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_addr");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_misc");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_misc3");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_ctl");
    EXPECT_STREQ(actual->valuestring, "N/A");
    actual = cJSON_GetObjectItemCaseSensitive(CBO1_actual, "cbo1_ctl2");
    EXPECT_STREQ(actual->valuestring, "N/A");
}