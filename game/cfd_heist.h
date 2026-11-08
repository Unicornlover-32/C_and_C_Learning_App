/* Memory Heist - the street's half of the game.
 *
 * Memory is a street of houses: street[0] lives at address 1000, street[1] at 1004 and
 * so on, and you can only get around by following pointers. The app puts this file in
 * front of your code. It sets up the street from the input, writes everything you do
 * to cfd_events.txt for the app to animate, and sounds the alarm when a pointer goes
 * somewhere it shouldn't.
 *
 * World format (the test's input), one item per line, finished by END:
 *     STREET n loot0 loot1 ...     the houses and the loot inside them
 *     TIP k                        (CFD_TIP)    tip points at street[k], or NULL when k is -1
 *     BLOCK f c                    (CFD_BLOCK)  first = &street[f], count = c, the rest is guarded
 *     DOORS KSW...                 (CFD_DOORS)  the vault doors' locks
 *     ESTATE n  then n lines "number loot alarm"   (CFD_ESTATE, CFD_TRAIL)
 *     TRAIL k i0 i1 ...            (CFD_TRAIL)  estate houses linked by their next pointers
 *     CHAIN k i0 i1 ...            (CFD_CHAIN)  safehouse nodes made with new, one per street house
 *
 * More options: CFD_MALLOC tracks malloc/free, CFD_VAN adds drive(), CFD_NEW tracks
 * new/delete (C++), CFD_CRATES adds deliver().
 */
#ifndef CFD_HEIST_H
#define CFD_HEIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#ifdef __cplusplus
#include <new>
#endif

#define CFD_HOUSES 24

static FILE *cfd_ev;
static int   cfd_over;

static void cfd_log(const char *text)
{
    if (cfd_ev) {
        fputs(text, cfd_ev);
        fputc('\n', cfd_ev);
        fflush(cfd_ev);
    }
}

static void cfd_end(int win, const char *reason)
{
    char line[320];
    if (cfd_over) return;
    cfd_over = 1;
    if (win) snprintf(line, sizeof line, "END WIN");
    else     snprintf(line, sizeof line, "END FAIL %s", reason);
    cfd_log(line);
    if (cfd_ev) { fclose(cfd_ev); cfd_ev = NULL; }
}

/* The alarm went off: report it and stop the program. */
static void cfd_stop(const char *reason)
{
    cfd_end(0, reason);
    fflush(stdout);
    exit(0);
}

/* ------------------------------------------------------------------ the street */

int street[CFD_HOUSES];             /* the loot in every house */
int hideout;                        /* your hideout - you start here */
static int cfd_n;                   /* how many houses this street has */
static int cfd_orig[CFD_HOUSES];    /* the loot before you arrived */
static int cfd_pos = -1;            /* where you are: a house number, or -1 for the hideout */
static int cfd_guardFirst = 0, cfd_guardEnd = CFD_HOUSES;   /* the unguarded houses */
static int cfd_tipIndex = -2;       /* -2 no tip, -1 a NULL tip, else the house */
static int cfd_blockFirst, cfd_blockCount;

#ifdef CFD_TIP
int *tip;                           /* the fence's tip: the loot is here (or NULL) */
#endif
#ifdef CFD_BLOCK
int *first;                         /* the first unguarded house */
int count;                          /* how many unguarded houses there are */
#endif

static int cfd_house_index(const void *p)
{
    uintptr_t a = (uintptr_t)p, s = (uintptr_t)street;
    if (p == NULL) return -2;
    if (p == (const void *)&hideout) return -1;
    if (a >= s && a < s + (uintptr_t)cfd_n * sizeof(int) && (a - s) % sizeof(int) == 0)
        return (int)((a - s) / sizeof(int));
    return -3;
}

