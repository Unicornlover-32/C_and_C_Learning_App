// Save file format (plain text, so it is easy to inspect):
//
//   CFD-PROGRESS 1
//   <tutorial id> <completed 0/1> <quiz passed 0/1> <code length in bytes>
//   <exactly that many bytes of code>
//   ...one block per tutorial...
//
// Bigger projects use the same block, with the number of finished steps in place of
// "completed" and 0 for "quiz passed".
//
// Tutorials and projects are matched by id, so adding or reordering them keeps old progress.
#include "progress.h"
#include "tutorials.h"
#include "projects.h"
#include <stdio.h>
#include <string.h>

#define HEADER "CFD-PROGRESS 1"

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

void ProgressLoad(Progress *p, const char *path)
{
    memset(p, 0, sizeof(*p));
    FILE *f = fopen(path, "rb");
    if (!f) return;

    char header[64];
    if (!fgets(header, sizeof(header), f) || strncmp(header, HEADER, strlen(HEADER)) != 0) {
        fclose(f);
        return;
    }

    char id[64];
    int done, quiz, len;
    while (fscanf(f, "%63s %d %d %d", id, &done, &quiz, &len) == 4) {
        if (fgetc(f) != '\n' || len < 0 || len >= EDITOR_CAP) break;

        int t = FindTutorial(id);
        int pr = (t < 0) ? FindProject(id) : -1;
        static char discard[EDITOR_CAP];
        char *dst = (t >= 0) ? p->drafts[t] : (pr >= 0) ? p->projDrafts[pr] : discard;
        if (fread(dst, 1, (size_t)len, f) != (size_t)len) break;
        dst[len] = '\0';
        fgetc(f);   // newline after the code

        if (t >= 0) {
            p->completed[t] = (done != 0);
            p->quizPassed[t] = (quiz != 0);
            p->hasDraft[t] = (len > 0);
        } else if (pr >= 0) {
            p->projSteps[pr] = done;
            p->projHasDraft[pr] = (len > 0);
        }
    }
    fclose(f);
}

bool ProgressSave(const Progress *p, const char *path)
{
    FILE *f = fopen(path, "wb");
    if (!f) return false;

    fprintf(f, "%s\n", HEADER);
    for (int i = 0; i < TutorialCount() && i < MAX_TUTORIALS; i++) {
        if (!p->completed[i] && !p->quizPassed[i] && !p->hasDraft[i]) continue;
        int len = p->hasDraft[i] ? (int)strlen(p->drafts[i]) : 0;
        fprintf(f, "%s %d %d %d\n", GetTutorial(i)->id, p->completed[i], p->quizPassed[i], len);
        fwrite(p->drafts[i], 1, (size_t)len, f);
        fputc('\n', f);
    }
    fclose(f);
    return true;
}
