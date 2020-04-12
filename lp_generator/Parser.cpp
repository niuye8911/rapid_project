#include "RSDG.h"
#include "rapidxml.hpp"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace std;
using namespace rapidxml;

// Message Printing
extern bool DEBUG_MSGS, SUPPRESS_ERRORS;

#define THRESHOLD 1e-14
/*
 * Parser for a scheme list representation of an RSDG into an in memory
 * representation.
 */

/* GLOBALS */
ifstream input;
string str;

// Hash an index type to use as key in the RSDG map
long long RSDG::hashIndex(index_t *idx) {
  // 64 bits of 0's
  long long n = 0;
  n = INDEX_TOP(*idx);
  n = n << 16; // make room for level
  n += INDEX_LEVEL(*idx);
  n = n << 16; // make room for basic
  n += INDEX_BASIC(*idx);

  //  bytes  7 and  8    | bytes 5 and 6   |   bytes 3 and 4 |  bytes 1 and 2
  // 0000 0000 0000 0000 | (idx -> top )   |  (idx->level)   |  idx->basic
  // n = (i1 << 32) | (i2 << 16) | i3;
  return n;
}

// Get a solid format with only DELIM_CHAR and index numbers string
static void clean_up_node() {
  size_t pos;
  // Erase comment embeded in node str
  if ((pos = str.find(';')) != string::npos) {
    str.erase(pos);
  }

  if ((pos = str.find('\'')) != string::npos) {
    str.erase(pos, 1);
  }

  // Drop All Parens => only will work for nodes --> not edges --> not weights
  while ((pos = str.find('(')) != string::npos) {
    str.erase(pos, 1);
  }

  while ((pos = str.find(')')) != string::npos) {
    str.erase(pos, 1);
  }

  // Now that all else is gone, get rid of the START_CHAR + DELIM_CHAR
  if ((pos = str.find(START_CHAR)) != string::npos) {
    str.erase(0, 2);
  }

  // Add a terminating _ for index building if there is stll anything left to
  // parse
  if (!str.empty()) {
    str += DELIM_CHAR;
  }
}

// Checks if the globabl parse string - str - contains a comment then parses it
// out
static void check_for_comment() {
  string tmp; // holding temporary value of string that's not a comment
  size_t spos;
  char c;

  if ((spos = str.find(";;")) != string::npos) {
    tmp = str.substr(0, spos);

    // consume comment
    input.get(c);
    while (c != '\n') {
      input.get(c);
    }

    // make str = (str - the comment part)
    str = tmp;
  }
}

// Quick print tool for an index
static void print_index(index_t *idx) {
  cout << "T: " << INDEX_TOP(*idx) << " ";
  cout << "L: " << INDEX_LEVEL(*idx) << " ";
  cout << "B: " << INDEX_BASIC(*idx);
  cout << endl;
}

// Builds index from node's string representation (optional edge index for
// populating weights)
static void str_to_index(string str, index_t *idx, index_t *edge = NULL) {
  size_t s_pos, e_pos;
  string tmp;
  int cnt = 0;

  if (edge != NULL) {
    ZERO_OUT_IDX_T(*edge);
  }

  ZERO_OUT_IDX_T(*idx);

  // Go through the entire string   -- probably should be done with
  // string::iterator ...
  while (str.find_first_not_of('_') != string::npos) {
    s_pos = str.find_first_not_of('_');
    e_pos = str.find_first_of('_');

    if (s_pos != string::npos) {
      tmp = str.substr(s_pos, e_pos);
      str = str.substr(e_pos + 1, string::npos);

      if (cnt == 0) {
        INDEX_TOP(*idx) = stoi(tmp);
      } else if (cnt == 1) {
        INDEX_LEVEL(*idx) = std::stoi(tmp);
      } else if (cnt == 2) {
        INDEX_BASIC(*idx) = std::stoi(tmp);
      } else if (cnt == 3 && edge != NULL) {
        INDEX_TOP(*edge) = std::stoi(tmp);
      } else if (cnt == 4 && edge != NULL) {
        INDEX_LEVEL(*edge) = std::stoi(tmp);
      } else if (cnt == 5 && edge != NULL) {
        INDEX_BASIC(*edge) = std::stoi(tmp);
      } else { // something went wrong
        break;
      }
      cnt++;
    }
  }
}

// Get a string representation of an index using START_CHAR + DELIM_CHAR format
static string index_to_str(const index_t *addr) {
  string retval;

  retval += START_CHAR;
  retval += DELIM_CHAR;

  retval += to_string(INDEX_TOP(*addr)); //('0' + INDEX_TOP(*addr));

  if (INDEX_LEVEL(*addr) != 0) {
    retval += DELIM_CHAR;
    retval += to_string(INDEX_LEVEL(*addr));
  }

  if (INDEX_BASIC(*addr) != 0) {
    retval += DELIM_CHAR;
    retval += to_string(INDEX_BASIC(*addr));
  }

  return retval;
}

// Go through the stack and build a vector of or_edges or a single and edge then
// add to node's edges
static void add_edges_from_stack(vector<string> *stack, Node *node,
                                 bool or_flg) {
  index_t idx;
  vector<edge_t> edges;

  while (!stack->empty()) {
    str_to_index(stack->back(), &idx);

    if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
      print_index(&idx);
    }

    edges.push_back(make_tuple(
        0.0f, 0.0f,
        make_tuple(INDEX_TOP(idx), INDEX_LEVEL(idx), INDEX_BASIC(idx))));

    if (!or_flg) {
      node->addEdges(edges);
      edges.clear();
    }

    stack->pop_back();
  }
  if (or_flg) {
    node->addEdges(edges);
    edges.clear();
  }
}

// Parse rsdg-nodes and build the RSDG map (from Scheme list)
static void schemeNodes(RSDG *rsdg) {
  index_t index;

  // Parse all the way to next define statement  or end of file
  while (str.compare("(define") != 0 && !input.eof()) {
    input >> str;

    // Check for comments
    check_for_comment();

    // Get node strings of format S_x_y_z_ ...
    if (IS_NODE(str)) {
      clean_up_node();

      // Got index -> build a node
      if (!str.empty()) {

        str_to_index(str, &index);

        if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
          print_index(&index);
        }

        rsdg->addNode(&index);
      }
    }
  }
}

