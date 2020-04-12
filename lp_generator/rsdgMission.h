#ifndef RSDGMISSION_H
#define RSDGMISSION_H
#include <map>

#include "rsdgService.h"
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

using namespace std;

extern bool verboseRSDG;

void *startSolver(void *);

long long getCurrentTimeInMilli();

extern string RSDG_TAG;

class rsdgMission {
public:
  const static bool LP_SOLVE = true;
  const static bool GUROBI = false;
  const static bool LOCAL = true;
  const static bool REMOTE = false;
  const static bool RAPIDS = false;
  const static bool RAPIDM = true;
  const string LOG_HEADER =
      "Selection,RC_by_budget,RC_by_result,RC_by_rapidm,Real_"
      "Cost,RC_Time,RC_Num,Budget,Exec,SCALE_UP,SUCCESS,FAILED_REASON\n";

  string app_name;
  bool DEBUG = false; // set to true if debug info is needed
  int numThreads;
  const string pythonScript =
      "/home/ubuntu/parsec-3.0/rsdglib/modelConstr/parseData.py";
  const string rapidScript =
      "/home/liuliu/Research/rapidlib-linux/modelConstr/source/rapid.py";
  double predictedCost = 0.0;
  double realCost = 0.0;

  // FACT file for cost
  ofstream fact;
  // Observed FACT file
  ofstream RS_fact;

  map<string, int> runnableID;
  map<string, int> threadID;
  map<string, string> basicToService;
  map<string, rsdgService *> serviceMap;
  map<string, pair<rsdgPara *, int>> paraList;
  map<string, rsdgPara *> contParaList;
  map<string, vector<string>> depChain; // kto a service's dependence
  map<string, int> threadPref;
  map<string, string> selected;
  map<string, double> offlineCost;
  map<string, double> offlineMV;
  map<string, double> contServiceValue;
  vector<vector<string>> all_configs;
  int curTrainingId = -1;
  int curUpdatingId = -1;
  bool lpsolve;
  bool local;
  bool rapidm;
  bool rush_to_end;
  bool rapidm_started;
  vector<long long> lastCheckPoints;
  vector<long long> timeSinceLastCheckPoints;
  long long startTime = -1;
  bool TRAINING_MODE = false;
  int unitBetweenCheckPoints = 0;
  int finished_unit = 0;
  bool CONT = false;
  ofstream logfile;
  ofstream inputDepFile;
  double cur_budget_per_unit = 0.1;
  int total_unit = 0;

  // mission related;
  RSDG *graph;
  string outfileName;
  int budgetType;
  rsdgUnit unit;
  long budgetTracker;
  int budget;
  int curBudget;
  double objValue;
  pthread_t monitor;
  int num_of_reconfig = 0;
  double total_reconfig_time = 0;
  vector<vector<string>> RS;
  bool offline_search = false;
  bool LOGGER = false;
  bool update = false;
  bool reject = false;
  long freq;
  string xml_path;
  int slowdown_scale_up = 0;
  string failed_reason = "NA";

  // result
  double maxMV;
  double minEnergy;
  bool solvable = true;
  //    RAPID_M related
  double slowdown = 1.0;
  string bucket;
  vector<string> candidate_configs;

  // power saving mode
  bool power_saving_mode = false;
  string lowest_config="";
  double lowest_cost=999999;

  // thread+runnable list
  vector<void *(*)(void *)> runnableList;
  vector<rsdgService *> serviceList;

  void genAllConfigs(int, vector<string>);

private: // private member functions
  void setRushToEnd();
  void parseRunConfig(string config_path);
  void consultServer();
  bool consultServer_M();
  void updateSelection(vector<string> &result);
  bool applyResult();
  bool updateThread(rsdgService *s, string basicNode, double value);
  vector<string> getDep(string);
  void setOutput(string);
  void getRes(vector<string> &, string);
  void logWarning(string msg);
  void logDebug(string msg);
  void logInfo(string msg);
  void printToLog(int, bool, bool, bool);
  void checkPoint(int index = 0);
  void updateModel(int);
  void readProfile(string file_path, bool COST);
  vector<string> searchProfile();
  vector<string> searchProfile(vector<string>);
  void filterConfigs();
  void readRS(string);
  void updateRSDG();
  void resetTimer();
  void update_slowdown_if_almost_done();
  double predicted_cost; // predicted_cost of current selection

public: // public API's
  rsdgMission(string config_file_name, bool from_config = false);
  rsdgService *getService(string name);
  void regService(string, string, void *(*)(void *), bool,
                  pair<rsdgPara *, int>);
  void finalizeRSDG(vector<int> &preference);
  void regService(string, string, void *(*)(void *), bool);
  void regContService(string, string, void *(*)(void *), rsdgPara *);
  void setupSolverFreq(int);
  void setUnit(int);
  void start();
  void generateProb(string);
  void setBudget(int);
  void updateMV(string, int, bool);
  int getBudget();
  void updateWeight(string, double);
  void updateWeight(string, string, double);
  int getNumThreads();
  int printPref(string serviceName);
  void setSolver(bool, bool, bool);
  void finish_one_unit();
  void reconfig();
  void printProb(string);
  void stopSolver();
  void cancel();
  virtual bool updateBudget();
  void reset() {
    selected.clear();
    for (auto it = serviceMap.begin(); it != serviceMap.end(); it++) {
      if (it->second != NULL)
        it->second->setCurNode("");
    }
  }
  void addConstraint(string, bool);
  void addConstraint(string, string, bool);
  vector<string> localSolve();
  double genProductProfile();
  double getObj();
  void setUpdate(bool);
  void setUnitBetweenCheckpoints(int);
  void setTraining();
  void genFact();
  void reconfig_training();
  void reconfig_updating();
  bool validate(vector<string> &);
  bool isFailed();
  void setLogger();
  void setOfflineSearch();
  void readContTrainingSet();
  void setDebug();
  void finish(bool FINISH = true);
  double getFreq();
};
#endif
