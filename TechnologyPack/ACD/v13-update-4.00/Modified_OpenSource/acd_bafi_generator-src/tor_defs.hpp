/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2022 Intel Corporation.
 *
 * This software and the related documents are Intel copyrighted materials, and
 * your use of them is governed by the express license under which they were
 * provided to you ("License"). Unless the License provides otherwise, you may
 * not use, modify, copy, publish, distribute, disclose or transmit this
 * software or the related documents without Intel's prior written permission.
 *
 * This software and the related documents are provided as is, with no express
 * or implied warranties, other than those that are expressly stated in
 * the License.
 *
 ******************************************************************************/

#pragma once
#include <array>

union IerrLoggingReg
{
    struct
    {
        uint32_t firstIerrSrcIdCha : 8, firstIerrValid : 1,
            firstIerrSrcFromCbo : 1, reserved0 : 22;
    };
    uint32_t value = 0;
};

union IerrLoggingRegSpr
{
    struct
    {
        uint32_t firstIerrSrcIdCha : 10, firstIerrValid : 1,
            firstIerrSrcFromCbo : 1, reserved0 : 20;
    };
    uint32_t value = 0;
};

union McerrLoggingReg
{
    struct
    {
        uint32_t firstMcerrSrcIdCha : 8, firstMcerrValid : 1,
            firstMCerrSrcFromCbo : 1, reserved0 : 22;
    };
    uint32_t value = 0;
};

union McerrLoggingRegSpr
{
    struct
    {
        uint32_t firstMcerrSrcIdCha : 10, firstMcerrValid : 1,
            firstMCerrSrcFromCbo : 1, reserved0 : 20;
    };
    uint32_t value = 0;
};

union McerrErrorSource
{
    struct
    {
        uint32_t reserved0 : 18, msmi_mcerr_internal: 1, msmi_ierr_internal: 1,
            reserved1: 1, msmi_mcerr: 1, msmi_ierr: 1, reserved2: 1, msmi_mcp_smbus: 1,
            reserved3: 1, mcerr_internal: 1, ierr_internal: 1, reserved4: 1,
            mcerr: 1, ierr: 1, reserved5: 1;
    };
    uint32_t value = 0;
};

union McerrErrorSourceSpr
{
    struct
    {
        uint32_t reserved0 : 14, rmsmi_internal: 1, rmca_internal: 1, reserved1: 2,
            msmi_mcerr_internal: 1, msmi_ierr_internal: 1, reserved2 : 1,
            msmi_mcerr: 1, msmi_ierr: 1, reserved3 : 1, msmi_mcp_smbus: 1,
            cam_parity_err: 1, mcerr_internal: 1, ierr_internal: 1, reserved4 : 1,
            mcerr: 1, ierr: 1, reserved5: 1;
    };
    uint32_t value = 0;
};

union RMcerrLoggingReg
{
    struct
    {
        uint32_t firstMcerrSrcIdCha : 10, firstMcerrValid : 1,
            firstMCerrSrcFromCbo : 1, reserved0 : 20;
    };
    uint32_t value = 0;
};

struct SocketCtx
{
    IerrLoggingReg ierr;
    IerrLoggingRegSpr ierrSpr;
    McerrLoggingReg mcerr;
    McerrLoggingRegSpr mcerrSpr;
    McerrErrorSource mcerrErr;
    McerrErrorSourceSpr mcerrErrSpr;
    RMcerrLoggingReg rMcerrErr;
};

enum class SadValues
{
    HOM = 0,
    MMIO,
    CFG,
    MMIOPartialRead,
    IO,
    IntelReserved0,
    SPC,
    IntelReserved1
};

union BDFFormatter
{
    struct
    {
        uint32_t reserved0 : 12, func : 3, dev : 5, bus : 8, reserved1 : 4;
    };
    uint32_t address;
};

const std::array<const char*, 8> SAD_RESULT = {
    "HOM", "MMIO",           "CFG", "MMIO Partial Read",
    "IO",  "Intel Reserved", "SPC", "Intel Reserved",
};

struct TORDataGeneric
{
    union
    {
        struct
        {
            uint32_t master_valid : 1, valid : 1, retry : 1, in_pipe : 1,
                cha_inside : 5, tor : 5, core_id : 6, thread_id : 1,
                request_opCode : 11;
        };
        uint32_t subindex0;
    };
    union
    {
        struct
        {
            uint32_t addr_lo : 14, fsm : 6, target : 5, sad : 3, lcs : 4;
        };
        uint32_t subindex1;
    };
    union
    {
        uint32_t address;
        uint32_t subindex2;
    };

