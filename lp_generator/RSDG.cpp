#include "RSDG.h"
#include <iostream>

bool SUPPRESS_ERRORS = false;
bool DEBUG_MSGS = false;

using namespace std;

// Check if the basic index of node is > 0
bool Node::isBasic() { return basic; }

// Checks if basic index of node == 0 and that the level index > 0
bool Node::isLevel() { return level; }

// Checks if both level and basic indexes are 0
bool Node::isTop() { return top; }

// Used only for node creation .. do not call
void Node::setAddr(index_t *addr) {
  INDEX_TOP(address) = INDEX_TOP(*addr);
  INDEX_LEVEL(address) = INDEX_LEVEL(*addr);
  INDEX_BASIC(address) = INDEX_BASIC(*addr);
}

// Set node name
void Node::setName(string n) { name = n; }

// Returns pointer to index_t representation of node's address
index_t *Node::getAddr() { return &address; }

// Get node name
string Node::getName() { return name; }

// Gets a list of dependencies from a scheme list
vector<vector<edge_t>> *Node::getSchemeEdges() { return &edges; }
// Gets a list of dependencies from xml representation
vector<vector<xml_edge_t>> *Node::getXMLEdges() { return &xml_edges; }

vector<vector<cont_edge_t>> *Node::getContEdges() { return &cont_edges; }

// Gets a list of dependencies from xml representation
vector<vector<xml_edge_t>> *Node::getXMLOutEdges() { return &xml_outedges; }

// Add a new dependency (form of edge_t vector) to a node
void Node::addEdges(vector<edge_t> deps) { edges.push_back(deps); }

void Node::addContEdges(vector<cont_edge_t> deps) {
  cont_edges.push_back(deps);
}

// Add a new dependency (form of xml_edge_t vector) to a node
void Node::addEdges(vector<xml_edge_t> deps) { xml_edges.push_back(deps); }

// Add a new dependency (form of xml_edge_t vector) to a node
void Node::addOutEdges(vector<xml_edge_t> deps) {
  xml_outedges.push_back(deps);
}

// Find the given edge inside of the Node's dependency list - set the new weight
// / cost
void Node::setEdgeWeight(index_t edge_idx, float value, float cost) {
  index_t *idx;
  edge_t edge;
  short top, level, basic; // to avoid repeated get<>()

  top = INDEX_TOP(edge_idx);
  level = INDEX_LEVEL(edge_idx);
  basic = INDEX_BASIC(edge_idx);

  for (size_t i = 0; i < edges.size(); i++) {
    for (size_t j = 0; j < edges.at(i).size(); j++) {
      edge = edges.at(i).at(j);
      idx = &EDGE_INDEX(edge); // index_t field of an edge_t

      // Check for matching edges and update their weight
      if (INDEX_TOP(*idx) == top && INDEX_LEVEL(*idx) == level &&
          INDEX_BASIC(*idx) == basic) {
        EDGE_COST(edges.at(i).at(j)) = cost;
        if (value != 0) {
          EDGE_VALUE(edges.at(i).at(j)) = value;
        }
      }
    }
  }
}

void Node::setType(int t) {
  if (t == 0) {
    top = true;
  } else if (t == 1) {
    level = true;
  } else if (t == 2) {
    basic = true;
  } else {
    cout << "Wrong node type, 0 = top , 1 = level, 2 = basic" << endl;
  }
}

Basic::Basic(index_t *addr, string n) {
  setName(n);
  setAddr(addr);
  setType(2); // 2 is basic
}

Basic::Basic(string n) {
  setName(n);
  setType(2); // 2 is basic
}

void Basic::setContinuous() { CONTINUOUS = true; }

void Basic::setPieceWise() { PIECEWISE = true; }

void Basic::addSegment(string name, float min, float max, float l, float c,
                       bool COST) {
  // determine whether the seg has been added before
  int id = 0;
  string segName = "";
  for (auto it = segments.begin(); it != segments.end(); it++) {
    if (min == it->second.first) {
      segName = it->first;
    }
  }
  // if it's not found
  if (segName == "") {
    id = segments.size();
    segName = getName() + "_" + name + to_string(id);
    segments[segName] = make_pair(min, max);
  }
  // add the seg value
  if (COST)
    segment_costvalues[segName] = make_pair(l, c);
  else
    segment_mvvalues[segName] = make_pair(l, c);
}

double Basic::getValue() { return value; }

void Basic::addContPieceCoeff(string name, float a, float b, float c,
                              bool COST) {
  vector<float> coeffs;
  coeffs.push_back(a);
  coeffs.push_back(b);
  coeffs.push_back(c);
  if (COST)
    piececoeffs[name] = coeffs;
  else
    piecemvcoeffs[name] = coeffs;
}

void Basic::addContCoeff(string name, float coeff) { coeffs[name] = coeff; }

void Basic::addContMVCoeff(string name, float coeff) { mvcoeffs[name] = coeff; }

map<string, float> &Basic::getCoeffs() { return coeffs; }

map<string, float> &Basic::getMVCoeffs() { return mvcoeffs; }

void Basic::getValueOrder(vector<double> &orders) {
  orders.clear();
  orders.push_back(mvo2);
  orders.push_back(mvo1);
  orders.push_back(mvc);
}

float Basic::getCost() { return cost; }

void Basic::getCostOrder(vector<double> &orders) {
  orders.clear();
  orders.push_back(costo2);
  orders.push_back(costo1);
  orders.push_back(costc);
}

// Set the weight of this basic node
void Basic::setValue(double val) { value = val; }

void Basic::setValueOrder(double o2, double o1, double c) {
  mvo2 = o2;
  mvo1 = o1;
  mvc = c;
}

