#!/usr/bin/env python

"""
This program prints the properties or advices in a tabular format,
based on the type of the file.
"""

# importing the necessary modules
import argparse
try:
    import xml.etree.cElementTree as ET
except ImportError:
    import xml.etree.ElementTree as ET

# Declaring the global variables
TYPE = ''
MAX_LEN = {}
DATA = []
BESTSCENARIO = []

# creates the element tree according to file type
def tree_builder(filename):
    """
    Parses XML file in order to get the tree object and the root.
    It also determines the type of the file.
    """
    try:
        tree = ET.parse(filename)
    except Exception:
        return
    root = tree.getroot()
    global TYPE
    if root.tag[0] == "{" and root.tag.split("}")[1][
        0:].lower() == 'experiment':
        TYPE = 'property'
    elif root.tag.lower() == 'advices':
        TYPE = 'advice'
    else:
        TYPE = 'error'
    return tree

# removes unncessary elements from the tree
def remove_element(parent, element):
    """
    Removes the element, given the parent
    """
    parent.remove(parent.find(element))

# datastructure for the property file
def property_structure(properties, restricted, ispurpose):
    """
    Creates a list of properties, which are dictionaries of elements
    """
    record = {}
    if not ispurpose:
        restricted.append('purpose')
    for prop in properties:
        for element in prop.iter():
            element.tag = element.tag.split("}")[1][0:]
            if element.tag not in restricted:
                if not element.text or str(element.text).isspace():
                    element.text = ''
                if not bool(element.attrib):
                    element.attrib = ''
                if element.tag == 'context':
                    record['RegionId'.lower()] = element.attrib['RegionId']
                else:
                    record[element.tag.lower()] = element.text
        DATA.append(record.copy())

# datastructure for the advice file
def advice_structure(scenarios, restricted):
    """
    Creates a list of scenarios, which are dictionaries of elements
    """
    record = {}
    for scenario in scenarios:
        dictionary = {}
        region = []
        tuning = []
        for ele in scenario.iter():
            if str(ele.text).isspace() or not bool(ele.text):
                ele.text = ''
            if ele.tag.lower() == "tuningspecification":
                remove_element(ele, "VariantContext")
                remove_element(ele, "Ranks")
            if ele.tag.lower() == "tuningparameter":
                tuning = list(ele)
                temp = {tuning[0].text: tuning[1].text}
                dictionary.update(temp)
                record["tuningparameter"] = ', '.join(
                    "{!s}={!r}".format(
                        k, v) for (
                        k, v) in dictionary.iteritems())
            if ele.tag.lower() == "region":
                remove_element(ele, "LastLine")
                region = list(ele)
                record["region"] = region[0].tag + "=" + region[0].text + \
                    ", " + region[1].tag + "=" + region[1].text
            elif not (
                ele in tuning or ele in region or ele.tag.lower() in restricted
            ):
                if ele.tag.lower() == 'id':
                    ele.tag = 'scenario' + ele.tag
                record[ele.tag.lower()] = ele.text
        DATA.append(record.copy())


def data_builder(tree, ispurpose):
    """
    Builds a datastructure, based on the type of file provided
    """
    scenarios = []
    properties = []
    global BESTSCENARIO
    if TYPE == 'property':
        properties = [
            element for element in tree.iter() if (
                element.tag.split("}")[1][0:].lower() == 'property'
            )
        ]
        # elements that shouldn't be printed
        restricted = [
            'property',
            'execObj',
            'addInfo',
            'cycles',
            'nestedCycles']
        property_structure(properties, restricted, ispurpose)

    elif TYPE == 'advice':
        scenarios = [
            element for element in tree.findall(
                ".//SearchPath/ScenarioResult/Scenario"
            ) if (
                element.tag.lower() == 'scenario'
            )
        ]
        # list of the best scenarios, if more than one.
        BESTSCENARIO = [element for element in tree.findall(
            ".//BestScenarios/ScenarioResult/Scenario/ID")]
        restricted = [
            'scenario',
            'tuningspecification',
            'tuningparameter',
            'description',
            'variant']
        advice_structure(scenarios, restricted)


