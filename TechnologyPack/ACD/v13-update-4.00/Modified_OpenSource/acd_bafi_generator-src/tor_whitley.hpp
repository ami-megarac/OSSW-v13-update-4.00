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

#include "aer.hpp"
#include "mca_defs.hpp"
#include "tor_defs_cpx.hpp"
#include "tor_defs_icx.hpp"
#include "tor_defs_skx.hpp"
#include "utils.hpp"

using json = nlohmann::json;

const static uint8_t decimal = 10;

union Version
{
    struct
    {
        uint32_t revision : 8, reserved0 : 4, cpu_id : 12,
                crash_record_type : 5, reserved1 : 3;
    };
    uint32_t version;
};

union Retry_rd_err_log
{
    struct
    {
        uint32_t valid: 1, uc: 1, over: 1, mode: 4, reserved1: 15,
        system_addr_valid: 1, reserved2: 9;
    };
    uint32_t retry_rd_err_log = 0;
};

union Retry_rd_err_log_address1
{
    struct
    {
        uint32_t reserved: 6, chip_select: 3, cbit: 4, bank: 6, column: 10,
        reserved1: 3;
    };
    uint32_t retry_rd_err_log_address1 = 0;
};

union Retry_rd_err_log_address3
{
    struct
    {
       uint64_t address: 52, reserved: 12;
    };
    uint64_t retry_rd_err_log_address3 = 0;
};

union Retry_rd_err_set2_log_address2
{
    struct
    {
        uint32_t row: 18, reserved: 14;
    };
    uint32_t retry_rd_err_set2_log_address2 = 0;
};

std::array<std::string, 4> imcChannels = {
    "B30_D26_F0_",
    "B30_D27_F0_",
    "B30_D28_F0_",
    "B30_D29_F0_"
};

std::array<std::string, 4> offsets = {
    "0x22C60",
    "0x22E54",
    "0x26C60",
    "0x26E54"
};

const std::map<std::string, const char*> ImcChannel = {
    {"B30_D26_F0_0x22C60", "IMC 0, Channel 0"},
    {"B30_D26_F0_0x22E54", "IMC 0, Channel 0"},
    {"B30_D26_F0_0x22C58", "IMC 0, Channel 0"},
    {"B30_D26_F0_0x22E58", "IMC 0, Channel 0"},
    {"B30_D26_F0_0x26C60", "IMC 0, Channel 1"},
    {"B30_D26_F0_0x26E54", "IMC 0, Channel 1"},
    {"B30_D26_F0_0x26C58", "IMC 0, Channel 1"},
    {"B30_D26_F0_0x26E58", "IMC 0, Channel 1"},
    {"B30_D27_F0_0x22C60", "IMC 1, Channel 0"},
    {"B30_D27_F0_0x22E54", "IMC 1, Channel 0"},
    {"B30_D27_F0_0x22C58", "IMC 1, Channel 0"},
    {"B30_D27_F0_0x22E58", "IMC 1, Channel 0"},
    {"B30_D27_F0_0x26C60", "IMC 1, Channel 1"},
    {"B30_D27_F0_0x26E54", "IMC 1, Channel 1"},
    {"B30_D27_F0_0x26C58", "IMC 1, Channel 1"},
    {"B30_D27_F0_0x26E58", "IMC 1, Channel 1"},
    {"B30_D28_F0_0x22C60", "IMC 2, Channel 0"},
    {"B30_D28_F0_0x22E54", "IMC 2, Channel 0"},
    {"B30_D28_F0_0x22C58", "IMC 2, Channel 0"},
    {"B30_D28_F0_0x22E58", "IMC 2, Channel 0"},
    {"B30_D28_F0_0x26C60", "IMC 2, Channel 1"},
    {"B30_D28_F0_0x26E54", "IMC 2, Channel 1"},
    {"B30_D28_F0_0x26C58", "IMC 2, Channel 1"},
    {"B30_D28_F0_0x26E58", "IMC 2, Channel 1"},
    {"B30_D29_F0_0x22C60", "IMC 3, Channel 0"},
    {"B30_D29_F0_0x22E54", "IMC 3, Channel 0"},
    {"B30_D29_F0_0x22C58", "IMC 3, Channel 0"},
    {"B30_D29_F0_0x22E58", "IMC 3, Channel 0"},
    {"B30_D29_F0_0x26C60", "IMC 3, Channel 1"},
    {"B30_D29_F0_0x26E54", "IMC 3, Channel 1"},
    {"B30_D29_F0_0x26C58", "IMC 3, Channel 1"},
    {"B30_D29_F0_0x26E58", "IMC 3, Channel 1"}
};

