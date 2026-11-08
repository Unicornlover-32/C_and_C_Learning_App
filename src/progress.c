// Save file format (plain text, so it is easy to inspect):
//
//   CFD-PROGRESS 2
//   PLAYER <xp> <badges> <streak> <best streak> <last day YYYYMMDD> <warm-up best>
//   LEVEL <tutorial id> <Output Rush stars> <briefing read 0/1>
//   MISSION <mission id> <star bits> <code length in bytes>
//   <exactly that many bytes of code>
//   PROJECT <project id> <finished steps> <code length in bytes>
//   <exactly that many bytes of code>
//
// Everything is matched by id, so adding or reordering levels and missions keeps old
// progress. Version 1 files (from before the games) are converted when they load.
#include "progress.h"
#include "tutorials.h"
#include "projects.h"
#include <stdio.h>
#include <string.h>

#define HEADER_V1 "CFD-PROGRESS 1"
#define HEADER_V2 "CFD-PROGRESS 2"

int StarCount(unsigned char bits)
{
    return (bits & 1) + ((bits >> 1) & 1) + ((bits >> 2) & 1);
}

static int FindTutorial(const char *id)
{
    for (int i = 0; i < TutorialCount() && i < MAX_TUTORIALS; i++)
        if (strcmp(GetTutorial(i)->id, id) == 0) return i;
    return -1;
}

static int FindProject(const char *id)
{
    for (int i = 0; i < ProjectCount() && i < MAX_PROJECTS; i++)
        if (strcmp(GetProject(i)->id, id) == 0) return i;
    return -1;
}

// Reads `len` bytes of code (plus the newline after it) into dst, or skips them.
static bool ReadCode(FILE *f, char *dst, int len)
{
    static char discard[EDITOR_CAP];
    if (len < 0 || len >= EDITOR_CAP) return false;
    char *to = dst ? dst : discard;
    if (fread(to, 1, (size_t)len, f) != (size_t)len) return false;
    to[len] = '\0';
    fgetc(f);
    return true;
}

static void LoadV1(Progress *p, FILE *f)
{
    char id[64];
    int done, quiz, len;
    while (fscanf(f, "%63s %d %d %d", id, &done, &quiz, &len) == 4) {
        if (fgetc(f) != '\n') break;
        int t = FindTutorial(id);
        int pr = (t < 0) ? FindProject(id) : -1;
        int m = (t >= 0) ? FindMission(id) : -1;     // a level's own project is the mission with its id
        char *dst = (m >= 0 && m < MAX_MISSIONS) ? p->drafts[m] : (pr >= 0) ? p->projDrafts[pr] : NULL;
        if (!ReadCode(f, dst, len)) break;
        if (t >= 0) {
            p->rushStars[t] = quiz ? 1 : 0;
            if (m >= 0 && m < MAX_MISSIONS) {
                if (done) p->stars[m] |= STAR_CLEAR;
                p->hasDraft[m] = (len > 0);
            }
        } else if (pr >= 0) {
            p->projSteps[pr] = done;
            p->projHasDraft[pr] = (len > 0);
        }
    }
}

static void LoadV2(Progress *p, FILE *f)
{
    char kind[16], id[64];
    while (fscanf(f, "%15s", kind) == 1) {
        if (strcmp(kind, "PLAYER") == 0) {
            if (fscanf(f, "%d %u %d %d %d %d", &p->xp, &p->badges, &p->streak, &p->bestStreak, &p->lastDay, &p->warmupBest) != 6) break;
        } else if (strcmp(kind, "LEVEL") == 0) {
            int rush, read;
            if (fscanf(f, "%63s %d %d", id, &rush, &read) != 3) break;
            int t = FindTutorial(id);
            if (t >= 0) {
                p->rushStars[t] = (unsigned char)(rush < 0 ? 0 : rush > 3 ? 3 : rush);
                p->lessonRead[t] = (read != 0);
            }
        } else if (strcmp(kind, "MISSION") == 0 || strcmp(kind, "PROJECT") == 0) {
            bool mission = (kind[0] == 'M');
            int value, len;
            if (fscanf(f, "%63s %d %d", id, &value, &len) != 3 || fgetc(f) != '\n') break;
            int i = mission ? FindMission(id) : FindProject(id);
            char *dst = NULL;
            if (i >= 0 && mission && i < MAX_MISSIONS) dst = p->drafts[i];
            if (i >= 0 && !mission) dst = p->projDrafts[i];
            if (!ReadCode(f, dst, len)) break;
            if (i < 0) continue;
            if (mission && i < MAX_MISSIONS) {
                p->stars[i] = (unsigned char)(value & 7);
                p->hasDraft[i] = (len > 0);
            } else if (!mission) {
                p->projSteps[i] = value;
                p->projHasDraft[i] = (len > 0);
            }
        } else {
            break;
        }
    }
}

void ProgressLoad(Progress *p, const char *path)
{
    memset(p, 0, sizeof(*p));
    FILE *f = fopen(path, "rb");
    if (!f) return;

    char header[64];
    if (fgets(header, sizeof(header), f)) {
        if (strncmp(header, HEADER_V2, strlen(HEADER_V2)) == 0) LoadV2(p, f);
        else if (strncmp(header, HEADER_V1, strlen(HEADER_V1)) == 0) LoadV1(p, f);
    }
    fclose(f);
}

bool ProgressSave(const Progress *p, const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;

    fprintf(f, "%s\n", HEADER_V2);
    fprintf(f, "PLAYER %d %u %d %d %d %d\n", p->xp, p->badges, p->streak, p->bestStreak, p->lastDay, p->warmupBest);
    for (int i = 0; i < TutorialCount() && i < MAX_TUTORIALS; i++) {
        if (!p->rushStars[i] && !p->lessonRead[i]) continue;
        fprintf(f, "LEVEL %s %d %d\n", GetTutorial(i)->id, p->rushStars[i], p->lessonRead[i]);
    }
    for (int i = 0; i < MissionCount() && i < MAX_MISSIONS; i++) {
        if (!p->stars[i] && !p->hasDraft[i]) continue;
        int len = p->hasDraft[i] ? (int)strlen(p->drafts[i]) : 0;
        fprintf(f, "MISSION %s %d %d\n", GetMission(i)->id, p->stars[i], len);
        fwrite(p->drafts[i], 1, (size_t)len, f);
        fputc('\n', f);
    }
    for (int i = 0; i < ProjectCount() && i < MAX_PROJECTS; i++) {
        if (p->projSteps[i] == 0 && !p->projHasDraft[i]) continue;
        int len = p->projHasDraft[i] ? (int)strlen(p->projDrafts[i]) : 0;
        fprintf(f, "PROJECT %s %d %d\n", GetProject(i)->id, p->projSteps[i], len);
        fwrite(p->projDrafts[i], 1, (size_t)len, f);
        fputc('\n', f);
    }
    fclose(f);
    return true;
}