void go(int *house)
{
    char line[200];
    int i = cfd_house_index(house);
    if (cfd_over) return;
    if (i == -2) {
        cfd_log("G NULL");
        cfd_stop("ALARM! You tried to go to the NULL house - it doesn't exist. Check a pointer isn't NULL before you follow it.");
    }
    if (i == -3) {
        cfd_log("G LOST");
        cfd_stop("ALARM! That address isn't a house on this street - a pointer went somewhere it shouldn't.");
    }
    snprintf(line, sizeof line, "G %d", i);
    cfd_log(line);
    if (i >= 0 && (i < cfd_guardFirst || i >= cfd_guardEnd)) {
        snprintf(line, sizeof line, "ALARM! House %d (address %d) is guarded tonight - you stepped outside the block.", i, 1000 + 4 * i);
        cfd_stop(line);
    }
    cfd_pos = i;
}

void grab(int *house)
{
    char line[80];
    int i = cfd_house_index(house), amount;
    if (cfd_over) return;
    if (i == -2) cfd_stop("ALARM! You tried to grab from the NULL house.");
    if (i < 0) cfd_stop("grab() needs a pointer to one of the houses on the street.");
    if (i != cfd_pos) cfd_stop("You can only grab from the house you're standing at - go() there first.");
    amount = street[i];
    street[i] = 0;
    snprintf(line, sizeof line, "T %d %d", i, amount);
    cfd_log(line);
}

/* ------------------------------------------------------------------ malloc and free (C) */
#ifdef CFD_MALLOC
typedef struct { void *p; size_t size; int line, live; } CfdAlloc;
static CfdAlloc cfd_allocs[256];
static int cfd_nallocs, cfd_allocOverflow;

static int cfd_find_alloc(const void *p)
{
    int i;
    for (i = cfd_nallocs - 1; i >= 0; i--)
        if (cfd_allocs[i].p == p) return i;
    return -1;
}

static void *cfd_malloc(size_t n, int line)
{
    char text[64];
    void *p = malloc(n ? n : 1);
    if (!p) return NULL;
    if (cfd_nallocs < 256) {
        CfdAlloc *a = &cfd_allocs[cfd_nallocs];
        a->p = p;
        a->size = n;
        a->line = line;
        a->live = 1;
        snprintf(text, sizeof text, "A %d %d %d", cfd_nallocs, (int)n, line);
        cfd_log(text);
        cfd_nallocs++;
    } else {
        cfd_allocOverflow = 1;
    }
    return p;
}

static void *cfd_calloc(size_t count, size_t size, int line)
{
    void *p = cfd_malloc(count * size, line);
    if (p) memset(p, 0, count * size);
    return p;
}

static void cfd_free(void *p, int line)
{
    char text[200];
    int i;
    if (p == NULL || cfd_over) return;
    i = cfd_find_alloc(p);
    if (i < 0) {
        if (cfd_allocOverflow) return;
        snprintf(text, sizeof text, "ALARM! free() on line %d was given an address that never came from malloc.", line);
        cfd_stop(text);
    }
    if (!cfd_allocs[i].live) {
        snprintf(text, sizeof text, "ALARM! Line %d frees memory that was already freed - a double free.", line);
        cfd_stop(text);
    }
    cfd_allocs[i].live = 0;
    snprintf(text, sizeof text, "F %d %d", i, line);
    cfd_log(text);
    memset(p, 0xDD, cfd_allocs[i].size);   /* kept but scrambled, so using it after free shows up */
}

static void *cfd_realloc(void *p, size_t n, int line)
{
    char text[200];
    void *q;
    int i;
    if (p == NULL) return cfd_malloc(n, line);
    i = cfd_find_alloc(p);
    if (i < 0 || !cfd_allocs[i].live) {
        snprintf(text, sizeof text, "ALARM! realloc() on line %d was given memory that isn't yours any more.", line);
        cfd_stop(text);
    }
    q = cfd_malloc(n, line);
    if (!q) return NULL;
    memcpy(q, p, cfd_allocs[i].size < n ? cfd_allocs[i].size : n);
    cfd_free(p, line);
    return q;
}

#define malloc(n)     cfd_malloc((n), __LINE__)
#define calloc(c, n)  cfd_calloc((c), (n), __LINE__)
#define realloc(p, n) cfd_realloc((p), (n), __LINE__)
#define free(p)       cfd_free((p), __LINE__)
#endif

#ifdef CFD_VAN
static int cfd_driven;

