#ifdef __cplusplus
extern "C" {
#endif

void *newRAPIDMission(char *, int);
void *newRAPIDPara();

int getParaVal(void *);
void regService(void *, char *, char *, void *(*)(void *), int, void *, int);
void regContService(void *, char *, char *, void *(*)(void *), void *);
void read_rsdg(void *, char *fileName);
void print_prob(void *, char *fileName);
void updateMV(void *, char *name, int mv, int linear);
void setBudget(void *, int totalBudget);
void reconfig(void *);
void finish_one_unit(void *);
void setUnit(void *, int);
void setUnitBetweenCheckpoints(void *, int);
void setSovler(void *, int, int, int);
long long getCurrentTimeInMilli();
void finish_one_unit(void *);
void addConstraint(void *, char *, int);
void setTraining(void *);
int isFailed(void *);
void setLogger(void *);
void setOfflineSearch(void *);
void readContTrainingSet(void *);
void setDebug(void *);
void finish(void *);
void start(void *);
#ifdef __cplusplus
}
#endif
