#ifndef PROJECTS_H
#define PROJECTS_H

#include "runner.h"

#define MAX_STEPS 6

// One step of a bigger project. The student's code carries on from step to step,
// so `solution` is the whole program as it should look after this step.
typedef struct {
    const char *title;
    const char *brief;
    const char *hint;
    const char *solution;
    int         testCount;
    TestCase    tests[MAX_TESTS];
} ProjectStep;

typedef struct {
    const char *id;         // stable name used in the save file - never change it once released
    Language    lang;
    const char *title;
    const char *summary;    // one line for the project list
    const char *intro;      // a paragraph for the project overview
    const char *needs;      // which tutorials to do first
    const char *lecture;    // which lecture notes it is based on
    const char *starterCode;

    int         stepCount;
    ProjectStep steps[MAX_STEPS];
} Project;

int            ProjectCount(void);
const Project *GetProject(int index);

#endif