void drive(int *van, int n)
{
    char text[400];
    int i, k, len;
    if (cfd_over) return;
    if (van == NULL) cfd_stop("ALARM! drive() was given NULL - you don't have a van.");
    i = cfd_find_alloc(van);
    if (i < 0) cfd_stop("drive() needs the van you got from malloc.");
    if (!cfd_allocs[i].live) cfd_stop("ALARM! That van was already freed - you're driving a dangling pointer.");
    if (n != cfd_blockCount) cfd_stop("drive() needs to know how many houses you loaded: pass count.");
    if (cfd_allocs[i].size < (size_t)n * sizeof(int))
        cfd_stop("The van is too small: ask malloc for count * sizeof(int) bytes.");
    len = snprintf(text, sizeof text, "V %d", i);
    for (k = 0; k < n && len < (int)sizeof text - 12; k++) len += snprintf(text + len, sizeof text - len, " %d", van[k]);
    cfd_log(text);
    for (k = 0; k < n; k++) {
        if (van[k] != cfd_orig[cfd_blockFirst + k]) {
            snprintf(text, sizeof text, "The van's load doesn't match the houses: van[%d] holds %d, but house %d had %d.",
                     k, van[k], cfd_blockFirst + k, cfd_orig[cfd_blockFirst + k]);
            cfd_stop(text);
        }
    }
    cfd_driven = 1;
}
#endif

/* ------------------------------------------------------------------ new and delete (C++) */
#if defined(CFD_NEW) && defined(__cplusplus)
typedef struct { void *p; size_t size; int isArray, deleted, tag; } CfdNew;
static CfdNew cfd_news[512];
static int cfd_nnews, cfd_trackNew, cfd_newTag;

static int cfd_find_new(const void *p)
{
    int i;
    for (i = cfd_nnews - 1; i >= 0; i--)
        if (cfd_news[i].p == p) return i;
    return -1;
}

static void *cfd_new(std::size_t n, int isArray)
{
    char text[64];
    void *p = malloc(n ? n : 1);
    if (!p) throw std::bad_alloc();
    if (cfd_trackNew && cfd_nnews < 512) {
        CfdNew *a = &cfd_news[cfd_nnews];
        a->p = p;
        a->size = n;
        a->isArray = isArray;
        a->deleted = 0;
        a->tag = cfd_newTag;
        snprintf(text, sizeof text, "N %d %d %d", cfd_nnews, (int)n, isArray);
        cfd_log(text);
        cfd_nnews++;
    }
    return p;
}

static void cfd_delete(void *p, int isArray)
{
    char text[64];
    int i;
    if (p == NULL) return;
    i = cfd_find_new(p);
    if (i < 0) { free(p); return; }        /* made before your code started: not ours to judge */
    if (cfd_over) return;
    if (cfd_news[i].deleted)
        cfd_stop("ALARM! This memory was already deleted - deleting the same pointer twice.");
    if (cfd_news[i].isArray != isArray)
        cfd_stop(isArray ? "ALARM! delete[] was used on memory from plain new - use delete."
                         : "ALARM! This memory came from new[] - free it with delete[], not delete.");
    cfd_news[i].deleted = 1;
    snprintf(text, sizeof text, "K %d", i);
    cfd_log(text);
    memset(p, 0xDD, cfd_news[i].size);     /* kept but scrambled, so using it after delete shows up */
}

void *operator new(std::size_t n) { return cfd_new(n, 0); }
void *operator new[](std::size_t n) { return cfd_new(n, 1); }
void operator delete(void *p) noexcept { cfd_delete(p, 0); }
void operator delete[](void *p) noexcept { cfd_delete(p, 1); }
void operator delete(void *p, std::size_t) noexcept { cfd_delete(p, 0); }
void operator delete[](void *p, std::size_t) noexcept { cfd_delete(p, 1); }
#endif

#if defined(CFD_CRATES) && defined(__cplusplus)
static int cfd_crates = -1;

