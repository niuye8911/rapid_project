#include "rsdgService.h"
#include "stdlib.h"
#include "string.h"
#include <ctime>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <unistd.h>

void rsdgUnit::set(int u) { unit = u; }

int rsdgUnit::get() { return unit; }

//////rsdgService////

rsdgService::rsdgService(std::string serviceName) {
  name = serviceName;
  READY = true;
  bufferReady = 2;
  unit = new rsdgUnit();
  parents.resize(10);
}

string rsdgService::getName() { return name; }

int rsdgService::getUnit() { return unit->get(); }

vector<rsdgService *> rsdgService::getParents() { return parents; }

void *rsdgService::get(pthread_t getter) {
  while (bufferReady != 1) {
    sleep(1000);
  }
  logDebug("buffer ready to be taken");
  setBufferUsed();
  return buffer;
}

void rsdgService::set(pthread_t setter, void *obj) {
  buffer = obj;
  setBufferReady();
  logDebug("buffer changed");
}

void rsdgService::run(string name, void *f(void *)) {
  setCurNode(name);
  pthread_create(&sThread, NULL, f, NULL);
  return;
}

void rsdgService::setCurNode(string node) { curNode = node; }

void rsdgService::setSingle(bool s) { single = s; }

bool rsdgService::isSingle() { return single; }

void rsdgService::updateNode(void *(*f)(void *), string name) {
  //#TODO: updateNode should consider discrete and continuous nodes separately
  /*if(name==curNode){
   cout<<RSDG_TAG+"same config:"<<curNode<<"->"<<name<<endl;
   return;
   }*/
  logInfo("change config:" + curNode + "->" + name);
  if (f != NULL) {
    if (isSingle()) {
      f(NULL);
      curNode = name;
      return;
    }
    if (sThread != 0) {
      // if (pthread_kill(sThread, 0) == 0) {
      pthread_cancel(sThread);
      setStatus(false);
      pthread_join(sThread, NULL);
      //  }
    }
    pthread_create(&sThread, NULL, f, NULL);

    curNode = name;
  }
}

pthread_t rsdgService::getThread() { return sThread; }

void rsdgService::setDebug() { DEBUG = true; }

void rsdgService::logWarning(string msg) {
  cout << "RSDGService-WARNING!:" + msg;
}

void rsdgService::logDebug(string msg) {
  if (DEBUG)
    cout << "RSDGService-DEBUG:" + msg;
}

void rsdgService::logInfo(string msg) { cout << "RSDGService-INFO:" + msg; }
