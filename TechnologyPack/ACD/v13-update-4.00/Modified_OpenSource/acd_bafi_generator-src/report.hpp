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
#include <nlohmann/json.hpp>
#include <string>
#include <regex>

#include "generic_report.hpp"
#include "pcilookup.hpp"
#include "tor_eaglestream.hpp"
#include "tor_defs.hpp"
#include "tor_defs_cpx.hpp"
#include "tor_defs_icx.hpp"
#include "tor_defs_skx.hpp"
#include "tor_defs_spr.hpp"
#include "tor_purley.hpp"
#include "tor_whitley.hpp"
#include "utils.hpp"

using json = nlohmann::json;

template <typename T>
class Report : public GenericReport
{
  public:
    Report() = delete;
    Report(Summary& summary, T& tor, std::string cpuType, std::string cpuId, json deviceMapFile,
            json silkscreenMapFile, const json& input) :
            GenericReport(summary), tor(tor), input(input) {

        Cpu = cpuType;
        Id = cpuId;
        deviceMap = deviceMapFile;
        silkscreenMap = silkscreenMapFile;

        if (Cpu == "ICX")
        {
            PORT_ID = ICX_PORT_ID;
            LLCS = ICX_LLCS;
            FirstErrorCha = IcxfirstErrorCha;
            FirstError = IcxfirstError;
            OpCodeDecode = IcxOpCodeDecode;
        }
        else if (Cpu == "CPX" || Cpu == "CLX")
        {
            PORT_ID = CPX_PORT_ID;
            LLCS = CPX_LLCS;
            FirstErrorCha = CpxfirstErrorCha;
            FirstError = CpxfirstError;
            OpCodeDecode = CpxOpCodeDecode;
        }
        else if (Cpu == "SKX")
        {
            PORT_ID = SKX_PORT_ID;
            LLCS = SKX_LLCS;
            FirstErrorCha = SkxfirstErrorCha;
            FirstError = SkxfirstError;
            OpCodeDecode = SkxOpCodeDecode;
        }
        else if (Cpu == "SPR" || Cpu == "SPR-HBM")
        {
            PORT_ID = SPR_PORT_ID;
            LLCS = SPR_LLCS;
            FirstErrorCha = SprfirstErrorCha;
            FirstError = SprfirstError;
            OpCodeDecode = SprOpCodeDecode;
        }
    };

    [[nodiscard]] bool chaNotPresentInMce(uint8_t chaNum, json& summaryEntry, json& errorsPerSocket)
    {
        std::string firstError;
        for (const auto& [key, val] : summaryEntry[0]["MCE"].items())
        {
            std::string chaBank = "bank CHA" + std::to_string(chaNum);
            std::string firstMcerr = "FirstMCERR = CHA" + std::to_string(chaNum) + ",";
            if (std::string(key).find(chaBank) != std::string::npos &&
                std::string(errorsPerSocket).find(firstMcerr) == std::string::npos)
            {
                return true;
            }
        }
        return false;
    }

