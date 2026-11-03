#ifndef TUTORIALS_H
#define TUTORIALS_H

#include "runner.h"

#define MAX_PAGES   6
#define MAX_QUIZ    4
#define MAX_OPTIONS 4

typedef struct {
    const char *heading;
    const char *body;   // paragraph text, '\n' starts a new line
    const char *code;   // optional code example (NULL for none)
} LessonPage;

typedef struct {
    const char *question;
    const char *code;                  // optional code shown under the question
    int         optionCount;
    const char *options[MAX_OPTIONS];
    int         correct;               // index into options
    const char *explanation;           // shown once the right answer is picked
} QuizQuestion;

typedef struct {
    const char *id;       // stable name used in the save file - never change it once released
    Language    lang;
    const char *title;
    const char *summary;

    int        pageCount;
    LessonPage pages[MAX_PAGES];

    int          quizCount;
    QuizQuestion quiz[MAX_QUIZ];

    // The practical project at the end of the tutorial
    const char *projectTitle;
    const char *projectBrief;
    const char *starterCode;
    const char *hint;
    const char *solution;
    int         testCount;
    TestCase    tests[MAX_TESTS];
} Tutorial;

// All tutorials in order: the C section first, then C++.
int             TutorialCount(void);
const Tutorial *GetTutorial(int index);

#endif