const std::map<uint8_t, const char*> Mode = {
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

const std::map<uint16_t, const char*> decodedCpuId = {
    {0x2A, "SKX"},
    {0x1A, "ICX"},
    {0x1B, "ICX"},
    {0x34, "CPX"},
    {0x2C, "CLX"},
    {0x1C, "SPR"},
};

const std::map<uint16_t, const char*> decodedCpuFullName = {
    {0x2A, "Sky Lake SP"},
    {0x1A, "Icelake SP"},
    {0x1B, "Icelake D"},
    {0x34, "Cooper Lake"},
    {0x2C, "Cascade Lake SP"},
    {0x1C, "Sapphire Rapids SP"},
    {0x6F, "Sapphire Rapids HBM"},
  };

const std::map<std::string, std::reference_wrapper<const json>>
 prepareJson(const json& input);

[[nodiscard]] std::optional<std::reference_wrapper<const json>>
    getCpuSection(std::string cpu, const json& input)
{
    const auto& cpuSection = input.find(cpu);
    if (cpuSection != input.cend())
    {
        return *cpuSection;
    }
    return {};
}

[[nodiscard]] std::map<std::string, std::reference_wrapper<const json>>
    getAllCpuSections(const json& processorsSection,
    std::vector<std::string> cpus)
{
    std::map<std::string, std::reference_wrapper<const json>> allCpuSections;
    for (const auto& cpu : cpus)
    {
        auto cpuSection = getCpuSection(cpu, processorsSection);
        if (!cpuSection)
        {
            continue;
        }

        allCpuSections.insert(
            std::pair<std::string, std::reference_wrapper<const json>>(
            cpu, *cpuSection));
    }
    return allCpuSections;
}

[[nodiscard]] std::optional<std::reference_wrapper<const json>>
    getProperRootNode(const json& input)
{
    if (input.find("METADATA") != input.cend())
    {
        return input;
    }
    else if (input.find("sys_info") != input.cend())
    {
        return input;
    }
    else if (input.find("crash_data") != input.cend())
    {
        return input["crash_data"];
    }
    else if (input.find("crashdump") != input.cend())
    {
        return input["crashdump"]["cpu_crashdump"]["crash_data"];
    }
    else if (input.find("Oem") != input.cend())
    {
        return input["Oem"]["Intel"]["crash_data"];
    }
    return {};
}

[[nodiscard]] std::string getCpuType(const json& input)
{
    Version decodedVersion;
    if (input.contains("metadata")){
        if (!input["metadata"].contains("_version")){
            return "SKX";
        }
    }

    if (input.contains("METADATA")){
        if (input["METADATA"].contains("cpu0")){
            if (input["METADATA"]["cpu0"].contains("platform_id")){
                if (input["METADATA"]["cpu0"]["platform_id"] == "0x4")
                    return "SPR-HBM";
            }
        }
    }

    if (!input["METADATA"].contains("_version") ||
        !str2uint(input["METADATA"]["_version"], decodedVersion.version))
    {
        return {};
    }

    auto cpuId = getDecoded(decodedCpuId,
        static_cast<uint16_t>(decodedVersion.cpu_id));

    if (!cpuId)
    {
        return {};
    }
    return *cpuId;
}

std::string getTimestamp(const json input)
{
  if (input.contains("metadata"))
  {
    if (input["metadata"].contains("timestamp"))
    {
      return input["metadata"]["timestamp"];
    }
  }
  else if (input.contains("METADATA"))
  {
    if (input["METADATA"].contains("timestamp"))
    {
      return input["METADATA"]["timestamp"];
    }
  }
  return "";
}

std::string getFullVersionName(const json input)
{
  Version decodedVersion;
  std::string metadataKey = "";
  std::string versionKey = "";

  if (input.contains("metadata"))
  {
    metadataKey = "metadata";
    versionKey = "crashdump_version";
  }
  else if (input.contains("METADATA"))
  {
    metadataKey = "METADATA";
    versionKey = "_version";
  }

  if (!input[metadataKey].contains(versionKey) ||
      !str2uint(input[metadataKey][versionKey], decodedVersion.version))
  {
    return "";
  }

  auto cpuId = getDecoded(decodedCpuFullName,
        static_cast<uint16_t>(decodedVersion.cpu_id));
  if (!cpuId)
  {
    return "";
  }

  return *cpuId;
}

[[nodiscard]] std::optional<std::string> getCpuId(const json& input)
{
    if (input.contains("metadata"))
    {
        if (input["metadata"].contains("cpu0"))
        {
            if (input["metadata"]["cpu0"].contains("cpuid"))
                return input["metadata"]["cpu0"]["cpuid"].get_ref<const std::string&>();
        }
    }
    else
    {
        if (input["METADATA"].contains("cpu0"))
        {
            if (input["METADATA"]["cpu0"].contains("cpuid"))
                return input["METADATA"]["cpu0"]["cpuid"].get_ref<const std::string&>();
        }
    }
    return {};
}

[[nodiscard]] std::vector<std::string> findCpus(const json& input)
{
    std::vector<std::string> cpus;
    const auto& metaData = input.find("METADATA");
    if (metaData != input.cend())
    {
        for (const auto& metaDataItem : metaData.value().items())
        {
            if (startsWith(metaDataItem.key(), "cpu"))
            {
                cpus.push_back(metaDataItem.key());
            }
        }
    }
    return cpus;
}

[[nodiscard]] json decodeRetryLogWhitley(const json& input, json silkscreenMap, std::string socketId)
{
    Retry_rd_err_log decodedErrLog;
    Retry_rd_err_log_address3 decodedErrLogAddress3;
    Retry_rd_err_set2_log_address2 decodedErrSet2LogAddress2;
    Retry_rd_err_log_address1 decodedErrLogAddress;
    json memoryErrors;
    if (input.find("uncore") != input.cend())
    {
        for (auto channel : imcChannels)
        {
            for (auto offset : offsets)
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

                    std::optional<std::string> imcChannel = getDecoded(ImcChannel, reg);
                    if (!imcChannel)
                    {
                        continue;
                    }

                    std::optional<std::string> mode = getDecoded(Mode,
                        static_cast<uint8_t>(decodedErrLog.mode));
                    if (!mode)
                    {
                        continue;
                    }

                    if (reg.find("C60") != std::string::npos)
                    {
                        reg.replace(15, 3, "C58");
                    }
                    else if (reg.find("E54") != std::string::npos)
                    {
                        reg.replace(15, 3, "E58");
                    }

                    if (input["uncore"].find(reg) != input["uncore"].cend())
                    {
                        if (reg.find("C58") != std::string::npos || reg.find("E58") != std::string::npos)
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
                                static_cast<uint8_t>(decodedErrLogAddress.bank & 0xc) >> 2);

                                std::string bankGroup = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.bank & 0x3));

                                std::string rank = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.chip_select % 4));

                                std::string subRank = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.cbit));

                                std::string dimm = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.chip_select >>
                                    2 & 0b00000001));

                                std::string column = std::to_string(
                                static_cast<uint8_t>(decodedErrLogAddress.column >> 3));

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
                                ssValue << "Mode: " << *mode << ", ";
                                ssValue << "Column: " << column;
                            }
                        }

                        reg.replace(15, 3, "E5C");

                        if (str2uint(input["uncore"][reg], decodedErrSet2LogAddress2.retry_rd_err_set2_log_address2))
                        {
                            std::string row = std::to_string(
                            static_cast<uint16_t>(decodedErrSet2LogAddress2.row));
                            ssValue << ", Row: " << row;
                        }

                        // Check if there is a valid system address to be displayed
                        reg.replace(15, 3, "C60");
                        if (str2uint(input["uncore"][reg], decodedErrLog.retry_rd_err_log))
                        {
                            if (decodedErrLog.system_addr_valid)
                            {
                                if (reg.find("22C60") != std::string::npos)
                                {
                                    reg.replace(13, 5, "20ED8");
                                }
                                else if (reg.find("26C60") != std::string::npos)
                                {
                                    reg.replace(13, 5, "24ED8");
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
                        reg.replace(15, 3, "E54");
                        if (str2uint(input["uncore"][reg], decodedErrLog.retry_rd_err_log))
                        {
                            if (decodedErrLog.system_addr_valid)
                            {
                                if (reg.find("22E54") != std::string::npos)
                                {
                                    reg.replace(13, 5, "20EE0");
                                }
                                else if (reg.find("26E54") != std::string::npos)
                                {
                                    reg.replace(13, 5, "24EE0");
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

                    if (decodedErrLog.uc)
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

[[nodiscard]] std::optional<std::reference_wrapper<const json>>
    getProcessorsSection(const json& input)
{
    const auto& processorsSection = input.find("CPU");
    if (processorsSection != input.cend())
    {
        return *processorsSection;
    }

    const auto& processorsSection1 = input.find("PROCESSORS");
    if (processorsSection1 != input.cend())
    {
        return *processorsSection1;
    }
    return {};
}

[[nodiscard]] const std::map<std::string, std::reference_wrapper<const json>>
    prepareJson(const json& input)
{
    std::vector<std::string> cpus = findCpus(input);
    std::optional processorsSection = getProcessorsSection(input);
    auto cpuSections = getAllCpuSections(*processorsSection, cpus);
    return cpuSections;
}

[[nodiscard]] std::optional<std::reference_wrapper<const json>>
    getMetadataSection(const json& input)
{
    const auto& metadataSection = input.find("METADATA");
    if (metadataSection != input.cend())
    {
        return *metadataSection;
    }

    const auto& metadataSection1 = input.find("metadata");
    if (metadataSection1 != input.cend())
    {
        return *metadataSection1;
    }
    return {};
}

class WhitleyCpu
{
  public:
    [[nodiscard]] std::map<std::string, std::array<uint64_t, 2>>
    getMemoryMap(const json& input)
    {
        // map is the same for all CPUs, so if once created other CPUs can be
        // skipped
        bool mapCreated = false;
        std::map<std::string, std::array<uint64_t, 2>> memoryMap;
        auto cpuSections = prepareJson(input);
        for (auto const& [cpu, cpuSection] : cpuSections)
        {
            if (mapCreated)
            {
                break;
            }

            if  (cpuSection.get().find("address_map") ==
                cpuSection.get().cend())
            {
                continue;
            }

            for (auto const& [name, value] :
                cpuSection.get()["address_map"].items())
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

    [[nodiscard]] std::optional<std::string>
        getSysInfoData(const json& input, std::string socket,
                       std::string varName)
    {
        if (input.contains("METADATA"))
        {
            if (input["METADATA"].contains(socket))
            {
                if (input["METADATA"][socket].find(varName) !=
                 input["METADATA"][socket].cend())
                {
                    if (input["METADATA"][socket][varName]
                    .is_string() && !checkInproperValue(
                    input["METADATA"][socket][varName]))
                    {
                        return input["METADATA"][socket][varName]
                        .get<std::string>();
                    }
                }
            }
        }
        return {};
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

        MCAData mc = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                     false, false, false, false, false};
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
            mc.cbo = false;
            if (!str2uint(mcLower.substr(2), mc.bank, 10))
            {
                return {};
            }
        }
        else if (startsWith(mcLower, "cbo"))
        {
            mc.cbo = true;
            if (!str2uint(mcLower.substr(3), mc.bank, 10))
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
        parseCoreMcas(std::reference_wrapper<const json> input)
    {
        std::vector<MCAData> allCoreMcas;
        if (input.get().find("MCA") == input.get().cend())
        {
            return allCoreMcas;
        }

        for (auto const& [core, threads] : input.get()["MCA"].items())
        {
            if (!startsWith(std::string(core), "core"))
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
            if (countTscCfg(tsc))
            {
                locateFirstErrorsOnSockets(socketId, firstIerrTimestampSocket,
                    firstMcerrTimestampSocket, tsc);
                output.insert({socketId, tsc});
            }
        }
        if (firstIerrTimestampSocket.second != 0)
        {
            output.at(firstIerrTimestampSocket.first).ierr_occured_first = true;
        }
        if (firstMcerrTimestampSocket.second != 0)
        {
            output.at(firstMcerrTimestampSocket.first).mcerr_occured_first =
                true;
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
        if (!status1 || !status2 || !status3 || !status4)
        {
            return false;
        }
        socketTscData.pcu_first_ierr_tsc_cfg =
            static_cast<uint64_t>(pcu_first_ierr_tsc_lo_cfg) |
            static_cast<uint64_t>(pcu_first_ierr_tsc_hi_cfg) << 32;
        socketTscData.pcu_first_mcerr_tsc_cfg =
            static_cast<uint64_t>(pcu_first_mcerr_tsc_lo_cfg) |
            static_cast<uint64_t>(pcu_first_mcerr_tsc_hi_cfg) << 32;

        if (socketTscData.pcu_first_ierr_tsc_cfg != 0)
        {
            socketTscData.ierr_on_socket = true;
        }
        if (socketTscData.pcu_first_mcerr_tsc_cfg != 0)
        {
            socketTscData.mcerr_on_socket = true;
        }
        return true;
    }

    void locateFirstErrorsOnSockets(uint32_t socketId,
         std::pair<uint32_t, uint64_t>& firstIerrTimestampSocket,
         std::pair<uint32_t, uint64_t>& firstMcerrTimestampSocket, TscData tsc)
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
