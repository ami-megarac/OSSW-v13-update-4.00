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

#define MEMORY_MAP_FILENAME "default_memory_map.json"
#define DEVICE_MAP_FILENAME "default_device_map.json"
#define SILKSCREEN_MAP_FILENAME "default_silkscreen_map.json"
#define BAFI_VERSION "2.07"

#ifdef _WIN32
#define OS_SEP "\\"
#define BMC_PATH "C:\\temp\\bafi\\"
#else
#define OS_SEP "/"
#define BMC_PATH "/conf/crashdump/input/" //AMI
#endif

#include <cstring>
#include <iostream>
#include <nlohmann/json.hpp>
#include <regex>
#include <string>

#include "cpu_factory.hpp"
#include "report.hpp"
#include "summary.hpp"
#include "utils.hpp"

using json = nlohmann::json;

enum ReturnCode
{
  SUCCESS = 0,
  CRASHDUMP_NOT_PROVIDED = 1,
  FAILED_TO_PARSE_CRASHDUMP_TO_JSON = 2,
  INCORRECT_JSON_ROOT_NODE = 3,
  CPU_TYPE_NOT_RECOGNIZED = 4,
  FAILED_TO_PREPARE_REPORT = 5,
  SYMLINK_FILE = 6,
  HARDLINK_FILE = 7,
};

bool checkIfMapsAreSymlinkFiles()
{
  std::ifstream iDev(BMC_PATH + std::string(DEVICE_MAP_FILENAME));
  std::ifstream iSil(BMC_PATH + std::string(SILKSCREEN_MAP_FILENAME));
  std::ifstream iMem(BMC_PATH + std::string(MEMORY_MAP_FILENAME));
  if (iDev.good())
  {
    std::filesystem::path file_path(BMC_PATH + std::string(DEVICE_MAP_FILENAME));
    if (std::filesystem::is_symlink(file_path))
    {
      return true;
    }
  }
  if (iSil.good())
  {
    std::filesystem::path file_path(BMC_PATH + std::string(SILKSCREEN_MAP_FILENAME));
    if (std::filesystem::is_symlink(file_path))
    {
      return true;
    }
  }
  if (iMem.good())
  {
    std::filesystem::path file_path(BMC_PATH + std::string(MEMORY_MAP_FILENAME));
    if (std::filesystem::is_symlink(file_path))
    {
      return true;
    }
  }
  return false;
}

bool checkIfMapsAreHardlinkFiles()
{
  std::ifstream iDev(BMC_PATH + std::string(DEVICE_MAP_FILENAME));
  std::ifstream iSil(BMC_PATH + std::string(SILKSCREEN_MAP_FILENAME));
  std::ifstream iMem(BMC_PATH + std::string(MEMORY_MAP_FILENAME));
  if (iDev.good())
  {
    std::filesystem::path file_path(BMC_PATH + std::string(DEVICE_MAP_FILENAME));
    if (std::filesystem::hard_link_count(file_path) > 1)
    {
      return true;
    }
  }
  if (iSil.good())
  {
    std::filesystem::path file_path(BMC_PATH + std::string(SILKSCREEN_MAP_FILENAME));
    if (std::filesystem::hard_link_count(file_path) > 1)
    {
      return true;
    }
  }
  if (iMem.good())
  {
    std::filesystem::path file_path(BMC_PATH + std::string(MEMORY_MAP_FILENAME));
    if (std::filesystem::hard_link_count(file_path) > 1)
    {
      return true;
    }
  }
  return false;
}

Summary prepareSummary(const json& inputJson)
{
  Summary summary;
  summary.cpuType = getCpuType(inputJson);
  if (summary.cpuType == "")
    return summary;

  auto cpu = CpuFactory::createInstance(summary.cpuType, "");

  if (cpu != nullptr)
  {
    // Gather address_map if exists
    summary.memoryMap = cpu->getMemoryMap(inputJson);
    // Analyze MCA registers
    summary.mca = cpu->analyzeMca(inputJson);
    // Analyze correctable PCIe AER registers
    summary.corAer = cpu->analyzeCorAer(inputJson);
    // Analyze uncorrectable PCIe AER registers
    summary.uncAer = cpu->analyzeUncAer(inputJson);
    // Get TSC registers data
    summary.tsc = cpu->getTscData(inputJson);
    // Get Package Therm Status data
    auto thermStatus = cpu->getThermData(inputJson);
    if (thermStatus)
    {
        summary.thermStatus = *thermStatus;
    }
  }
  return summary;
}

