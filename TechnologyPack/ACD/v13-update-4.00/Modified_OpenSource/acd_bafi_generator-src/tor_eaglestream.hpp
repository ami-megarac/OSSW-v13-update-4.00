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
#include <algorithm>
#include <array>
#include <functional>
#include <map>
#include <nlohmann/json.hpp>
#include <optional>
#include <string>
#include <tuple>
#include <utility>
#include <vector>
#include <locale>

#include "aer.hpp"
#include "mca_defs.hpp"
#include "tor_defs_spr.hpp"
#include "utils.hpp"
#include "tor_whitley.hpp"

using json = nlohmann::json;

union Retry_rd_err_logSpr
{
    struct
    {
        uint32_t valid: 1, uc: 1, over: 1, mode: 4, reserved1: 16,
        system_addr_valid: 1, reserved2: 8;
    };
    uint32_t retry_rd_err_log = 0;
};

union Retry_rd_err_log_address1Spr
{
    struct
    {
       uint32_t reserved: 6, chip_select: 3, cbit: 5, bank: 6, column: 10,
       reserved1: 2;
    };
    uint32_t retry_rd_err_log_address1 = 0;
};

union Retry_rd_err_log_address3Spr
{
    struct
    {
       uint64_t address: 52, reserved: 12;
    };
    uint64_t retry_rd_err_log_address3 = 0;
};

union Retry_rd_err_set2_log_address2Spr
{
    struct
    {
        uint32_t row: 18, reserved5: 14;
    };
    uint32_t retry_rd_err_set2_log_address2 = 0;
};

union CounterRegister
{
    struct
    {
        uint32_t count: 15, reserved: 17;
    };
    uint32_t counterRegister = 0;
};

std::array<std::string, 4> imcChannelsSpr = {
    "B30_D26_F0_",
    "B30_D27_F0_",
    "B30_D28_F0_",
    "B30_D29_F0_"
};

std::array<std::string, 4> offsetsSpr = {
    "0x22c60",
    "0x22e54",
    "0x2ac60",
    "0x2ae54"
};

const std::map<std::string, const char*> ImcChannelSpr = {
    {"B30_D26_F0_0x22c60", "IMC 0, Channel 0"},
    {"B30_D26_F0_0x22e54", "IMC 0, Channel 0"},
    {"B30_D26_F0_0x22c58", "IMC 0, Channel 0"},
    {"B30_D26_F0_0x22e58", "IMC 0, Channel 0"},
    {"B30_D26_F0_0x2ac60", "IMC 0, Channel 1"},
    {"B30_D26_F0_0x2ae54", "IMC 0, Channel 1"},
    {"B30_D26_F0_0x2ac58", "IMC 0, Channel 1"},
    {"B30_D26_F0_0x2ae58", "IMC 0, Channel 1"},
    {"B30_D27_F0_0x22c60", "IMC 1, Channel 0"},
    {"B30_D27_F0_0x22e54", "IMC 1, Channel 0"},
    {"B30_D27_F0_0x22c58", "IMC 1, Channel 0"},
    {"B30_D27_F0_0x22e58", "IMC 1, Channel 0"},
    {"B30_D27_F0_0x2ac60", "IMC 1, Channel 1"},
    {"B30_D27_F0_0x2ae54", "IMC 1, Channel 1"},
    {"B30_D27_F0_0x2ac58", "IMC 1, Channel 1"},
    {"B30_D27_F0_0x2ae58", "IMC 1, Channel 1"},
    {"B30_D28_F0_0x22c60", "IMC 2, Channel 0"},
    {"B30_D28_F0_0x22e54", "IMC 2, Channel 0"},
    {"B30_D28_F0_0x22c58", "IMC 2, Channel 0"},
    {"B30_D28_F0_0x22e58", "IMC 2, Channel 0"},
    {"B30_D28_F0_0x2ac60", "IMC 2, Channel 1"},
    {"B30_D28_F0_0x2ae54", "IMC 2, Channel 1"},
    {"B30_D28_F0_0x2ac58", "IMC 2, Channel 1"},
    {"B30_D28_F0_0x2ae58", "IMC 2, Channel 1"},
    {"B30_D29_F0_0x22c60", "IMC 3, Channel 0"},
    {"B30_D29_F0_0x22e54", "IMC 3, Channel 0"},
    {"B30_D29_F0_0x22c58", "IMC 3, Channel 0"},
    {"B30_D29_F0_0x22e58", "IMC 3, Channel 0"},
    {"B30_D29_F0_0x2ac60", "IMC 3, Channel 1"},
    {"B30_D29_F0_0x2ae54", "IMC 3, Channel 1"},
    {"B30_D29_F0_0x2ac58", "IMC 3, Channel 1"},
    {"B30_D29_F0_0x2ae58", "IMC 3, Channel 1"}
};

