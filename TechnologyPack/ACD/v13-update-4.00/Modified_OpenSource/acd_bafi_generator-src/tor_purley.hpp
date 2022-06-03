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
#include <regex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "aer.hpp"
#include "mca_defs.hpp"
#include "tor_defs_skx.hpp"
#include "utils.hpp"

using json = nlohmann::json;

union Retry_rd_err_logPurley
{
    struct
    {
        uint32_t valid: 1, uc: 1, over: 1, mode: 4, reserved: 25;
    };
    uint32_t retry_rd_err_log = 0;
};

union Retry_rd_err_log_address1Purley
{
    struct
    {
       uint32_t reserved1: 6, chip_select: 3, cbit: 3, bank: 4, column: 12,
       reserved2: 4;
    };
    uint32_t retry_rd_err_log_address1 = 0;
};

union Retry_rd_err_log_address2Purley
{
    struct
    {
        uint32_t row: 21, reserved: 11;
    };
    uint32_t retry_rd_err_log_address2 = 0;
};

const std::map<std::string, const char*> ImcChannelPurley = {
    {"B02_D0a_F3_0x154", "IMC 0, Channel 0"},
    {"B02_D0a_F7_0x154", "IMC 0, Channel 1"},
    {"B02_D0b_F3_0x154", "IMC 0, Channel 2"},
    {"B02_D0c_F3_0x154", "IMC 1, Channel 0"},
    {"B02_D0c_F7_0x154", "IMC 1, Channel 1"},
    {"B02_D0d_F3_0x154", "IMC 1, Channel 2"},
};