// Parse rsdg-edges and add AND / OR edges to a specified node (from Scheme
// list)
static void schemeEdges(RSDG *rsdg) {
  Node *node;
  vector<string> stack;
  index_t idx;
  string cur_node;
  bool or_flg = false;
  bool first_node = true;

  while (str.compare("(define") != 0 && !input.eof()) {

    input >> str;
    check_for_comment();

    // 1. Find node push string on the stack
    if (IS_NODE(str)) {
      clean_up_node();
      stack.push_back(str);
    }
    // 2. find a delimiter
    else if (str.compare("AND") == 0 || str.compare("OR") == 0) {

      if (first_node) {
        cur_node = stack.back(); // get first dependency sink node
        stack.pop_back();
        first_node = false;
        str.compare("OR") == 0 ? or_flg = true : or_flg = false;
      } else {
        if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
          or_flg ? cout << "OR " : cout << "AND ";
          cout << "edges for:" << cur_node << endl;
        }

        // Dependency sink node built
        str_to_index(cur_node, &idx);
        node = rsdg->getNodeFromIndex(&idx);

        // Next sink node
        cur_node = stack.back();
        stack.pop_back();

        // Error - clean up and keep going
        if (node == NULL) {
          if (!SUPPRESS_ERRORS) {
            print_index(&idx);
            cout << " Does not exist!" << endl;
          }
          stack.clear();
          continue;
        }

        // Add everything from stack to the node
        add_edges_from_stack(&stack, node, or_flg);

        // Set flag for next set of nodes
        str.compare("OR") == 0 ? or_flg = true : or_flg = false;
      }
    }
  } // end while

  if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
    or_flg ? cout << "OR " : cout << "AND ";
    cout << "edges for:" << cur_node << endl;
  }

  // get Node
  str_to_index(cur_node, &idx);
  node = rsdg->getNodeFromIndex(&idx);

  // Error - last node wrong
  if (node == NULL) {
    stack.clear();
    return;
  }

  // Add last set of edges to the node
  add_edges_from_stack(&stack, node, or_flg);

  if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
    std::cout << endl << endl;
  }
}

// Parse rsdg-weights list and add the weight to a specified node (from Scheme
// list)
static void schemeWeights(RSDG *rsdg) {
  // Index containers
  Node *node = NULL;
  index_t node_idx, edge_idx;
  size_t val;
  string w; // weight
  float value = 0, cost = 0;

  // Parse all the way to next define statement  or end of file
  while (str.compare("(define") != 0 && !input.eof()) {
    input >> str;
    check_for_comment();

    // Get node strings of format S_x_y_z_ ...
    if (IS_NODE(str)) {
      clean_up_node();

      // Got index
      if (!str.empty()) {
        str_to_index(str, &node_idx, &edge_idx);

        if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
          print_index(&node_idx);
        }

        node = rsdg->getNodeFromIndex(&node_idx);

        // ERROR - node not defined in rsdg-nodes
        if (node == NULL) {
          if (!SUPPRESS_ERRORS) {
            cout << "Node ";
            print_index(&node_idx);
            cout << "Not defined in rsdg-nodes" << endl;
          }
          continue;
        }

        // EDGE WEIGHT
        if (INDEX_TOP(edge_idx) != 0) { // Edge weight ?

          // Cost
          input >> str;

          if ((val = str.find_first_of(')')) !=
              string::npos) { // value = 0  was not specified in the edge  ex.
                              // (S_2_1_1_4_1_1 9)
            str = str.substr(0, val);

            cost = stof(str);
            node->setEdgeWeight(edge_idx, 0, cost);

            value = 0;
            cost = 0;
            continue;
          } else {
            // save value
            cost = stof(str);

            input >> str; // move on to cost

            if ((val = str.find_first_of(')')) != string::npos) {
              str = str.substr(0, val);
            }

            value = stof(str);
            node->setEdgeWeight(edge_idx, value, cost);

            value = 0;
            cost = 0;
          }

          if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
            cout << "edge: ";
            print_index(&edge_idx);
          }
        } else { // REGULAR WEIGHT

          // Cost
          input >> str;
          cost = stof(str);
          ((Basic *)node)->setCost(cost);

          // Value
          input >> str;
          if ((val = str.find_first_of(')')) != string::npos) {
            str = str.substr(0, val);
          }
          value = stof(str);
          ((Basic *)node)->setValue(value);

          value = 0;
          cost = 0;
        }
      }
    }
  } // end main while loop
}