    [[nodiscard]] json createJSONReport() {
        json report = createGenericReport();
        json& torReport = report["TOR"];
        json& summaryReport = report["output_summary"];
        summaryReport["CPU_type"] = getFullVersionName(input);
        summaryReport["Crashdump_timestamp"] = getTimestamp(input);
        for (auto const& [socketId, reportData] : tor)
        {
            std::string socket = "socket" + std::to_string(socketId);
            auto ctxSocket = reportData.first;
            auto tors = reportData.second;
            json& summaryEntry = summaryReport[socket];
            json& socketEntry = torReport[socket];
            json errorsPerSocket;
            json uboxAdditionalInformation = nlohmann::ordered_json::object();
            json thermalStatus;
            socketEntry = json::array();
            errorsPerSocket["Errors_Per_Socket"] =
                createSummaryString(ctxSocket, FirstError, Cpu, summaryEntry);
            std::regex uboxMCERR("FirstMCERR = .*, bank 6");
            std::regex uboxMCE("UBOX");
            std::regex iioMCE("IIO");
            if(std::regex_search(
                std::string(errorsPerSocket["Errors_Per_Socket"]), uboxMCERR))
            {
                for (auto& el : summaryEntry[0]["MCE"].items())
                {
                    if (std::regex_search(std::string(el.key()), uboxMCE) ||
                        std::regex_search(std::string(el.key()), iioMCE))
                    {
                        std::size_t left = std::string(el.value()).find("Bus");
                        if (left > std::string(el.value()).size())
                            continue;

                        std::size_t right = std::string(el.value()).find(" from");
                        uboxAdditionalInformation["IO_Errors"]["Device"] =
                            std::string(el.value()).substr(left, right - left);

                        auto bdfObj = nlohmann::ordered_json::object();
                        getBdfFromIoErrorsSection(std::string(el.value()).
                            substr(left, right - left), bdfObj);
                        showBdfDescription(deviceMap, bdfObj);
                        if (bdfObj.contains("Description"))
                        {
                            uboxAdditionalInformation["IO_Errors"]["Description"] =
                                std::string(bdfObj["Description"]);
                        }
                    }
                }
            }
            summaryEntry.push_back(errorsPerSocket);
            if (!uboxAdditionalInformation.empty())
                summaryEntry.push_back(uboxAdditionalInformation);

            auto packageThermStatus = createThermStatusString(socketId);
            if (packageThermStatus)
            {
                thermalStatus["Package_Therm_Status"] = *packageThermStatus;
                summaryEntry.push_back(thermalStatus);
            }

            for (auto& torEntry : tors)
            {
                json entry;
                json firstMcerr;
                json& details = entry["Details"];
                uint8_t core_id = torEntry.core_id;
                uint8_t thread_id = torEntry.thread_id;
                uint16_t request_opCode = torEntry.request_opCode;
                uint8_t in_pipe = torEntry.in_pipe;
                uint8_t retry = torEntry.retry;
                uint8_t fsm = torEntry.fsm;
                uint8_t lcs = torEntry.lcs;
                uint8_t target = torEntry.target;
                uint8_t sad = torEntry.sad;
                if (Cpu == "ICX")
                {
                    core_id = torEntry.core_id1;
                    thread_id = torEntry.thread_id1;
                    request_opCode = torEntry.request_opCode1;
                    in_pipe = torEntry.in_pipe1;
                    retry = torEntry.retry1;
                    fsm = torEntry.fsm1;
                    lcs = torEntry.lcs1;
                    target = torEntry.target1;
                    sad = torEntry.sad1;
                }

                if (Cpu == "SPR" || Cpu == "SPR-HBM")
                {
                    if (Id == "0x806f0")
                    {
                        core_id = torEntry.core_id2_2_0 |
                                    torEntry.core_id2_6_3 << 3;
                        thread_id = torEntry.thread_id2;
                        request_opCode = torEntry.orig_req1_6_0 |
                                    torEntry.orig_req1_10_7 << 7;
                        in_pipe = torEntry.in_pipe2;
                        retry = torEntry.retry2;
                        fsm = torEntry.fsm2_3_0 | torEntry.fsm2_4_4 << 4;
                        lcs = torEntry.lcs2;
                        target = torEntry.target2;
                        sad = torEntry.sad2;
                    }
                    else if (Id == "0x806f1")
                    {
                        core_id = torEntry.core_id3;
                        thread_id = torEntry.thread_id2;
                        request_opCode = torEntry.orig_req2_3_0 |
                                            torEntry.orig_req2_10_4 << 4;
                        in_pipe = torEntry.in_pipe2;
                        retry = torEntry.retry2;
                        fsm = torEntry.fsm2_3_0 | torEntry.fsm2_4_4 << 4;
                        lcs = torEntry.lcs3;
                        target = torEntry.target3;
                        sad = torEntry.sad3;
                    }
                    else
                    {
                        core_id = torEntry.core_id4;
                        thread_id = torEntry.thread_id3;
                        request_opCode = torEntry.orig_req3_1_0 |
                                            torEntry.orig_req3_10_2 << 2;
                        in_pipe = torEntry.in_pipe2;
                        retry = torEntry.retry2;
                        fsm = torEntry.fsm3_3_0 | torEntry.fsm3_4_4 << 4;
                        lcs = torEntry.lcs3;
                        target = torEntry.target4;
                        sad = torEntry.sad4;
                    }
                }
                details["CoreId"] =
                    int_to_hex(static_cast<uint8_t>(core_id));
                details["ThreadId"] =
                    int_to_hex(static_cast<uint8_t>(thread_id));
                details["CHA"] = int_to_hex(static_cast<uint8_t>(torEntry.cha));
                details["IDX"] = int_to_hex(static_cast<uint8_t>(torEntry.idx));
                details["Request"] =
                    int_to_hex(static_cast<uint16_t>(request_opCode));
                auto requestDecoded = getDecoded(
                    OpCodeDecode,
                    static_cast<uint32_t>(request_opCode));
                if (requestDecoded)
                {
                    details["Request_decoded"] = *requestDecoded;
                }

                uint64_t address = getAddress(torEntry);
                auto addressMapped = mapToMemory(address);
                if (addressMapped)
                {
                    details["Address"] = int_to_hex(address) + " ("
                        + *addressMapped + ")";
                }
                else
                {
                    details["Address"] = int_to_hex(address);
                }

                details["InPipe"] =
                    int_to_hex(static_cast<uint8_t>(in_pipe));
                details["Retry"] =
                    int_to_hex(static_cast<uint8_t>(retry));
                details["TorFSMState"] =
                    int_to_hex(static_cast<uint8_t>(fsm));
                details["LLC"] = int_to_hex(static_cast<uint8_t>(lcs));
                if (static_cast<uint8_t>(lcs) < LLCS.size())
                {
                    details["LLC_decoded"] = LLCS[lcs];
                }

                details["Target"] =
                    int_to_hex(static_cast<uint8_t>(target));
                if (static_cast<uint8_t>(target) < PORT_ID.size())
                {
                    details["Target_decoded"] = PORT_ID[target];
                }

                details["SAD"] = int_to_hex(static_cast<uint8_t>(sad));
                if (static_cast<uint8_t>(sad) < SAD_RESULT.size())
                {
                    details["SAD_decoded"] = SAD_RESULT[sad];
                }

                json& decoded = entry["Decoded"];

                if (sad == static_cast<uint8_t>(SadValues::CFG))
                {
                    decoded["ErrorType"] = "TOR Timeout Error";
                    decoded["ErrorSubType"] =
                        "Type 1: PCIe* MMCFG access cause TOR Timeout";
                    if (static_cast<uint8_t>(target) <
                        PORT_ID.size())
                    {
                        decoded["Port"] = PORT_ID[target];
                    }

                    // Type 1 gets BDF from TOR address
                    decoded["BDF"] =
                        BDFtoString(getBDFFromAddress(getAddress(torEntry)));
                    auto bdfObj = nlohmann::ordered_json::object();
                    getBdfFromFirstMcerrSection(decoded["BDF"], bdfObj);
                    if (bdfObj.contains("Bus"))
                        showBdfDescription(deviceMap, bdfObj);

                    if (bdfObj.contains("Description"))
                        decoded["Description"] = bdfObj["Description"];
                }
                else if (sad == static_cast<uint8_t>(SadValues::MMIOPartialRead))
                {
                    decoded["ErrorType"] = "TOR Timeout Error";
                    decoded["ErrorSubType"] =
                        "Type 2: PCIe* MMIO access cause TOR timeout.";
                    if (static_cast<uint8_t>(target) <
                        PORT_ID.size())
                    {
                        decoded["Port"] = PORT_ID[target];
                    }

                    // Type 2 gets BDF from memory map
                    decoded["BDF"] =
                        BDFtoString(PciBdfLookup::lookup(getAddress(torEntry)));
                    auto bdfObj = nlohmann::ordered_json::object();
                    getBdfFromFirstMcerrSection(decoded["BDF"], bdfObj);
                    if (bdfObj.contains("Bus"))
                        showBdfDescription(deviceMap, bdfObj);

                    if (bdfObj.contains("Description"))
                        decoded["Description"] = bdfObj["Description"];
                }
                else if (sad == static_cast<uint8_t>(SadValues::IO))
                {
                    decoded["ErrorType"] = "TOR Timeout Error";
                    decoded["ErrorSubType"] =
                        "Type 3: I/O Port in access cause TOR timeout.";
                    if (static_cast<uint8_t>(target) <
                        PORT_ID.size())
                    {
                        decoded["Port"] = PORT_ID[target];
                    }

                    // Type 3 gets BDF from memory map
                    decoded["BDF"] =
                        BDFtoString(PciBdfLookup::lookup(getAddress(torEntry)));
                    auto bdfObj = nlohmann::ordered_json::object();
                    getBdfFromFirstMcerrSection(decoded["BDF"], bdfObj);
                    if (bdfObj.contains("Bus"))
                        showBdfDescription(deviceMap, bdfObj);

                    if (bdfObj.contains("Description"))
                        decoded["Description"] = bdfObj["Description"];
                }

                auto firstMcerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.mcerr.firstMcerrSrcIdCha);
                if (Cpu == "SPR" || Cpu == "SPR-HBM")
                    firstMcerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.mcerrSpr.firstMcerrSrcIdCha);

                auto firstRmcaMcerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.rMcerrErr.firstMcerrSrcIdCha);