const std::map<uint8_t, const char*> ModeSpr = {
    {0x0, "sddc 2LM"},
    {0x1, "sddc 1LM"},
    {0x2, "sddc +1 2LM"},
    {0x3, "sddc +1 1LM"},
    {0x4, "adddc 2LM"},
    {0x5, "adddc 1LM"},
    {0x6, "adddc +1 2LM"},
    {0x7, "adddc +1 1LM"},
    {0x8, "read is from ddrt dimm"},
    {0x9, "x8 sddc"},
    {0xA, "x8 sddc +1"},
    {0xB, "not a valid ecc mode"}
};

[[nodiscard]] json decodeRetryLogEaglestream(const json& input, json silkscreenMap, std::string socketId)
{
    Retry_rd_err_logSpr decodedErrLog;
    Retry_rd_err_log_address3Spr decodedErrLogAddress3;
    Retry_rd_err_set2_log_address2Spr decodedErrSet2LogAddress2;
    Retry_rd_err_log_address1Spr decodedErrLogAddress;
    json memoryErrors;
    bool uncorErr;
    if (input.find("uncore") != input.cend())
    {
        for (auto channel : imcChannelsSpr)
        {
            for (auto offset : offsetsSpr)
            {
                std::stringstream ssKey;
                std::stringstream ssValue;
                bool addressEntry = false;
                std::string reg = channel + offset;
                if (input["uncore"].find(reg) != input["uncore"].cend())
                {
                    if (!str2uint(input["uncore"][reg], decodedErrLog.retry_rd_err_log))
                    {
                        continue;
                    }

                    if (!decodedErrLog.valid)
                    {
                        continue;
                    }

                    std::optional<std::string> imcChannel = getDecoded(ImcChannelSpr, reg);
                    if (!imcChannel)
                    {
                        continue;
                    }

                    std::optional<std::string> mode = getDecoded(ModeSpr,
                        static_cast<uint8_t>(decodedErrLog.mode));
                    if (!mode)
                    {
                        continue;
                    }

                    if (decodedErrLog.uc)
                        uncorErr = true;
                    else
                        uncorErr = false;

                    reg.replace(15, 3, "c58");
                    if (input["uncore"][reg] == "0x0")
                    {
                        reg.replace(15, 3, "e58");
                        if (input["uncore"][reg] == "0x0")
                        {
                            reg.replace(15, 3, "d58");
                        }
                    }

                    if (input["uncore"].find(reg) != input["uncore"].cend())
                    {
                        if (reg.find("c58") != std::string::npos || reg.find("e58") != std::string::npos
                        || reg.find("d58") != std::string::npos)
                        {
                            if (str2uint(input["uncore"][reg],
                                decodedErrLogAddress.retry_rd_err_log_address1))
                            {
                                std::string imcInfo = *imcChannel;
                                std::size_t pos = imcInfo.find("IMC ");
                                std::string imc = imcInfo.substr(pos + 4, 1);
                                pos = imcInfo.find("Channel ");
                                std::string channel = imcInfo.substr(pos + 8, 1);

                                std::string bankAddress = std::to_string(
                                (static_cast<uint8_t>(decodedErrLogAddress.bank) & 0xc) >> 2);

                                std::string bankGroup = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.bank & 0x3));

                                std::string rank = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.chip_select & 0x3));

                                std::string subRank = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.cbit));

                                std::string dimm = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.chip_select >>
                                    2 & 0x1));

                                std::string column = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.column >> 3));

                                std::string subChannel = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.chip_select));

                                if (silkscreenMap != NULL)
                                {
                                    for (const auto& [slotKey, slotVal]
                                        : silkscreenMap["MemoryLocationPcb"].items())
                                    {
                                        if (slotVal["Socket"] != std::stoi(socketId))
                                        {
                                            continue;
                                        }

                                        if (slotVal["IMC"] != std::stoi(imc))
                                        {
                                            continue;
                                        }

                                        if (slotVal["Channel"] != std::stoi(channel))
                                        {
                                            continue;
                                        }

                                        if (slotVal["Slot"] != std::stoi(dimm))
                                        {
                                            continue;
                                        }

                                        ssValue << std::string(slotVal["SlotName"]) << ", ";
                                    }
                                }
                                ssKey << *imcChannel << ", ";
                                ssKey << "SLOT: " << dimm;
                                ssValue << "Bank: " << bankAddress << ", ";
                                ssValue << "Bank group: " << bankGroup << ", ";
                                ssValue << "Rank: " << rank << ", ";
                                ssValue << "Sub-Rank: " << subRank << ", ";
                                ssValue << "Sub-Channel: " << subChannel << ", ";
                                ssValue << "Mode: " << *mode << ", ";
                                ssValue << "Column: " << column;
                            }
                        }

                        // Add row field
                        reg.replace(15, 3, "e5c");

                        if (str2uint(input["uncore"][reg], decodedErrSet2LogAddress2.retry_rd_err_set2_log_address2))
                        {
                            std::string row = std::to_string(
                            static_cast<uint16_t>(decodedErrSet2LogAddress2.row));
                            ssValue << ", Row: " << row;
                        }
                        // Check if there is a valid system address to be displayed
                        reg.replace(15, 3, "c60");
                        if (str2uint(input["uncore"][reg], decodedErrLog.retry_rd_err_log))
                        {
                            if (decodedErrLog.system_addr_valid)
                            {
                                if (reg.find("22c60") != std::string::npos)
                                {
                                    reg.replace(13, 5, "20ed8");
                                }
                                else if (reg.find("2ac60") != std::string::npos)
                                {
                                    reg.replace(13, 5, "28ed8");
                                }

                                if (input["uncore"].contains(reg))
                                {
                                    if (str2uint(input["uncore"][reg], decodedErrLogAddress3.retry_rd_err_log_address3))
                                    {
                                        std::string address = int_to_hex(
                                            static_cast<uint64_t>(decodedErrLogAddress3.address));
                                        if (!addressEntry)
                                        {
                                            addressEntry = true;
                                            ssValue << " System address for last entry: " << address;
                                        }
                                    }
                                }
                            }
                        }
                        reg = channel + offset;
                        reg.replace(15, 3, "e54");
                        if (str2uint(input["uncore"][reg], decodedErrLog.retry_rd_err_log))
                        {
                            if (decodedErrLog.system_addr_valid)
                            {
                                if (reg.find("22e54") != std::string::npos)
                                {
                                    reg.replace(13, 5, "20ee0");
                                }
                                else if (reg.find("2ae54") != std::string::npos)
                                {
                                    reg.replace(13, 5, "28ee0");
                                }

                                if (input["uncore"].contains(reg))
                                {
                                    if (str2uint(input["uncore"][reg], decodedErrLogAddress3.retry_rd_err_log_address3))
                                    {
                                        std::string address = int_to_hex(
                                            static_cast<uint64_t>(decodedErrLogAddress3.address));
                                        if (!addressEntry)
                                        {
                                            addressEntry = true;
                                            ssValue << " System address for last entry: " << address;
                                        }
                                    }
                                }
                            }
                        }
                        reg = channel + offset;
                        reg.replace(15, 3, "c70");
                        if (str2uint(input["uncore"][reg], decodedErrLog.retry_rd_err_log))
                        {
                            if (decodedErrLog.system_addr_valid)
                            {
                                if (reg.find("22c70") != std::string::npos)
                                {
                                    reg.replace(13, 5, "20f10");
                                }
                                else if (reg.find("2ac70") != std::string::npos)
                                {
                                    reg.replace(13, 5, "28f10");
                                }

                                if (input["uncore"].contains(reg))
                                {
                                    if (str2uint(input["uncore"][reg], decodedErrLogAddress3.retry_rd_err_log_address3))
                                    {
                                        std::string address = int_to_hex(
                                            static_cast<uint64_t>(decodedErrLogAddress3.address));
                                        if (!addressEntry)
                                        {
                                            addressEntry = true;
                                            ssValue << " System address for last entry: " << address;
                                        }
                                    }
                                }
                            }
                        }
                    }
                    if (uncorErr)
                        memoryErrors["Memory_errors"]["Uncorrectable"][ssKey.str()]
                            = ssValue.str();
                    else
                        memoryErrors["Memory_errors"]["Correctable"][ssKey.str()]
                            = ssValue.str();
                }
            }
        }
    }
    return memoryErrors;
}