// set any defined basic node fields to their correspondding XML val
static void get_basic_node_info(xml_node<> *xml_bnode, Basic *basic) {
  string n_name, n_cost, f_name;
  float cost;

  for (xml_node<> *fields = xml_bnode->first_node(); fields;
       fields = fields->next_sibling()) {

    if (fields->name() == NULL) {
      continue;
    }
    string f_name = fields->name();

    if (f_name.compare("nodename") == 0) {
      if (fields->value() != NULL) {
        basic->setName(fields->value());
        //    cout << "node name: " << fields->value() << endl;
      }
    } else if (f_name.compare("contcost") == 0) {
      double order2 = 0.0;
      double order1 = 0.0;
      double constant = 0.0;
      for (xml_node<> *order_value = fields->first_node(); order_value;
           order_value = order_value->next_sibling()) {
        string name = order_value->name();
        if (name.compare("o2") == 0) {
          order2 = stof(order_value->value());
        } else if (name.compare("o1") == 0) {
          order1 = stof(order_value->value());
        } else if (name.compare("c") == 0) {
          constant = stof(order_value->value());
        }
      }
      // indicate the node being continuous
      basic->setContinuous();
      basic->setCostOrder(order2, order1, constant);
    } else if (f_name.compare("contmv") == 0) {
      double order2 = 0.0;
      double order1 = 0.0;
      double constant = 0.0;
      for (xml_node<> *order_value = fields->first_node(); order_value;
           order_value = order_value->next_sibling()) {
        string name = order_value->name();
        if (name.compare("o2") == 0) {
          order2 = stof(order_value->value());
        } else if (name.compare("o1") == 0) {
          order1 = stof(order_value->value());
        } else if (name.compare("c") == 0) {
          constant = stof(order_value->value());
        }
      }
      // indicate the node being continuous
      basic->setContinuous();
      basic->setValueOrder(order2, order1, constant);
    } else if (f_name.compare("contpiece") == 0) {
      for (xml_node<> *seg = fields->first_node(); seg;
           seg = seg->next_sibling()) {
        float segmin = 0.0;
        float segmax = 0.0;
        float segLinear = 0.0;
        float segConst = 0.0;
        float segLinearMV = 0.0;
        float segConstMV = 0.0;
        for (xml_node<> *attr = seg->first_node(); attr;
             attr = attr->next_sibling()) {
          string name = attr->name();
          if (name.compare("min") == 0) {
            segmin = stof(attr->value());
          }
          if (name.compare("max") == 0) {
            segmax = stof(attr->value());
          }
          if (name.compare("costL") == 0) {
            segLinear = stof(attr->value());
          }
          if (name.compare("costC") == 0) {
            segConst = stof(attr->value());
          }
          if (name.compare("mvL") == 0) {
            segLinearMV = stof(attr->value());
          }
          if (name.compare("mvC") == 0) {
            segConstMV = stof(attr->value());
          }
        }
        basic->setPieceWise();
        basic->addSegment("seg", segmin, segmax, segLinear, segConst, true);
        basic->addSegment("seg", segmin, segmax, segLinearMV, segConstMV,
                          false);
      }

    }

    // the boundaries for continuous service
    else if (f_name.compare("contmax") == 0) {
      double max = stof(fields->value());
      basic->setContMax(max);
      basic->setContinuous();
    } else if (f_name.compare("contmin") == 0) {
      double min = stof(fields->value());
      basic->setContMin(min);
      basic->setContinuous();
    } else if (f_name.compare("contmv") == 0) {
      double order2 = 0.0;
      double order1 = 0.0;
      double constant = 0.0;
      for (xml_node<> *order_value = fields->first_node(); order_value;
           order_value = order_value->next_sibling()) {
        string name = order_value->name();
        if (name.compare("o2") == 0) {
          order2 = stof(order_value->value());
        } else if (name.compare("o1") == 0) {
          order1 = stof(order_value->value());
        } else if (name.compare("c") == 0) {
          constant = stof(order_value->value());
        }
      }
      // indicate the node being continuous
      basic->setContinuous();
      basic->setValueOrder(order2, order1, constant);
    } else if (f_name.compare("cost") == 0) {
      try {
        cost = stof(fields->value());
      } catch (exception e) { // either out of range or invalid arg
        cost = 0;
      }
      basic->setCost(cost);
      // cout << "Node cost: " << cost << endl;
    } else if (f_name.compare("quality") == 0) {
      try {
        cost = stof(fields->value());
      } catch (exception e) { // either out of range or invalid arg
        cost = 0;
      }
      basic->setValue(cost);
      // cout << "Node cost: " << cost << endl;
    } else if (f_name.compare("contpiecewith") == 0 ||
               f_name.compare("contwith") == 0) {
      for (xml_node<> *sink = fields->first_node(); sink;
           sink = sink->next_sibling()) {
        string knobname;
        float costa, costb, costc, mva, mvb, mvc;
        for (xml_node<> *attr = sink->first_node(); attr;
             attr = attr->next_sibling()) {
          string name = attr->name();
          if (name.compare("name") == 0) {
            knobname = attr->value();
          }
          if (name.compare("costa") == 0) {
            costa = stof(attr->value());
          }
          if (name.compare("costb") == 0) {
            costb = stof(attr->value());
          }
          if (name.compare("costc") == 0) {
            costc = stof(attr->value());
          }
          if (name.compare("mva") == 0) {
            mva = stof(attr->value());
          }
          if (name.compare("mvb") == 0) {
            mvb = stof(attr->value());
          }
          if (name.compare("mvc") == 0) {
            mvc = stof(attr->value());
          }
        }
        basic->addContPieceCoeff(knobname, costa, costb, costc, true);
        basic->addContPieceCoeff(knobname, mva, mvb, mvc, false);
      }

    }
    // there's an inter-relationship with other service
    else if (f_name.compare("contwithbla") == 0) {
      /*for (xml_node<> *sink = fields->first_node(); sink; sink =
       sink->next_sibling()){ string knobname; float costa, costb, costc, mva,
       mvb, mvc; for (xml_node<> *attr = sink->first_node(); attr; attr =
       attr->next_sibling()){ string name = attr->name();
       if(name.compare("name")==0){
       knobname = attr->value();
       }
       if(name.compare("costa")==0){
       costa = stof(attr->value());
       }
       if(name.compare("costb")==0){
       costb = stof(attr->value());
       }
       if(name.compare("costc")==0){
       costc = stof(attr->value());
       }
       if(name.compare("mva")==0){
       mva = stof(attr->value());
       }
       if(name.compare("mvb")==0){
       mvb = stof(attr->value());
       }
       if(name.compare("mvc")==0){
       mvc = stof(attr->value());
       }
       }
       basic->add
       }*/
      string name =
          fields->first_node("name")->value(); // the inter-service name
      float para = stof(
          fields->first_node("mvcoeff")->value()); // the inter-service value
      basic->addContMVCoeff(name, para);
      float costpara = stof(fields->first_node("costcoeff")->value());
      basic->addContCoeff(name, costpara);
    } else if (f_name.compare("contand") == 0) {
      // the AND edge for continuous service
      vector<cont_edge_t> edges;
      cont_edge_t tmp_edge;
      float ifmin = -99999;
      float ifmax = 99999;
      float thenmin = ifmin;
      float thenmax = ifmax;
      string name;
      name = fields->first_node("name")->value();
      try {
        ifmin = stof(fields->first_node("ifrangemin")->value());
        ifmax = stof(fields->first_node("ifrangemax")->value());
        thenmin = stof(fields->first_node("thenrangemin")->value());
        thenmax = stof(fields->first_node("thenrangemax")->value());
      } catch (exception e) {
        ifmin = 0.0;
        ifmax = 0.0;
        thenmin = 0.0;
        thenmax = 0.0;
      }
      get<0>(tmp_edge) = ifmin;
      get<1>(tmp_edge) = ifmax;
      get<2>(tmp_edge) = name;
      get<3>(tmp_edge) = thenmin;
      get<4>(tmp_edge) = thenmax;
      edges.push_back(tmp_edge);
      basic->addContEdges(edges);
      edges.clear();
    }

    else if (f_name.compare("contor") == 0) {
      vector<cont_edge_t> edges;
      cont_edge_t tmp_edge;
      float ifmin = -99999;
      float ifmax = 99999;
      float thenmin = -99999;
      float thenmax = 99999;
      string name;
      for (xml_node<> *or_dep = fields->first_node(); or_dep;
           or_dep = or_dep->next_sibling()) {
        xml_node<> *dep = or_dep->first_node(); // dep is <dep> dep </dep>
        string xml_name = dep->name();
        name = dep->first_node("name")->value();
        ifmin = stof(fields->first_node("ifrangemin")->value());
        ifmax = stof(fields->first_node("ifrangemax")->value());
        thenmin = stof(fields->first_node("thenrangemin")->value());
        thenmax = stof(fields->first_node("thenrangemax")->value());
        get<0>(tmp_edge) = ifmin;
        get<1>(tmp_edge) = ifmax;
        get<2>(tmp_edge) = name;
        get<3>(tmp_edge) = thenmin;
        get<4>(tmp_edge) = thenmax;
        edges.push_back(tmp_edge);
      }
      basic->addContEdges(edges);
      edges.clear();
    }

    else if (f_name.compare("and") == 0) {
      vector<xml_edge_t> edges;
      xml_edge_t and_edge;
      float weight = 0;
      string name;

      if (fields->first_node("name") == NULL ||
          fields->first_node("name")->value() == NULL) {
        continue;
      }
      name = fields->first_node("name")->value();
      // cout << "And edge name: " << name << endl;

      try {
        weight = stof(fields->first_node("weight")->value());
        // cout << "And edge weight: " << weight << endl;
      } catch (exception e) { // either out of range or invalid arg
        weight = 0;
      }

      EDGE_VALUE(and_edge) = weight;
      EDGE_NAME(and_edge) = name;
      edges.push_back(and_edge);

      basic->addEdges(edges);
      edges.clear(); // clean out for or_edge parsing
    } else if (f_name.compare("or") == 0) {

      // Parse or edges
      vector<xml_edge_t> edges;
      xml_edge_t or_edge;
      string name;
      for (xml_node<> *or_dep = fields->first_node(); or_dep;
           or_dep = or_dep->next_sibling()) {

        string xml_name = or_dep->name();

        if (xml_name.compare("name") == 0) {
          name = or_dep->value();
          // added by Liu
          EDGE_NAME(or_edge) = name;
          edges.push_back(or_edge);
        }
        // Modified by Liu that there won't be edge weight
        /*                else if (xml_name.compare("weight") == 0) {

         try {
         weight = stof(or_dep->value());
         EDGE_VALUE(or_edge) = weight;
         EDGE_NAME(or_edge) = name;
         edges.push_back(or_edge);
         //cout << "Or edge name: " << name << "\nOr edge weight: " << weight <<
         endl;
         }
         catch (exception e) {
         weight = 0;
         name.clear();
         continue;
         }
         }*/
      }
      // Add the set of or edges to the node
      basic->addEdges(edges);
    }

    else if (f_name.compare("orrange") == 0) {

      // Parse a group of or edges
      vector<xml_edge_t> edges;
      xml_edge_t or_edge;
      string name;
      int min_level;
      int max_level;

      if (fields->first_node("svcname") == NULL ||
          fields->first_node("svcname")->value() == NULL) {
        continue;
      }
      name = fields->first_node("svcname")->value();

      min_level = stoi(fields->first_node("MIN")->value());
      max_level = stoi(fields->first_node("MAX")->value());

      for (int i = min_level; i <= max_level; i++) {
        EDGE_VALUE(or_edge) = 0;
        EDGE_NAME(or_edge) = name + "_" + to_string(i);
        edges.push_back(or_edge);
      }
      // Add the range of or edges to the node
      basic->addEdges(edges);
    }

    else if (f_name.compare("orout") == 0) {

      // Parse or edges
      vector<xml_edge_t> edges;
      xml_edge_t or_out_edge;
      string name;
      // float weight;

      for (xml_node<> *or_sink = fields->first_node(); or_sink;
           or_sink = or_sink->next_sibling()) {

        string xml_name = or_sink->name();

        if (xml_name.compare("name") == 0) {
          name = or_sink->value();
          EDGE_NAME(or_out_edge) = name;
          edges.push_back(or_out_edge);
        }
      }
      // Add the set of or edges to the node
      basic->addOutEdges(edges);
    }
  }
}

