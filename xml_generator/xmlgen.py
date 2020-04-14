# xml generator

from xml.dom import minidom

from KDG import *
from lxml import etree


INDENT = "    "
KNOB_IND = "<Knobs>"
DEP_IND = "<Dependencies>"


def genXmlFile(depfile, outdir):
    kdg = readdep(depfile)
    if not kdg:
        return

    # write the root:resource
    xml = genXML(kdg)
    writeXML(kdg.name, xml, outdir)
    print("Done...")


def genXML(kdg):
    print("translating... KDG XML")
    # write the root:resource
    xml = etree.Element("resource")
    # create all the service with range
    for knob_name, knob in kdg.knobs.items():
        knobfield = etree.SubElement(xml, "knob")
        etree.SubElement(knobfield, "knobname").text = knob_name
        for lvl, node in knob.nodes.items():
            layer = etree.SubElement(knobfield, "knoblayer")
            basic_node = etree.SubElement(layer, "basicnode")
            etree.SubElement(basic_node, "nodename").text = node.name
            etree.SubElement(basic_node, "cost").text = node.cost
            etree.SubElement(basic_node, "quality").text = node.quality
            # dependencies
            deps = kdg.getConstraintForNode(node.name)
            if len(deps) > 0:
                for constraint in deps:
                    source = constraint.source_node
                    etree.SubElement(basic_node, "and").text = source

    return xml


def writeXML(appname, xml, outdir):
    print("writing... KDG XML")
    xml_byte = prettify(xml)
    file_name = appname + ".xml"
    outputfile = open(outdir + "/" + file_name, "wb")
    outputfile.write(xml_byte)
    outputfile.close()


def parseDepLine(line):
    col = line.rstrip().split("<-")
    sink = col[0].strip().split(".")
    assert len(sink) == 2, "sink format error:" + sink
    sources = col[1].strip()
    assert sources[0] == "[" and sources[-1] == "]", "sources format error:" + sources
    sources = sources[1:-1].split(",")

    sink_knob = sink[0]
    sink_node_lvl = int(sink[1])

    constraints = []

    for source in sources:
        source = source.split(".")
        assert len(source) == 2, "source format error:" + source
        source_knob = source[0]
        source_node_lvl = int(source[1])
        constraint = Constraint(
            source_knob + "_" + str(source_node_lvl),
            sink_knob + "_" + str(sink_node_lvl),
        )
        constraints.append(constraint)

    return constraints


def parseKnobLine(line):
    col = line.split(" ")
    knob_name = col[0]
    knob = Knob(knob_name)
    knob_weights = col[1].rstrip()
    assert knob_weights[0] == "[" and knob_weights[-1] == "]", (
        "knob weights format error:" + knob_weights[-1]
    )
    weights = (knob_weights[1:-1]).split(",")
    lvl = 0
    for weight_pairs in weights:
        assert weight_pairs[0] == "(" and weight_pairs[-1] == ")", (
            "knob weights format error:" + weight_pairs
        )
        weight_pairs = weight_pairs[1:-1].split("-")
        cost = weight_pairs[0]
        quality = weight_pairs[1]
        name = knob_name + "_" + str(lvl)
        node = Node(name, cost, quality)
        knob.addNode(node, lvl)
        lvl += 1
    return knob


def readdep(descfile):
    if descfile == "":
        print("Description File Not Provided")
        return None
    kdg_file = open(descfile, "r")
    lines = [line for line in kdg_file.readlines() if line.strip()]
    app_name = ""
    name_read = False
    line_is_knob = False
    line_is_dep = False
    rsdg = KDG()
    for line in lines:
        line = line.strip()
        if not name_read:
            app_name = line
            rsdg.setName(app_name)
            print("Setting Name: " + app_name)
            name_read = True
            continue
        if line in [KNOB_IND, DEP_IND]:
            line_is_knob = line == KNOB_IND
            line_is_dep = not line_is_knob
            continue
        # actual lines
        if line_is_knob:
            knob = parseKnobLine(line)
            rsdg.addKnob(knob)
        elif line_is_dep:
            constraints = parseDepLine(line)
            rsdg.addConstraints(constraints)

    kdg_file.close()
    return rsdg


def prettify(elem):
    """Return a pretty-printed XML string for the Element.
    """
    rough_string = etree.tostring(elem)
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent=INDENT, encoding="utf-8")
