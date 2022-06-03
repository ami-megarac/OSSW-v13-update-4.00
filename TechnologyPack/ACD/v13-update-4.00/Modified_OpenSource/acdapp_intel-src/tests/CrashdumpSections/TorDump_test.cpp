#include "../mock/test_crashdump.hpp"

extern "C" {
#include "CrashdumpSections/TorDump.h"
#include "CrashdumpSections/crashdump.h"
#include "CrashdumpSections/utils.h"
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using ::testing::Return;

void validateSubIndexData(cJSON* output, int cha, int index, int subIndex,
                          std::string expectedValue)
{
    char sCha[8];
    char sIndex[9];
    char sSubIndex[12];
    sprintf(sCha, "cha%d", cha);
    sprintf(sIndex, "index%d", index);
    sprintf(sSubIndex, "subindex%d", subIndex);

    ASSERT_TRUE(cJSON_HasObjectItem(output, sCha));
    cJSON* jCha = cJSON_CreateObject();
    jCha = cJSON_GetObjectItemCaseSensitive(output, sCha);
    ASSERT_NE(jCha, nullptr);
    ASSERT_TRUE(cJSON_HasObjectItem(jCha, sIndex));

    cJSON* jIndex = cJSON_CreateObject();
    jIndex = cJSON_GetObjectItemCaseSensitive(jCha, sIndex);
    ASSERT_NE(jIndex, nullptr);
    ASSERT_TRUE(cJSON_HasObjectItem(jIndex, sSubIndex));

    cJSON* jSubIndex = cJSON_CreateObject();
    jSubIndex = cJSON_GetObjectItemCaseSensitive(jIndex, sSubIndex);
    ASSERT_NE(jSubIndex, nullptr);

    EXPECT_STREQ(jSubIndex->valuestring, expectedValue.c_str());
}

void validateFullIndex(cJSON* jCha, int index, std::string expectedValue,
                       bool hasFailure)
{
    char sIndex[9];
    sprintf(sIndex, "index%d", index);

    ASSERT_TRUE(cJSON_HasObjectItem(jCha, sIndex));
    cJSON* jIndex = cJSON_CreateObject();
    jIndex = cJSON_GetObjectItemCaseSensitive(jCha, sIndex);
    ASSERT_NE(jIndex, nullptr);

    std::string value = expectedValue;
    if ((index) && hasFailure)
    {
        value = "N/A";
    }

    for (uint32_t subIndex = 0; subIndex < TD_SUBINDEX_PER_TOR_ICX1; subIndex++)
    {
        char sSubIndex[12];
        sprintf(sSubIndex, "subindex%d", subIndex);
        ASSERT_TRUE(cJSON_HasObjectItem(jIndex, sSubIndex));

        cJSON* jSubIndex = cJSON_CreateObject();
        jSubIndex = cJSON_GetObjectItemCaseSensitive(jIndex, sSubIndex);
        ASSERT_NE(jSubIndex, nullptr);

        EXPECT_STREQ(jSubIndex->valuestring, value.c_str());

        if ((!subIndex) && hasFailure)
        {
            value = "N/A";
        }
    }
}

void validateCha(cJSON* output, int cha, std::string expectedValue,
                 bool hasFailure)
{
    char sCha[8];
    sprintf(sCha, "cha%d", cha);
    ASSERT_TRUE(cJSON_HasObjectItem(output, sCha));
    cJSON* jCha = cJSON_CreateObject();
    jCha = cJSON_GetObjectItemCaseSensitive(output, sCha);
    ASSERT_NE(jCha, nullptr);

    for (uint32_t index = 0; index < TD_TORS_PER_CHA_ICX1; index++)
    {
        validateFullIndex(jCha, index, expectedValue, hasFailure);
    }
}

TEST(TorDumpTestFixture, logTorDump_NULL_pJsonChild)
{
    int ret;

    TestCrashdump crashdump(cd_spr);
    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, NULL);
        EXPECT_EQ(ret, ACD_INVALID_OBJECT);
    }
}