                if (firstMcerrCha == static_cast<uint8_t>(torEntry.cha) &&
                    (ctxSocket.mcerr.firstMcerrValid ||
                    ctxSocket.mcerrSpr.firstMcerrValid))
                {
                    firstMcerr["First_MCERR"] = entry["Decoded"];
                    firstMcerr["First_MCERR"]["Address"] = details["Address"];
                    firstMcerr["First_MCERR"]["CHA"] = details["CHA"];
                    if (!entry["Decoded"].empty())
                    {
                        summaryEntry.push_back(firstMcerr);
                    }

                    entry["First_MCERR"] = true;
                }
                else if (!entry["Decoded"].empty() &&
                        (sad == static_cast<uint8_t>(SadValues::CFG) ||
                        sad == static_cast<uint8_t>(SadValues::MMIOPartialRead) ||
                        sad == static_cast<uint8_t>(SadValues::IO)) &&
                        chaNotPresentInMce(torEntry.cha, summaryEntry,
                            errorsPerSocket["Errors_Per_Socket"]))
                {
                    firstMcerr["First_MCERR_FALSE"] = entry["Decoded"];
                    firstMcerr["First_MCERR_FALSE"]["Address"] = details["Address"];
                    firstMcerr["First_MCERR_FALSE"]["CHA"] = details["CHA"];
                    if (!entry["Decoded"].empty())
                    {
                        summaryEntry.push_back(firstMcerr);
                    }
                    entry["First_MCERR"] = false;
                }
                else if (Cpu == "SPR" || Cpu == "SPR-HBM")
                {
                    if (firstRmcaMcerrCha == static_cast<uint8_t>(torEntry.cha) &&
                        ctxSocket.rMcerrErr.firstMcerrValid)
                    {
                        firstMcerr["First_MCERR"] = entry["Decoded"];
                        firstMcerr["First_MCERR"]["Address"] = details["Address"];
                        if (!entry["Decoded"].empty())
                        {
                            summaryEntry.push_back(firstMcerr);
                        }
                        entry["First_MCERR"] = true;
                    }
                    else
                    {
                        entry["First_MCERR"] = false;
                    }
                }
                else
                {
                    entry["First_MCERR"] = false;
                }