void deliver(int *crates, int n)
{
    char text[400];
    int i, k, len;
    if (cfd_over) return;
    if (crates == nullptr) cfd_stop("ALARM! deliver() was given a null pointer - you have no crates.");
    i = cfd_find_new(crates);
    if (i < 0) cfd_stop("deliver() needs the crates you made with new[].");
    if (cfd_news[i].deleted) cfd_stop("ALARM! Those crates were already deleted - a dangling pointer.");
    if (!cfd_news[i].isArray) cfd_stop("Make the crates with new int[count] - an array of ints.");
    if (n != cfd_blockCount) cfd_stop("deliver() needs to know how many crates there are: pass count.");
    if (cfd_news[i].size < (size_t)n * sizeof(int)) cfd_stop("There aren't enough crates: make count of them.");
    len = snprintf(text, sizeof text, "C %d", i);
    for (k = 0; k < n && len < (int)sizeof text - 12; k++) len += snprintf(text + len, sizeof text - len, " %d", crates[k]);
    cfd_log(text);
    for (k = 0; k < n; k++) {
        if (crates[k] != cfd_orig[cfd_blockFirst + k]) {
            snprintf(text, sizeof text, "The crates don't match the houses: crates[%d] holds %d, but house %d had %d.",
                     k, crates[k], cfd_blockFirst + k, cfd_orig[cfd_blockFirst + k]);
            cfd_stop(text);
        }
    }
    cfd_crates = i;
}
#endif

/* ------------------------------------------------------------------ gadgets (function pointers) */
#ifdef CFD_DOORS
static char cfd_doors[CFD_HOUSES + 1];
static int  cfd_ndoors, cfd_door;
int noise;                               /* global: how much noise you've made */

int pick(int lock)  { noise += 1;  return lock == 'K'; }    /* quiet - keyhole locks */
int drill(int lock) { noise += 3;  return lock == 'S'; }    /* steel locks */
int blast(int lock) { noise += 10; return lock == 'W'; }    /* loud - wall vaults */

int doors_left(void) { return cfd_ndoors - cfd_door; }
char lock_type(void) { return cfd_door < cfd_ndoors ? cfd_doors[cfd_door] : 0; }

void use(int (*gadget)(int))
{
    char text[160];
    const char *name;
    if (cfd_over) return;
    if (cfd_door >= cfd_ndoors) cfd_stop("use() was called, but every door is already open.");
    if (gadget == NULL) cfd_stop("ALARM! use() was given a NULL gadget - point your function pointer at pick, drill or blast first.");
    name = gadget == pick ? "pick" : gadget == drill ? "drill" : gadget == blast ? "blast" : "mystery";
    snprintf(text, sizeof text, "U %d %s", cfd_door, name);
    cfd_log(text);
    if (!gadget(cfd_doors[cfd_door])) {
        snprintf(text, sizeof text, "ALARM! The %s doesn't open a '%c' lock - you picked the wrong gadget.", name, cfd_doors[cfd_door]);
        cfd_stop(text);
    }
    snprintf(text, sizeof text, "O %d", cfd_door);
    cfd_log(text);
    cfd_door++;
}
#endif

/* ------------------------------------------------------------------ structs: the estate */
#if defined(CFD_ESTATE) || defined(CFD_TRAIL)
struct House {
    int number;             /* the house number on the street */
    int loot;               /* how much is inside */
    int alarm;              /* 1 if the alarm is switched on */
    struct House *next;     /* the next house in a trail (NULL at the end) */
};
static struct House cfd_estate[CFD_HOUSES];
static int cfd_estateN, cfd_estateAlarm[CFD_HOUSES], cfd_estateLoot[CFD_HOUSES], cfd_robbed[CFD_HOUSES];
#ifdef CFD_ESTATE
struct House *start;        /* the first house on the estate */
int count;                  /* how many houses there are */
#endif
#ifdef CFD_TRAIL
struct House *first_clue;   /* the first house in the trail */
static int cfd_trail[CFD_HOUSES], cfd_trailN, cfd_reported;
#endif

