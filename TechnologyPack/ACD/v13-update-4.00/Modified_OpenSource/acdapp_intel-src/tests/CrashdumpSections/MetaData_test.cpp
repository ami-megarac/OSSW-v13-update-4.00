#include "../mock/test_crashdump.hpp"

extern "C" {
#include "CrashdumpSections/MetaData.h"
#include "CrashdumpSections/crashdump.h"
#include "CrashdumpSections/utils.h"
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using ::testing::Return;

void validateCPUFieldInOutputFile(cJSON* output, std::string field,
                                  std::string expectedValue)
{
    cJSON* actual = cJSON_CreateObject();

    actual = cJSON_GetObjectItemCaseSensitive(output->child, field.c_str());
    char* token = strtok(actual->valuestring, ",");

    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(token, expectedValue.c_str());
}

void validateFieldInOutputFile(cJSON* output, std::string field,
                               std::string expectedValue)
{
    cJSON* actual = cJSON_CreateObject();
    actual = cJSON_GetObjectItemCaseSensitive(output, field.c_str());

    ASSERT_NE(actual, nullptr);
    EXPECT_STREQ(actual->valuestring, expectedValue.c_str());
}

TEST(MetaDataTestFixture, logSysInfo_fail_NULL_pJsonChild)
{
    int ret;

    TestCrashdump crashdump(cd_icx);
    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logSysInfo(&cpuinfo, NULL);
        EXPECT_EQ(ret, ACD_INVALID_OBJECT);
    }
}

TEST(MetaDataTestFixture, logSysInfo_fullFlow)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0x80,
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

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logSysInfo(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateCPUFieldInOutputFile(crashdump.root, "core_mask", "0xdb7e");
    validateFieldInOutputFile(crashdump.root->child, "crashcore_mask", "0x0");
    validateCPUFieldInOutputFile(crashdump.root, "cha_count", "0x0");
}

TEST(MetaDataTestFixture,
     logSysInfo_fullFlow_fail_peciRdPkgConfig_PECI_CC_INVALID_REQ)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0x80,
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

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(Return(PECI_CC_INVALID_REQ));

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logSysInfo(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_FAILURE);
    }

    validateCPUFieldInOutputFile(crashdump.root, "core_mask", "0xdb7e");
    validateFieldInOutputFile(crashdump.root->child, "crashcore_mask", "0x0");
    validateCPUFieldInOutputFile(crashdump.root, "cha_count", "0x0");
}

TEST(MetaDataTestFixture,
     logSysInfo_fullFlow_peciRdPkgConfig_PECI_CC_DRIVER_ERR)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0x80,
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

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(Return(PECI_CC_DRIVER_ERR));

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logSysInfo(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateCPUFieldInOutputFile(crashdump.root, "core_mask", "0xdb7e");
    validateFieldInOutputFile(crashdump.root->child, "crashcore_mask", "0x0");
    validateCPUFieldInOutputFile(crashdump.root, "cha_count", "0x0");
}

TEST(MetaDataTestFixture, logSysInfoInputfile_fullFlow)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0x80,
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

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logSysInfoInputfile(&cpuinfo, crashdump.root,
                                  &crashdump.inputFileInfo);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateFieldInOutputFile(crashdump.root, "_input_file",
                              crashdump.inputFileInfo.filenames[0]);
}

TEST(MetaDataTestFixture,
     logSysInfoInputfile_fullFlow_peciRdPkgConfig_PECI_CC_INVALID_REQ)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0x80,
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

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(Return(PECI_CC_INVALID_REQ));

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logSysInfoInputfile(&cpuinfo, crashdump.root,
                                  &crashdump.inputFileInfo);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateFieldInOutputFile(crashdump.root, "_input_file",
                              crashdump.inputFileInfo.filenames[0]);
}

TEST(MetaDataTestFixture,
     logSysInfoInputfile_fullFlow_peciRdPkgConfig_PECI_CC_DRIVER_ERR)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
                       .sectionMask = 0x80,
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

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdPkgConfig)
        .WillRepeatedly(Return(PECI_CC_DRIVER_ERR));

    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(Return(PECI_CC_SUCCESS));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logSysInfoInputfile(&cpuinfo, crashdump.root,
                                  &crashdump.inputFileInfo);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }

    validateFieldInOutputFile(crashdump.root, "_input_file",
                              crashdump.inputFileInfo.filenames[0]);
}

TEST(MetaDataTestFixture, logResetDetected)
{
    int ret;

    std::string resetSection = "cpu0.MCA";
    cJSON* metadata = cJSON_CreateObject();
    ret = logResetDetected(metadata, 0, MCA_CORE);

    EXPECT_EQ(ret, ACD_SUCCESS);
    validateFieldInOutputFile(metadata, "_reset_detected", resetSection);
}

TEST(MetaDataTestFixture, fillMeVersion)
{
    int ret;

    TestCrashdump crashdump(cd_icx);
    char* tstSection = "testSectionName";

    ret = fillMeVersion(tstSection, crashdump.root);
    EXPECT_EQ(ret, ACD_SUCCESS);

    validateFieldInOutputFile(crashdump.root, tstSection, MD_NA);
}

TEST(MetaDataTestFixture, fillCrashdumpVersion)
{
    int ret;

    TestCrashdump crashdump(cd_icx);
    char* tstSection = "testSectionName";

    ret = fillCrashdumpVersion(tstSection, crashdump.root);
    EXPECT_EQ(ret, ACD_SUCCESS);

    validateFieldInOutputFile(crashdump.root, tstSection, SI_CRASHDUMP_VER);
}
