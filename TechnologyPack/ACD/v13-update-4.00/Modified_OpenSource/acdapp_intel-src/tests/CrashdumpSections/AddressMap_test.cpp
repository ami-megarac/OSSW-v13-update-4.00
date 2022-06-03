#include "../mock/test_crashdump.hpp"

extern "C" {
#include "CrashdumpSections/AddressMap.h"
#include "CrashdumpSections/crashdump.h"
#include "CrashdumpSections/utils.h"
}

#include "gmock/gmock.h"
#include "gtest/gtest.h"

using namespace ::testing;
using ::testing::Return;

TEST(AddressMapTestFixture, logAddressMap_NULL_pJsonChild)
{
    int ret;

    TestCrashdump crashdump(cd_icx);
    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logAddressMap(&cpuinfo, NULL);
        EXPECT_EQ(ret, ACD_INVALID_OBJECT);
    }
}

TEST(AddressMapTestFixture, logAddressMap_InvalidModel)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_spr, // invalid model#
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

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logAddressMap(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_FAILURE);
    }
}

TEST(AddressMapTestFixture, logAddressMap_fullFlow_ICX)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
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

    uint8_t returnValue[2] = {0x86, 0x80}; // 0x8086;
    uint8_t cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                              SetArgPointee<9>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logAddressMap(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_SUCCESS);
    }
}

TEST(AddressMapTestFixture, logAddressMap_fullFlow_ICX_fail_peciLock)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
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
        .WillRepeatedly(Return(PECI_CC_INVALID_REQ));

    uint8_t returnValue[2] = {0x86, 0x80}; // 0x8086;
    uint8_t cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                              SetArgPointee<9>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logAddressMap(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_FAILURE);
    }
}

TEST(AddressMapTestFixture,
     logAddressMap_fullFlow_ICX_fail_peciRdendPointConfig_NULL_cc)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
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

    uint8_t returnValue[2] = {0x86, 0x80}; // 0x8086;
    uint8_t* cc = nullptr;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                              SaveArg<9>(&cc), Return(PECI_CC_INVALID_REQ)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logAddressMap(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_FAILURE);
    }
}

TEST(AddressMapTestFixture,
     logAddressMap_fullFlow_ICX_fail_peciRdendPointConfig_NULL_pPCIData)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
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

    uint8_t* ptr = nullptr;
    uint8_t cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillRepeatedly(DoAll(SaveArg<7>(&ptr), SetArgPointee<9>(cc),
                              Return(PECI_CC_INVALID_REQ)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logAddressMap(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, ACD_FAILURE);
    }
}

TEST(AddressMapTestFixture,
     logAddressMap_fullFlow_ICX_fail_peciRdendPointConfig_2ndRep_fail)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
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

    uint8_t returnValue[2] = {0x86, 0x80}; // 0x8086;
    uint8_t cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillOnce(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                        SetArgPointee<9>(cc), Return(PECI_CC_SUCCESS)))
        .WillOnce(Return(PECI_CC_INVALID_REQ))
        .WillRepeatedly(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                              SetArgPointee<9>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logAddressMap(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_SUCCESS);
    }
}

TEST(AddressMapTestFixture,
     logAddressMap_fullFlow_ICX_fail_peciRdendPointConfig_1stRep_fail)
{
    int ret;

    CPUInfo cpuInfo = {.clientAddr = 48,
                       .model = cd_icx,
                       .coreMask = 0x0000db7e,
                       .crashedCoreMask = 0,
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

    uint8_t returnValue[2] = {0x86, 0x80}; // 0x8086;
    uint8_t cc = 0x94;
    EXPECT_CALL(*crashdump.libPeciMock, peci_RdEndPointConfigPciLocal_seq)
        .WillOnce(Return(PECI_CC_INVALID_REQ))
        .WillRepeatedly(DoAll(SetArrayArgument<7>(returnValue, returnValue + 2),
                              SetArgPointee<9>(cc), Return(PECI_CC_SUCCESS)));

    for (auto cpuinfo : crashdump.cpusInfo)
    {
        ret = logAddressMap(&cpuinfo, crashdump.root);
        EXPECT_EQ(ret, PECI_CC_SUCCESS);
    }
}