// Go through scheme list and populate nodes, edges, weights
void RSDG::parseSchemeList(string infile) {
  input.open(infile);
  input >> str; // First token

  // Get to rsdg-nodes
  while (!str.compare("rsdg-nodes")) {
    input >> str;
  }

  if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
    cout << endl << str << endl;
  }

  schemeNodes(this);

  // Get to rsdg-edges
  while (!str.compare("rsdg-edges")) {
    input >> str;
  }

  if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
    cout << endl << str << endl;
  }

  schemeEdges(this);

  // Get to rsdg-weights
  while (!str.compare("rsdg-weights")) {
    input >> str;
  }

  if (DEBUG_MSGS && !SUPPRESS_ERRORS) {
    cout << endl << str << endl;
  }

  schemeWeights(this);
}

// Translation of Soham's solution to creating the lp file
void RSDG::writeSchemeLp(string outfile) {
  Node *node = NULL;
  ofstream out(outfile);
  vector<vector<edge_t>> *edges;
  vector<index_t *> maximize_deps;
  bool first = true;
  stringstream obj, net_obj;
  string objective = "";

  // Maximize
  out << "Maximize" << endl;

  for (auto it = rsdg.begin(); it != rsdg.end(); it++) {
    node = it->second; // for convenience
    // index_t *idx = node->getAddr();

    if (node->isBasic()) {
      // PRINT MAXIMIZE
      if (first) {

        obj << "\t" << ((Basic *)node)->getValue() << "\t"
            << index_to_str(node->getAddr());
        net_obj << "\t" << ((Basic *)node)->getCost() << "\t"
                << index_to_str(node->getAddr()) << '\n';

        out << obj.str() << endl;
        obj.str("");

        first = false;
      } else {
        obj << "+ \t" << ((Basic *)node)->getValue() << "\t"
            << index_to_str(node->getAddr());
        net_obj << "+  \t" << ((Basic *)node)->getCost() << "\t"
                << index_to_str(node->getAddr()) << '\n';

        out << obj.str() << endl;
        obj.str("");
      }

      maximize_deps.push_back(node->getAddr());
    }
  }

  objective = "";
  // Print dependencies for maximize
  for (index_t *idx : maximize_deps) {
    node = getNodeFromIndex(idx);
    edges = node->getSchemeEdges();

    // go through edges print all the edge costs  ( # loops = # of dependencies
    // )
    for (vector<edge_t> vec : *edges) {
      for (edge_t e : vec) {

        index_t idx = EDGE_INDEX(e);

        if (EDGE_VALUE(e) != 0) {
          obj << "+\t" << EDGE_VALUE(e) << "\t" << index_to_str(node->getAddr())
              << '_';
          obj << INDEX_TOP(idx) << '_' << INDEX_LEVEL(idx) << '_'
              << INDEX_BASIC(idx) << endl;

          out << obj.str();
          obj.str("");
        } else if (EDGE_COST(e) != 0) {

          net_obj << "+\t" << EDGE_COST(e) << "\t"
                  << index_to_str(node->getAddr()) << '_';
          net_obj << INDEX_TOP(idx) << '_' << INDEX_LEVEL(idx) << '_'
                  << INDEX_BASIC(idx) << '\n';
        }
      }
    }
  }
  out << endl;

  // Constraints
  out << "Subject To" << endl;

  int c = 1;
  string constraint = "";

  /*
   * Generate constraints as follows -
   * All basic nodes in a service - top node of service =0
   * All basic nodes in a service - levels/implementations of a service = 0
   * All levels in a service - top node of service = 0
   * All basic nodes - weighted edges = 0
   */
  vector<index_t *> top_nodes;
  for (auto it = rsdg.begin(); it != rsdg.end(); it++) {
    index_t *svc_idx = it->second->getAddr();

    // Top node
    if (it->second->isTop()) {
      top_nodes.push_back(it->second->getAddr());

      // For each top node, store the basic and level nodes in a list
      // Why ? Causing issues with how the actual format looks like
      vector<index_t *> level_nodes, basic_nodes;

      // Find nodes for each service
      for (auto svc_node = rsdg.begin(); svc_node != rsdg.end(); svc_node++) {

        index_t *cur_idx = svc_node->second->getAddr();
        constraint = "";

        if (INDEX_TOP(*cur_idx) ==
            INDEX_TOP(*svc_idx)) { // Belong to same service

          // level node
          if (svc_node->second->isLevel()) {
            level_nodes.push_back(cur_idx); // save level node
          }
          // basic node
          else if (svc_node->second->isBasic()) {
            basic_nodes.push_back(cur_idx);
          }
        }
      }

      // 1. All basic nodes in a service - top of service = 0     // change to
      // for (index_t *basic : basic_nodes)
      for (auto it = basic_nodes.begin(); it != basic_nodes.end(); it++) {
        constraint = constraint + " " + index_to_str(*it) + " +";
      }

      // generate string
      constraint = "c" + to_string(c) + ": " + constraint;
      c++;
      constraint = constraint.substr(0, constraint.find_last_of(' '));
      constraint =
          constraint + " - S_" + to_string(INDEX_TOP(*svc_idx)) + " = 0";

      out << constraint << endl;

      // 2. All basic nodes in a service - level nodes of a service = 0
      for (index_t *level : level_nodes) {
        constraint = "c" + to_string(c) + ": ";
        c++;

        for (index_t *basic : basic_nodes) {
          if (INDEX_LEVEL(*basic) == INDEX_LEVEL(*level)) {
            constraint = constraint + " " + index_to_str(basic) + " +";
          }
        }

        constraint = constraint.substr(0, constraint.find_last_of(' '));
        // constraint = constraint + " - S_" + to_string(INDEX_TOP(*level)) + "
        // = 0";
        constraint = constraint + " - " + index_to_str(level) + " = 0";

        out << constraint << endl;
      }

      // 3. All levels in a service - top node of service = 0
      constraint = "c" + to_string(c) + ": ";
      c++;

      for (index_t *level : level_nodes) {
        constraint = constraint + " " + index_to_str(level) + " +";
      }
      constraint = constraint.substr(0, constraint.find_last_of(' '));
      constraint =
          constraint + " - S_" + to_string(INDEX_TOP(*svc_idx)) + " = 0";

      out << constraint << endl;

      // 4. Basic node - all weighted edges from it = 0
      int e = 0;
      for (index_t *b_idx : basic_nodes) {
        constraint = "c" + to_string(c) + ": ";
        e = 0;
        Basic *b_node = (Basic *)getNodeFromIndex(b_idx);

        if (b_node->getSchemeEdges()->size() != 0) {
          constraint += " " + index_to_str(b_idx) + " -";

          for (vector<edge_t> edges : *(b_node->getSchemeEdges())) {
            for (edge_t edge : edges) {

              if (EDGE_COST(edge) != 0) {
                e = 1;
                index_t e_idx = EDGE_INDEX(edge);
                constraint += " " + index_to_str(b_idx) + "_" +
                              to_string(INDEX_TOP(e_idx)) + "_" +
                              to_string(INDEX_LEVEL(e_idx)) + "_" +
                              to_string(INDEX_BASIC(e_idx)) + " -";
              }
            }
          }

          if (e == 1) {
            c++;
            constraint = constraint.substr(0, constraint.find_last_of("-"));
            constraint += "= 0";

            out << constraint << endl;
          }
        }
      }

      // 5. AND edges : Source - sink >= 0
      for (index_t *b_idx : basic_nodes) {
        Basic *b_node = (Basic *)getNodeFromIndex(b_idx);

        for (vector<edge_t> edges : *(b_node->getSchemeEdges())) {

          // size = 1 means AND edge
          if (edges.size() == 1) {
            index_t e_idx = EDGE_INDEX(edges[0]);
            constraint = "c" + to_string(c++) + ":  ";
            constraint += to_string(INDEX_TOP(e_idx)) + "_" +
                          to_string(INDEX_LEVEL(e_idx)) + "_" +
                          to_string(INDEX_BASIC(e_idx)) + " - ";
            constraint += index_to_str(b_idx) + " >= 0";

            out << constraint << endl;
          }
          // 6. OR edges : All sources - sink >= 0
          else {
            constraint = "c" + to_string(c++) + ":  ";

            // Sum of all OR edges
            for (edge_t edge : edges) {
              constraint += index_to_str(&EDGE_INDEX(edge)) + " + ";
            }
            // Truncate extra '+'
            constraint = constraint.substr(0, constraint.find_last_of('+'));
            constraint += " - " + index_to_str(b_idx) + " >= 0";

            out << constraint << endl;
          }

          // 7. Weighted Edge - Source <= 0
          for (edge_t edge : edges) {
            if (EDGE_COST(edge) != 0) {

              constraint = "c" + to_string(c++) + ":  ";
              string source =
                  index_to_str(&EDGE_INDEX(edge)).substr(1, string::npos);
              constraint +=
                  index_to_str(b_idx) + source + " - S" + source + " <= 0";

              out << constraint << endl;
            }
          }
        }
      } // end for basic edge
    }
  }

  // 8. overall constraint with energy
  constraint = "\nc" + to_string(c) + ":  " + net_obj.str() + " - energy = 0\n";
  out << constraint << endl;
  c++;

  // Set energy constraint
  cout << "Enter Budget: " << endl;
  string budget;
  cin >> budget;

  // No sanity check here .. it will just put garbage if you type in garbage
  constraint = "\nc" + to_string(c) + ": energy <= " + budget;
  out << constraint << endl;

  // Bounds
  out << endl << "Bounds" << endl;
  string bounds = "";

  for (auto it = rsdg.begin(); it != rsdg.end(); it++) {
    index_t *gen_idx = it->second->getAddr();

    bounds = "S_" + to_string(INDEX_TOP(*gen_idx));

    if (INDEX_LEVEL(*gen_idx) != 0)
      bounds += "_" + to_string(INDEX_LEVEL(*gen_idx));
    if (INDEX_BASIC(*gen_idx) != 0)
      bounds += "_" + to_string(INDEX_BASIC(*gen_idx));

    bounds += " <= 1\n";

    out << bounds;

    // print weighted edges
    vector<vector<edge_t>> *edges = it->second->getSchemeEdges();
    if (edges->size() != 0) {

      // if edges exist , check for weighted edges
      // Basic *basic = (Basic *)it->second; // has to be basic if it has edges
      for (size_t i = 0; i < edges->size(); i++) {
        for (edge_t edge : edges->at(i)) {
          if (EDGE_COST(edge) != 0) {
            bounds = "";
            bounds = index_to_str(it->second->getAddr()) + "_" +
                     index_to_str(&EDGE_INDEX(edge));
            bounds += " <= 1\n";

            out << bounds;
          }
        }
      }
    }
  }

  // Integers
  out << endl << "Integers" << endl;
  string ints = "";

  for (auto it = rsdg.begin(); it != rsdg.end(); it++) {
    index_t *gen_idx = it->second->getAddr();

    ints = "S_" + to_string(INDEX_TOP(*gen_idx));

    if (INDEX_LEVEL(*gen_idx) != 0)
      ints += "_" + to_string(INDEX_LEVEL(*gen_idx));
    if (INDEX_BASIC(*gen_idx) != 0)
      ints += "_" + to_string(INDEX_BASIC(*gen_idx));

    out << ints << endl;
    ints.clear();
  }

  out << endl << "End";
  out.close();
}