def col_width():
    """
    Determines the width of the column to be printed
    """
    for record in DATA:
        for key, val in record.items():
            if key in MAX_LEN.keys():
                if isinstance(val, dict):
                    length = [(len(x) + len(y)) for x, y in val.items()]
                    if sum(length) > MAX_LEN[key]:
                        MAX_LEN[key] = sum(length)
                elif len(val) > MAX_LEN[key]:
                    MAX_LEN[key] = len(val)
            else:
                MAX_LEN[key] = len(key)


def justified(value):
    """
    Justifies the data Right/Left according to the type of value
    """
    try:
        float(value)
        return "{0:>{key}}"
    except ValueError:
        return "{0:<{key}}"


def header_sep(character):
    """
    Prints the table header with the provided character
    """
    print "".join(character * (sum(MAX_LEN.values()) + len(MAX_LEN) - 1))


def print_order():
    """
    This function just determines the order in which
    the columns are printed
    """
    if TYPE == 'property':
        order_list = ['name', 'severity']
    elif TYPE == 'advice':
        order_list = ['scenarioid']
    temp = [
        order_list.append(j) for j in MAX_LEN.keys() if j not in order_list
    ]
    del temp[:]
    return order_list


def print_data():
    """
    Prints out the data of the properties file in tabular format.
    """
    # getting the maximum width of the columns according to the element's value
    # or respective headers
    col_width()
    order_list = print_order()
    header_sep('=')
    print (" ".join(justified(DATA[0][l]).format(
        l.upper(), key=MAX_LEN[l]
    )
        for l in order_list
    ))

    header_sep('=')

    for record in DATA:
        print (" ".join(justified(record[k]).format(
            record[k], key=MAX_LEN[k]
        )
            for k in order_list
        ))

    header_sep('-')
    if TYPE == 'advice':
        for record in BESTSCENARIO:
            print "Best Scenario: Scenario_ID = " + record.text

        header_sep('-')


def sort_data(key):
    """
    Sorts the properties according to the provided key
    """
    if key == 'severity':
        descending = True
    else:
        descending = False
    try:
        DATA.sort(key=lambda x: float(x[key]), reverse=descending)
    except ValueError:
        DATA.sort(key=lambda x: x[key], reverse=descending)


def main():
    """The main function"""
    parser = argparse.ArgumentParser(
        description="The program print properties or advices in a "
            "well-structured fashion, depending upon the type of file"
    )
    parser.add_argument(
        "-p", "--purpose", help="Prints out purpose as part of the property, "
        "for properties file",
        action="store_true"
    )
    parser.add_argument(
        "filename", help=("The name of the property file. Please provide it,"
                          " it's required"
                          )
    )
    parser.add_argument(
        "-s",
        "--sort",
        help=(
            "Sorts properties according to provided key;"
            "valid values are: {name, severity, confidence, regionid, purpose}"
            " or {scenarioid, region, tuningparameter}"
        ))
    args = parser.parse_args()
    tree = tree_builder(args.filename)
    if tree == None:
        raise parser.error(
            "Not a valid xml file; tool reads valid files in xml format"
        )
    if TYPE == 'error':
        raise parser.error(
            'Not a valid filetype; provide properties or advice file'
        )
    if TYPE != 'property' and args.purpose == True:
        raise parser.error(
            "-p or --purpose option are only allowed with properties file"
        )
    data_builder(tree, args.purpose)
    if not args.sort is None:
        if args.sort in DATA[0].keys():
            sort_data(args.sort)
        else:
            raise parser.error(
                'not a valid sorting parameter for this type of file')
    print_data()

# The main boiler plate
if __name__ == "__main__":
    main()