json prepareReport(Summary summary, const json& inputJson, json deviceMap,
                  json silkscreenMap)
{
  std::string cpuId = "";
  auto optionalCpuId = getCpuId(inputJson);
  if (optionalCpuId)
  {
    cpuId = optionalCpuId.value();
  }
  auto cpu = CpuFactory::createInstance(summary.cpuType, cpuId);
  // Analyze TOR data in CPU sections
  auto tor = cpu->analyze(inputJson);
  // Initialize report from collected data
  if (summary.cpuType == "SPR" && optionalCpuId)
  {
    cpu->cpuId = optionalCpuId.value();
  }
  Report<TORData> report(summary, tor, summary.cpuType, cpu->cpuId, deviceMap,
                         silkscreenMap, inputJson);
  // Generate JSON report from collected data
  json reportJson = report.createJSONReport();
  return reportJson;
}

void findSocketsFromSummary(std::vector<std::string> &sockets, json reportJson)
{
  for (const auto& metaDataItem : reportJson["output_summary"].items())
  {
    if (startsWith(metaDataItem.key(), "socket"))
    {
        sockets.push_back(metaDataItem.key());
    }
  }
}

std::string findFirstErrorSocket(json reportJson, std::string &firstError)
{
  std::vector<std::string> sockets;
  bool ierr = false;
  bool mcerr = false;
  bool rmcaMcerr = false;
  std::string firstSocket;

  findSocketsFromSummary(sockets, reportJson);
  for (const auto& socket : sockets)
  {
    for (const auto& [pciKey, pciVal] : reportJson["output_summary"][socket].items())
    {
      if (pciVal.contains("TSC"))
      {
        for (const auto& [metaKey, metaVal] : pciVal.items())
        {
          for (const auto& [tscKey, tscVal] : metaVal.items())
          {
            if (std::string(tscVal).find("(Occurred first between all TSCs)") !=
              std::string::npos)
            {
              if (std::string(tscVal).find("pcu_first_ierr_tsc_cfg") !=
                std::string::npos)
              {
                firstError = "FirstIERR";
                ierr = true;
              }
              else if (std::string(tscVal).find("pcu_first_mcerr_tsc_cfg") !=
                std::string::npos)
              {
                firstError = "FirstMCERR";
                mcerr = true;
              }
              else if (std::string(tscVal).find("pcu_first_rmca_tsc_cfg") !=
                std::string::npos)
              {
                firstError = "RMCA_FirstMCERR";
                rmcaMcerr = true;
              }
              else
              {
                firstError = "FirstMCERR";
                mcerr = true;
              }

              firstSocket = socket;
            }
          }
        }
      }
    }
  }

  if (ierr || mcerr || rmcaMcerr)
    return firstSocket;
  else
    return "";
}

std::string findBankNumber(std::stringstream &ss)
{
  std::string bankNumber;
  int found;
  std::stringstream ss2;
  std::size_t left = ss.str().find("bank");
  if (left > ss.str().size())
    return "";
  ss2 << ss.str().substr(left, 7);
  while (!ss2.eof())
  {
    ss2 >> bankNumber;
    if (std::stringstream(bankNumber) >> found)
    {
      break;
    }
    bankNumber = "";
  }
  return bankNumber;
}

void getErrorStrings(std::string entry, std::string &ierr, std::string &mcerr,
                     std::string &rmcaMcerr, std::string cpuType)
{
  std::size_t left = std::string(entry).find("FirstIERR");

  std::size_t right = std::string(entry)
  .substr(left, std::string(entry).size()).find(", FirstMCERR");
  ierr = std::string(entry)
  .substr(left, right).substr(12);

  left = std::string(entry).find("FirstMCERR");
  right = std::string(entry)
  .substr(left, std::string(entry).size()).find(", RMCA_FirstMCERR");
  mcerr = std::string(entry)
  .substr(left, right).substr(13);

  if (cpuType == "SPR")
  {
    left = std::string(entry).find("RMCA_FirstMCERR");
    rmcaMcerr = std::string(entry)
    .substr(left, std::string(entry).size()).substr(18);
  }
}

