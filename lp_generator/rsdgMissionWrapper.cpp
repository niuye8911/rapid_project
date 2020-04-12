#include "rsdgMissionWrapper.h"
#include "rsdgMission.h"
#include <string>
extern "C" {

// C-wrapper for constructor
void *newRAPIDMission(char *name, int from_file) {
  string app_name(name);
  bool from_config = from_file == 1;
  rsdgMission *mission = new rsdgMission(app_name, from_config);
  return (void *)mission;
}

// for para constructor
void *newRAPIDPara() {
  rsdgPara *para = new rsdgPara();
  return (void *)para;
}

int getParaVal(void *para) {
  rsdgPara *target_para = (rsdgPara *)para;
  return target_para->intPara;
}

// generateProb()
void read_rsdg(void *mission, char *file) {
  string input(file);
  rsdgMission *target_mission = (rsdgMission *)mission;
  target_mission->generateProb(input);
}

void print_prob(void *mission, char *file) {
  string output(file);
  rsdgMission *target_mission = (rsdgMission *)mission;
  target_mission->printProb(output);
}

// regContService
void regContService(void *mission, char *service_name, char *node_name,
                    void *(*func)(void *), void *para) {
  rsdgMission *targetMission = (rsdgMission *)mission;
  string service(service_name);
  string node(node_name);
  rsdgPara *para_cpp = (rsdgPara *)para;
  targetMission->regContService(service, node, func, para_cpp);
}

void setDebug(void *mission) {
  rsdgMission *targetMission = (rsdgMission *)mission;
  targetMission->setDebug();
}

// regService
void regService(void *mission, char *service_name, char *node_name,
                void *(*func)(void *), int single, void *para, int para_value) {
  rsdgMission *targetMission = (rsdgMission *)mission;
  string service(service_name);
  string node(node_name);
  bool single_or_not = single == 1;
  rsdgPara *para_cpp = (rsdgPara *)para;
  targetMission->regService(service, node, func, single_or_not,
                            make_pair(para_cpp, para_value));
}

// updateMV
void updateMV(void *mission, char *name, int mv, int exp) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  string service_name(name);
  bool is_exp = exp == 1;
  target_mission->updateMV(service_name, mv, is_exp);
}

// setBudget
void setBudget(void *mission, int budget) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  target_mission->setBudget(budget);
}

// setSolver
void setSolver(void *mission, int gurobi, int local, int rapidm) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  bool is_gurobi = gurobi == 1;
  bool is_local = local == 1;
  bool is_rapidm = rapidm == 1;
  target_mission->setSolver(is_gurobi, is_local, is_rapidm);
}

// setUnit
void setUnit(void *mission, int unit) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  target_mission->setUnit(unit);
}

// setUnitBetween...
void setUnitBetweenCheckpoints(void *mission, int interval) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  target_mission->setUnitBetweenCheckpoints(interval);
}

// reconfig
void reconfig(void *mission) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  target_mission->reconfig();
}

// finished_one_unit
void finish_one_unit(void *mission) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  target_mission->finish_one_unit();
}

// add a constraint
void addConstraint(void *mission, char *name, int on) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  string constname(name);
  target_mission->addConstraint(constname, on == 1);
}

// set to be in training mode
void setTraining(void *mission) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  target_mission->setTraining();
}

int isFailed(void *mission) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  if (target_mission->isFailed() == 1)
    return 1;
  return 0;
}

void setUpdate(void *mission, int up) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  if (up == 1)
    target_mission->setUpdate(true);
  else
    target_mission->setUpdate(false);
}

void setLogger(void *mission) {
  rsdgMission *target_mission = (rsdgMission *)mission;
  target_mission->setLogger();
}

void setOfflineSearch(void *mission) {
  rsdgMission *target = (rsdgMission *)mission;
  target->setOfflineSearch();
}

void readContTrainingSet(void *mission) {
  rsdgMission *target = (rsdgMission *)mission;
  target->readContTrainingSet();
}
void finish(void *mission) {
  rsdgMission *target = (rsdgMission *)mission;
  target->finish();
}
void start(void *mission) {
  rsdgMission *target = (rsdgMission *)mission;
  target->start();
}
} // extern "C"
