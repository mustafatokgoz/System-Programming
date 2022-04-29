#ifndef MAIN_H
#define MAIN_H

#include "helper.h"
void openSem(int bufferSize);
void createNurses(clinic *biontech);
void createVaccinators(clinic *biontech);
void createCitizens(clinic *biontech);
void createSignalHandler();
void createSignalHandler2();
void cleanAndExit();
void nurse(clinic *biontech, process *process);
void vaccinator(clinic *biontech, process *process);
void citizen(clinic *biontech, process *process);
void removeAll();
void exitHandler(int signal);
void wakeHandler(int signal);
clinic *getSharedMemory(args givenArgs);
void reapDeadChildren();
pid_t *getHelperSharedMem();
#endif