void rob(struct House *h)
{
    char text[200];
    uintptr_t a = (uintptr_t)h, s = (uintptr_t)cfd_estate;
    int i;
    if (cfd_over) return;
    if (h == NULL) {
        cfd_log("R NULL");
        cfd_stop("ALARM! You tried to rob the NULL house. Check a pointer isn't NULL before you follow it.");
    }
    if (a < s || a >= s + (uintptr_t)cfd_estateN * sizeof(struct House) || (a - s) % sizeof(struct House) != 0)
        cfd_stop("rob() needs a pointer to one of the houses on the estate.");
    i = (int)((a - s) / sizeof(struct House));
    snprintf(text, sizeof text, "R %d %d", i, h->loot);
    cfd_log(text);
    if (cfd_estateAlarm[i]) {
        snprintf(text, sizeof text, "ALARM! House number %d had its alarm on - check h->alarm before you rob a house.", h->number);
        cfd_stop(text);
    }
    cfd_robbed[i] = 1;
    h->loot = 0;
    cfd_pos = i;
}
#endif

#ifdef CFD_TRAIL
void report(int *numbers, int n)
{
    char text[400];
    int i, k, len;
    if (cfd_over) return;
    if (numbers == NULL) cfd_stop("ALARM! report() was given NULL instead of your list of house numbers.");
#ifdef CFD_MALLOC
    i = cfd_find_alloc(numbers);
    if (i < 0) cfd_stop("report() needs the list you made with malloc.");
    if (!cfd_allocs[i].live) cfd_stop("ALARM! That list was already freed - report() before you free() it.");
#endif
    if (n != cfd_trailN) {
        snprintf(text, sizeof text, "The trail has %d houses, but report() was told %d.", cfd_trailN, n);
        cfd_stop(text);
    }
    len = snprintf(text, sizeof text, "P %d", n);
    for (k = 0; k < n && len < (int)sizeof text - 12; k++) len += snprintf(text + len, sizeof text - len, " %d", numbers[k]);
    cfd_log(text);
    for (k = 0; k < n; k++) {
        if (numbers[k] != cfd_estate[cfd_trail[k]].number) {
            snprintf(text, sizeof text, "Your list says house %d is number %d, but the trail's house %d is number %d.",
                     k, numbers[k], k, cfd_estate[cfd_trail[k]].number);
            cfd_stop(text);
        }
    }
    cfd_reported = 1;
}
#endif

/* ------------------------------------------------------------------ linked list of safehouses (C++) */
#if defined(CFD_CHAIN) && defined(__cplusplus)
struct Node {
    int house;      /* which street house this safehouse is */
    Node *next;     /* the next safehouse, or nullptr at the end */
};
Node *head;          /* the first safehouse */
static int cfd_chain[CFD_HOUSES], cfd_chainN, cfd_chainFirstNew;
#endif

/* ------------------------------------------------------------------ setting up and checking */

static int cfd_read_int(void)
{
    int v = 0;
    if (scanf("%d", &v) != 1) v = 0;
    return v;
}

