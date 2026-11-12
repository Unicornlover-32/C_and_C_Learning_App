// The mission registry: every game mission from missions_data.c, plus each level's own
// project as a Code Factory mission, with everything resolved (tests, starter code,
// the game header to put in front of the code, where the bugs sit...).
#include "missions.h"
#include "tutorials.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static MissionInfo missions[MAX_MISSIONS];
static int missionCount;

static int regionStart[MAX_REGIONS + 1];
static int regionCount;

static int FindTutorialId(const char *id)
{
    if (!id) return -1;
    for (int i = 0; i < TutorialCount(); i++)
        if (strcmp(GetTutorial(i)->id, id) == 0) return i;
    return -1;
}

int RegionCount(void) { return regionCount; }

int RegionOf(int tut)
{
    for (int r = regionCount - 1; r >= 0; r--)
        if (tut >= regionStart[r]) return r;
    return 0;
}

int RegionFirstTut(int region) { return regionStart[region]; }
int RegionTutCount(int region) { return regionStart[region + 1] - regionStart[region]; }

const char *ModeName(GameMode mode)
{
    switch (mode) {
        case MODE_ROBOT: return "Robot Rescue";
        case MODE_HEIST: return "Memory Heist";
        case MODE_BUGS:  return "Bug Hunter";
        default:         return "Code Factory";
    }
}

// A line counts towards par unless it is blank, a comment, a #directive or only braces.
int CountCodeLines(const char *code)
{
    int count = 0;
    const char *p = code;
    while (*p) {
        const char *end = strchr(p, '\n');
        if (!end) end = p + strlen(p);
        const char *s = p;
        while (s < end && (*s == ' ' || *s == '\t' || *s == '\r')) s++;
        bool counts = false;
        if (s < end && *s != '#' && !(s + 1 < end && s[0] == '/' && (s[1] == '/' || s[1] == '*')) && *s != '*') {
            for (const char *c = s; c < end; c++)
                if (*c != '{' && *c != '}' && *c != ';' && *c != ' ' && *c != '\t' && *c != '\r') { counts = true; break; }
        }
        if (counts) count++;
        p = (*end == '\n') ? end + 1 : end;
    }
    return count;
}

static int LineOfOffset(const char *text, int offset)
{
    int line = 0;
    for (int i = 0; i < offset && text[i]; i++) if (text[i] == '\n') line++;
    return line;
}

static char *Dup(const char *s)
{
    size_t n = strlen(s) + 1;
    char *d = malloc(n);
    if (d) memcpy(d, s, n);
    return d;
}

// Builds the buggy program: the solution with every bug put back in, and works out
// which line each bug sits on.
static char *BuildBuggy(const char *solution, const Mission *m, MissionInfo *info)
{
    typedef struct { int at, bug; } Hit;
    Hit hits[MAX_BUGS];
    int n = 0;
    for (int b = 0; b < m->bugCount; b++) {
        const char *found = strstr(solution, m->bugs[b].fixedText);
        if (!found) {
            fprintf(stderr, "mission %s: bug %d not found in the solution\n", m->id, b);
            continue;
        }
        hits[n].at = (int)(found - solution);
        hits[n].bug = b;
        n++;
    }
    // Apply them from the top down so offsets stay easy to follow
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++)
            if (hits[j].at < hits[i].at) { Hit t = hits[i]; hits[i] = hits[j]; hits[j] = t; }

    size_t cap = strlen(solution) + 1;
    for (int i = 0; i < n; i++) cap += strlen(m->bugs[hits[i].bug].buggyText);
    char *out = malloc(cap);
    if (!out) return Dup(solution);

    int len = 0, from = 0;
    int placedAt[MAX_BUGS];
    for (int i = 0; i < n; i++) {
        const BugDef *bd = &m->bugs[hits[i].bug];
        memcpy(out + len, solution + from, (size_t)(hits[i].at - from));
        len += hits[i].at - from;
        placedAt[i] = len;
        int bl = (int)strlen(bd->buggyText);
        memcpy(out + len, bd->buggyText, (size_t)bl);
        len += bl;
        from = hits[i].at + (int)strlen(bd->fixedText);
    }
    strcpy(out + len, solution + from);

    // Each bug sits on the first line where the buggy text differs from the fixed text
    info->bugCount = n;
    for (int i = 0; i < n; i++) {
        const BugDef *bd = &m->bugs[hits[i].bug];
        const char *f = bd->fixedText, *g = bd->buggyText;
        int lineInBug = 0;
        while (*f && *g && *f == *g) { if (*f == '\n') lineInBug++; f++; g++; }
        int line = LineOfOffset(out, placedAt[i]) + lineInBug;
        // the buggy line's own text, so the bug can follow it if lines move
        const char *ls = out;
        for (int l = 0; l < line && ls; l++) { ls = strchr(ls, '\n'); if (ls) ls++; }
        const char *le = ls ? strchr(ls, '\n') : NULL;
        size_t ll = ls ? (le ? (size_t)(le - ls) : strlen(ls)) : 0;
        char *lineText = malloc(ll + 1);
        if (lineText) { if (ls) memcpy(lineText, ls, ll); lineText[ll] = '\0'; }
        info->bugs[i].bugLine = line;
        info->bugs[i].buggyLine = lineText ? lineText : "";
        info->bugs[i].note = bd->note;
    }
    return out;
}

