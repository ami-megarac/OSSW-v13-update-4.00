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

#include "nvd.h"

#include "smbus_peci.h"
#include "utils.h"

static acdStatus readCSRUsingInputFile(cJSON** regList,
                                       const CPUInfo* const cpuInfo,
                                       const uint8_t dimm, cJSON* jsonChild,
                                       bool* const enable)
{
    *regList =
        getNVDSectionRegList(cpuInfo->inputFile.bufferPtr, "csr", enable);
    if (*regList == NULL)
    {
        if (cJSON_GetObjectItemCaseSensitive(jsonChild, NVD_FILE_CSR_KEY) ==
            NULL)
        {
            cJSON_AddStringToObject(jsonChild, NVD_FILE_CSR_KEY,
                                    NVD_FILE_CSR_ERR);
        }
        return ACD_INVALID_OBJECT;
    }

    if (*enable == false)
    {
        if (cJSON_GetObjectItemCaseSensitive(jsonChild, RECORD_ENABLE) == NULL)
        {
            cJSON_AddFalseToObject(jsonChild, RECORD_ENABLE);
        }
        return ACD_SECTION_DISABLE;
    }

    cJSON* itRegs = NULL;
    cJSON* itParams = NULL;
    uint16_t count = 0;
    nvdCSR reg = {0};

    cJSON_ArrayForEach(itRegs, *regList)
    {
        int position = 0;
        count++;
        cJSON_ArrayForEach(itParams, itRegs)
        {
            switch (position)
            {
                case NVD_CSR_NAME:
                    reg.name = itParams->valuestring;
                    break;
                case NVD_CSR_DEV:
                    reg.dev = itParams->valueint;
                    break;
                case NVD_CSR_FUNC:
                    reg.func = itParams->valueint;
                    break;
                case NVD_CSR_OFFSET:
                    reg.offset = strtoull(itParams->valuestring, NULL, 16);
                    break;
                default:
                    break;
            }
            position++;
        }
        uint32_t val = 0;
        PECIStatus status;
        char jsonStr[NVD_JSON_STRING_LEN];
        SmBusCsrEntry csr = {
            .dev = reg.dev, .func = reg.func, .offset = reg.offset};
        status = csrRd(cpuInfo->clientAddr, dimm, csr, &val, NULL, false);

        if (status != PECI_SUCCESS)
        {
            cd_snprintf_s(jsonStr, NVD_JSON_STRING_LEN, NVD_RC, status);
        }
        else
        {
            cd_snprintf_s(jsonStr, NVD_JSON_STRING_LEN, NVD_UINT32_FMT, val);
        }
        cJSON_AddStringToObject(jsonChild, reg.name, jsonStr);
    }

    return ACD_SUCCESS;
}

acdStatus logCSRSection(const CPUInfo* const cpuInfo, const uint8_t dimm,
                        cJSON* jsonChild)
{
    cJSON* regList = NULL;
    cJSON* csrSection = NULL;
    bool enable = true;

    CRASHDUMP_PRINT(INFO, stderr, "Logging %s on PECI address %d\n", "NVD CSR",
                    cpuInfo->clientAddr);

    cJSON_AddItemToObject(jsonChild, "csr", csrSection = cJSON_CreateObject());

    return readCSRUsingInputFile(&regList, cpuInfo, dimm, csrSection, &enable);
}

acdStatus fillNVDSection(const CPUInfo* const cpuInfo, const uint8_t cpuNum,
                         cJSON* jsonChild)
{
    acdStatus status = ACD_SUCCESS;
    for (uint32_t dimm = 0; dimm < NVD_MAX_DIMM; dimm++)
    {
        if (!CHECK_BIT(cpuInfo->dimmMask, dimm))
        {
            continue;
        }
        cJSON* dimmSection = NULL;
        char jsonStr[NVD_JSON_STRING_LEN];
        cd_snprintf_s(jsonStr, sizeof(jsonStr), dimmMap[dimm], cpuNum);
        cJSON_AddItemToObject(jsonChild, jsonStr,
                              dimmSection = cJSON_CreateObject());
        status = logCSRSection(cpuInfo, dimm, dimmSection);
    }
    return status;
}