    // ICX
    union
    {
        struct
        {
            uint32_t reserved0 : 2, valid1 : 1, reserved1 : 2, retry1 : 1,
                reserved2 : 1, in_pipe1 : 1, reserved3 : 24;
        };
        uint32_t tordump0_subindex0;
    };
    union
    {
        struct
        {
            uint32_t reserved4 : 29, address_8_6 : 3;
        };
        uint32_t tordump1_subindex0;
    };
    union
    {
        struct
        {
            uint32_t address_16_9 : 8, reserved5 : 24;
        };
        uint32_t tordump2_subindex0;
    };
    union
    {
        struct
        {
            uint32_t reserved6 : 8, thread_id1 : 3, reserved7 : 21;
        };
        uint32_t tordump0_subindex1;
    };
    union
    {
        struct
        {
            uint32_t reserved8 : 29, address_19_17 : 3;
        };
        uint32_t tordump1_subindex1;
    };
    union
    {
        struct
        {
            uint32_t address_27_20 : 8, reserved9 : 24;
        };
        uint32_t tordump2_subindex1;
    };
    union
    {
        struct
        {
            uint32_t reserved10 : 7, target1 : 5, reserved11 : 6, sad1 : 3,
                reserved12 : 11;
        };
        uint32_t tordump0_subindex2;
    };
    union
    {
        struct
        {
            uint32_t reserved13 : 29, address_30_28 : 3;
        };
        uint32_t tordump1_subindex2;
    };
    union
    {
        struct
        {
            uint32_t address_38_31 : 8, reserved14 : 24;
        };
        uint32_t tordump2_subindex2;
    };
    union
    {
        struct
        {
            uint32_t reserved15 : 21, core_id1 : 6, reserved16 : 5;
        };
        uint32_t tordump0_subindex3;
    };
    union
    {
        struct
        {
            uint32_t reserved17 : 29, address_41_39 : 3;
        };
        uint32_t tordump1_subindex3;
    };
    union
    {
        struct
        {
            uint32_t address_49_42 : 8, reserved18 : 24;
        };
        uint32_t tordump2_subindex3;
    };
    union
    {
        struct
        {
            uint32_t reserved19 : 17, request_opCode1 : 11, reserved20 : 4;
        };
        uint32_t tordump0_subindex4;
    };
    union
    {
        struct
        {
            uint32_t reserved21 : 29, address_51_50 : 2, reserved22 : 1;
        };
        uint32_t tordump1_subindex4;
    };
    union
    {
        struct
        {
            uint32_t reserved23 : 32;
        };
        uint32_t tordump2_subindex4;
    };
    union
    {
        struct
        {
            uint32_t reserved24 : 3, lcs1 : 4, reserved25 : 25;
        };
        uint32_t tordump0_subindex7;
    };
    union
    {
        struct
        {
            uint32_t reserved26 : 1, fsm1 : 5, reserved27 : 26;
        };
        uint32_t tordump1_subindex7;
    };
    union
    {
        struct
        {
            uint32_t reserved28 : 32;
        };
        uint32_t tordump2_subindex7;
    };
    // SPR CPU_ID = 0x806f0
    union
    {
        struct
        {
            uint32_t valid2: 1, reserved29: 2, retry2: 1, reserved30: 3, in_pipe2: 1,
            reserved31: 24;
        };
        uint32_t tordump0_subindex0_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved32: 9, thread_id2: 3, reserved33: 20;
        };
        uint32_t tordump0_subindex1_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved34: 7, target2: 5, reserved35: 9, sad2: 3,
            reserved36: 2, non_coherent: 1, reserved37: 5;
        };
        uint32_t tordump0_subindex2_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved38: 28, core_id2_2_0: 3, reserved39: 1;
        };
        uint32_t tordump0_subindex3_spr11;
    };
    union
    {
        struct
        {
            uint32_t core_id2_6_3: 4, reserved40: 20, orig_req1_6_0: 7, reserved41: 1;
        };
        uint32_t tordump0_subindex4_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved42: 3, lcs2: 4, reserved43: 25;
        };
        uint32_t tordump0_subindex7_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved44: 31, address_6_6_spr1: 1;
        };
        uint32_t tordump1_subindex0_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved45: 31, address_17_17_spr1: 1;
        };
        uint32_t tordump1_subindex1_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved46: 31, address_28_28_spr1: 1;
        };
        uint32_t tordump1_subindex2_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved47: 31, address_39_39_spr1: 1;
        };
        uint32_t tordump1_subindex3_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved48: 31, address_50_50_spr1: 1;
        };
        uint32_t tordump1_subindex4_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved49: 6, request_opCode2_1_0: 2, reserved50: 24;
        };
        uint32_t tordump1_subindex5_spr1;
    };
    union
    {
        struct
        {
            uint32_t request_opCode2_5_3: 3, reserved51: 29;
        };
        uint32_t tordump1_subindex6_spr1;
    };
    union
    {
        struct
        {
            uint32_t address_16_7_spr1: 10, reserved52: 22;
        };
        uint32_t tordump2_subindex0_spr1;
    };
    union
    {
        struct
        {
            uint32_t address_27_18_spr1: 10, reserved53: 22;
        };
        uint32_t tordump2_subindex1_spr1;
    };
    union
    {
        struct
        {
            uint32_t address_38_29_spr1: 10, reserved54: 22;
        };
        uint32_t tordump2_subindex2_spr1;
    };
    union
    {
        struct
        {
            uint32_t address_49_40_spr1: 10, reserved55: 22;
        };
        uint32_t tordump2_subindex3_spr1;
    };
    union
    {
        struct
        {
            uint32_t address_51_51_spr1: 1, reserved56: 31;
        };
        uint32_t tordump2_subindex4_spr1;
    };
    // SPR CPU_ID = 0x806f1
    union
    {
        struct
        {
            uint32_t reserved57: 7, target3: 5, reserved58: 10, sad3: 3,
            reserved59: 2, non_coherent2: 1, reserved60: 4;
        };
        uint32_t tordump0_subindex2_spr2;
    };
    union
    {
        struct
        {
            uint32_t core_id3: 7, reserved61: 20, orig_req2_3_0: 4, reserved62: 1;
        };
        uint32_t tordump0_subindex4_spr2;
    };
    union
    {
        struct
        {
            uint32_t orig_req2_10_4: 7, reserved63: 25;
        };
        uint32_t tordump0_subindex5_spr2;
    };
    union
    {
        struct
        {
            uint32_t reserved64: 4, fsm2_3_0: 4, reserved65: 24;
        };
        uint32_t tordump1_subindex6_spr2;
    };
    union // SAME FOR 0x806f2, 0x806f3, ...
    {
        struct
        {
            uint32_t reserved66: 6, lcs3: 4, reserved67: 21, fsm2_4_4: 1;
        };
        uint32_t tordump0_subindex7_spr2;
    };
    // SPR CPU_ID = 0x806f2, 0x806f3, ...
    union
    {
        struct
        {
            uint32_t reserved68: 8, thread_id3: 3, reserved69: 21;
        };
        uint32_t tordump0_subindex1_spr3;
    };
    union
    {
        struct
        {
            uint32_t reserved70: 5, target4: 6, reserved71: 10, sad4: 3,
            reserved72: 2, non_coherent3: 1, reserved73: 5;
        };
        uint32_t tordump0_subindex2_spr3;
    };
    union
    {
        struct
        {
            uint32_t reserved74: 3, core_id4: 7, reserved75: 20, orig_req3_1_0: 2;
        };
        uint32_t tordump0_subindex4_spr3;
    };
    union
    {
        struct
        {
            uint32_t orig_req3_10_2 : 9, reserved76: 23;
        };
        uint32_t tordump0_subindex5_spr3;
    };
    union
    {
        struct
        {
            uint32_t reserved77: 5, fsm3_3_0: 4, reserved78: 23;
        };
        uint32_t tordump1_subindex6_spr3;
    };
    union
    {
        struct
        {
            uint32_t fsm3_4_4: 1, reserved79: 31;
        };
        uint32_t tordump1_subindex7_spr3;
    };
    union
    {
        struct
        {
            uint32_t address_16_6_spr3: 11, reserved80: 21;
        };
        uint32_t tordump2_subindex0_spr3;
    };
    union
    {
        struct
        {
            uint32_t address_27_17_spr3: 11, reserved81: 21;
        };
        uint32_t tordump2_subindex1_spr3;
    };
    union
    {
        struct
        {
            uint32_t address_38_28_spr3: 11, reserved82: 21;
        };
        uint32_t tordump2_subindex2_spr3;
    };
    union
    {
        struct
        {
            uint32_t address_49_39_spr3: 11, reserved83: 21;
        };
        uint32_t tordump2_subindex3_spr3;
    };
    union
    {
        struct
        {
            uint32_t address_51_50_spr3: 2, reserved84: 30;
        };
        uint32_t tordump2_subindex4_spr3;
    };

    union
    {
        struct
        {
            uint32_t reserved85: 32;
        };
        uint32_t tordump0_subindex3_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved86: 32;
        };
        uint32_t tordump2_subindex5_spr1;
    };
     union
    {
        struct
        {
            uint32_t reserved87: 32;
        };
        uint32_t tordump1_subindex7_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved88: 32;
        };
        uint32_t tordump2_subindex7_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved89: 32;
        };
        uint32_t tordump0_subindex6_spr1;
    };
    union
    {
        struct
        {
            uint32_t orig_req1_10_7: 4, reserved90: 28;
        };
        uint32_t tordump0_subindex5_spr1;
    };
    union
    {
        struct
        {
            uint32_t reserved91: 32;
        };
        uint32_t tordump2_subindex6_spr1;
    };

    uint32_t cha;
    uint32_t idx;
};

using TORData =
    std::map<uint32_t, std::pair<SocketCtx, std::vector<TORDataGeneric>>>;
