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

#ifndef NVD_H
#define NVD_H

#include <time.h>

#include "crashdump.h"

#define NVD_JSON_STRING_LEN 32
#define NVD_UINT32_FMT "0x%" PRIx32 ""
#define NVD_RC "RC:0x%x"
#define NVD_FILE_CSR_KEY "_input_file_nvd_csr"
#define NVD_FILE_CSR_ERR "Error parsing nvd csr section"
#define NVD_MAX_DIMM 16
#define NVD_DIMM_MAP_STR_LEN 10

typedef struct
{
    char* name;
    uint8_t dev;
    uint8_t func;
    uint16_t offset;
} nvdCSR;

enum NVD_CSR
{
    NVD_CSR_NAME,
    NVD_CSR_DEV,
    NVD_CSR_FUNC,
    NVD_CSR_OFFSET,
};

acdStatus fillNVDSection(const CPUInfo* const cpuInfo, const uint8_t cpuNum,
                         cJSON* jsonChild);

const char dimmMap[NVD_MAX_DIMM][NVD_DIMM_MAP_STR_LEN] = {
    // cpu, imc, channel, slot
    "dimm%x000", "dimm%x001", "dimm%x010", "dimm%x011",
    "dimm%x100", "dimm%x101", "dimm%x110", "dimm%x111",
    "dimm%x200", "dimm%x201", "dimm%x210", "dimm%x211",
    "dimm%x300", "dimm%x301", "dimm%x310", "dimm%x311",
};

#endif // NVD_H
