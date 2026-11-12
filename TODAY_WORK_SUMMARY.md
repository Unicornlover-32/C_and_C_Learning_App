# Work Completed Today

**Date:** November 10, 2026  
**Project:** C and C++ Learning App

## Overview

Today's work expanded the learning app from a tutorial-focused application into a more structured, game-like learning experience. The changes add a reusable game layer, connect missions to playable levels, improve the build workflow, and introduce automated tests for level parsing and gameplay events.

## Main Changes

### 1. Added the game framework

The new [`src/game/`](C:/raylib/raylib/projects/C_and_C++_Learning_App/src/game) module provides the foundation for interactive learning levels:

- Shared event and parser types for turning program output into gameplay events.
- Level registration and lookup through a central level table.
- Scene, stage, camera, model, label, particle, shader, and tween helpers.
- Reusable visual kits for factory, robot, and street/heist environments.
- Playback support for animating a level from parsed test events.
- Initial level implementations for introductory output, loops, receipts, and rectangles.

### 2. Connected missions to playable levels

The mission flow was extended so missions can:

- Open and navigate between level steps.
- Run the learner's code and interpret its output.
- Display compile output, test results, objectives, hints, solutions, and reset controls.
- Show level-specific 3D scenes and animated success/failure states.
- Track failures and present a bug/failure clock during attempts.
- Save and restore draft code while moving through mission steps.

### 3. Improved game presentation

The gamification and heist views were updated to use the new game data and presentation helpers. Robot movement, cheering, doors, vaults, crates, gold, houses, roads, particles, labels, and animated transitions now support the level scenes.

### 4. Updated build and test workflow

The [`Makefile`](C:/raylib/raylib/projects/C_and_C++_Learning_App/Makefile) now:

- Builds both the existing source files and the new `src/game/*.c` files.
- Supports separate optimized and debug targets.
- Adds a `make test` target for the level test executable.
- Supports configurable raylib include and library paths.
- Includes the new generated executables and practice files in cleanup/ignore rules.

### 5. Added automated level tests

[`tests/test_levels.c`](C:/raylib/raylib/projects/C_and_C++_Learning_App/tests/test_levels.c) was added to exercise:

- Level parsers against expected output.
- Winning, failing, and crashing run outcomes.
- Event sequence and ending-event consistency.
- Mutated output and fuzz-style input cases.
- Parser behavior across the available tutorials and levels.

## Validation

The requested test command was run:

```text
make test
```

The test executable did not build because the new helper declaration `TextCopy(char *, int, const char *)` in [`src/game/stage.h`](C:/raylib/raylib/projects/C_and_C++_Learning_App/src/game/stage.h) conflicts with raylib's existing `TextCopy(char *, const char *)` declaration. As a result, the automated tests have not run yet.

## Current Repository State

The work described above is currently present as uncommitted working-tree changes. The next corrective step is to rename or otherwise isolate the project helper named `TextCopy`, update its callers, and rerun `make test`.
