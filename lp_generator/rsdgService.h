#ifndef RSDGSERVICE_H
#define RSDGSERVICE_H

#include "RSDG.h"
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

using namespace std;

// Class for parameter holder
class rsdgPara {
public:
  int intPara;
};

// Class for work unit
class rsdgUnit {
  int unit;

public:
  void set(int u);
  int get();
};

// Class for a rsdg Service (service >> layer >> node)
class rsdgService {
  string name; // name of the service
  bool READY;
  int bufferReady;
  void *buffer;

  bool DEBUG = false;
  rsdgUnit *unit;                // the total work unit
  rsdgPara *para;                // the rsdgPara associated to this service
  vector<rsdgService *> parents; // for dependency checks, all parents of this
                                 // service in RSDG. parent: ancestors
  pthread_t sThread;             // the thread managing this service
  bool single; // set to true if the service only manages 1 runnable
  vector<string> node_lists; // a list of nodes in this service
  string curNode;

public:
  // Constructor(s)
  rsdgService();
  rsdgService(string);
  // Return the name
  string getName();
  // Set the total work unit
  void setUnit(int m);
  // Return the finished unit
  int getUnit();
  // Return the thread
  pthread_t getThread();
  // Buffer setter(s)
  void setBufferReady() { bufferReady = 1; }
  void setBufferUsed() { bufferReady = 0; }
  void setBufferHold() { bufferReady = -1; }
  // Status setter
  void setStatus(bool b) { READY = b; }
  // Status getter
  int getBufferStatus() { return READY; }
  // Return the parents
  vector<rsdgService *> getParents();
  // Buffer getter
  void *get(pthread_t t);
  // Buffer setter
  void set(pthread_t t, void *obj);
  // Run the runnable associated with the selected node
  void run(string node, void *f(void *));
  // Update the selected node (and activate the new node)
  void updateNode(void *(*)(void *), string);
  // Add a node to the service
  void addNode(string node_name);
  // Set the single indicator
  void setSingle(bool single);
  // Return the single indicator
  bool isSingle();
  // Return the node lists
  vector<string> &getList();
  // Set the curnode
  void setCurNode(string node);
  void setDebug();
  // Logger for service
  void logWarning(string msg);
  void logDebug(string msg);
  void logInfo(string msg);
};
#endif
