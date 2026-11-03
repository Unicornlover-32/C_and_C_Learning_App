#ifndef TUTORIALS_H
#define TUTORIALS_H

#include "runner.h"

#define MAX_PAGES 6

typedef struct {
    const char *heading;
    const char *body;   // paragraph text, '\n' starts a new line
    const char *code;   // optional code example (NULL for none)
} LessonPage;

typedef struct {
    const char *title;
    const char *summary;

    int        pageCount;
    LessonPage pages[MAX_PAGES];

    // The practical project at the end of the tutorial
    const char *projectTitle;
    const char *projectBrief;
    const char *starterCode;
    int         testCount;
    TestCase    tests[MAX_TESTS];
} Tutorial;

extern const Tutorial TUTORIALS[];
extern const int TUTORIAL_COUNT;

#endif