static char *BuildPrelude(const Mission *m)
{
    const char *header = (m->mode == MODE_ROBOT) ? "cfd_robot.h" : "cfd_heist.h";
    const char *config = m->config ? m->config : "";
    size_t n = strlen(config) + 64;
    char *p = malloc(n);
    if (!p) return NULL;
    snprintf(p, n, "%s#include \"%s\"\n", config, header);
    return p;
}

void MissionsInit(void)
{
    // Regions are the tutorial sections
    regionCount = 0;
    for (int i = 0; i < TutorialCount() && regionCount < MAX_REGIONS; i++)
        if (i == 0 || GetTutorial(i)->section) regionStart[regionCount++] = i;
    regionStart[regionCount] = TutorialCount();

    missionCount = 0;

    // The game missions
    for (int i = 0; i < MissionTableCount() && missionCount < MAX_MISSIONS; i++) {
        const Mission *m = GetMissionTable(i);
        MissionInfo *info = &missions[missionCount];
        memset(info, 0, sizeof(*info));
        int tut = FindTutorialId(m->level);
        if (m->role != ROLE_BOSS && tut < 0) {
            fprintf(stderr, "mission %s: unknown level %s\n", m->id, m->level ? m->level : "(none)");
            continue;
        }
        info->id = m->id;
        info->mode = m->mode;
        // Each level's own project is its main mission now: the robot and heist games
        // become optional side quests, and every bug hunt is a bonus round
        info->role = m->role;
        if (m->role == ROLE_MAIN) info->role = (m->mode == MODE_ROBOT || m->mode == MODE_HEIST) ? ROLE_SIDE : ROLE_BONUS;
        info->lang = m->lang;
        info->tut = (m->role == ROLE_BOSS) ? -1 : tut;
        info->region = (m->role == ROLE_BOSS) ? m->region : RegionOf(tut);
        info->title = m->title;
        info->brief = m->brief;
        info->api = m->api;
        info->hint = m->hint;
        info->timeLimit = m->timeLimit;

        if (m->mode == MODE_BUGS) {
            const Tutorial *t = GetTutorial(tut);
            info->lang = t->lang;
            info->solution = m->solution ? m->solution : t->solution;
            info->tests = m->testCount ? m->tests : t->tests;
            info->testCount = m->testCount ? m->testCount : t->testCount;
            info->starter = BuildBuggy(info->solution, m, info);
        } else {
            info->solution = m->solution;
            info->starter = m->starterCode;
            info->tests = m->tests;
            info->testCount = m->testCount;
        }
        if (m->mode == MODE_ROBOT || m->mode == MODE_HEIST) {
            info->prelude = BuildPrelude(m);
            info->parLines = m->parLines ? m->parLines : CountCodeLines(m->solution);
        }
        missionCount++;
    }

    // Each level's own project: its main mission (a 3D level where there is one)
    for (int t = 0; t < TutorialCount() && missionCount < MAX_MISSIONS; t++) {
        const Tutorial *tu = GetTutorial(t);
        MissionInfo *info = &missions[missionCount++];
        memset(info, 0, sizeof(*info));
        info->id = tu->id;
        info->mode = MODE_FACTORY;
        info->role = ROLE_MAIN;
        info->lang = tu->lang;
        info->tut = t;
        info->region = RegionOf(t);
        info->title = tu->projectTitle;
        info->brief = tu->projectBrief;
        info->starter = tu->starterCode;
        info->hint = tu->hint;
        info->solution = tu->solution;
        info->tests = tu->tests;
        info->testCount = tu->testCount;
    }
}

int MissionCount(void) { return missionCount; }

const MissionInfo *GetMission(int index)
{
    if (index < 0 || index >= missionCount) return &missions[0];
    return &missions[index];
}

int FindMission(const char *id)
{
    for (int i = 0; i < missionCount; i++)
        if (strcmp(missions[i].id, id) == 0) return i;
    return -1;
}

int MainMissionOf(int tut)
{
    for (int i = 0; i < missionCount; i++)
        if (missions[i].tut == tut && missions[i].role == ROLE_MAIN) return i;
    return -1;
}

// The level's own project first, then its side quests, then its bonus rounds
int LevelMissions(int tut, int *out)
{
    int n = 0;
    int main = MainMissionOf(tut);
    if (main >= 0) out[n++] = main;
    for (int pass = 0; pass < 2; pass++) {
        MissionRole want = pass == 0 ? ROLE_SIDE : ROLE_BONUS;
        for (int i = 0; i < missionCount && n < 8; i++)
            if (missions[i].tut == tut && i != main && missions[i].role == want) out[n++] = i;
    }
    return n;
}

int BossOf(int region)
{
    for (int i = 0; i < missionCount; i++)
        if (missions[i].role == ROLE_BOSS && missions[i].region == region) return i;
    return -1;
}