TEST(TorDumpTestFixture, logTorDump_icx_fullFlow)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx, // icx not supported,
                                        // icx2, icxd and spr are supported
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 32,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }
}

TEST(TorDumpTestFixture, logTorDump_spr_cc_success_1cha)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 1,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t cc = 0x40;
    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t u64UniqueId[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t PayloadExp[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t Data[8] = {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(u64UniqueId, u64UniqueId + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(PayloadExp, PayloadExp + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(DoAll(SetArrayArgument<5>(Data, Data + 8),
                              SetArgPointee<6>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateCha(crashdump.root, 0, "0xdeadbeefdeadbeef", false);
}

TEST(TorDumpTestFixture, logTorDump_spr_cc_mca_detected_1cha)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 1,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t cc = 0x94;
    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t u64UniqueId[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t PayloadExp[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t Data[8] = {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(u64UniqueId, u64UniqueId + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(PayloadExp, PayloadExp + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(DoAll(SetArrayArgument<5>(Data, Data + 8),
                              SetArgPointee<6>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateCha(crashdump.root, 0, "0xdeadbeefdeadbeef", false);
}

TEST(TorDumpTestFixture, logTorDump_spr_cc_ua_0x80_1cha)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 1,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t cc = 0x80;
    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t u64UniqueId[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t PayloadExp[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t Data[8] = {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(u64UniqueId, u64UniqueId + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(PayloadExp, PayloadExp + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(DoAll(SetArrayArgument<5>(Data, Data + 8),
                              SetArgPointee<6>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateCha(crashdump.root, 0, "0xdeadbeefdeadbeef,CC:0x80,RC:0x0", true);
}

TEST(TorDumpTestFixture,
     logTorDump_spr_cc_ua_0x80_1cha_peci_CrashDump_Discovery_INVALID_REQ)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 1,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t cc = 0x80;

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_SUCCESS))
        .WillOnce(Return(PECI_CC_SUCCESS));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(Return(PECI_CC_INVALID_REQ));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_INVALID_REQ);
    }

    validateCha(crashdump.root, 0, "0x0,CC:0x80,RC:0x1", true);
}

TEST(TorDumpTestFixture,
     logTorDump_spr_cc_ua_0x80_1cha_peci_CrashDump_Discovery_HW_ERR)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 1,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t cc = 0x80;

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_SUCCESS))
        .WillOnce(Return(PECI_CC_SUCCESS));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(Return(PECI_CC_HW_ERR));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_HW_ERR);
    }

    validateCha(crashdump.root, 0, "0x0,CC:0x80,RC:0x2", true);
}

TEST(TorDumpTestFixture,
     logTorDump_spr_cc_ua_0x80_1cha_peci_CrashDump_Discovery_DRIVER_ERR)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 1,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t cc = 0x80;

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_SUCCESS))
        .WillOnce(Return(PECI_CC_SUCCESS));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(Return(PECI_CC_DRIVER_ERR));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_DRIVER_ERR);
    }

    validateCha(crashdump.root, 0, "0x0,CC:0x80,RC:0x3", true);
}

TEST(TorDumpTestFixture,
     logTorDump_spr_cc_ua_0x80_1cha_peci_CrashDump_Discovery_CPU_NOT_PRESENT)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 1,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t cc = 0x80;

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_SUCCESS))
        .WillOnce(Return(PECI_CC_SUCCESS));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(Return(PECI_CC_CPU_NOT_PRESENT));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_CPU_NOT_PRESENT);
    }

    validateCha(crashdump.root, 0, "0x0,CC:0x80,RC:0x4", true);
}

TEST(TorDumpTestFixture,
     logTorDump_spr_cc_ua_0x80_1cha_peci_CrashDump_Discovery_MEM_ERR)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 1,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t cc = 0x80;

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_SUCCESS))
        .WillOnce(Return(PECI_CC_SUCCESS));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(Return(PECI_CC_MEM_ERR));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_MEM_ERR);
    }

    validateCha(crashdump.root, 0, "0x0,CC:0x80,RC:0x5", true);
}