std::string determineFirstErrorType(std::string entry, std::string &ierr,
                                    std::string &mcerr, std::string &rmcaMcerr,
                                    std::string cpuType)
{
  getErrorStrings(entry, ierr, mcerr, rmcaMcerr, cpuType);

  if (ierr != "0x0" && mcerr == "0x0")
    return "FirstIERR";
  else if (rmcaMcerr != "0x0" && rmcaMcerr != "")
    return "RMCA_FirstMCERR";
  else
    return "FirstMCERR";
}

void getMemoryErrors(json reportJson, std::string socket,
                     nlohmann::ordered_json &triageReport)
{
  bool corMemErr = false;
  bool uncorMemErr = false;
  for (const auto& [pciKey, pciVal] : reportJson["output_summary"][socket].items())
  {
    if (pciVal.contains("Memory_errors"))
    {
      for (const auto& [metaKey, metaVal] : pciVal.items())
      {
        if (metaVal.contains("Correctable"))
        {
          for (const auto& [strKey, strVal] : metaVal.items())
          {
            for (const auto& [key, val] : strVal.items())
            {
              std::string slotName;
              if (!startsWith(val, "Bank"))
              {
                slotName = std::string(val)
                  .substr(0, std::string(val).find(",")) + ", " + std::string(key);
              }
              else
              {
                slotName = "N/A, " + std::string(key);
              }
              triageReport[socket]["CorrectableMemoryErrors"]
                .push_back(slotName);
              corMemErr = true;
            }
          }
        }
        if (metaVal.contains("Uncorrectable"))
        {
          for (const auto& [strKey, strVal] : metaVal.items())
          {
            for (const auto& [key, val] : strVal.items())
            {
              std::string slotName;
              if (!startsWith(val, "Bank"))
              {
                slotName = std::string(val)
                  .substr(0, std::string(val).find(",")) + ", " + std::string(key);
              }
              else
              {
                slotName = "N/A, " + std::string(key);
              }
              triageReport[socket]["UncorrectableMemoryErrors"]
                .push_back(slotName);
              uncorMemErr = true;
            }
          }
        }
      }
    }
  }
  if (!corMemErr)
  {
    triageReport[socket]["CorrectableMemoryErrors"] = json::object();
  }
  if (!uncorMemErr)
  {
    triageReport[socket]["UncorrectableMemoryErrors"] = json::object();
  }
}

void getThermalErrors(std::string socket,
    nlohmann::ordered_json &triageReport, Summary& summary)
{
  uint32_t socketId = std::stoi(socket.substr(6));
  if (summary.thermStatus.find(socketId) == summary.thermStatus.end())
  {
    triageReport[socket]["PackageThermalStatus"] = json::object();
  }
  else
  {
    if (!bool(summary.thermStatus[socketId].prochot_log)
      && !bool(summary.thermStatus[socketId].prochot_status)
      && !bool(summary.thermStatus[socketId].pmax_log)
      && !bool(summary.thermStatus[socketId].pmax_status)
      && !bool(summary.thermStatus[socketId].out_of_spec_log)
      && !bool(summary.thermStatus[socketId].out_of_spec_status)
      && !bool(summary.thermStatus[socketId].thermal_monitor_log)
      && !bool(summary.thermStatus[socketId].thermal_monitor_status))
    {
      triageReport[socket]["PackageThermalStatus"] = "Ok";
    }
    else
    {
      triageReport[socket]["PackageThermalStatus"] = "Error";
    }
  }
}