// Go through XML format RSDG build data structure
void RSDG::parseXML(string infile) {

  // temporary macros for readability (might come up with better ways
#define FOR_EACH_SVC_NODE(root_node)                                           \
  for (xml_node<> *svc_node = root_node->first_node("knob"); svc_node;      \
       svc_node = svc_node->next_sibling("knob"))

#define FOR_EACH_LEVEL_NODE(svc_node)                                          \
  for (xml_node<> *level_node = svc_node->first_node("knoblayer");          \
       level_node; level_node = level_node->next_sibling("knoblayer"))

#define FOR_EACH_BASIC_NODE(level_node)                                        \
  for (xml_node<> *basic_node = level_node->first_node("basicnode");           \
       basic_node; basic_node = basic_node->next_sibling("basicnode"))

  // Read in the xml list
  string content;
  stringstream buffer;
  xml_document<> doc;
  ifstream xml_file;
  Top *cur_top = NULL;
  Level *cur_level = NULL;

  unsigned short level = 0; // Level index

  try {
    // xml_file.open(infile);
    xml_file.open(infile);
  } catch (ios_base::failure fail) {
    // something went wrong
    cout << "Could not open file " << infile << endl;
    cout << fail.what() << endl;
    return;
  }

  buffer << xml_file.rdbuf();
  xml_file.close();

  content = (buffer.str()); // save buffer in string
  doc.parse<0>(&content[0]);

  // Whole Doc ...
  xml_node<> *root_node = doc.first_node(); // resource tag...

  if (root_node == NULL) {
    cout << "Could not begin parsing XML file. Possibly wrong format" << endl;
  }

  // get each service tag node saved in xml_node<> svc_node
  FOR_EACH_SVC_NODE(root_node) {
    level = 0;

    string svc_name = svc_node->first_node("knobname")->value();

    if (svc_name.compare("") != 0) {
      //    cout << "service name " << svc_name << endl;
    }

    cur_top = new Top(svc_name);
    xml_rsdg.push_back(cur_top);

    // get each servicelayer tag node save in xml_node<> level_node
    FOR_EACH_LEVEL_NODE(svc_node) {

      string name = svc_name + "_" + to_string(level);
      level++;

      cur_level = new Level(name);
      cur_top->addLevelNode(cur_level);

      // get each basic node save in xml_node<> basic_node
      FOR_EACH_BASIC_NODE(level_node) {
        Basic *basic = new Basic("");
        get_basic_node_info(basic_node, basic);

        cur_level->addBasicNode(basic);
      }
    }
  }
}

