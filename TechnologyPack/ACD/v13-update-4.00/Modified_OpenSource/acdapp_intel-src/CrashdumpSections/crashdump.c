/******************************************************************************
 *
 * INTEL CONFIDENTIAL
 *
 * Copyright 2019 Intel Corporation.
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

#include "crashdump.h"

#include "CrashdumpSections/AddressMap.h"
#include "CrashdumpSections/BigCore.h"
#include "CrashdumpSections/CoreMca.h"
#include "CrashdumpSections/PowerManagement.h"
#include "CrashdumpSections/TorDump.h"
#include "CrashdumpSections/Uncore.h"
#include "CrashdumpSections/UncoreMca.h"
#include "CrashdumpSections/Crashlog.h"
#include "utils.h"

const CrashdumpSection sectionNames_old[NUMBER_OF_SECTIONS] = {
    {"MCA", MCA_UNCORE, RECORD_TYPE_MCALOG, NULL},
    {"uncore", UNCORE, RECORD_TYPE_UNCORESTATUSLOG, NULL},
    {"TOR", TOR, RECORD_TYPE_TORDUMP, NULL},
    {"PM_info", PM_INFO, RECORD_TYPE_PMINFO, NULL},
    {"address_map", ADDRESS_MAP, RECORD_TYPE_ADDRESSMAP, NULL},
    {"big_core", BIG_CORE, RECORD_TYPE_CORECRASHLOG, logCrashdump},
    {"MCA", MCA_CORE, RECORD_TYPE_MCALOG, NULL},
    {"METADATA", METADATA, RECORD_TYPE_METADATA}};

const CrashdumpSection sectionNames[NUMBER_OF_SECTIONS] = {
    {"MCA", MCA_UNCORE, RECORD_TYPE_MCALOG, NULL},
    {"uncore", UNCORE, RECORD_TYPE_UNCORESTATUSLOG, NULL},
    {"TOR", TOR, RECORD_TYPE_TORDUMP, NULL},
    {"PM_info", PM_INFO, RECORD_TYPE_PMINFO, NULL},
    {"address_map", ADDRESS_MAP, RECORD_TYPE_ADDRESSMAP, NULL},
#ifdef OEMDATA_SECTION
    {"OEM", OEM, RECORD_TYPE_OEM, NULL},
#endif
    {"big_core", BIG_CORE, RECORD_TYPE_CORECRASHLOG, NULL},
    {"MCA", MCA_CORE, RECORD_TYPE_MCALOG, NULL},
    {"METADATA", METADATA, RECORD_TYPE_METADATA},
    {"crashlog", CRASHLOG, RECORD_TYPE_METADATA, NULL}};
int revision_uncore = 0;
bool commonMetaDataEnabled = true;

void logCrashdumpVersion(cJSON* parent, CPUInfo* cpuInfo, int recordType)
{
    VersionInfo cpuProduct[cd_numberOfModels] = {
        {cd_icx, PRODUCT_TYPE_ICXSP},
        {cd_icx2, PRODUCT_TYPE_ICXSP},
        {cd_spr, PRODUCT_TYPE_SPR},
        {cd_icxd, PRODUCT_TYPE_ICXDSP},
    };

    VersionInfo cpuRevision[cd_numberOfModels] = {
        {cd_icx, REVISION_1},
        {cd_icx2, REVISION_1},
        {cd_spr, REVISION_1},
        {cd_icxd, REVISION_1},
    };

    int productType = 0;
    for (int i = 0; i < cd_numberOfModels; i++)
    {
        if (cpuInfo->model == cpuProduct[i].cpuModel)
        {
            productType = cpuProduct[i].data;
        }
    }

    int revisionNum = 0;
    for (int i = 0; i < cd_numberOfModels; i++)
    {
        if (cpuInfo->model == cpuRevision[i].cpuModel)
        {
            revisionNum = cpuRevision[i].data;
        }
    }

    if (recordType == RECORD_TYPE_UNCORESTATUSLOG)
    {
        revisionNum = revision_uncore;
    }

    // Build the version number:
    //  [31:30] Reserved
    //  [29:24] Crash Record Type
    //  [23:12] Product
    //  [11:8] Reserved
    //  [7:0] Revision
    int version = recordType << RECORD_TYPE_OFFSET |
                  productType << PRODUCT_TYPE_OFFSET |
                  revisionNum << REVISION_OFFSET;
    char versionString[64];
    cd_snprintf_s(versionString, sizeof(versionString), "0x%x", version);
    cJSON_AddStringToObject(parent, "_version", versionString);
}
