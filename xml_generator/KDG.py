# classes for KDG


class Node:
    """a node
    """

    def __init__(self, node_name, cost, quality):
        """ Initialization
        :param node_name: name of node
        :param cost: cost of the node
        :param quality: quality of the node
        """
        self.name = node_name
        self.cost = cost
        self.quality = quality


class Knob:
    """ a collection of Node, referenced by lvl (id)
    """

    def __init__(self, knob_name):
        self.nodes = {}
        self.name = knob_name

    def addNode(self, node, lvl):
        """ Add a node
        :param node: the node to be added
        """
        self.nodes[lvl] = node

    def getNode(self, lvl):
        """ Get a node
        :param lvl: level of the node
        """
        return self.nodes[lvl]


class Constraint:
    """ a node pairwise constraint
    The semantic of a constraint in a KDG graph is:
    sink <----- source
    if source_node is chosen, then the sink_node must be chosen
    """

    def __init__(self, source_node, sink_node):
        self.source_node = source_node
        self.sink_node = sink_node


class KDG:
    """ a collection of a knobs and a list of constraints
    """

    def __init__(self):
        self.knobs = {}
        self.constraints = []

    def setName(self, app_name):
        self.name = app_name

    def addKnob(self, knob):
        self.knobs[knob.name] = knob

    def addConstraint(self, constraint):
        self.constraints.append(constraint)

    def addConstraints(self, constraints):
        self.constraints.extend(constraints)

    def getConstraintForNode(self, node_name):
        """ retrieve the list of Constraints in which the node_name is the sink
        :param node_name: the name of the node (sink)
        """
        sources = []
        for c in self.constraints:
            if c.sink_node == node_name:
                sources.append(c)
        return sources