// C++ transcribe of Soham's lp writing functions
void RSDG::writeXMLLp(string outfile, bool lp) {
  // Node *node = NULL;
  ofstream out(outfile);
  // vector<vector<xml_edge_t>> *edges;
  stringstream obj, net_obj, net_quadvalue,
      net_quadobj; // net_obj is the final representation for cost function
  string objective, net_objective;
  vector<string> segments;
  // Maximize {{{
  if (minmax) {
    if (!lp)
      out << "Maximize" << endl;
    else
      out << "max:" << endl;
  }
  // Minimize
  else {
    if (!lp)
      out << "Minimize" << endl;
    else
      out << "min:" << endl;
  }
  net_quadobj << "[ ";
  net_quadvalue << "[ ";
  for (Top *top : xml_rsdg) {
    for (Level *lvl : *(top->getLevelNodes())) {
      for (Basic *b : *(lvl->getBasicNodes())) {
        // setup the mv function
        if (!(b->isContinuous())) {
          if (obj.str().size() == 0) {
            obj << b->getValue() << " " << b->getName();
          } else {
            obj << " + " << b->getValue() << " " << b->getName();
          }
        } else {
          // continuous nodes
          string node_name = b->getName();
          if (b->isPieceWise()) {
            // TODO!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
            for (std::map<string, pair<float, float>>::iterator it =
                     b->segment_mvvalues.begin();
                 it != b->segment_mvvalues.end(); it++) {
              string segID = it->first;
              string segVal = segID + "_V";
              float linearC = it->second.first;
              float constC = it->second.second;
              obj << " + " << linearC << " " << segVal << " + " << constC << " "
                  << segID;
            }

          } else { // quad
            vector<double> node_value_orders;
            b->getValueOrder(node_value_orders);
            int i = 0;
            if (node_value_orders[i] >= THRESHOLD ||
                node_value_orders[i] <= -THRESHOLD) {
              if (net_quadvalue.str().size() >= 3)
                net_quadvalue << " + ";
              net_quadvalue << node_value_orders[i] << " " << node_name
                            << " ^ 2 ";
            }
            i++;
            if (node_value_orders[i] >= THRESHOLD ||
                node_value_orders[i] <= -THRESHOLD) {
              if (obj.str().size() > 0)
                obj << " + ";
              obj << node_value_orders[i] << " " << node_name << " ";
            }
            i++;
            if (node_value_orders[i] >= THRESHOLD ||
                node_value_orders[i] <= -THRESHOLD) {
              if (obj.str().size() > 0)
                obj << " + ";
              obj << node_value_orders[i] << " "
                  << "indicator "
                  << "\n";
            }
          }
          // write the coeff
          map<string, float> mvcoeffs = b->getMVCoeffs();
          for (auto coeff : mvcoeffs) {
            string coeff_name = coeff.first;
            float coeff_value = coeff.second;
            net_quadvalue << " + " << coeff_value << " " << coeff_name << " * "
                          << node_name << " ";
          }
          // write the piececoeff
          map<string, vector<float>> piecemv = b->piecemvcoeffs;
          for (auto coeff : piecemv) {
            string coeff_name = coeff.first;
            float a = coeff.second[0];
            float b = coeff.second[1];
            float c = coeff.second[2];
            net_quadvalue << " + " << a << " " << node_name << " ^ 2 + " << b
                          << " " << coeff_name << " ^ 2 + " << c << " "
                          << coeff_name << " * " << node_name << " ";
          }
        }
        // setup the cost function
        if (!(b->isContinuous())) {
          if (net_obj.str().size() != 0)
            net_obj << " + " << b->getCost() << " " << b->getName();
          else
            net_obj << b->getCost() << " " << b->getName();
        } else {
          string node_name = b->getName();
          if (b->isPieceWise()) {
            for (std::map<string, pair<float, float>>::iterator it =
                     b->segment_costvalues.begin();
                 it != b->segment_costvalues.end(); it++) {
              string segID = it->first;
              string segVal = segID + "_V";
              float linearC = it->second.first;
              float constC = it->second.second;
              net_obj << " + " << linearC << " " << segVal << " + " << constC
                      << " " << segID;
            }

          } else {
            // continuous nodes
            vector<double> node_cost_orders;
            b->getCostOrder(node_cost_orders);
            int i = 0;
            if (node_cost_orders[i] >= THRESHOLD ||
                node_cost_orders[i] <= -THRESHOLD) {
              if (net_quadobj.str().size() >= 3)
                net_quadobj << " + ";
              net_quadobj << node_cost_orders[i] << " " << node_name << " ^ 2 ";
            }
            i++;
            if (node_cost_orders[i] >= THRESHOLD ||
                node_cost_orders[i] <= -THRESHOLD) {
              if (net_obj.str().size() > 0)
                net_obj << " + ";
              net_obj << node_cost_orders[i] << " " << node_name << " ";
            }
            i++;
            if (node_cost_orders[i] >= THRESHOLD ||
                node_cost_orders[i] <= -THRESHOLD) {
              if (net_obj.str().size() > 0)
                net_obj << " + ";
              net_obj << node_cost_orders[i] << " "
                      << "indicator "
                      << "\n";
            }
          }
          // write the coeff
          map<string, float> coeffs = b->getCoeffs();

          for (auto coeff : coeffs) {
            string coeff_name = coeff.first;
            float coeff_value = coeff.second;
            net_quadobj << " + " << coeff_value << " " << coeff_name << " * "
                        << node_name << " ";
          }
          // write the piecewise coeff
          map<string, vector<float>> piecemv = b->piececoeffs;
          for (auto coeff : piecemv) {
            string coeff_name = coeff.first;
            float a = coeff.second[0];
            float b = coeff.second[1];
            float c = coeff.second[2];
            net_quadobj << " + " << a << " " << node_name << " ^ 2 + " << b
                        << " " << coeff_name << " ^ 2 + " << c << " "
                        << coeff_name << " * " << node_name << " ";
          }
        }
        // print all edge costs
        // modified by Liu, now that we don't have edge costs
        /*                for(vector<xml_edge_t> vec : *(b->getXMLEdges())){
         for(xml_edge_t e : vec) {
         if(EDGE_VALUE(e) != 0) {
         net_obj << "\t" << EDGE_VALUE(e) << " " << b->getName();
         net_obj << "$" << EDGE_NAME(e) << "\n+";
         }
         }
         }*/
      }
    }
  }
  net_quadobj << "]";
  net_quadvalue << "]";
  // combine the linear and quad cost

  if (net_quadobj.str().size() > 3) {
    net_obj << " + " << net_quadobj.str();
  }
  if (net_quadvalue.str().size() > 3) {
    obj << " + " << net_quadvalue.str();
  }

  if (minmax) // maximize MV
    out << obj.str();
  else // minimize cost
    out << net_obj.str();
  if (lp)
    out << ";";
  out << endl;

  // }}}

  /*
   * Generate constraints as follows -
   * 1. All basic nodes in a service - top node of service =0
   * 2. All levels in a service - top node of service =0
   * 3. All basic nodes in a service - levels/implementations of a service =0
   * 4. Sink - all weighted edges = 0
   * 5. AND Edges: Source - Sink >= 0
   * 6. OR  Edges: All Sources - Sink >=0
   * 6.5 Or out Edges: All out edges sum to 1
   * 7. Weighted Edge - Source <=0
   * 8. Overall Energy constraint
   */

  // CONSTRAINTS
  // MIN MAX remains unchanged
  if (!lp)
    out << "Subject To" << endl;

  int c = 1;
  stringstream constraint;
  // 1. All basic nodes in a service - top node of service =0 NOT NECCESSARY
  /*    for (Top *top : xml_rsdg) {
   constraint.str("");
   constraint << "c" << c++ << ": ";

   for (Level *lvl : *(top->getLevelNodes())) {
   for (Basic *b : *(lvl->getBasicNodes())) {
   constraint << b->getName() << " + ";
   }
   }

   out << constraint.str().substr(0, constraint.str().find_last_of('+'));
   out << "- " << top->getName() << " = 0";
   if(lp)out<<";";
   out<<endl;
   }
   out << endl;*/

  // 2. All levels in a service - top node of service = 0
  for (Top *top : xml_rsdg) {
    constraint.str("");
    constraint << "c" << c++ << ": ";

    for (Level *lvl : *(top->getLevelNodes())) {
      constraint << lvl->getName() << " + ";
    }

    out << constraint.str().substr(0, constraint.str().find_last_of('+'));
    out << "- " << top->getName() << " = 0";
    if (lp)
      out << ";";
    out << endl;
  }
  out << endl;

  // 3. All basic nodes in a service - levels/implementations of svc = 0
  for (Top *top : xml_rsdg) {
    for (Level *lvl : *(top->getLevelNodes())) {
      bool isCont = false;
      constraint.str("");
      constraint << "c" << c++ << ": ";
      for (Basic *b : *(lvl->getBasicNodes())) {
        if (!b->isContinuous())
          constraint << b->getName() << " + ";
        else {
          // continuous single node service
          constraint << lvl->getName() << " = 1 -> " << b->getName() << " >= 0";
          isCont = true;
          break;
        }
      }
      if (!isCont) {
        out << constraint.str().substr(0, constraint.str().find_last_of('+'));
        out << "- " << lvl->getName() << " = 0";
      } else {
        out << constraint.str();
      }
      if (lp)
        out << ";";
      out << endl;
    }
  }

  // Added by Liu: continuous edges
  stringstream constraintCont;
  for (Top *top : xml_rsdg) {
    // get the service name
    string service_name = top->getName();
    for (Level *lvl : *(top->getLevelNodes())) {
      for (Basic *b : *(lvl->getBasicNodes())) {
        string bName = b->getName();
        constraintCont.str("");
        vector<string> segment_for_this_service;
        // All continuous edges
        int segID = 0;
        for (vector<cont_edge_t> vec : *(b->getContEdges())) {
          // each vec is a group of or edges
          if (vec.size() < 1)
            continue;
          for (cont_edge_t e : vec) {
            // each e is a range edge
            // setup a variable indicating the range
            string sourceBname = get<2>(e);
            float sinkMin = get<0>(e);
            float sinkMax = get<1>(e);
            float sourceMin = get<3>(e);
            float sourceMax = get<4>(e);
            string segmentName = service_name + to_string(segID++);
            segments.push_back(segmentName);
            segment_for_this_service.push_back(segmentName);
            // segment = 1 -> min<source_basic<max, min<sink_basic<max
            constraintCont << "c" << c++ << ":" << segmentName << " = 1 -> "
                           << bName << " >= " << sinkMin << endl;
            constraintCont << "c" << c++ << ":" << segmentName << " = 1 -> "
                           << bName << " <= " << sinkMax << endl;
            constraintCont << "c" << c++ << ":" << segmentName << " = 1 -> "
                           << sourceBname << " >= " << sourceMin << endl;
            constraintCont << "c" << c++ << ":" << segmentName << " = 1 -> "
                           << sourceBname << " <= " << sourceMax << endl;
          }
        }
        // service = 1 -> sum_of_segments = 1
        string segments_string = "";
        if (segment_for_this_service.size() == 0)
          continue;
        segments_string += segment_for_this_service[0];
        for (int i = 1; i < segment_for_this_service.size(); i++) {
          segments_string += " + ";
          segments_string += segment_for_this_service[i];
        }
        constraintCont << "c" << c++ << ":" << service_name << " = 1 -> "
                       << segments_string << " <= 1" << endl;
        out << constraintCont.str();
      }
    }
  }

  // Added by Liu: continuous segments
  stringstream constraintContSeg;
  for (Top *top : xml_rsdg) {
    // get the service name
    string service_name = top->getName();
    for (Level *lvl : *(top->getLevelNodes())) {
      for (Basic *b : *(lvl->getBasicNodes())) {
        string bName = b->getName();
        if (b->isPieceWise()) {
          constraintContSeg.str("");
          string sum_of_segments = "";
          for (std::map<string, pair<float, float>>::iterator it =
                   b->segments.begin();
               it != b->segments.end(); it++) {
            string segID = it->first;
            string segVal = segID + "_V";
            float min = it->second.first;
            float max = it->second.second;
            constraintContSeg << "c" << c++ << ":" << segID << " = 1 -> "
                              << bName << " <= " << max << endl;
            constraintContSeg << "c" << c++ << ":" << segID << " = 1 -> "
                              << bName << " >= " << min << endl;
            constraintContSeg << "c" << c++ << ":" << segID << " = 1 -> "
                              << segVal << " - " << bName << " = " << 0 << endl;
            constraintContSeg << "c" << c++ << ":" << segID << " = 0 -> "
                              << segVal << " = 0" << endl;
            sum_of_segments += segID + " + ";
          }
          sum_of_segments =
              sum_of_segments.substr(0, sum_of_segments.length() - 2);
          constraintContSeg << "c" << c++ << ":" << sum_of_segments << " = 1"
                            << endl;
          out << constraintContSeg.str();
        }
      }
    }
  }

  // 4. Sink - all weighted edges = 0
  // 5. AND Edges: Source - Sink >= 0
  // 6. OR  Edges: All Sources - Sink >=0
  // 7. Weighted Edge - Source <=0

  stringstream constraint4;
  stringstream constraint65;
  for (Top *top : xml_rsdg) {
    for (Level *lvl : *(top->getLevelNodes())) {
      for (Basic *b : *(lvl->getBasicNodes())) {
        constraint4.str("");

        // All edges
        for (vector<xml_edge_t> vec : *(b->getXMLEdges())) {
          if (vec.size() < 1)
            continue;
          for (xml_edge_t e : vec) {
            if (EDGE_VALUE(e) >= 0) {
              // Sink - all weighted edges = 0
              constraint4 << b->getName() << "$" << EDGE_NAME(e) << " - ";

              // Weighted Edge - Source <= 0
              out << "c" << c++ << ": " << b->getName() << "$" << EDGE_NAME(e);
              out << " - " << EDGE_NAME(e) << " <= 0";
              if (lp)
                out << ";";
              out << endl;
            }
            // And edges : Source - Sink >= 0
            if (vec.size() == 1) {
              out << "c" << c++ << ": " << EDGE_NAME(e) << " - " << b->getName()
                  << " >= 0";
              if (lp)
                out << ";";
              out << endl;
            }
          }

          // OR Edges: All Sources - Sink >= 0
          if (vec.size() > 1) {
            stringstream constraint6("");

            for (xml_edge_t e : vec) {
              constraint6 << b->getName() << "$" << EDGE_NAME(e) << " + ";
            }
            out << "c" << c++ << ": ";
            out << constraint6.str().substr(
                0, constraint6.str().find_last_of('+'));
            out << "- " << b->getName() << " >= 0";
            if (lp)
              out << ";";
            out << endl;
          }
        }
        if (constraint4.str() != "") {
          out << "c" << c++ << ": " << b->getName() << " - ";
          out << constraint4.str().substr(0,
                                          constraint4.str().find_last_of('-'))
              << " = 0";
          if (lp)
            out << ";";
        }
        out << endl;
        // 6.5 or out edges
        for (vector<xml_edge_t> vec : *(b->getXMLOutEdges())) {
          if (vec.size() < 1)
            continue;
          constraint65.str("");
          string curEdgeName = "";
          for (xml_edge_t e : vec) {
            constraint65 << EDGE_NAME(e) << "$" << b->getName() << " + ";
          }
          out << "c" << c++ << ": "
              << constraint65.str().substr(0,
                                           constraint65.str().find_last_of('+'))
              << " <= 1";
          if (lp)
            out << ";";
          out << endl;
        }
      }
    }
  }

  // 8. Overall Energy constraint
  // 8. Overall MV
  string budget = to_string(this->budget);
  string mv = to_string(this->targetMV);

  // sometimes it would lead to gurobi failure: so sacrifice this min/max
  // problem
  //    if(minmax){
  out << "c" << c++ << ": ";
  // out<< "- energy + " << net_obj.str();
  // out << " >= -0.001\n\n";
  out << net_obj.str() << " <= " << budget;
  /*
   out << "c" << c++ << ": " ;
   out<< "- energy + " << net_obj.str();
   out << " <= 0.001\n";*/
  /*    }
   else{
   out << "c" << c++ << ": " << obj.str();
   out << "- mv = 0";
   }*/
  if (lp)
    out << ";";
  out << endl;

  /*    if(minmax)
   out << "c" << c++ << ": energy <= " << budget;
   else
   out << "c" << c++ << ": mv >= " << mv;
   if(lp)out<<";";out << endl;
   */
  // 9.5. ForceOn constraints
  for (string on : forceOn) {
    out << "c" << c++ << ":" << on << " = 1";
    if (lp)
      out << ";";
    out << endl;
  }

  // 9.6. Temp ForceOn constraints
  for (string on : forceOnTmp) {
    out << "c" << c++ << ":" << on << " = 1";
    if (lp)
      out << ";";
    out << endl;
  }

  // Bounds
  out << endl << "Bounds" << endl;

  stringstream integers;
  stringstream generals;
  for (Top *top : xml_rsdg) {
    // out << top->getName() << " <= 1";
    if (lp)
      out << ";" << endl;
    integers << top->getName() << endl;

    for (Level *lvl : *(top->getLevelNodes())) {
      //    out << lvl->getName() << " <= 1";
      if (lp)
        out << ";" << endl;
      integers << lvl->getName() << endl;

      for (Basic *b : *(lvl->getBasicNodes())) {
        //        if(!b->isContinuous())out << b->getName() << " <= 1";
        if (lp)
          out << ";" << endl;
        //        if(!b->isContinuous())integers << b->getName() << endl;
        if (b->isContinuous()) {
          // this is a continuous service
          //            out<< b->getName() <<" >= 0"<<endl;
          generals << b->getName() << endl;
          // write the minmax boundaries if any
          if (b->hasMinMax()) {
            out << b->getMinValue() << " <= " << b->getName()
                << " <= " << b->getMaxValue() << "\n";
          }
          if (b->isPieceWise()) {
            for (auto it = b->segments.begin(); it != b->segments.end(); it++) {
              integers << it->first << endl;
            }
          }
        }

        for (vector<xml_edge_t> vec : *(b->getXMLEdges())) {
          for (xml_edge_t e : vec) {
            if (EDGE_VALUE(e) != 0) {
              integers << b->getName() << "$" << EDGE_NAME(e) << endl;
              //                    out << b->getName() << "$" << EDGE_NAME(e)
              //                    << " <= 1";
              if (lp)
                out << ";" << endl;
            }
          }
        }
      }
    }
  }

  // indicator should be 1
  out << "indicator = 1" << endl;

  // GENERALS
  if (!lp) {
    out << endl << "Generals" << endl;
    out << generals.str();
  }

  // INTEGERS
  if (!lp)
    out << endl << "Binaries" << endl;
  else
    out << endl << "int" << endl;
  out << integers.str();
  // add all segments to binaries
  for (string seg : segments) {
    out << seg << endl;
  }
  if (lp)
    out << ";";
  out << endl;
  if (!lp)
    out << "End";
  out.close();
}

void RSDG::addOn(string name) { forceOn.push_back(name); }

// temporarily add in new constraint
void RSDG::addTmpOn(string name) { forceOnTmp.push_back(name); }

void RSDG::resetTmpOn() { forceOnTmp.clear(); }

void RSDG::addOff(string name) { forceOff.push_back(name); }
vector<string> RSDG::getOn() { return forceOn; }
vector<string> RSDG::getOff() { return forceOff; }
