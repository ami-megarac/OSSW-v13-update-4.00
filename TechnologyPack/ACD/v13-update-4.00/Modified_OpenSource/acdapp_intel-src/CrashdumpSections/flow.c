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

#include "flow.h"

void ProcessPECICmds(ENTRY* entry, CPUInfo* cpuInfo, cJSON* peciCmds,
                     CmdInOut* cmdInOut, InputParserErrInfo* errInfo,
                     LoggerStruct* loggerStruct, cJSON* outRoot,
                     RunTimeInfo* runTimeInfo)
{
    cJSON* cmdGroup = NULL;

    cJSON_ArrayForEach(cmdGroup, peciCmds)
    {
        entry->key = cmdGroup->child->string;
        errInfo->cmdGroup = entry->key;
        errInfo->cmdGroupPos = 0;

        executionStatus sectionExecStatus, globalExecStatus;
        cJSON* paramsGroup =
            cJSON_GetObjectItemCaseSensitive(cmdGroup, cmdGroup->child->string);

        cJSON* params = NULL;
        cJSON_ArrayForEach(params, paramsGroup)
        {
            int repeats = 1;

            loggerStruct->nameProcessing.logRegister = true;
          //  printf("ProcessPECICmds 3 \n");//test line
            cmdInOut->in.params =
                cJSON_GetObjectItemCaseSensitive(params, "Params");
//printf("ProcessPECICmds 4 %08x\n",cmdInOut->in.params);//test line
            cmdInOut->in.outputPath =
                cJSON_GetObjectItemCaseSensitive(params, "Output");
//printf("ProcessPECICmds 5 %08x\n",cmdInOut->in.outputPath);//test line
            ParseNameSection(cmdInOut, loggerStruct);
          //  printf("ProcessPECICmds 51\n");//test line
            cmdInOut->in.repeats =
                cJSON_GetObjectItemCaseSensitive(params, "Repeat");
           //  printf("ProcessPECICmds52 \n");//test line
            if (cmdInOut->in.repeats != NULL)
            {
                repeats = cmdInOut->in.repeats->valueint;
            }
            cmdInOut->internalVarName = NULL;
          //  printf("ProcessPECICmds 53\n");//test line
            cJSON* var =
                cJSON_GetObjectItemCaseSensitive(params, "InternalVar");
           // printf("ProcessPECICmds 54\n");//test line
            if (var != NULL)
            {
                cmdInOut->internalVarName = var->valuestring;
            }
          //  printf("ProcessPECICmds 55\n");//test line
            cmdInOut->out.ret = PECI_CC_INVALID_REQ;
            cmdInOut->paramsTracker = cJSON_CreateObject();
          //  printf("ProcessPECICmds 56\n");//test line
            UpdateParams(cpuInfo, cmdInOut, loggerStruct, errInfo);
//printf("ProcessPECICmds 57\n");//test line
            for (int n = 1; n <= repeats; n++)
            {
               // printf("ProcessPECICmds 6 \n");//test line
                sectionExecStatus = checkMaxTimeElapsed(
                    runTimeInfo->maxSectionTime, runTimeInfo->sectionRunTime);
              //  printf("ProcessPECICmds 7\n");//test line
                globalExecStatus = checkMaxTimeElapsed(
                    runTimeInfo->maxGlobalTime, runTimeInfo->globalRunTime);
              //  printf("ProcessPECICmds 8\n");//test line
                cmdInOut->runTimeInfo = runTimeInfo;
                if (globalExecStatus == EXECUTION_ABORTED)
                {
                    loggerStruct->contextLogger.skipFlag = true;
                    cmdInOut->runTimeInfo->maxGlobalRunTimeReached = true;
                }
               // printf("ProcessPECICmds9 \n");//test line
                if (sectionExecStatus == EXECUTION_ABORTED)
                {
                    loggerStruct->contextLogger.skipFlag = true;
                    cmdInOut->runTimeInfo->maxSectionRunTimeReached = true;
                }
             //   printf("ProcessPECICmds 10\n");//test line
                if (!loggerStruct->contextLogger.skipFlag)
                {
                    cmdInOut->out.printString = false;
                    Execute(entry, cmdInOut);
                }
             //   printf("ProcessPECICmds11 \n");//test line
                if (loggerStruct->nameProcessing.logRegister)
                {
                    loggerStruct->contextLogger.repeats = n;
                    Logger(cmdInOut, outRoot, loggerStruct);
                }
///   printf("ProcessPECICmds12 \n");//test line
                if (cmdInOut->out.ret != PECI_CC_SUCCESS ||
                    PECI_CC_UA(cmdInOut->out.cc))
                {
                    if (loggerStruct->contextLogger.skipOnFailFromInputFile)
                    {
                        loggerStruct->contextLogger.skipFlag = true;
                    }
                }
            }
           // printf("ProcessPECICmds 13\n");//test line
            ResetParams(cmdInOut->in.params, cmdInOut->paramsTracker);
            cJSON_Delete(cmdInOut->paramsTracker);
            errInfo->cmdGroupPos++;
        }
    }
//printf("ProcessPECICmds 99\n");//test line
}

