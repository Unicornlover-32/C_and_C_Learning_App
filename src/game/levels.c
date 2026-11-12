// The level table: which 3D level each tutorial's project plays as.
#include "levels.h"
#include <string.h>

extern const LevelDef LEVEL_HELLO, LEVEL_RECEIPT, LEVEL_TIME, LEVEL_GRADES, LEVEL_LOOPS, LEVEL_MATRIX, LEVEL_STRINGS, LEVEL_TOOLKIT;
extern const LevelDef LEVEL_SWAP, LEVEL_PTRWALK, LEVEL_MALLOC, LEVEL_FUNCPTR, LEVEL_STRUCTS, LEVEL_FILES;
extern const LevelDef LEVEL_RECT, LEVEL_REFS, LEVEL_GRID, LEVEL_POINTS, LEVEL_BANK, LEVEL_DEEPCOPY, LEVEL_LIST, LEVEL_BOXES;
extern const LevelDef LEVEL_TEMPLATES, LEVEL_VECTORS, LEVEL_ALGORITHMS, LEVEL_QUEUE, LEVEL_MAPS;

static const LevelDef *const LEVELS[] = {
    // C: Intro to C
    &LEVEL_HELLO, &LEVEL_RECEIPT, &LEVEL_TIME, &LEVEL_GRADES, &LEVEL_LOOPS, &LEVEL_MATRIX, &LEVEL_STRINGS, &LEVEL_TOOLKIT,
    // C: Pointers and Memory
    &LEVEL_SWAP, &LEVEL_PTRWALK, &LEVEL_MALLOC, &LEVEL_FUNCPTR, &LEVEL_STRUCTS, &LEVEL_FILES,
    // C++: C++ and Classes
    &LEVEL_RECT, &LEVEL_REFS, &LEVEL_GRID, &LEVEL_POINTS, &LEVEL_BANK, &LEVEL_DEEPCOPY, &LEVEL_LIST, &LEVEL_BOXES,
    // C++: Templates and the STL
    &LEVEL_TEMPLATES, &LEVEL_VECTORS, &LEVEL_ALGORITHMS, &LEVEL_QUEUE, &LEVEL_MAPS,
};

int LevelTableCount(void) { return (int)(sizeof(LEVELS) / sizeof(LEVELS[0])); }

const LevelDef *LevelTableAt(int i) { return (i >= 0 && i < LevelTableCount()) ? LEVELS[i] : NULL; }

const LevelDef *LevelFor(const char *tutId)
{
    if (!tutId) return NULL;
    for (int i = 0; i < LevelTableCount(); i++)
        if (strcmp(LEVELS[i]->tutId, tutId) == 0) return LEVELS[i];
    return NULL;
}