const std::map<uint8_t, const char*> ModePurley = {
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

std::array<std::string, 6> imcChannelsPurley = {
    "B02_D0a_F3_",
    "B02_D0a_F7_",
    "B02_D0b_F3_",
    "B02_D0c_F3_",
    "B02_D0c_F7_",
    "B02_D0d_F3_",
};

[[nodiscard]] std::optional<std::reference_wrapper<const json>>
    getSocketSection(std::string socket, const json& input)
{
    const auto& socketSection = input.find(socket);
    if (socketSection != input.cend())
    {
        return *socketSection;
    }
    return {};
}

[[nodiscard]] std::map<std::string, std::reference_wrapper<const json>>
    getAllSocketSections(const json& dumpSection,
    std::vector<std::string> sockets)
{
    std::map<std::string, std::reference_wrapper<const json>> allSocketsSections;
    for (const auto& socket : sockets)
    {
        if (!dumpSection)
        {
            continue;
        }

        allSocketsSections.insert(
            std::pair<std::string, std::reference_wrapper<const json>>(
            socket, dumpSection));
    }
    return allSocketsSections;
}

[[nodiscard]] std::vector<std::string> findSocketsPurley(const json& input)
{
    std::vector<std::string> cpus;
    const auto& metaData = input.find("sys_info");
    if (metaData != input.cend())
    {
        for (const auto& metaDataItem : metaData.value().items())
        {
            if (startsWith(metaDataItem.key(), "socket"))
            {
                cpus.push_back(metaDataItem.key());
            }
        }
    }
    return cpus;
}

void decodeImcRegisters(const json& input, json& memoryErrors, json silkscreenMap, std::string socketId)
{
    Retry_rd_err_logPurley decodedErrLog;
    Retry_rd_err_log_address1Purley decodedErrLogAddress1;
    Retry_rd_err_log_address2Purley decodedErrLogAddress2;
    std::regex retryRdErrLogRegex("^imc(.)_c(.)_retry_rd_err_log$");
    std::regex retryRdErrLogAddress1Regex("^imc(.)_c(.)_retry_rd_err_log_address1$");
    std::regex retryRdErrLogAddress2Regex("^imc(.)_c(.)_retry_rd_err_log_address2$");
    std::smatch logMatches;
    std::string socket = "socket" + socketId;
    bool uncorErr;

    if (input["uncore_status_regs"].find(socket) != input["uncore_status_regs"].cend())
    {
        for (const auto& [entry, value] : input["uncore_status_regs"][socket].items())
        {
            std::stringstream ssKey;
            std::stringstream ssValue;
            if (std::regex_search(entry, logMatches, retryRdErrLogRegex))
            {
                if (!str2uint(std::string(value), decodedErrLog.retry_rd_err_log))
                {
                    continue;
                }
                if (!decodedErrLog.valid)
                {
                    continue;
                }
                std::optional<std::string> mode = getDecoded(ModePurley,
                    static_cast<uint8_t>(decodedErrLog.mode));
                if (!mode)
                {
                    continue;
                }
                if (decodedErrLog.uc)
                {
                    uncorErr = true;
                }
                else
                {
                    uncorErr = false;
                }

                std::string imcNumber = logMatches[1].str();
                std::string channelNumber = logMatches[2].str();
                if (str2uint(input["uncore_status_regs"][socket][std::string(entry + "_address1")],
                    decodedErrLogAddress1.retry_rd_err_log_address1))
                {
                    std::string bankAddress = std::to_string(
                        (static_cast<uint8_t>(decodedErrLogAddress1.bank) & 0xc) >> 2);
                    std::string bankGroup = std::to_string(
                        static_cast<uint8_t>(decodedErrLogAddress1.bank & 0x3));
                    std::string rank = std::to_string(
                        static_cast<uint8_t>(decodedErrLogAddress1.chip_select & 0x3));
                    std::string subRank = std::to_string(
                        static_cast<uint8_t>(decodedErrLogAddress1.cbit));
                    std::string dimm = std::to_string(
                        static_cast<uint8_t>(decodedErrLogAddress1.chip_select >>
                        2 & 0x1));
                    std::string column = std::to_string(
                    static_cast<uint8_t>(decodedErrLogAddress1.column >> 3));
                    std::string subChannel = std::to_string(
                    static_cast<uint8_t>(decodedErrLogAddress1.chip_select));
                    if (silkscreenMap != NULL)
                    {
                        for (const auto& [slotKey, slotVal]
                            : silkscreenMap["MemoryLocationPcb"].items())
                        {
                            if (slotVal["Socket"] != std::stoi(socketId))
                            {
                                continue;
                            }
                            if (slotVal["IMC"] != std::stoi(imcNumber))
                            {
                                continue;
                            }
                            if (slotVal["Channel"] != std::stoi(channelNumber))
                            {
                                continue;
                            }
                            if (slotVal["Slot"] != std::stoi(dimm))
                            {
                                continue;
                            }
                            ssValue << std::string(slotVal["SlotName"]) << ", ";
                            break;
                        }
                    }
                    ssKey << "IMC " << imcNumber << ", Channel " << channelNumber << ", ";
                    ssKey << "SLOT: " << dimm;
                    ssValue << "Bank: " << bankAddress << ", ";
                    ssValue << "Bank group: " << bankGroup << ", ";
                    ssValue << "Rank: " << rank << ", ";
                    ssValue << "Sub-Rank: " << subRank << ", ";
                    ssValue << "Sub-Channel: " << subChannel << ", ";
                    ssValue << "Mode: " << *mode << ", ";
                    ssValue << "Column: " << column;
                }

                if (str2uint(input["uncore_status_regs"][socket][std::string(entry + "_address2")], decodedErrLogAddress2.retry_rd_err_log_address2))
                {
                    std::string row = std::to_string(
                    static_cast<uint16_t>(decodedErrLogAddress2.row));
                    ssValue << ", Row: " << row;
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

void decodeBdfRegisters(const json& input, json& memoryErrors, json silkscreenMap, std::string socketId)
{
    Retry_rd_err_logPurley decodedErrLog;
    Retry_rd_err_log_address1Purley decodedErrLogAddress1;
    Retry_rd_err_log_address2Purley decodedErrLogAddress2;
    bool uncorErr;
    std::string retryRdErrLogOffset = "0x154";
    for (auto channel : imcChannelsPurley)
    {
        std::stringstream ssKey;
        std::stringstream ssValue;
        std::string reg = channel + retryRdErrLogOffset;
        std::string socket = "socket" + socketId;
        if (input["uncore_status_regs"].find(socket) == input["uncore_status_regs"].cend())
        {
            continue;
        }
        if (input["uncore_status_regs"][socket].find(reg) != input["uncore_status_regs"][socket].cend())
        {
            if (!str2uint(input["uncore_status_regs"][socket][reg], decodedErrLog.retry_rd_err_log))
            {
                continue;
            }
            if (!decodedErrLog.valid)
            {
                continue;
            }
            std::optional<std::string> imcChannel = getDecoded(ImcChannelPurley, reg);
            if (!imcChannel)
            {
                continue;
            }
            std::optional<std::string> mode = getDecoded(ModePurley,
                static_cast<uint8_t>(decodedErrLog.mode));
            if (!mode)
            {
                continue;
            }
            if (decodedErrLog.uc)
            {
                uncorErr = true;
            }
            else
            {
                uncorErr = false;
            }
            // Decode corresponding retry_rd_err_log_address1
            reg.replace(13, 3, "15C");
            if (input["uncore_status_regs"][socket].find(reg) != input["uncore_status_regs"][socket].cend())
            {
                if (reg.find("15C") != std::string::npos)
                {
                    if (str2uint(input["uncore_status_regs"][socket][reg],
                        decodedErrLogAddress1.retry_rd_err_log_address1))
                    {
                        std::string imcInfo = *imcChannel;
                        std::size_t pos = imcInfo.find("IMC ");
                        std::string imc = imcInfo.substr(pos + 4, 1);
                        pos = imcInfo.find("Channel ");
                        std::string channel = imcInfo.substr(pos + 8, 1);
                        std::string bankAddress = std::to_string(
                            (static_cast<uint8_t>(decodedErrLogAddress1.bank) & 0xc) >> 2);
                        std::string bankGroup = std::to_string(
                            static_cast<uint8_t>(decodedErrLogAddress1.bank & 0x3));
                        std::string rank = std::to_string(
                            static_cast<uint8_t>(decodedErrLogAddress1.chip_select & 0x3));
                        std::string subRank = std::to_string(
                            static_cast<uint8_t>(decodedErrLogAddress1.cbit));
                        std::string dimm = std::to_string(
                            static_cast<uint8_t>(decodedErrLogAddress1.chip_select >>
                            2 & 0x1));
                        std::string column = std::to_string(
                        static_cast<uint8_t>(decodedErrLogAddress1.column >> 3));
                        std::string subChannel = std::to_string(
                        static_cast<uint8_t>(decodedErrLogAddress1.chip_select));

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
                // Add row field from corresponding retry_rd_err_log_address2
                reg.replace(13, 3, "114");
                if (str2uint(input["uncore_status_regs"][socket][reg], decodedErrLogAddress2.retry_rd_err_log_address2))
                {
                    std::string row = std::to_string(
                    static_cast<uint16_t>(decodedErrLogAddress2.row));
                    ssValue << ", Row: " << row;
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

[[nodiscard]] json decodeRetryLogPurley(const json& input, json silkscreenMap, std::string socketId)
{
    json memoryErrors;
    if (input.find("uncore_status_regs") != input.cend())
    {
        decodeBdfRegisters(input, memoryErrors, silkscreenMap, socketId);
        decodeImcRegisters(input, memoryErrors, silkscreenMap, socketId);
    }
    return memoryErrors;
}

class PurleyCpu
{
    public:
    [[nodiscard]] std::map<std::string, std::array<uint64_t, 2>>
    getMemoryMap(const json& input)
    {
    // map is the same for all CPUs, so if once created other CPUs can be skipped
        std::map<std::string, std::array<uint64_t, 2>> memoryMap;
        if (input.find("addr_map") == input.cend())
        {
            return memoryMap;
        }

        if (input["addr_map"].find("sys_mem") == input["addr_map"].cend())
        {
            return memoryMap;
        }

        for (auto const& [entry, entryVal] :
            input["addr_map"]["sys_mem"].items())
        {
            if (entry == "_record_enable" && entryVal == false)
            {
               break;
            }

            if (entry == "_version")
            {
                continue;
            }

            auto shortName = entryVal["region"];
            uint64_t uintValueBase;
            uint64_t uintValueLimit;
            if (entryVal.contains("range"))
            {
                if (checkInproperValue(entryVal["range"][0]))
                {
                    continue;
                }

                if (checkInproperValue(entryVal["range"][1]))
                {
                    continue;
                }

                if (!str2uint(entryVal["range"][0], uintValueBase))
                {
                    continue;
                }

                if (!str2uint(entryVal["range"][1], uintValueLimit))
                {
                    continue;
                }

                memoryMap[shortName][0] = uintValueBase;
                memoryMap[shortName][1] = uintValueLimit;
                continue;
            }
            for (auto const& [name, value] : entryVal.items())
            {
                if (name.find("_BASE") != std::string::npos)
                {
                    if (!str2uint(value, uintValueBase))
                    {
                        continue;
                    }

                    shortName = name.substr(0, name.find("_BASE"));
                    memoryMap[shortName][0] = uintValueBase;
                }
                if (name.find("_LIMIT") != std::string::npos)
                {
                    if (!str2uint(value, uintValueLimit))
                    {
                        continue;
                    }

                    shortName = name.substr(0, name.find("_LIMIT"));
                    memoryMap[shortName][1] = uintValueLimit;
                }
            }
        }
        return memoryMap;
    }

    [[nodiscard]] std::optional<std::string>
        getUncoreData(const json& input, std::string socket, std::string varName)
    {
        if (input.contains("uncore_status_regs"))
        {
            if (input["uncore_status_regs"].contains(socket))
            {
                if (input["uncore_status_regs"][socket].find(varName) !=
                 input["uncore_status_regs"][socket].cend())
                {
                    if (input["uncore_status_regs"][socket][varName]
                    .is_string() && !checkInproperValue(
                    input["uncore_status_regs"][socket][varName]))
                    {
                        return input["uncore_status_regs"][socket][varName]
                        .get<std::string>();
                    }
                }
            }
        }

        if (input.contains("crashdump"))
        {
            if (input["crashdump"].contains("uncore_regs"))
            {
                if (input["crashdump"][socket]["uncore_regs"].find(varName) !=
                 input["crashdump"][socket]["uncore_regs"].cend())
                 {
                    if (input["crashdump"][socket]["uncore_regs"][varName]
                    .is_string() && !checkInproperValue(
                    input["crashdump"][socket]["uncore_regs"][varName]))
                    {
                        return input["crashdump"][socket]["uncore_regs"][varName]
                        .get<std::string>();
                    }
                 }
            }
        }
        return {};
    }

    [[nodiscard]] std::optional<std::string>
        getSysInfoData(const json& input, std::string socket,
                       std::string varName)
    {
        if (input.contains("sys_info"))
        {
            if (input["sys_info"].contains(socket))
            {
                if (input["sys_info"][socket].find(varName) !=
                 input["sys_info"][socket].cend())
                {
                    if (input["sys_info"][socket][varName]
                    .is_string() && !checkInproperValue(
                    input["sys_info"][socket][varName]))
                    {
                        return input["sys_info"][socket][varName]
                        .get<std::string>();
                    }
                }
            }
        }
        return {};
    }

    [[nodiscard]] std::optional<MCAData>
        getCboData(const json& mcData, uint32_t coreId, uint32_t cboId)
    {
        if (mcData.is_string())
        {
            return {};
        }

        MCAData mc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     false, false, false, false, false};
        mc.core = coreId;
        std::string ctl = std::string("cbo") + std::to_string(cboId)
                            + std::string("_mc_ctl");
        std::string status = std::string("cbo") + std::to_string(cboId)
                            + std::string("_mc_status");
        std::string addr = std::string("cbo") + std::to_string(cboId)
                            + std::string("_mc_addr");
        std::string misc = std::string("cbo") + std::to_string(cboId)
                            + std::string("_mc_misc");

        if (mcData.contains("status"))
        {
            if(checkInproperValue(mcData["status"]))
            {
                return {};
            }
        }

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

        mc.cbo = true;
        mc.bank = cboId;
        if (mcData.find(ctl) == mcData.cend() ||
            checkInproperValue(mcData[ctl]))
        {
            mc.ctl = 0;
        }
        else if (!str2uint(mcData[ctl], mc.ctl))
        {
            return {};
        }

        if (mcData.find(addr) == mcData.cend() ||
            checkInproperValue(mcData[addr]))
        {
            mc.address = 0;
        }
        else if (!str2uint(mcData[addr], mc.address))
        {
            return {};
        }

        if (mcData.find(misc) == mcData.cend() ||
            checkInproperValue(mcData[misc]))
        {
            mc.misc = 0;
        }
        else if (!str2uint(mcData[misc], mc.misc))
        {
            return {};
        }
        return mc;
    }

    [[nodiscard]] std::optional<MCAData> parseMca(const json& mcSection,
        const json& mcData, uint32_t coreId)
    {
        if (mcData.is_string())
        {
            return {};
        }

        MCAData mc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     false, false, false, false, false};
        mc.core = coreId;
        std::string ctl = "CTL";
        std::string status = "STATUS";
        std::string addr = "ADDR";
        std::string misc = "MISC";
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

        if (startsWith(mcSection, "MC"))
        {
            mc.cbo = false;
            if (!str2uint(std::string(mcSection).substr(2), mc.bank, 10))
            {
                return {};
            }
        }
        else
        {
            return {};
        }

        if (mcData.find(ctl) == mcData.cend() ||
            checkInproperValue(mcData[ctl]))
        {
            mc.ctl = 0;
        }
        else if (!str2uint(mcData[ctl], mc.ctl))
        {
            return {};
        }

        if (mcData.find(addr) == mcData.cend() ||
            checkInproperValue(mcData[addr]))
        {
            mc.address = 0;
        }
        else if (!str2uint(mcData[addr], mc.address))
        {
            return {};
        }

        if (mcData.find(misc) == mcData.cend() ||
            checkInproperValue(mcData[misc]))
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
        parseCoreMcas(const json& input, std::string socket)
    {
        std::vector<MCAData> allCoreMcas;
        if (!input.contains("core_MCA"))
        {
            return allCoreMcas;
        }

        if (input["core_MCA"].find(socket) == input["core_MCA"].cend())
        {
            return allCoreMcas;
        }

        for (auto const& [core, coreData] : input["core_MCA"][socket].items())
        {
            if (!startsWith(core, "core"))
            {
                continue;
            }

            uint32_t coreId;
            if (!str2uint(core.substr(4), coreId, 10))
            {
                continue;
            }

            for (auto const& [mcSection, mcData] : coreData.items())
            {
                auto coreMca =
                    parseMca(mcSection, mcData, coreId);
                if (coreMca)
                {
                    allCoreMcas.push_back(*coreMca);
                }
            }
        }
        return allCoreMcas;
    }

    [[nodiscard]] std::vector<MCAData> parseUncoreMcas(
        const json& input, std::string socket)
    {
        std::vector<MCAData> output;
        if (!input.contains("uncore_MCA"))
        {
            return output;
        }

        if (input["uncore_MCA"].find(socket) == input["uncore_MCA"].cend())
        {
            return output;
        }

        for (auto const& [mcSection, mcData] :
        input["uncore_MCA"][socket].items())
        {
            auto uncoreMc = parseMca(mcSection, mcData, 0);
            if (uncoreMc)
            {
                output.push_back(*uncoreMc);
            }
        }

        if (input.contains("uncore_status_regs"))
        {
            if (input["uncore_status_regs"].contains(socket))
            {
                for (uint32_t i = 0; i < 28; i++)
                {
                    auto cboData =
                        getCboData(input["uncore_status_regs"][socket], 0, i);
                    if (cboData)
                    {
                        output.push_back(*cboData);
                    }
                }
            }
        }
        else if (input.contains("crashdump"))
        {
            if (input["crashdump"].contains(socket))
            {
                if (input["crashdump"][socket].contains("uncore_status_regs"))
                {
                    for (uint32_t i = 0; i < 28; i++)
                    {
                        auto cboData = getCboData(
                            input["crashdump"][socket]["uncore_status_regs"],
                            0, i);
                        if (cboData)
                        {
                            output.push_back(*cboData);
                        }
                    }
                }
            }
        }
        return output;
    }
};