[[nodiscard]] json parseCounterRegisters(const json& input)
{
    json counterErrors;
    CounterRegister decodedCounterRegister;
    std::regex crcCounter("B0[0-5]_D0[1-8]_F0_0x4e0");
    std::regex rcvCounter("B0[0-5]_D0[1-8]_F0_0x4e4");

    if (input.find("uncore") != input.cend())
    {
        for (const auto& [pciKey, pciVal] : input["uncore"].items())
        {
            if (pciVal == false || checkInproperValue(pciVal) || pciVal == "0x0")
            {
                continue;
            }
            if (std::regex_search(std::string(pciKey), crcCounter))
            {
                if (!str2uint(input["uncore"][pciKey], decodedCounterRegister.counterRegister))
                {
                    continue;
                }
                if (decodedCounterRegister.count != 0)
                {
                    counterErrors["PCIe_correctable_errors_counters"]
                    [pciKey.substr(0, 10)]["CRC_error_count"] =
                        static_cast<uint16_t>(decodedCounterRegister.count);
                }
            }
            if (std::regex_search(std::string(pciKey), rcvCounter))
            {
                if (!str2uint(input["uncore"][pciKey], decodedCounterRegister.counterRegister))
                {
                    continue;
                }
                if (decodedCounterRegister.count != 0)
                {
                    counterErrors["PCIe_correctable_errors_counters"]
                    [pciKey.substr(0, 10)]["Receiver_error_count"] =
                        static_cast<uint16_t>(decodedCounterRegister.count);
                }
            }
        }
    }
    return counterErrors;
}

