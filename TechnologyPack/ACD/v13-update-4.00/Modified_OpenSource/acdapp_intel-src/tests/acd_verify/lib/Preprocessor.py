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

from lib.Metadata import Metadata
from lib.Tor import Tor
from lib.Uncore import Uncore
from lib.Mca import Mca
from lib.PM_info import PM_info
from lib.Address_map import Address_map
from lib.Big_core import Big_core


class Preprocessor():
    def __init__(self, outputData):
        self.sections = {
            "metadata": Metadata(outputData)
        }

        cpus = self.getCPUs(outputData)

        for cpu in cpus:
            if "address_map" in cpus[cpu]:
                self.sections[cpu] = {
                    "tor": Tor(cpus[cpu]),
                    "uncore": Uncore(cpus[cpu]),
                    "mca": Mca(cpus[cpu]),
                    "pm_info": PM_info(cpus[cpu]),
                    "address_map": Address_map(cpus[cpu]),
                    "big_core": Big_core(cpus[cpu])
                }
            else:
                self.sections[cpu] = {
                    "tor": Tor(cpus[cpu]),
                    "uncore": Uncore(cpus[cpu]),
                    "mca": Mca(cpus[cpu]),
                    "pm_info": PM_info(cpus[cpu]),
                    "big_core": Big_core(cpus[cpu])
            }

    def getCPUs(self, outputData):
        cpus = {}
        for key in outputData["PROCESSORS"]:
            if "cpu" in key:
                cpus[key] = outputData["PROCESSORS"][key]

        return cpus
