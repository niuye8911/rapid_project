#include "Parser.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;
using namespace rapidxml;

Parser::Parser(string appName):graph_(new KDG(appName)), budget_(-1.), appName_(appName){};

// set any defined basic node fields to their correspondding XML val
void Parser::getBasicNodeInfo(xml_node<> *xml_bnode, Basic *basic) {
    string n_name, n_cost, f_name;
    float cost;
    float quality;
    
    for (xml_node<> *fields = xml_bnode->first_node(); fields;
         fields = fields->next_sibling()) {
        
        if (fields->name() == NULL) {
            continue;
        }
        string f_name = fields->name();
        
        if (f_name.compare("nodename") == 0) {
            if (fields->value() != NULL) {
                basic->setName(fields->value());
                cout << "found node name: " << fields->value() << endl;
            }
        } else if (f_name.compare("cost") == 0) {
            try {
                cost = stof(fields->value());
                cout << "found node cost: " << fields->value() << endl;
            } catch (exception e) { // either out of range or invalid arg
                cost = 0;
            }
            basic->setCost(cost);
        } else if (f_name.compare("quality") == 0) {
            try {
                quality = stof(fields->value());
                cout << "found node quality: " << fields->value() << endl;
            } catch (exception e) { // either out of range or invalid arg
                quality = 0;
            }
            basic->setQuality(quality);
        } else if (f_name.compare("and") == 0) {
            string source_name = fields->value();
            cout << "found And edge with source: " << source_name << endl;
        }
    }
}

// Go through XML format RSDG build data structure
void Parser::genKDGwithXML(string infile) {
    
    // temporary macros for readability
#define FOR_EACH_knob_node(root_node)                                           \
for (xml_node<> *knob_node = root_node->first_node("knob"); knob_node;      \
knob_node = knob_node->next_sibling("knob"))
    cout<<endl;
    
#define FOR_EACH_LEVEL_NODE(knob_node)                                          \
for (xml_node<> *level_node = knob_node->first_node("knoblayer");          \
level_node; level_node = level_node->next_sibling("knoblayer"))
    cout<<endl;
    
#define FOR_EACH_BASIC_NODE(level_node)                                        \
for (xml_node<> *basic_node = level_node->first_node("basicnode");           \
basic_node; basic_node = basic_node->next_sibling("basicnode"))
    cout<<endl;
    
    // Read in the xml list
    string content;
    stringstream buffer;
    xml_document<> doc;
    ifstream xml_file;
    Knob *cur_knob = NULL;
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
    
    // get each service tag node saved in xml_node<> knob
    FOR_EACH_knob_node(root_node) {
        level = 0;
        
        string knob_name = knob_node->first_node("knobname")->value();
        
        if (knob_name.compare("") != 0) {
            cout << "knob name " << knob_name << endl;
        }
        
        cur_knob = new Knob(knob_name);
        
        // get each servicelayer tag node save in xml_node<> level_node
        FOR_EACH_LEVEL_NODE(knob_node) {
            
            string name = knob_name + "_" + to_string(level);
            cout << "getting to level: " << level << endl;
            level++;
            cur_level = new Level(level);
            cur_knob->addLevelNode(cur_level);
            
            // get each basic node save in xml_node<> basic_node
            FOR_EACH_BASIC_NODE(level_node) {
                Basic *basic = new Basic("");
                getBasicNodeInfo(basic_node, basic);
                cout << "finished a basic_node: " << basic->getName() << endl << endl;
            }
        }
    }
}

string Parser::genObjectiveFunction(){
    // generate the obj func
    return "I'm a dummy Objective Function";
}

string Parser::genbudgetConstraint(){
    // generate the obj func
    return "I'm a dummy budget constraint <= "+to_string(budget_);
}

string Parser::genKnobConstraints(){
    // generate the obj func
    return "I'm a dummy knob constraints";
}

string Parser::genBinaries(){
    // generate the list of binaries
    return "I'm a dummy binary constraints";
}

void Parser::setBudget(float budget){
    budget_ = budget;
}

void Parser::writeLp(string outfile_dir) {
    ofstream out(outfile_dir+ appName_+".lp");
    // objective functions
    out << "Maximize" << endl;
    
    out << genObjectiveFunction() << endl;
    
    // constraints
    out << endl << "Subject To" << endl;
    
    // first constraint: budget
    out << genbudgetConstraint() << endl;
    
    // following constraints: node dependency
    out << genKnobConstraints() << endl;
    
    // generals
    out << endl << "Generals" << endl;
    
    out << genBinaries() <<endl;
    
    // end
    out << "End";
    out.close();
}
