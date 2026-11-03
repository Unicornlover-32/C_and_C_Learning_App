#ifndef PROGRESS_H
#define PROGRESS_H

#include <stdbool.h>
#include "editor.h"

#define MAX_TUTORIALS 32
#define PROGRESS_FILE "cfd_progress.txt"

// Everything the app remembers between runs, indexed like GetTutorial().
typedef struct {
    bool completed[MAX_TUTORIALS];
    bool quizPassed[MAX_TUTORIALS];
    bool hasDraft[MAX_TUTORIALS];
    char drafts[MAX_TUTORIALS][EDITOR_CAP];   // the student's code for each project
} Progress;

// Loads the save file if there is one (missing or broken files just mean a fresh start).
void ProgressLoad(Progress *p, const char *path);
bool ProgressSave(const Progress *p, const char *path);

#endif
