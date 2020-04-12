from xml.dom import minidom
from io import BytesIO
from lxml import etree

INDENT = "    "


def genxml(depfile, outdir):
    print
    "RAPID-C / STAGE-1.2 : generating... structural RSDG xml"
    app_name, rsdg_map = readdep(depfile)

    # write the root:resource
    xml = genXML(rsdg_map)
    writeXML(app_name, xml, outdir)


def genXML(rsdgMap):
    print("translating... KDG XML")
    #write the root:resource
    xml = etree.Element("resource")
    #create all the service with range
    for knob_name, nodes in rsdgMap.items():
        servicefield = etree.SubElement(xml, "knob")
        etree.SubElement(servicefield, "knobname").text = knob_name
        for node in nodes:
            layer = etree.SubElement(servicefield, "knoblayer")
            basic_node = etree.SubElement(layer, "basicnode")
            etree.SubElement(basic_node, "nodename").text = node["name"]
            etree.SubElement(basic_node, "cost").text = node["cost"]
            etree.SubElement(basic_node, "mv").text = node["quality"]
    return xml


def writeXML(appname, xml, outdir):
    xml_byte = prettify(xml)
    file_name = appname + ".xml"
    outputfile = open(outdir + '/' + file_name, 'wb')
    outputfile.write(xml_byte)
    outputfile.close()


def readdep(rsdgfile):
    if rsdgfile == "":
        print("[INFO] RSDG not provided, generating strucutral info only")
        return [None, None]
    rsdg = open(rsdgfile, 'r')
    rsdg_map = {}
    app_name = ""
    name_read = False
    for line in rsdg:
        if (not name_read):
            app_name = line.rstrip()
            name_read = True
            continue
        col = line.split(" ")
        knob_name = col[0]
        knob_weights = col[1].rstrip()
        assert (knob_weights[0] == '[' and knob_weights[-1] == ']'
                ), "knob weights format error:" + knob_weights[-1]
        rsdg_map[knob_name] = []
        weights = (knob_weights[1:-1]).split(",")
        lvl = 0
        for weight_pairs in weights:
            assert (weight_pairs[0] == '(' and weight_pairs[-1] == ')'
                    ), "knob weights format error:" + weight_pairs
            weight_pairs = weight_pairs[1:-1].split("-")
            cost = weight_pairs[0]
            mv = weight_pairs[1]
            name = knob_name + "_" + str(lvl)
            lvl += 1
            rsdg_map[knob_name].append({
                "cost": cost,
                "quality": mv,
                "name": name
            })
    rsdg.close()
    return app_name, rsdg_map


def prettify(elem):
    """Return a pretty-printed XML string for the Element.
    """
    rough_string = etree.tostring(elem)
    reparsed = minidom.parseString(rough_string)
    return reparsed.toprettyxml(indent=INDENT, encoding='utf-8')
