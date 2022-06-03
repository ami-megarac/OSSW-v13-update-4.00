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

import collections

from lib.Error import Error


class Section():
    def __init__(self, jOutput, sectionName):
        self.sectionName = sectionName

        self.sectionInfo = jOutput[sectionName]
        self.expectedOutputStruct = None
        self.eHandler = Error()
        self.rootNodes = 0
        self.nRegs = 0

    def search(self, key, value):
        if type(value) == dict:
            for vKey in value:
                self.search(f"{key}.{vKey}", value[vKey])
        elif (type(value) == str) and not value.startswith('_'):
            self.nRegs += 1  # count regs
            if self.eHandler.isError(value):
                error = self.eHandler.extractError(value)
                self.eHandler.errors[key] = error

    def verifySection(self):
        for key in self.sectionInfo:
            self.search(key, self.sectionInfo[key])

    def getTableInfo(self):
        tableInfo = {
            "Section": self.sectionName,
            "rootNodes": self.rootNodes,
            "regs": self.nRegs
        }

        level = dict(collections.Counter(self.eHandler.errors.values()))
        for error in level:
            if error not in tableInfo:
                tableInfo[error] = level[error]
            elif error in tableInfo:
                tableInfo[error] = tableInfo[error] + level[error]

        return tableInfo

    def getErrorList(self):
        return self.eHandler.errors
