###############################################################################
# INTEL CONFIDENTIAL                                                          #
#                                                                             #
# Copyright 2021 Intel Corporation.                                           #
#                                                                             #
# This software and the related documents are Intel copyrighted materials,    #
# and your use of them is governed by the express license under which they    #
# were provided to you ("License"). Unless the License provides otherwise,    #
# you may not use, modify, copy, publish, distribute, disclose or transmit    #
# this software or the related documents without Intel's prior written        #
# permission.                                                                 #
#                                                                             #
# This software and the related documents are provided as is, with no express #
# or implied warranties, other than those that are expressly stated in the    #
# License.                                                                    #
###############################################################################

from lib.Section import Section

import warnings


class Metadata(Section):
    def __init__(self, jOutput):
        Section.__init__(self, jOutput, "METADATA")

        self.cpus = []
        self.verifySection()
        self.rootNodes = f"sockets: {len(self.cpus)}"

    def verifyCpuID(self):
        # Obtain all cpuIDs
        cpuIDs = [self.sectionInfo[cpu].get('cpuid') for cpu in self.cpus]
        # Eliminate duplicates
        cpuIDs = list(dict.fromkeys(cpuIDs))
        if len(cpuIDs) > 1:   # More than 1 cpuID found
            tmp = {}
            for cpu in self.cpus:
                tmp[cpu] = self.sectionInfo[cpu]["cpuid"]
            return tmp
        return cpuIDs

    def getSummaryInfo(self):
        summaryInfo = {}
        summaryInfo["nCPUs"] = len(self.cpus)
        summaryInfo["IDs"] = self.verifyCpuID()
        summaryInfo["totalTime"] = self.sectionInfo["_total_time"]
        summaryInfo["triggerType"] = self.sectionInfo["trigger_type"]
        summaryInfo["crashcoreCounts"] = self.getCrashcoreCounts()

        return summaryInfo

    def getCrashcoreCounts(self):
        tmp = {}
        for cpu in self.cpus:
            tmp[cpu] = self.sectionInfo[cpu]["crashcore_count"]
        return tmp

    def verifySection(self):
        for key in self.sectionInfo:
            regValueNotStr = (type(self.sectionInfo[key]) != str)
            underScoreReg = key.startswith('_')

            if key.startswith("cpu"):
                self.cpus.append(key)
            # Considering cpuXXs as the valid sections
            #  for the METADATA region of the output file
            elif not underScoreReg and regValueNotStr:
                warnings.warn(
                    f"Section {key}, not expected in {self.sectionName}")
            self.search(key, self.sectionInfo[key])
        if len(self.cpus) == 0:
            warnings.warn("No cpus found in METADATA")
