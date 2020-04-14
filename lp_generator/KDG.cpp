#include "KDG.h"
#include <iostream>

using namespace std;

/****** Node ******/

void Node::setName(string n) { name_ = n; }

string Node::getName() { return name_; }


/****** Basic ******/

Basic::Basic(string basicName):cost_(0.0), quality_(0.0) {
    setName(basicName);
}

void Basic::setCost(float val) { cost_ = val; }

void Basic::setQuality(float val) { quality_ = val; }

float Basic::getCost() { return cost_; }

float Basic::getQuality() { return quality_; }

void Basic::addDependency(Basic * source) { dependencies_.push_back(source); }

/****** Level ******/


Level::Level(int lvl):level_(lvl) {}

vector<Basic *> *Level::getBasicNodes() { return &basicNodes_; }

void Level::addBasicNode(Basic *basic) { basicNodes_.push_back(basic); }

int Level::getLevelNum() { return level_; }

Level::~Level() {
    for (Basic *basic : basicNodes_) {
        delete basic;
    }
}


/****** Knob ******/

Knob::Knob(string knob_name){
    setName(knob_name);
}

vector<Level *> *Knob::getLevelNodes() { return &levelNodes_; }

void Knob::addLevelNode(Level *level) { levelNodes_.push_back(level); }

Knob::~Knob() {
    for (Level *level : levelNodes_) {
        delete level;
    }
}


/****** KDG ******/

KDG::KDG(string appName):appName_(appName) {}

void KDG::addKnob(Knob *knob) {
    knobs_.push_back(knob);
}

Node *KDG::getNodeFromName(string name) {
    for (Knob *knob : knobs_) {
        if (knob->getName().compare(name) == 0) {
            return knob;
        }
        for (Level *lvl : *(knob->getLevelNodes())) {
            for (Basic *b : *(lvl->getBasicNodes())) {
                if (b->getName().compare(name) == 0) {
                    return b;
                }
            }
        }
    }
    return NULL;
}

string KDG::getName(){
    return appName_;
}

KDG::~KDG() {
    for (Knob *knob : knobs_) {
        delete knob;
    }
}