TEST(TorDumpTestFixture,
     logTorDump_spr_cc_ua_0x80_1cha_peci_CrashDump_Discovery_TIMEOUT)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 1,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t cc = 0x80;

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_SUCCESS))
        .WillOnce(Return(PECI_CC_SUCCESS));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(Return(PECI_CC_TIMEOUT));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_TIMEOUT);
    }

    validateCha(crashdump.root, 0, "0x0,CC:0x80,RC:0x6", true);
}

TEST(TorDumpTestFixture, logTorDump_spr_3cha_fail_all_cha)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 3,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t cc = 0x80;

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_SUCCESS))
        .WillOnce(Return(PECI_CC_SUCCESS));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillRepeatedly(
            DoAll(SetArgPointee<6>(cc), Return(PECI_CC_INVALID_REQ)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_INVALID_REQ);
    }

    validateCha(crashdump.root, 0, "0x0,CC:0x80,RC:0x1", true);
    validateCha(crashdump.root, 1, "0x0,CC:0x80,RC:0x1", true);
    validateCha(crashdump.root, 2, "0x0,CC:0x80,RC:0x1", true);
}

TEST(TorDumpTestFixture, logTorDump_spr_cc_ua_0x80_3cha_fail_1st_cha_peci_fail)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 3,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t cc = 0x80;
    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t u64UniqueId[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t PayloadExp[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t Data[8] = {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(u64UniqueId, u64UniqueId + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(PayloadExp, PayloadExp + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillOnce(Return(PECI_CC_INVALID_REQ))
        .WillOnce(DoAll(SetArrayArgument<5>(Data, Data + 8),
                        SetArgPointee<6>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<5>(Data, Data + 8),
                        SetArgPointee<6>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateCha(crashdump.root, 0, "0x0,CC:0x80,RC:0x1", true);
    validateCha(crashdump.root, 1, "0xdeadbeefdeadbeef,CC:0x80,RC:0x0", true);
    validateCha(crashdump.root, 2, "0xdeadbeefdeadbeef,CC:0x80,RC:0x0", true);
}

TEST(TorDumpTestFixture, logTorDump_spr_cc_ua_0x80_3cha_fail_2nd_cha_peci_fail)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 3,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t cc = 0x80;
    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t u64UniqueId[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t PayloadExp[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t Data[8] = {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(u64UniqueId, u64UniqueId + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(PayloadExp, PayloadExp + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillOnce(DoAll(SetArrayArgument<5>(Data, Data + 8),
                        SetArgPointee<6>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_INVALID_REQ))
        .WillOnce(DoAll(SetArrayArgument<5>(Data, Data + 8),
                        SetArgPointee<6>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateCha(crashdump.root, 0, "0xdeadbeefdeadbeef,CC:0x80,RC:0x0", true);
    validateCha(crashdump.root, 1, "0x0,CC:0x80,RC:0x1", true);
    validateCha(crashdump.root, 2, "0xdeadbeefdeadbeef,CC:0x80,RC:0x0", true);
}

TEST(TorDumpTestFixture, logTorDump_spr_cc_ua_0x80_3cha_fail_3rd_cha_peci_fail)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 3,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t cc = 0x80;
    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t u64UniqueId[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t PayloadExp[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t Data[8] = {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(u64UniqueId, u64UniqueId + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(PayloadExp, PayloadExp + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillOnce(DoAll(SetArrayArgument<5>(Data, Data + 8),
                        SetArgPointee<6>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<5>(Data, Data + 8),
                        SetArgPointee<6>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_INVALID_REQ));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_INVALID_REQ);
    }

    validateCha(crashdump.root, 0, "0xdeadbeefdeadbeef,CC:0x80,RC:0x0", true);
    validateCha(crashdump.root, 1, "0xdeadbeefdeadbeef,CC:0x80,RC:0x0", true);
    validateCha(crashdump.root, 2, "0x0,CC:0x80,RC:0x1", true);
}

TEST(TorDumpTestFixture, logTorDump_spr_different_cc_3cha_1st_and_2nd_peci_fail)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 3,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t cc = 0x80;
    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t u64UniqueId[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t PayloadExp[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t Data[8] = {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(u64UniqueId, u64UniqueId + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(PayloadExp, PayloadExp + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillOnce(DoAll(SetArgPointee<6>(0x60), Return(PECI_CC_INVALID_REQ)))
        .WillOnce(DoAll(SetArgPointee<6>(0x50), Return(PECI_CC_MEM_ERR)))
        .WillRepeatedly(DoAll(SetArrayArgument<5>(Data, Data + 8),
                              SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateCha(crashdump.root, 0, "0x0,CC:0x60,RC:0x1", true);
    validateCha(crashdump.root, 1, "0x0,CC:0x50,RC:0x5", true);
    validateCha(crashdump.root, 2, "0xdeadbeefdeadbeef", false);
}

TEST(TorDumpTestFixture,
     logTorDump_spr_different_cc_3cha_1st_peci_CrashDump_Discovery_successful)
{
    int ret;
    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0,
                       .chaCount = 3,
                       .initialPeciWake = ON,
                       .inputFile = {},
                       .cpuidRead = {},
                       .chaCountRead = {},
                       .coreMaskRead = {},
                       .dimmMask = 0};

    std::vector<CPUInfo> cpusInfo = {cpuInfo};
    TestCrashdump crashdump(cpusInfo);

    uint8_t cc = 0x80;
    uint8_t u8CrashdumpEnabled = 0;
    uint16_t u16CrashdumpNumAgents = 2;
    uint8_t u64UniqueId[8] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t PayloadExp[8] = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t Data[8] = {0xef, 0xbe, 0xad, 0xde, 0xef, 0xbe, 0xad, 0xde};

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_Discovery)
        .WillOnce(DoAll(SetArgPointee<6>(u8CrashdumpEnabled),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(u16CrashdumpNumAgents),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(u64UniqueId, u64UniqueId + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArrayArgument<6>(PayloadExp, PayloadExp + 8),
                        SetArgPointee<7>(cc), Return(PECI_CC_SUCCESS)));

    EXPECT_CALL(*crashdump.libPeciMock, peci_CrashDump_GetFrame)
        .WillOnce(DoAll(SetArrayArgument<5>(Data, Data + 8),
                        SetArgPointee<6>(0x40), Return(PECI_CC_SUCCESS)))
        .WillOnce(DoAll(SetArgPointee<6>(0x60), Return(PECI_CC_INVALID_REQ)))
        .WillRepeatedly(DoAll(SetArgPointee<6>(0x50), Return(PECI_CC_MEM_ERR)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logTorDump(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_MEM_ERR);
    }

    validateSubIndexData(crashdump.root, 0, 0, 0, "0xdeadbeefdeadbeef");
    validateSubIndexData(crashdump.root, 0, 0, 1, "0x0,CC:0x60,RC:0x1");
    validateSubIndexData(crashdump.root, 0, 0, 2, "N/A");
    validateSubIndexData(crashdump.root, 0, 0, 3, "N/A");
    validateSubIndexData(crashdump.root, 0, 0, 4, "N/A");
    validateSubIndexData(crashdump.root, 0, 0, 5, "N/A");
    validateSubIndexData(crashdump.root, 0, 0, 6, "N/A");
    validateSubIndexData(crashdump.root, 0, 0, 7, "N/A");

    cJSON* jCha = cJSON_CreateObject();
    jCha = cJSON_GetObjectItemCaseSensitive(crashdump.root, "cha0");

    for (size_t i = 1; i < 32; i++)
    {
        validateFullIndex(jCha, i, "N/A", true);
    }

    validateCha(crashdump.root, 1, "0x0,CC:0x50,RC:0x5", true);
    validateCha(crashdump.root, 2, "0x0,CC:0x50,RC:0x5", true);
}
