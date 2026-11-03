#include "tutorials.h"

// The content lives in tutorials_c.c (C section) and tutorials_cpp.c (C++ section).
extern const Tutorial C_TUTORIALS[];
extern const int      C_TUTORIAL_COUNT;
extern const Tutorial CPP_TUTORIALS[];
extern const int      CPP_TUTORIAL_COUNT;

int TutorialCount(void) { return C_TUTORIAL_COUNT + CPP_TUTORIAL_COUNT; }

const Tutorial *GetTutorial(int index)
{
    if (index < 0 || index >= TutorialCount()) return &C_TUTORIALS[0];
    if (index < C_TUTORIAL_COUNT) return &C_TUTORIALS[index];
    return &CPP_TUTORIALS[index - C_TUTORIAL_COUNT];
}