acdStatus fillNewSection(cJSON* root, CPUInfo* cpuInfo, uint8_t cpu,
                         RunTimeInfo* runTimeInfo, uint8_t sectionIndex)
{
    bool validateInput = readInputFileFlag(cpuInfo[0].inputFile.bufferPtr, true,
                                           VALIDATE_ENABLE_KEY);
    cJSON* sections = cJSON_GetObjectItemCaseSensitive(
        cpuInfo[0].inputFile.bufferPtr, "Sections");
    cJSON* section = NULL;
    runTimeInfo->maxGlobalRunTimeReached = false;
    section = cJSON_GetArrayItem(sections, sectionIndex);

    if (section == NULL)
    {
        return ACD_SUCCESS;
    }
    ENTRY entry;
    LoggerStruct loggerStruct;
    BuildCmdsTable(&entry);
    uint8_t threadsPerCore = 1;
    runTimeInfo->maxSectionTime = 0xFFFFFFFF;
    runTimeInfo->maxSectionRunTimeReached = false;
    runTimeInfo->sectionMaxPrint = true;
    runTimeInfo->globalMaxPrint = true;
    loggerStruct.contextLogger.skipOnFailFromInputFile = false;
    InputParserErrInfo errInfo = {};
    CmdInOut cmdInOut;
    ValidatorParams validParams;
    validParams.validateInput = validateInput;
    cmdInOut.validatorParams = &validParams;
    LoopOnFlags loopOnFlags;
    cmdInOut.cpuInfo = cpuInfo;
    cmdInOut.root = root;
    cmdInOut.logger = &loggerStruct;
    cmdInOut.out.ret = PECI_CC_INVALID_REQ;
    CmdInOut preReqCmdInOut;
    preReqCmdInOut.out.ret = PECI_CC_INVALID_REQ;

    cJSON* sectionEnable =
        cJSON_GetObjectItemCaseSensitive(section->child, "RecordEnable");

    if (sectionEnable != NULL)
    {
        if (!cJSON_IsTrue(sectionEnable))
        {
            if (GetPath(section->child, &loggerStruct) == ACD_SUCCESS)
            {
                loggerStruct.contextLogger.cpu = cpu;
                ParsePath(&loggerStruct);
                logRecordDisabled(&cmdInOut, root, &loggerStruct);
            }

            return ACD_SUCCESS;
        }
    }
    else
    {
printf("section en %08x \n",sectionEnable);//test line
        return ACD_SUCCESS;
    }

    ReadLoops(section->child, &loopOnFlags);

    GenerateVersion(section->child, &loggerStruct.contextLogger.version);

    GetPath(section->child, &loggerStruct);

    ParsePath(&loggerStruct);

    cJSON* maxTimeJson =
        cJSON_GetObjectItemCaseSensitive(section->child, "MaxTimeSec");

    if (maxTimeJson != NULL)
    {
        runTimeInfo->maxSectionTime = maxTimeJson->valueint;
    }
    cJSON* skipOnErrorJson =
        cJSON_GetObjectItemCaseSensitive(section->child, "SkipOnFail");

    if (skipOnErrorJson != NULL)
    {
        loggerStruct.contextLogger.skipOnFailFromInputFile =
            cJSON_IsTrue(skipOnErrorJson);
    }
    loggerStruct.contextLogger.skipCrashCores = false;
    cJSON* skipCrashedCores =
        cJSON_GetObjectItemCaseSensitive(section->child, "SkipCrashedCores");

    if (skipCrashedCores != NULL)
    {
        loggerStruct.contextLogger.skipCrashCores =
            cJSON_IsTrue(skipCrashedCores);
    }
    cJSON* preReq = cJSON_GetObjectItemCaseSensitive(
        cJSON_GetObjectItemCaseSensitive(section->child, "PreReq"), "PECICmds");

    cJSON* peciCmds =
        cJSON_GetObjectItemCaseSensitive(section->child, "PECICmds");

    //AMI
    if(strcmp(section->child->string, "BigCore") == 0)
    {
        return ACD_SUCCESS;
    }
    else
    {
        CRASHDUMP_PRINT(ERR, stderr, "Logging %s on PECI address %d\n",
                    section->child->string, cpuInfo->clientAddr);
    }
    loggerStruct.contextLogger.currentSectionName = section->child->string;
    loggerStruct.contextLogger.cpu = cpu;
    logVersion(&cmdInOut, root, &loggerStruct);
    loggerStruct.contextLogger.skipFlag = false;
    preReqCmdInOut.internalVarsTracker = cJSON_CreateObject();
//printf("fillNewSection 1 \n");//test line
    ProcessPECICmds(&entry, cpuInfo, preReq, &preReqCmdInOut, &errInfo,
                    &loggerStruct, root, runTimeInfo);

    cmdInOut.internalVarsTracker = preReqCmdInOut.internalVarsTracker;
    if (loopOnFlags.loopOnCHA)
    {
        for (size_t cha = 0; cha < cpuInfo->chaCount; cha++)
        {
           // printf("fillNewSection 3 \n");//test line
            loggerStruct.contextLogger.cha = (uint8_t)cha;
            ProcessPECICmds(&entry, cpuInfo, peciCmds, &cmdInOut, &errInfo,
                            &loggerStruct, root, runTimeInfo);
           
            loggerStruct.contextLogger.skipFlag = false;
        }
    }
    else if (loopOnFlags.loopOnCore)
    {
        for (uint8_t u8CoreNum = 0; u8CoreNum < MAX_CORE_MASK; u8CoreNum++)
        {

            if (!CHECK_BIT(cpuInfo->coreMask, u8CoreNum))
            {
                continue;
            }

            if (loggerStruct.contextLogger.skipCrashCores &&
                CHECK_BIT(cpuInfo->crashedCoreMask, u8CoreNum))
            {
                continue;
            }

            loggerStruct.contextLogger.core = u8CoreNum;
            if (loopOnFlags.loopOnThread)
            {
                threadsPerCore = 2;
            }
            for (uint8_t threadNum = 0; threadNum < threadsPerCore; threadNum++)
            {

                loggerStruct.contextLogger.thread = threadNum;
              //  printf("fillNewSection 7 \n");//test line
                ProcessPECICmds(&entry, cpuInfo, peciCmds, &cmdInOut, &errInfo,
                                &loggerStruct, root, runTimeInfo);
                //printf("fillNewSection 8 \n");//test line
            }
            loggerStruct.contextLogger.skipFlag = false;
        }
    }
    else
    {
//printf("fillNewSection 10 \n");//test line
        ProcessPECICmds(&entry, cpuInfo, peciCmds, &cmdInOut, &errInfo,
                        &loggerStruct, root, runTimeInfo);
      // printf("fillNewSection 11 \n");//test line
        loggerStruct.contextLogger.skipFlag = false;
    }
    loggerStruct.nameProcessing.extraLevel = false;
//printf("fillNewSection 11-1 \n");//test line
    if (GenerateJsonPath(&cmdInOut, root, &loggerStruct, true) == ACD_SUCCESS)
    {
     //   printf("fillNewSection 12 \n");//test line
        logSectionRunTime(loggerStruct.nameProcessing.jsonOutput,
                          &runTimeInfo->sectionRunTime, TIME_KEY);
     //   printf("fillNewSection 13 \n");//test line
    }

    cJSON_Delete(cmdInOut.internalVarsTracker);

    hdestroy();

    return ACD_SUCCESS;
}
