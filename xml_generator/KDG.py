# classes for KDG

class Node:
    """a node
    """

    def __init__(self, node_name, cost, quality):
        """ Initialization
        :param node_name: name of node
        """
        self.name = node_name
        self.cost = cost
        self.quality = quality


class Knob:
    """ a collection of Node
    The Knob object can be retrieved by name
    """

    def __init__(self, knob_name):
        self.nodes = {}
        self.name = knob_name

    def addNode(self, node, i):
        """ Add a node
        :param node: the node to be added
        """
        self.nodes[i] = node

    def getNode(self, id):
        """ Get a node
        :param id: id of the node
        """
        return self.nodes[id]


class Constraint:
    """ a continuous constraint with source and sink
    The syntax of a constraint is:
    if sink_min <= sink <= sink_max, then source_min <= source <= source_max
    """

    def __init__(self, source_node, sink_node):
        self.source_node = source_node
        self.sink_node = sink_node


class KDG:
    """ a segment of a knob
    The segment of knob can be used both in dependencies and constraint
    """

    def __init__(self):
        self.knobs = {}
        self.constraints = []

    def setName(self,app_name):
        self.name = app_name


    def addKnob(self, knob):
        self.knobs[knob.name]=knob

    def addConstraint(self, constraint):
        self.constraints.append(constraint)

    def addConstraints(self, constraints):
        self.constraints.extend(constraints)

    def getConstraintForNode(self, node_name):
        sources = []
        for c in self.constraints:
            if c.sink_node == node_name:
                sources.append(c)
        return sources
