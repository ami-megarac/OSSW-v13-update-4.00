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

from lib.Preprocessor import Preprocessor
from lib.Processor import Processor
from lib.Postprocessor import Postprocessor

import json
import os
import argparse


def main():
    try:
        parser = argparse.ArgumentParser()
        parser.add_argument("--verbose",
                            action = 'store_true',
                            help = "Verbose details of report")
        parser.add_argument("--path",
                            type = str,
                            help = "Path to where the json output file(s) are located")
        args = parser.parse_args()

        rootPath = ""

        if args.path:
            rootPath = args.path
            print("Path given: ", rootPath)
        if not args.path:
            rootPath = os.getcwd()

        # Path given is a json file
        if os.path.isfile(rootPath) and rootPath.endswith('.json'):
            print("Processing file: ", rootPath)
            f = open(rootPath)
            jOutput = json.load(f)
            acdData = jOutput["crash_data"]

            preprocessor = Preprocessor(acdData)
            processor = Processor(preprocessor.sections)
            postprocessor = Postprocessor(
                processor.report, rootPath,
                "txt", args.verbose)
            postprocessor.saveFile()
        # Path given is a directory
        elif os.path.isdir(rootPath):
            jsonFiles = [f for f in os.listdir(rootPath) if f.endswith('.json')]

            for jsonFile in jsonFiles:
                print("Processing file: ", jsonFile)
                f = open(os.path.join(rootPath, jsonFile))
                jOutput = json.load(f)
                acdData = jOutput["crash_data"]

                preprocessor = Preprocessor(acdData)
                processor = Processor(preprocessor.sections)
                postprocessor = Postprocessor(
                    processor.report, os.path.join(rootPath, jsonFile),
                    "txt", args.verbose)
                postprocessor.saveFile()
        else:
            print("The path given is not a json file" +
                  f" nor a directory: {rootPath}")
            raise Exception(f"Invalid path given: {rootPath}")

    except Exception as e:
        print(e)


if __name__ == "__main__":
    main()