static int cfd_load(void)
{
    char key[32];
    int i, k;
    while (scanf("%31s", key) == 1) {
        if (strcmp(key, "END") == 0) return 1;
        if (strcmp(key, "STREET") == 0) {
            cfd_n = cfd_read_int();
            if (cfd_n > CFD_HOUSES) cfd_n = CFD_HOUSES;
            for (i = 0; i < cfd_n; i++) street[i] = cfd_orig[i] = cfd_read_int();
        } else if (strcmp(key, "TIP") == 0) {
            cfd_tipIndex = cfd_read_int();
            if (cfd_tipIndex >= cfd_n) cfd_tipIndex = -1;
#ifdef CFD_TIP
            tip = cfd_tipIndex >= 0 ? &street[cfd_tipIndex] : NULL;
#endif
        } else if (strcmp(key, "BLOCK") == 0) {
            cfd_blockFirst = cfd_read_int();
            cfd_blockCount = cfd_read_int();
            cfd_guardFirst = cfd_blockFirst;
            cfd_guardEnd = cfd_blockFirst + cfd_blockCount;
#ifdef CFD_BLOCK
            first = &street[cfd_blockFirst];
            count = cfd_blockCount;
#endif
        } else if (strcmp(key, "DOORS") == 0) {
            char doors[CFD_HOUSES + 1] = "";
            if (scanf("%24s", doors) != 1) return 0;
#ifdef CFD_DOORS
            strcpy(cfd_doors, doors);
            cfd_ndoors = (int)strlen(doors);
#endif
        } else if (strcmp(key, "ESTATE") == 0) {
            int n = cfd_read_int();
            for (i = 0; i < n; i++) {
                int number = cfd_read_int(), loot = cfd_read_int(), alarm = cfd_read_int();
#if defined(CFD_ESTATE) || defined(CFD_TRAIL)
                if (i < CFD_HOUSES) {
                    cfd_estate[i].number = number;
                    cfd_estate[i].loot = cfd_estateLoot[i] = loot;
                    cfd_estate[i].alarm = cfd_estateAlarm[i] = alarm;
                    cfd_estate[i].next = NULL;
                }
#else
                (void)number; (void)loot; (void)alarm;
#endif
            }
#if defined(CFD_ESTATE) || defined(CFD_TRAIL)
            cfd_estateN = n < CFD_HOUSES ? n : CFD_HOUSES;
#endif
#ifdef CFD_ESTATE
            start = &cfd_estate[0];
            count = cfd_estateN;
#endif
        } else if (strcmp(key, "TRAIL") == 0) {
            int n = cfd_read_int();
            for (i = 0; i < n; i++) {
                k = cfd_read_int();
#ifdef CFD_TRAIL
                if (i < CFD_HOUSES) cfd_trail[i] = k;
#else
                (void)k;
#endif
            }
#ifdef CFD_TRAIL
            cfd_trailN = n < CFD_HOUSES ? n : CFD_HOUSES;
            for (i = 0; i < cfd_trailN; i++)
                cfd_estate[cfd_trail[i]].next = i + 1 < cfd_trailN ? &cfd_estate[cfd_trail[i + 1]] : NULL;
            first_clue = cfd_trailN > 0 ? &cfd_estate[cfd_trail[0]] : NULL;
#endif
        } else if (strcmp(key, "CHAIN") == 0) {
            int n = cfd_read_int();
            for (i = 0; i < n; i++) {
                k = cfd_read_int();
#if defined(CFD_CHAIN) && defined(__cplusplus)
                if (i < CFD_HOUSES) cfd_chain[i] = k;
#else
                (void)k;
#endif
            }
#if defined(CFD_CHAIN) && defined(__cplusplus)
            cfd_chainN = n < CFD_HOUSES ? n : CFD_HOUSES;
            {
                Node *prev = nullptr;
                cfd_trackNew = 1;           /* the safehouses are yours to delete */
                cfd_newTag = 1;
                cfd_chainFirstNew = cfd_nnews;
                for (i = 0; i < cfd_chainN; i++) {
                    Node *node = new Node;
                    node->house = cfd_chain[i];
                    node->next = nullptr;
                    if (prev) prev->next = node; else head = node;
                    prev = node;
                }
                cfd_newTag = 0;
                cfd_trackNew = 0;
            }
#endif
        }
    }
    return 0;
}