class EaglestreamCpu
{
    public:
    [[nodiscard]] std::map<std::string, std::array<uint64_t, 2>>
    getMemoryMap(const json& input)
    {
        // map is the same for all CPUs, so if once created other CPUs can be skipped
        bool mapCreated = false;
        std::map<std::string, std::array<uint64_t, 2>> memoryMap;
        auto cpuSections = prepareJson(input);
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            if (mapCreated)
            {
                break;
            }

            if  (cpuSection.get().find("address_map") == cpuSection.get().cend())
            {
                continue;
            }

            for (auto const& [name, value] : cpuSection.get()["address_map"].items())
            {
                if (name == "_record_enable" && value == false)
                {
                   break;
                }

                if (name == "_version")
                {
                    continue;
                }

                if (checkInproperValue(value))
                {
                    continue;
                }

                uint64_t uintValue;
                if (!str2uint(value, uintValue))
                {
                    continue;
                }

                if (name.find("_BASE") != std::string::npos)
                {
                    auto shortName = name.substr(0, name.find("_BASE"));
                    memoryMap[shortName][0] = uintValue;
                }
                else if (name.find("_LIMIT") != std::string::npos)
                {
                    auto shortName = name.substr(0, name.find("_LIMIT"));
                    memoryMap[shortName][1] = uintValue;
                }
            }
            mapCreated = true;
        }
        return memoryMap;
    }

    [[nodiscard]] std::optional<std::string>
        getUncoreData(const json& input, std::string varName)
    {
        if (input.find("uncore") == input.cend())
        {
            return {};
        }

        if (input["uncore"].find(varName) == input["uncore"].cend())
        {
            return {};
        }

        if (!input["uncore"][varName].is_string())
        {
            return {};
        }

        if (checkInproperValue(input["uncore"][varName]))
        {
            return {};
        }
        return input["uncore"][varName].get<std::string>();
    }

    [[nodiscard]] std::optional<MCAData> parseBigCoreMca(
        const json& input, std::string bigCoreStr)
    {
        std::string bigcore_status =
            std::string(bigCoreStr) + std::string("_status");
        std::string bigcore_ctl = std::string(bigCoreStr) + std::string("_ctl");
        std::string bigcore_addr =
            std::string(bigCoreStr) + std::string("_addr");
        std::string bigcore_misc =
            std::string(bigCoreStr) + std::string("_misc");
        MCAData mc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     false, false, false, false, false};

        if (!input.contains(bigcore_status) || !input.contains(bigcore_ctl)
            || !input.contains(bigcore_addr) || !input.contains(bigcore_misc))
        {
            return {};
        }

        if (checkInproperValue(input[bigcore_status]))
        {
            return {};
        }

        if (!str2uint(input[bigcore_status], mc.mc_status))
        {
            return {};
        }

        if (!mc.valid)
        {
            return {};
        }

        if (checkInproperValue(input[bigcore_ctl]))
        {
            mc.ctl = 0;
        }
        else if (!str2uint(input[bigcore_ctl], mc.ctl))
        {
            return {};
        }

        if (checkInproperValue(input[bigcore_addr]))
        {
            mc.address = 0;
        }
        else if (!str2uint(input[bigcore_addr], mc.address))
        {
            return {};
        }

        if (checkInproperValue(input[bigcore_misc]))
        {
            mc.misc = 0;
        }
        else if (!str2uint(input[bigcore_misc], mc.misc))
        {
            return {};
        }
        return mc;
    }

    [[nodiscard]] std::vector<MCAData>
        parseBigCoreMcas(std::reference_wrapper<const json> input,
            uint32_t coreId, uint32_t threadId, std::string bigcore_mcas[4])
    {
        std::vector<MCAData> output;
        auto mc0 = parseBigCoreMca(input, bigcore_mcas[0]);
        auto mc1 = parseBigCoreMca(input, bigcore_mcas[1]);
        auto mc2 = parseBigCoreMca(input, bigcore_mcas[2]);
        auto mc3 = parseBigCoreMca(input, bigcore_mcas[3]);
        if (mc0)
        {
            mc0->core = coreId;
            mc0->thread = threadId;
            mc0->bank = 0;
            mc0->cbo = false;
            output.push_back(*mc0);
        }

        if (mc1)
        {
            mc1->core = coreId;
            mc1->thread = threadId;
            mc1->bank = 1;
            mc1->cbo = false;
            output.push_back(*mc1);
        }

        if (mc2)
        {
            mc2->core = coreId;
            mc2->thread = threadId;
            mc2->bank = 2;
            mc2->cbo = false;
            output.push_back(*mc2);
        }

        if (mc3)
        {
            mc3->core = coreId;
            mc3->thread = threadId;
            mc3->bank = 3;
            mc3->cbo = false;
            output.push_back(*mc3);
        }
        return output;
    }

    [[nodiscard]] std::optional<MCAData> parseMca(const json& mcSection,
        const json& mcData, uint32_t coreId, uint32_t threadId)
    {
        if (mcData.is_string())
        {
            return {};
        }

        MCAData mc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, false, false, false, false, false};
        mc.core = coreId;
        mc.thread = threadId;
        std::string mcLower = mcSection;
        std::transform(mcLower.begin(), mcLower.end(), mcLower.begin(),
                       ::tolower);
        std::string status = mcLower + "_status";
        std::string ctl = mcLower + "_ctl";
        std::string addr = mcLower + "_addr";
        std::string misc = mcLower + "_misc";
        if (mcData.find(status) == mcData.cend())
        {
            return {};
        }

        if (checkInproperValue(mcData[status]))
        {
            return {};
        }

        if (!str2uint(mcData[status], mc.mc_status))
        {
            return {};
        }

        if (!mc.valid)
        {
            return {};
        }

        if (startsWith(mcLower, "mc"))
        {
            uint32_t bank;
            if (!str2uint(mcLower.substr(2), bank, 10))
            {
                return {};
            }

            if (bank == 12 || bank == 24 || bank == 5 || bank == 7 || bank == 8)
            {
                return {};
            }
            mc.bank = bank;
        }
        else if (startsWith(mcLower, "cbo"))
        {
            mc.cbo = true;
            if (!str2uint(mcLower.substr(3), mc.bank, 10))
            {
                return {};
            }
        }
        else if (startsWith(mcLower, "m2mem"))
        {
            mc.m2m = true;
            mc.hbm_bank = -1;
            uint32_t bank;
            if (!str2uint(mcLower.substr(5), bank, 10))
            {
                return {};
            }

            mc.bank = bank;
        }
        else if (startsWith(mcLower, "hbm"))
        {
            if (!str2uint(mcLower.substr(0,4).substr(3), mc.hbm_bank, 10))
            {
                return {};
            }
            if (mcLower.find("m2mem") != std::string::npos)
            {
                mc.m2m = true;
                if (!str2uint(mcLower.substr(5,10).substr(5), mc.bank, 10))
                {
                    return {};
                }
            }
            if (mcLower.find("imc") != std::string::npos)
            {
                mc.imc = true;
                if (!str2uint(mcLower.substr(5,4).substr(3), mc.imc_num, 10))
                {
                    return {};
                }
                if (mc.imc_num == 0)
                {
                    mc.bank = 30;
                }
                else
                {
                    mc.bank = 31;
                }
                if (!str2uint(mcLower.substr(5,4).substr(3), mc.channel, 10))
                {
                    return {};
                }
            }
        }
        else if (startsWith(mcLower, "upi"))
        {
            mc.upi = true;
            uint32_t bank;
            if (!str2uint(mcLower.substr(3), bank, 10))
            {
                return {};
            }

            mc.bank = bank;
        }
        else if (startsWith(mcLower, "mdf"))
        {
            mc.mdf = true;
            if (!str2uint(mcLower.substr(3), mc.bank, 10))
            {
                return {};
            }
        }
        else
        {
            return {};
        }

        if (mcData.find(ctl) == mcData.cend() || checkInproperValue(mcData[ctl]))
        {
            mc.ctl = 0;
        }
        else if (!str2uint(mcData[ctl], mc.ctl))
        {
            return {};
        }

        if (mcData.find(addr) == mcData.cend() || checkInproperValue(mcData[addr]))
        {
            mc.address = 0;
        }
        else if (!str2uint(mcData[addr], mc.address))
        {
            return {};
        }

        if (mcData.find(misc) == mcData.cend() || checkInproperValue(mcData[misc]))
        {
            mc.misc = 0;
        }
        else if (!str2uint(mcData[misc], mc.misc))
        {
            return {};
        }
        return mc;
    }

    [[nodiscard]] std::vector<MCAData>
        parseCoreMcas(std::reference_wrapper<const json> input)
    {
        std::vector<MCAData> allCoreMcas;
        if (input.get().find("MCA") == input.get().cend())
        {
            return allCoreMcas;
        }

        for (auto const& [core, threads] : input.get()["MCA"].items())
        {
            if (!startsWith(core, "core"))
            {
                continue;
            }

            uint32_t coreId;
            if (!str2uint(core.substr(4), coreId, decimal))
            {
                continue;
            }

            for (auto const& [thread, threadData] : threads.items())
            {
                if (!startsWith(thread, "thread"))
                {
                    continue;
                }

                uint32_t threadId;
                if (!str2uint(thread.substr(6), threadId, decimal))
                {
                    continue;
                }

                for (auto const& [mcSection, mcData] : threadData.items())
                {
                    auto coreMca =
                        parseMca(mcSection, mcData, coreId, threadId);
                    if (coreMca)
                    {
                        allCoreMcas.push_back(*coreMca);
                    }
                }
            }
        }
        return allCoreMcas;
    }

    [[nodiscard]] std::vector<MCAData> parseAllBigcoreMcas(
        std::reference_wrapper<const json> input, std::string bigcoreMcas[4])
    {
        std::vector<MCAData> allBigcoreMca;
        if (input.get().find("big_core") == input.get().cend())
        {
            return allBigcoreMca;
        }

        for (auto const& [core, threads] : input.get()["big_core"].items())
        {
            if (!startsWith(core, "core"))
            {
                continue;
            }

            uint32_t coreId;
            if (!str2uint(core.substr(4), coreId, decimal))
            {
                continue;
            }

            for (auto const& [thread, threadData] : threads.items())
            {
                if (!startsWith(thread, "thread") || threadData.is_string())
                {
                    continue;
                }

                uint32_t threadId;
                if (!str2uint(thread.substr(6), threadId, decimal))
                {
                    continue;
                }

                std::vector<MCAData> bigCoreMC =
                    parseBigCoreMcas(threadData, coreId, threadId, bigcoreMcas);
                for (auto const& mcData : bigCoreMC)
                {
                    allBigcoreMca.push_back(mcData);
                }
            }
        }
        return allBigcoreMca;
    }

    [[nodiscard]] std::vector<MCAData> parseUncoreMcas(
        std::reference_wrapper<const json> input)
    {
        std::vector<MCAData> output;
        if (input.get().find("MCA") == input.get().cend())
        {
            return output;
        }

        if (input.get()["MCA"].find("uncore") == input.get()["MCA"].cend())
        {
            return output;
        }

        for (auto const& [mcSection, mcData] :
                input.get()["MCA"]["uncore"].items())
        {
            auto uncoreMc = parseMca(mcSection, mcData, 0, 0);
            if (uncoreMc)
            {
                output.push_back(*uncoreMc);
            }
        }
        return output;
    }

    [[nodiscard]] std::map<uint32_t, TscData> getTscDataForProcessorType(
        const std::map<std::string, std::reference_wrapper<const json>>
        cpuSections, TscVariablesNames tscVariablesNamesForProcessor)
    {
        std::map<uint32_t, TscData> output;
        std::pair<uint32_t, uint64_t> firstIerrTimestampSocket;
        std::pair<uint32_t, uint64_t> firstMcerrTimestampSocket;
        std::pair<uint32_t, uint64_t> firstRmcaTimestampSocket;
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            uint32_t socketId;
            TscData tsc;
            if (!str2uint(cpu.substr(3), socketId, decimal))
            {
                continue;
            }

            std::optional pcu_first_ierr_tsc_lo_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_ierr_tsc_lo_cfg_varname);
            if (!pcu_first_ierr_tsc_lo_cfg)
            {
                tsc.pcu_first_ierr_tsc_lo_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_ierr_tsc_lo_cfg = *pcu_first_ierr_tsc_lo_cfg;
            }

            std::optional pcu_first_ierr_tsc_hi_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_ierr_tsc_hi_cfg_varname);
            if (!pcu_first_ierr_tsc_hi_cfg)
            {
                tsc.pcu_first_ierr_tsc_hi_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_ierr_tsc_hi_cfg = *pcu_first_ierr_tsc_hi_cfg;
            }

            std::optional pcu_first_mcerr_tsc_lo_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_mcerr_tsc_lo_cfg_varname);
            if (!pcu_first_mcerr_tsc_lo_cfg)
            {
                tsc.pcu_first_mcerr_tsc_lo_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_mcerr_tsc_lo_cfg = *pcu_first_mcerr_tsc_lo_cfg;
            }

            std::optional pcu_first_mcerr_tsc_hi_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_mcerr_tsc_hi_cfg_varname);
            if (!pcu_first_mcerr_tsc_hi_cfg)
            {
                tsc.pcu_first_mcerr_tsc_hi_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_mcerr_tsc_hi_cfg = *pcu_first_mcerr_tsc_hi_cfg;
            }

            std::optional pcu_first_rmca_tsc_lo_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_rmca_tsc_lo_cfg_varname);
            if (!pcu_first_rmca_tsc_lo_cfg)
            {
                tsc.pcu_first_rmca_tsc_lo_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_rmca_tsc_lo_cfg = *pcu_first_rmca_tsc_lo_cfg;
            }

            std::optional pcu_first_rmca_tsc_hi_cfg = getUncoreData(cpuSection,
                tscVariablesNamesForProcessor.pcu_first_rmca_tsc_hi_cfg_varname);
            if (!pcu_first_rmca_tsc_hi_cfg)
            {
                tsc.pcu_first_rmca_tsc_hi_cfg = "0x0";
            }
            else
            {
                tsc.pcu_first_rmca_tsc_hi_cfg = *pcu_first_rmca_tsc_hi_cfg;
            }

            if (countTscCfg(tsc))
            {
                locateFirstErrorsOnSockets(socketId, firstIerrTimestampSocket,
                    firstMcerrTimestampSocket, firstRmcaTimestampSocket, tsc);
                output.insert({socketId, tsc});
            }
        }

        if (firstIerrTimestampSocket.second != 0)
        {
            output.at(firstIerrTimestampSocket.first).ierr_occured_first = true;
        }

        if (firstMcerrTimestampSocket.second != 0)
        {
            output.at(firstMcerrTimestampSocket.first).mcerr_occured_first = true;
        }

        if (firstRmcaTimestampSocket.second != 0)
        {
            output.at(firstRmcaTimestampSocket.first).rmca_occured_first = true;
        }
        return output;
    }

    [[nodiscard]] bool countTscCfg(TscData& socketTscData)
    {
        uint32_t pcu_first_ierr_tsc_lo_cfg;
        bool status1 = str2uint(socketTscData.pcu_first_ierr_tsc_lo_cfg,
                               pcu_first_ierr_tsc_lo_cfg);
        uint32_t pcu_first_ierr_tsc_hi_cfg;
        bool status2 = str2uint(socketTscData.pcu_first_ierr_tsc_hi_cfg,
                                pcu_first_ierr_tsc_hi_cfg);
        uint32_t pcu_first_mcerr_tsc_lo_cfg;
        bool status3 = str2uint(socketTscData.pcu_first_mcerr_tsc_lo_cfg,
                                pcu_first_mcerr_tsc_lo_cfg);
        uint32_t pcu_first_mcerr_tsc_hi_cfg;
        bool status4 = str2uint(socketTscData.pcu_first_mcerr_tsc_hi_cfg,
                                pcu_first_mcerr_tsc_hi_cfg);
        uint32_t pcu_first_rmca_tsc_lo_cfg;
        bool status5 = str2uint(socketTscData.pcu_first_rmca_tsc_lo_cfg,
                                pcu_first_rmca_tsc_lo_cfg);
        uint32_t pcu_first_rmca_tsc_hi_cfg;
        bool status6 = str2uint(socketTscData.pcu_first_rmca_tsc_hi_cfg,
                                pcu_first_rmca_tsc_hi_cfg);
        if (!status1 || !status2 || !status3 || !status4 || !status5 || !status6)
        {
            return false;
        }

        socketTscData.pcu_first_ierr_tsc_cfg =
            static_cast<uint64_t>(pcu_first_ierr_tsc_lo_cfg) |
            static_cast<uint64_t>(pcu_first_ierr_tsc_hi_cfg) << 32;
        socketTscData.pcu_first_mcerr_tsc_cfg =
            static_cast<uint64_t>(pcu_first_mcerr_tsc_lo_cfg) |
            static_cast<uint64_t>(pcu_first_mcerr_tsc_hi_cfg) << 32;
        socketTscData.pcu_first_rmca_tsc_cfg =
            static_cast<uint64_t>(pcu_first_rmca_tsc_lo_cfg) |
            static_cast<uint64_t>(pcu_first_rmca_tsc_hi_cfg) << 32;

        if (socketTscData.pcu_first_ierr_tsc_cfg != 0)
        {
            socketTscData.ierr_on_socket = true;
        }

        if (socketTscData.pcu_first_mcerr_tsc_cfg != 0)
        {
            socketTscData.mcerr_on_socket = true;
        }

        if (socketTscData.pcu_first_rmca_tsc_cfg != 0)
        {
            socketTscData.rmca_on_socket = true;
        }
        return true;
    }

    void locateFirstErrorsOnSockets(uint32_t socketId,
         std::pair<uint32_t, uint64_t>& firstIerrTimestampSocket,
         std::pair<uint32_t, uint64_t>& firstMcerrTimestampSocket,
         std::pair<uint32_t, uint64_t>& firstRmcaTimestampSocket, TscData tsc)
    {
        if (tsc.ierr_on_socket && firstIerrTimestampSocket.second == 0)
        {
            firstIerrTimestampSocket =
                std::make_pair(socketId, tsc.pcu_first_ierr_tsc_cfg);
        }
        else if (tsc.ierr_on_socket &&
                 firstIerrTimestampSocket.second > tsc.pcu_first_ierr_tsc_cfg)
        {
            firstIerrTimestampSocket =
                std::make_pair(socketId,tsc.pcu_first_ierr_tsc_cfg);
        }

        if (tsc.mcerr_on_socket && firstMcerrTimestampSocket.second == 0)
        {
            firstMcerrTimestampSocket =
                std::make_pair(socketId, tsc.pcu_first_mcerr_tsc_cfg);
        }
        else if (tsc.mcerr_on_socket &&
                 firstMcerrTimestampSocket.second > tsc.pcu_first_mcerr_tsc_cfg)
        {
            firstMcerrTimestampSocket =
                std::make_pair(socketId, tsc.pcu_first_mcerr_tsc_cfg);
        }

        if (tsc.rmca_on_socket && firstRmcaTimestampSocket.second == 0)
        {
            firstRmcaTimestampSocket =
                std::make_pair(socketId, tsc.pcu_first_rmca_tsc_cfg);
        }
        else if (tsc.rmca_on_socket &&
                 firstRmcaTimestampSocket.second > tsc.pcu_first_rmca_tsc_cfg)
        {
            firstRmcaTimestampSocket =
                std::make_pair(socketId, tsc.pcu_first_rmca_tsc_cfg);
        }
    }

    [[nodiscard]] std::map<uint32_t, PackageThermStatus>
        getThermDataForProcessorType(
        const std::map<std::string, std::reference_wrapper<const json>>
        cpuSections, const char* thermStatusVariableNameForProcessor)
    {
        std::map<uint32_t, PackageThermStatus> thermStatus;
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            uint32_t socketId;
            PackageThermStatus thermStatusSocket;
            if (!str2uint(cpu.substr(3), socketId, decimal))
            {
                continue;
            }
            std::optional thermStatusRaw = getUncoreData(cpuSection,
                thermStatusVariableNameForProcessor);
            if (!thermStatusRaw)
            {
                return {};
            }

            if (!str2uint(*thermStatusRaw,
                thermStatusSocket.package_therm_status))
            {
                return {};
            }
            thermStatus.insert({socketId, thermStatusSocket});
        }
        return thermStatus;
    }
};