                auto firstIerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.ierr.firstIerrSrcIdCha);
                if (Cpu == "SPR" || Cpu == "SPR-HBM")
                    firstIerrCha = getFirstErrorCha(
                    FirstErrorCha, ctxSocket.ierrSpr.firstIerrSrcIdCha);

                if (firstIerrCha == static_cast<uint8_t>(torEntry.cha) &&
                    (ctxSocket.ierr.firstIerrValid ||
                    ctxSocket.ierrSpr.firstIerrValid))
                {
                    entry["First_IERR"] = true;
                }
                else
                {
                    entry["First_IERR"] = false;
                }

                socketEntry.push_back(entry);
            }
        }
        // Get memory errors for specific CPUs
        if (Cpu == "SKX")
        {
            auto sockets = findSocketsPurley(input);
            for (const auto& socket : sockets)
            {
                json& summaryEntry = summaryReport[socket];
                auto socketId = socket.substr(6, 1);
                auto memoryErrors = decodeRetryLogPurley(input, silkscreenMap, socketId);
                if (memoryErrors != nullptr)
                    summaryEntry.push_back(memoryErrors);
            }
        }
        else
        {
            auto cpuSections = prepareJson(input);
            for (auto const& [socketId, cpuSection] : cpuSections)
            {
                std::string socket = "socket" + std::string(socketId).substr(3,1);
                json& summaryEntry = summaryReport[socket];
                if (Cpu == "ICX" || Cpu == "CPX")
                {
                    auto memoryErrors = decodeRetryLogWhitley(cpuSection, silkscreenMap,
                        std::string(socketId).substr(3,1));
                    if (memoryErrors != nullptr)
                        summaryEntry.push_back(memoryErrors);
                }
                else if (Cpu == "SPR" || Cpu == "SPR-HBM")
                {
                    auto memoryErrors = decodeRetryLogEaglestream(cpuSection, silkscreenMap,
                        std::string(socketId).substr(3,1));
                    if (memoryErrors != nullptr)
                        summaryEntry.push_back(memoryErrors);

                    auto counters = parseCounterRegisters(cpuSection);
                    if (counters != nullptr)
                        summaryEntry.push_back(counters);
                }
            }
        }
        return report;
    }

  private :
    std::string Cpu;
    std::string Id;
    json deviceMap;
    json silkscreenMap;
    std::array<const char*, 34> PORT_ID;
    std::array<const char*, 16> LLCS;
    std::map<uint8_t, uint8_t> FirstErrorCha;
    std::map<uint16_t, const char*> FirstError;
    std::map<uint32_t, const char*> OpCodeDecode;

    [[nodiscard]] uint64_t getAddress(const TORDataGeneric& torEntry)
    {
        if (Cpu == "ICX")
        {
        return static_cast<uint64_t>(torEntry.address_8_6) << 6 |
               static_cast<uint64_t>(torEntry.address_16_9) << 9 |
               static_cast<uint64_t>(torEntry.address_19_17) << 17 |
               static_cast<uint64_t>(torEntry.address_27_20) << 20 |
               static_cast<uint64_t>(torEntry.address_30_28) << 28 |
               static_cast<uint64_t>(torEntry.address_38_31) << 31 |
               static_cast<uint64_t>(torEntry.address_41_39) << 39 |
               static_cast<uint64_t>(torEntry.address_49_42) << 42 |
               static_cast<uint64_t>(torEntry.address_51_50) << 50;
        }
        else if (Cpu == "SPR" || Cpu == "SPR-HBM")
        {
            if (Id == "0x806f0" || Id == "0x806f1")
            {
                return static_cast<uint64_t>(torEntry.address_6_6_spr1) << 6 |
                    static_cast<uint64_t>(torEntry.address_16_7_spr1) << 7 |
                    static_cast<uint64_t>(torEntry.address_17_17_spr1) << 17 |
                    static_cast<uint64_t>(torEntry.address_27_18_spr1) << 18 |
                    static_cast<uint64_t>(torEntry.address_28_28_spr1) << 28 |
                    static_cast<uint64_t>(torEntry.address_38_29_spr1) << 29 |
                    static_cast<uint64_t>(torEntry.address_39_39_spr1) << 39 |
                    static_cast<uint64_t>(torEntry.address_49_40_spr1) << 40 |
                    static_cast<uint64_t>(torEntry.address_50_50_spr1) << 50 |
                    static_cast<uint64_t>(torEntry.address_51_51_spr1) << 51;
            }
            else
            {
                return static_cast<uint64_t>(torEntry.address_16_6_spr3) << 6 |
                    static_cast<uint64_t>(torEntry.address_27_17_spr3) << 17 |
                    static_cast<uint64_t>(torEntry.address_38_28_spr3) << 28 |
                    static_cast<uint64_t>(torEntry.address_49_39_spr3) << 39 |
                    static_cast<uint64_t>(torEntry.address_51_50_spr3) << 50;
            }
        }
        else
        {
            return static_cast<uint64_t>(torEntry.address) << 14 |
               static_cast<uint64_t>(torEntry.addr_lo);
        }
    }

    const T& tor;
    const json& input;
};
