import json
import optparse
import os
import sys

import xmlgen


# CMD line Parmeters
desc = ""
outdir = ""


def main(argv):
    # parse the argument
    parser = declareParser()
    options, args = parser.parse_args()
    parseCMD(options)
    xml = xmlgen.genXmlFile(desc, outdir)
    return 0


def declareParser():
    parser = optparse.OptionParser()
    parser.add_option("--desc", dest="desc", default="")
    parser.add_option("--outdir", dest="outdir", default="../example_output/")
    return parser


def parseCMD(options):
    global desc, outdir
    desc = options.desc
    outdir = options.outdir


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