uint32_t getFullOutput(char* inputPointer, std::size_t inputSize,
                      char*& fullOutput, size_t* fullOutputSize,
                      json deviceMap = NULL, json silkscreenMap = NULL,
                      bool standaloneApp = false)
{
  if (inputPointer == nullptr || inputSize == 0)
    return CRASHDUMP_NOT_PROVIDED;

  std::string inputStr(inputPointer);
  inputStr.erase(std::remove_if(inputStr.begin(),
    inputStr.end(), ::isspace), inputStr.end());

  auto inputJson = json::parse(inputStr, nullptr, false);
  if (inputJson.is_discarded())
    return FAILED_TO_PARSE_CRASHDUMP_TO_JSON;

  auto input = getProperRootNode(inputJson);
  if (!input)
    return INCORRECT_JSON_ROOT_NODE;

  auto summary = prepareSummary(*input);
  if (summary.cpuType == "")
  {
    return CPU_TYPE_NOT_RECOGNIZED;
  }

  if (!standaloneApp)
  {
    if (checkIfMapsAreSymlinkFiles())
    {
      return SYMLINK_FILE;
    }
    if (checkIfMapsAreHardlinkFiles())
    {
      return HARDLINK_FILE;
    }
    PciBdfLookup deviceLookup((char*)"", true, DEVICE_MAP_FILENAME);
    PciBdfLookup silkscreenLookup((char*)"", true, SILKSCREEN_MAP_FILENAME);
    PciBdfLookup PciBdfLookup((char*)"", true, MEMORY_MAP_FILENAME);
    deviceMap = deviceLookup.lookupMap;
    silkscreenMap = silkscreenLookup.lookupMap;
  }
  json reportJson = prepareReport(summary, *input, deviceMap, silkscreenMap);
  if (reportJson.is_discarded())
    return FAILED_TO_PREPARE_REPORT;

  json report;
  json& fullReport = report;
  fullReport["summary"] = reportJson;

  // Assign full output to char pointer
  std::string outputStr = fullReport.dump(4);
  std::size_t fullOutputSizeTemp = strlen(outputStr.c_str());
  *fullOutputSize = fullOutputSizeTemp;
  fullOutput = new char[*fullOutputSize + 1];
  strncpy(fullOutput, outputStr.c_str(), *fullOutputSize + 1);

  return SUCCESS;
}

