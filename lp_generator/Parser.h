#include "rapidxml.hpp"
#include "KDG.h"

using namespace std;
using namespace rapidxml;

class Parser{
private:
    KDG *graph_;
    float budget_;
    string appName_;
    void getBasicNodeInfo(xml_node<> *xml_bnode, Basic *basic); // example impl of parsing a basic node field
    string genKnobConstraints(); // generate the knob constraints (dependencies)
    string genbudgetConstraint(); // generate the budget constraint
    string genObjectiveFunction(); // generate the objective function (quality)
    string genBinaries(); // all the LP variables should be binary
    
public:
    Parser(string appName);
    void writeLp(string output);                  // lp from XML
    void setBudget(float budget);                // Set energy budget
    void genKDGwithXML(string input);        // generate the internal KDG with XML input
};
