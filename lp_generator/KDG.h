#include <map>
#include <string>
#include <vector>

using namespace std;

// Base class for Top, Level, and Basic nodes
class Node {
private:
    string name_;
    
public:
    void setName(string name);
    string getName();
    
};

// Basic nodes - the smallest unit on KDG graph
class Basic : public Node {
private:
    float cost_;
    float quality_;
    vector<Basic *> dependencies_;
    
public:
    Basic(string name);
    void setCost(float cost);
    void setQuality(float quality);
    float getCost();
    float getQuality();
    void addDependency(Basic * source);
};

// Level nodes - a level in a knob which may contain multiple nodes,
// but for this project, you could assume there'll ONLY be 1 node per level
class Level : public Node {
private:
    int level_;
    vector<Basic *> basicNodes_;
    
public:
    Level(int lvl);
    vector<Basic *> *getBasicNodes();
    void addBasicNode(Basic * node);
    int getLevelNum();
    ~Level();
};

// Top nodes - represent a knob
class Knob : public Node {
private:
    vector<Level *> levelNodes_;
public:
    Knob(string name);
    vector<Level *> *getLevelNodes();
    void addLevelNode(Level *);
    ~Knob();
};

// Main Data Structure
class KDG {
private:
    string appName_;
    vector<Knob *> knobs_;
    
public:
    KDG(string app_name);
    void addKnob(Knob *knob);                     // add a knob
    Node *getNodeFromName(string name);           // get a node from the name
    string getName();
    ~KDG();
};