uint32_t getTriageInformation(char* inputPointer, std::size_t inputSize,
                             char*& triageInfo, size_t* triageInfoSize,
                             json deviceMap = NULL, json silkscreenMap = NULL,
                             bool standaloneApp = false)
{
  if (inputPointer == nullptr || inputSize == 0)
    return CRASHDUMP_NOT_PROVIDED;

  std::string inputStr(inputPointer);
  inputStr.erase(std::remove_if(inputStr.begin(),
    inputStr.end(), ::isspace), inputStr.end());

  auto inputJson = json::parse(inputStr, nullptr, false);
  if (inputJson.is_discarded())
    return FAILED_TO_PARSE_CRASHDUMP_TO_JSON;

  auto input = getProperRootNode(inputJson);
  if (!input)
    return INCORRECT_JSON_ROOT_NODE;

  auto summary = prepareSummary(*input);
  if (summary.cpuType == "")
    return CPU_TYPE_NOT_RECOGNIZED;

  if (!standaloneApp)
  {
    if (checkIfMapsAreSymlinkFiles())
    {
      return SYMLINK_FILE;
    }
    if (checkIfMapsAreHardlinkFiles())
    {
      return HARDLINK_FILE;
    }
    PciBdfLookup deviceLookup((char*)"", true, DEVICE_MAP_FILENAME);
    PciBdfLookup silkscreenLookup((char*)"", true, SILKSCREEN_MAP_FILENAME);
    PciBdfLookup PciBdfLookup((char*)"", true, MEMORY_MAP_FILENAME);
    deviceMap = deviceLookup.lookupMap;
    silkscreenMap = silkscreenLookup.lookupMap;
  }

  json reportJson = prepareReport(summary, *input, deviceMap, silkscreenMap);
  if (reportJson.is_discarded())
    return FAILED_TO_PREPARE_REPORT;

  nlohmann::ordered_json report;
  nlohmann::ordered_json& triageReport = report["triage"];
  std::string firstError = "";
  std::stringstream ss;
  std::stringstream ss2;
  std::regex core_regex("([0-9])-([0-9])");
  std::smatch matches;
  bool imc_present = false;
  bool aerCorSection = false;
  bool aerUncorSection = false;
  bool fru_section = false;
  std::string imc_string;
  std::string bankNumber;
  std::string ierr;
  std::string mcerr;
  std::string rmcaMcerr;
  std::vector<std::string> sockets;
  triageReport["BAFI_version"] = BAFI_VERSION;
  triageReport["CPU_type"] = getFullVersionName(*input);
  triageReport["Crashdump_timestamp"] = getTimestamp(*input);

  // Determine on which socket the error occurred first
  std::string firstErrorSocket = findFirstErrorSocket(reportJson, firstError);
  if (firstErrorSocket != "")
  {
    triageReport["First_Error_Occurred_On_Socket"] = firstErrorSocket.substr(6);
    findSocketsFromSummary(sockets, reportJson);
  }
  else
  {
    findSocketsFromSummary(sockets, reportJson);
    if (sockets.size() > 1)
      triageReport["First_Error_Occurred_On_Socket"] = "N/A";
    else
      triageReport["First_Error_Occurred_On_Socket"] = "0";
  }
  // Get descriptive information for each error
  for (const auto& socket : sockets)
  {
    ss.str("");
    ss << "S" << socket.substr(6) << ".";
    for (const auto& [pciKey, pciVal] : reportJson["output_summary"][socket].items())
    {
      if (pciVal.contains("Errors_Per_Socket"))
      {
        for (const auto& [metaKey, metaVal] : pciVal.items())
        {
          for (const auto& [tscKey, tscVal] : metaVal.items())
          {
            firstError =
                determineFirstErrorType(std::string(tscVal), ierr, mcerr, rmcaMcerr, summary.cpuType);
            if (firstErrorSocket == "")
            {
              firstError =
                determineFirstErrorType(std::string(tscVal), ierr, mcerr, rmcaMcerr, summary.cpuType);
              if (sockets.size() == 1 && ierr == "0x0" && mcerr == "0x0" && rmcaMcerr == "0x0")
                triageReport["First_Error_Occurred_On_Socket"] = "N/A";
            }
            else
            {
              getErrorStrings(std::string(tscVal), ierr, mcerr, rmcaMcerr, summary.cpuType);
            }

            std::size_t left = std::string(tscVal).find(firstError);
            if (left > std::string(tscVal).size())
              continue;
            if (firstError == "FirstIERR")
            {
              std::size_t right = std::string(tscVal)
              .substr(left, std::string(tscVal).size()).find(", FirstMCERR");
              if (ierr == "0x0")
              {
                ss << "N/A";
                break;
              }
              ss << "IERR.";
              if (std::string(tscVal).substr(left, right).find("IMC")
                != std::string::npos)
              {
                ss2 << std::string(tscVal).substr(left, right);
                imc_present = true;
                bankNumber = findBankNumber(ss2);
                break;
              }
              ss << ierr;
              bankNumber = findBankNumber(ss);
              break;
            }
            else
            {
              if (mcerr == "0x0" && firstError == "RMCA_FirstMCERR" && rmcaMcerr != "0x0")
              {
                mcerr = rmcaMcerr;
              }
              if (mcerr == "0x0")
              {
                ss << "N/A";
                break;
              }

              if (std::string(mcerr).find("manual review may be required") !=
                  std::string::npos)
              {
                ss << "FirstMCERR != MCE bank#, manual review required";
                break;
              }
              ss << "MCERR.";
              if (std::string(tscVal).substr(left,
                std::string(tscVal).size()).find("IMC") != std::string::npos)
              {
                ss2 << std::string(tscVal).substr(left,
                                                  std::string(tscVal).size());
                imc_present = true;
                bankNumber = findBankNumber(ss2);
                break;
              }
              ss << mcerr;
              bankNumber = findBankNumber(ss);
              if (bankNumber == "" && (mcerr.find("CHA") != std::string::npos ||
                mcerr.find("M2MEM") != std::string::npos))
              {
                bankNumber = mcerr;
              }
              break;
            }
          }
        }
      }
    }

    for (const auto& [pciKey, pciVal] : reportJson["output_summary"][socket].items())
    {
      if (pciVal.contains("MCE"))
      {
        for (const auto& [metaKey, metaVal] : pciVal.items())
        {
          for (const auto& [tscKey, tscVal] : metaVal.items())
          {
            if (bankNumber == "9" || bankNumber == "10" || bankNumber == "11")
            {
              if (std::string(tscKey).find("bank " + bankNumber) !=
                  std::string::npos)
              {
                std::size_t left = std::string(tscVal).find("RAW ");
                if (left > std::string(tscVal).size())
                  continue;
                std::size_t right =
                  std::string(tscVal).find(std::string(tscVal).size());
                bankNumber = std::string(tscVal).substr(left + 4, left-right);
              }
            }
            std::string core_string = ss.str();
            if (std::regex_search(core_string, matches, core_regex))
            {
              for (int i = std::stoi(matches[1].str());
                i <= std::stoi(matches[2].str()); i++)
              {
                if (std::string(tscVal).find("bank " + std::to_string(i)) !=
                  std::string::npos)
                {
                  bankNumber = std::to_string(i);
                  break;
                }
              }
            }
            if (imc_present)
            {
              if (std::string(tscKey).find("bank " + bankNumber)
                != std::string::npos)
              {
                std::size_t left = std::string(tscKey).find("bank");
                if (left > std::string(tscKey).size())
                  continue;
                std::size_t right = std::string(tscKey).find(")");
                imc_string = std::string(tscKey).substr(left, right + 1);
                ss << imc_string;
                bankNumber = findBankNumber(ss);
                break;
              }
              else
              {
                std::stringstream ss3;
                ss3 << ss2.str().erase(ss2.str().find("bank"), 3);
                ss2.str("");
                ss2 << ss3.str();
                bankNumber = findBankNumber(ss2);
              }
            }

            if (std::string(tscKey).find("IMC") != std::string::npos)
            {
              triageReport[socket]["IMC_Error"].push_back(tscKey);
            }
            if (std::string(tscKey).find("HBM") != std::string::npos)
            {
              triageReport[socket]["HBM_Error"].push_back(tscKey);
            }
            if (std::string(tscKey).find("UPI") != std::string::npos)
            {
              triageReport[socket]["UPI_Error"].push_back(tscKey);
            }
          }
        }
      }
    }

    if (!triageReport[socket].contains("IMC_Error"))
    {
      triageReport[socket]["IMC_Error"] = json::object();
    }
    if (!triageReport[socket].contains("HBM_Error"))
    {
      triageReport[socket]["HBM_Error"] = json::object();
    }
    if (!triageReport[socket].contains("UPI_Error"))
    {
      triageReport[socket]["UPI_Error"] = json::object();
    }
    // Get MSCOD and MCACOD
    std::string mscod;
    std::string mcacod;
    std::string msec_fw = "";
    for (const auto& [pciKey, pciVal] : reportJson["MCA"][socket].items())
    {
      if (summary.cpuType == "SPR")
      {
        if (bankNumber == "12")
        {
          bankNumber = "M2MEM";
        }
        else if (bankNumber == "5")
        {
          bankNumber = "UPI";
        }
      }
      if (std::string(pciVal["Bank"]).rfind(bankNumber, 0) != 0 || bankNumber == "")
      {
        continue;
      }
      if (bankNumber == "4")
      {
        if (summary.cpuType == "ICX")
        {
          mscod = "";
          mcacod = pciVal["Status_decoded"]["MCACOD"];
          msec_fw = pciVal["Status_decoded"]["MSEC_FW"];
          break;
        }
      }
      mscod = std::string(pciVal["Status"]).substr(10, 4);
      mcacod = std::string(pciVal["Status"]).substr(14, 4);
      mscod.erase(0, mscod.find_first_not_of('0'));
      mcacod.erase(0, mcacod.find_first_not_of('0'));
      if (mcacod == "")
        break;
      if (mscod == "")
      {
        mscod = "00";
      }
      mscod = "0x" + mscod;
      mcacod = "0x" + mcacod;
      break;
    }

    if (mcacod != "")
      ss << " MCACOD: " << mcacod;
    if (mscod != "")
      ss << " MSCOD: " << mscod;
    if (msec_fw != "")
      ss << " MSEC_FW: " << msec_fw;

    nlohmann::ordered_json error_info = ss.str();
    triageReport[socket]["Error_Information"].push_back(error_info);

    // Get FRU section in case IMC bank is present in errors
    std::stringstream ss_fru;
    if (imc_present)
    {
      std::size_t left = std::string(imc_string).find("bank");
      if (left <= std::string(imc_string).size())
      {
        std::size_t right = std::string(imc_string).find(")");
        ss_fru << std::string(imc_string).substr(left + 8, right + 1);
        nlohmann::ordered_json fru_info = ss_fru.str();
        triageReport[socket]["FRU"].push_back(fru_info);
        fru_section = true;
      }
    }

    // Get FRU section for all other banks
    for (const auto& [pciKey, pciVal] : reportJson["output_summary"][socket].items())
    {
      if (pciVal.contains("First_MCERR"))
      {
        auto bdfObj = nlohmann::ordered_json::object();
        bdfObj["Address"] = pciVal["First_MCERR"]["Address"];
        std::string bdf_string = pciVal["First_MCERR"]["BDF"];
        if (bdf_string == "Please refer to system address map")
        {
          bdfObj["Bus"] = bdf_string;
          bdfObj["Device"] = bdf_string;
          bdfObj["Function"] = bdf_string;
          triageReport[socket]["FRU"].push_back(bdfObj);
        }
        else
        {
          getBdfFromFirstMcerrSection(bdf_string, bdfObj);
          showBdfDescription(deviceMap, bdfObj);
          triageReport[socket]["FRU"].push_back(bdfObj);
        }
        fru_section = true;
      }
      if (pciVal.contains("IO_Errors"))
      {
        auto bdfObj = nlohmann::ordered_json::object();
        std::string bdf_string = pciVal["IO_Errors"]["Device"];
        getBdfFromIoErrorsSection(bdf_string, bdfObj);
        if (pciVal["IO_Errors"].contains("Description"))
        {
          bdfObj["Description"] = pciVal["IO_Errors"]["Description"];
        }
        triageReport[socket]["FRU"].push_back(bdfObj);
        fru_section = true;
      }
    }
    if (!fru_section)
    {
      triageReport[socket]["FRU"] = json::object();
    }
    fru_section = false;

    // Get PPIN
    if (input->get().find("METADATA") != input->get().cend())
    {
      if (!checkInproperValue(input->get()["METADATA"]
      ["cpu" + socket.substr(6)]["ppin"]) && input->get()["METADATA"]
      ["cpu" + socket.substr(6)]["ppin"] != "dummy")
      {
        triageReport[socket]["PPIN"] = input->get()["METADATA"]
        ["cpu" + socket.substr(6)]["ppin"];
      }
      else
      {
        triageReport[socket]["PPIN"] = "N/A";
      }
    }
    else
    {
      triageReport[socket]["PPIN"] = json::object();
    }

    // Get ucode_patch_ver
    if (input->get().find("METADATA") != input->get().cend())
    {
      if (!checkInproperValue(input->get()["METADATA"]
      ["cpu" + socket.substr(6)]["ucode_patch_ver"]) && input->get()["METADATA"]
      ["cpu" + socket.substr(6)]["ucode_patch_ver"] != "dummy")
      {
        triageReport[socket]["Ucode_Patch_Ver"] = input->get()["METADATA"]
        ["cpu" + socket.substr(6)]["ucode_patch_ver"];
      }
      else
      {
        triageReport[socket]["Ucode_Patch_Ver"] = "N/A";
      }
    }
    else
    {
      triageReport[socket]["Ucode_Patch_Ver"] = json::object();
    }
  }

  // Get AER sections
  sockets.clear();
  findSocketsFromSummary(sockets, reportJson);
  for (const auto& socket : sockets)
  {
    getMemoryErrors(reportJson, socket, triageReport);
    for (const auto& [pciKey, pciVal] : reportJson["output_summary"][socket].items())
    {
      if (pciVal.contains("PCIe_AER_Uncorrectable_errors"))
      {
        triageReport[socket]["PcieAerUncorrectable"]
        = pciVal["PCIe_AER_Uncorrectable_errors"];
        aerUncorSection = true;
      }
      if (pciVal.contains("PCIe_AER_Correctable_errors"))
      {
        triageReport[socket]["PcieAerCorrectable"]
        = pciVal["PCIe_AER_Correctable_errors"];
        aerCorSection = true;
      }
    }

    if (!aerCorSection)
    {
      triageReport[socket]["PcieAerCorrectable"] = json::object();
    }
    if (!aerUncorSection)
    {
      triageReport[socket]["PcieAerUncorrectable"] = json::object();
    }

    getThermalErrors(socket, triageReport, summary);
    aerUncorSection = false;
    aerCorSection = false;
  }

  // Assign ErrorTriage to char pointer
  std::string triageStr = report.dump(4);
  std::size_t triageInfoSizeTemp = strlen(triageStr.c_str());
  *triageInfoSize = triageInfoSizeTemp;
  triageInfo = new char[*triageInfoSize + 1];
  strncpy(triageInfo, triageStr.c_str(), *triageInfoSize + 1);

  return SUCCESS;
}