// Set energy cost
void Basic::setCost(float c) { cost = c; }

void Basic::setCostOrder(double o2, double o1, double c) {
  costo2 = o2;
  costo1 = o1;
  costc = c;
}

void Basic::setContMax(double max) {
  MINMAX = true;
  maxValue = max;
}

void Basic::setContMin(double min) {
  MINMAX = true;
  minValue = min;
}

bool Basic::hasMinMax() { return MINMAX; }

double Basic::getMaxValue() { return maxValue; }

double Basic::getMinValue() { return minValue; }

bool Basic::isContinuous() { return CONTINUOUS; }

bool Basic::isPieceWise() { return PIECEWISE; }

Level::Level(index_t *addr) {
  this->setAddr(addr);
  this->setType(1); // 1 is Level
}

// XML level specified number
Level::Level(string name) {
  setName(name); // string representation of the name
  setType(1);    // 1 is Level
}

// Destructor - delete all Basic * from the vector
Level::~Level() {
  for (Basic *basic : basic_nodes) {
    delete basic;
  }
}

void Level::addBasicNode(Basic *basic) { basic_nodes.push_back(basic); }
// Get a pointer to the list of basic node ptrs
vector<Basic *> *Level::getBasicNodes() { return &basic_nodes; }

// Get the XML level number, service_name$lvl_num$basic_name
int Level::getLevelNum() { return level; }

// Constructor + overloads
Top::Top(index_t *addr, string name) {
  setAddr(addr);
  setType(0); // 0 = top
  setName(name);
}
Top::Top(string name) {
  setName(name);
  setType(0); // 0 = top
}

// Destructor - clean up all level nodes
Top::~Top() {
  for (Level *level : level_nodes) {
    delete level;
  }
}

// Get a pointer to the list of Level node ptrs
vector<Level *> *Top::getLevelNodes() { return &level_nodes; }

// Add a level node to list of levels
void Top::addLevelNode(Level *level) { level_nodes.push_back(level); }

// Destructor RSDG::~RSDG()
RSDG::~RSDG() {
  // Clean map
  for (auto it = rsdg.begin(); it != rsdg.end(); it++) {
    delete it->second;
  }
  // Clean vectors
  for (Top *top : xml_rsdg) {
    delete top;
  }
}
// Retrieve the node from RSDG using the index_t SCHEME only
Node *RSDG::getNodeFromIndex(index_t *idx) {
  try {
    if (rsdg.at(hashIndex(idx)) != NULL) {
      return rsdg.at(hashIndex(idx));
    }
  } catch (std::out_of_range) { // looking for node that's not there
    return NULL;
  }
  return NULL;
}

// solve the problem using heuristics
bool RSDG::getSolution() { return false; }

// Add a node to the rsdg using index_t
void RSDG::addNode(index_t *idx) {
  if (INDEX_BASIC(*idx) != 0) {
    rsdg.insert(make_pair(hashIndex(idx), new Basic(idx)));
  } else if (INDEX_LEVEL(*idx) != 0) {
    rsdg.insert(make_pair(hashIndex(idx), new Level(idx)));
  } else if (INDEX_TOP(*idx) != 0) {
    rsdg.insert(make_pair(hashIndex(idx), new Top(idx)));
  } else {
    cout << "Error! Something went wrong, cannot add requested node." << endl;
  }
}

// Set the energy budget for the system
void RSDG::setBudget(double b) { budget = b; }
// This will never return a level node because those have no name
Node *RSDG::getNodeFromName(string name) {
  for (Top *top : xml_rsdg) {

    if (top->getName().compare(name) == 0) {
      return top;
    }
    for (Level *lvl : *(top->getLevelNodes())) {

      for (Basic *b : *(lvl->getBasicNodes())) {
        if (b->getName().compare(name) == 0) {
          return b;
        }
      }
    }
  }
  return NULL;
}

// XML ONLY
void RSDG::updateMissionValue(string serviceName, int value, bool exp) {
  Node *node = getNodeFromName(serviceName);
  int tot_levels;
  if (node == NULL || !node->isTop()) {
    cout << "Specified node is not a service node" << endl;
    return;
  }
  Top *top = (Top *)node;
  tot_levels = (int)(top->getLevelNodes())->size();

  // Linear
  if (!exp) {
    int k = 0;
    for (Level *lvl : *(top->getLevelNodes())) {
      // double new_val = value / (lvl->getLevelNum() * tot_levels);
      double level_num = tot_levels - k;
      double new_val = value / tot_levels * level_num;
      k++;
      for (Basic *b : *(lvl->getBasicNodes())) {
        b->setValue(new_val);
      }
    }
  }
}

// XML ONLY
void RSDG::updateCost(string b_name, double cost) {
  Node *n = getNodeFromName(b_name);

  if (n == NULL || !n->isBasic()) {
    cout << b_name << " is not a basic node" << endl;
    return;
  }

  ((Basic *)n)->setCost(cost);
}

void RSDG::updateEdgeCost(string sink, string source, double cost) {
  Node *n = getNodeFromName(sink);
  if (n == NULL || !n->isBasic()) {
    cout << source << " is not a basic node" << endl;
    return;
  }
  vector<vector<xml_edge_t>> &edges = *(n->getXMLEdges());
  for (int i = 0; i < (int)edges.size(); i++) {
    for (int j = 0; j < (int)edges[i].size(); j++) {
      if (EDGE_NAME(edges[i][j]) == source) {
        EDGE_VALUE(edges[i][j]) = cost;
        return; // assuming only 1 edge will be from a certain node
      }
    }
  }
  return;
}

RSDG *rsdgGen(string input) {
  RSDG *graph = new RSDG();
  // default file
  string infile = input;
  graph->parseXML(infile);
  return graph;
}