static void cfd_finish(void)
{
    char reason[256];
    int i;
    if (cfd_over) return;
#ifdef CFD_TIP
    if (cfd_tipIndex >= 0 && street[cfd_tipIndex] != 0) {
        cfd_end(0, "The tip was real, but you left the loot behind. go(tip), then grab(tip).");
        return;
    }
    for (i = 0; i < cfd_n; i++) {
        if (i != cfd_tipIndex && street[i] != cfd_orig[i]) {
            cfd_end(0, "You grabbed from the wrong house.");
            return;
        }
    }
    if (cfd_pos != -1) {
        cfd_end(0, "You didn't make it back to the hideout - finish with go(&hideout).");
        return;
    }
#endif
#if defined(CFD_BLOCK) || defined(CFD_CHAIN)
    for (i = 0; i < cfd_n; i++) {
#ifdef CFD_CHAIN
        int wanted = 0, k;
        for (k = 0; k < cfd_chainN; k++) if (cfd_chain[k] == i) wanted = 1;
#else
        int wanted = i >= cfd_blockFirst && i < cfd_blockFirst + cfd_blockCount;
#endif
        if (wanted && street[i] != 0) {
            snprintf(reason, sizeof reason, "House %d (address %d) still has its loot - you missed it.", i, 1000 + 4 * i);
            cfd_end(0, reason);
            return;
        }
    }
#endif
#ifdef CFD_VAN
    if (!cfd_driven) {
        cfd_end(0, "You never drove the van to the hideout - call drive(van, count) before you free it.");
        return;
    }
#endif
#ifdef CFD_MALLOC
    for (i = 0; i < cfd_nallocs; i++) {
        if (cfd_allocs[i].live) {
            char text[64];
            snprintf(text, sizeof text, "L %d", i);
            cfd_log(text);
            snprintf(reason, sizeof reason, "Memory leak! The %d bytes from malloc on line %d were never freed - "
                                           "you left the evidence at the scene.", (int)cfd_allocs[i].size, cfd_allocs[i].line);
            cfd_end(0, reason);
            return;
        }
    }
#endif
#if defined(CFD_CRATES) && defined(__cplusplus)
    if (cfd_crates < 0) {
        cfd_end(0, "You never delivered the crates - call deliver(crates, count).");
        return;
    }
    if (!cfd_news[cfd_crates].deleted) {
        char text[32];
        snprintf(text, sizeof text, "L %d", cfd_crates);
        cfd_log(text);
        cfd_end(0, "Memory leak! The crates from new[] were never deleted - finish with delete[] crates.");
        return;
    }
#endif
#if defined(CFD_CHAIN) && defined(__cplusplus)
    {
        int left = 0;
        for (i = cfd_chainFirstNew; i < cfd_chainFirstNew + cfd_chainN && i < cfd_nnews; i++) {
            if (!cfd_news[i].deleted) {
                char text[32];
                snprintf(text, sizeof text, "L %d", i);
                cfd_log(text);
                left++;
            }
        }
        if (left > 0) {
            snprintf(reason, sizeof reason, "Memory leak! %d safehouse node%s never deleted - every node made with new needs a delete.",
                     left, left == 1 ? " was" : "s were");
            cfd_end(0, reason);
            return;
        }
    }
#endif
#ifdef CFD_DOORS
    if (cfd_door < cfd_ndoors) {
        snprintf(reason, sizeof reason, "%d door%s still locked - keep going until doors_left() is 0.",
                 cfd_ndoors - cfd_door, cfd_ndoors - cfd_door == 1 ? " is" : "s are");
        cfd_end(0, reason);
        return;
    }
#endif
#ifdef CFD_ESTATE
    for (i = 0; i < cfd_estateN; i++) {
        if (!cfd_estateAlarm[i] && cfd_estateLoot[i] > 0 && !cfd_robbed[i]) {
            snprintf(reason, sizeof reason, "House number %d had no alarm and %d in loot, but you didn't rob it.",
                     cfd_estate[i].number, cfd_estateLoot[i]);
            cfd_end(0, reason);
            return;
        }
    }
#endif
#ifdef CFD_TRAIL
    for (i = 0; i < cfd_trailN; i++) {
        if (!cfd_robbed[cfd_trail[i]]) {
            snprintf(reason, sizeof reason, "House number %d is on the trail, but you didn't rob it.", cfd_estate[cfd_trail[i]].number);
            cfd_end(0, reason);
            return;
        }
    }
    if (!cfd_reported) {
        cfd_end(0, "You never reported to the fence - call report(numbers, n) before you free the list.");
        return;
    }
#endif
    (void)reason;
    (void)i;
    cfd_end(1, "");
}

/* ------------------------------------------------------------------ the real main */

int cfd_student_main();

int main(void)
{
    cfd_ev = fopen("cfd_events.txt", "w");
    if (!cfd_load()) {
        cfd_end(0, "The street could not be set up - this is a problem with the level, not your code.");
        return 1;
    }
#if defined(CFD_NEW) && defined(__cplusplus)
    cfd_trackNew = 1;
#endif
    atexit(cfd_finish);
    return cfd_student_main();
}

#endif
