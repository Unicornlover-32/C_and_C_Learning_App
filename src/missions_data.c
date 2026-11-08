// Generated content: every game mission. Robot Rescue and Memory Heist levels describe their
// world in each test's input (see game/cfd_robot.h and game/cfd_heist.h), and expected is
// what the program must print (NULL when anything goes). Bug Hunter rounds are built from
// their level's own project: the solution with the bugs below put back in.
#include "missions.h"
#include <stddef.h>

static const Mission MISSIONS[] = {

// =====================================================================
{
    .id = "robot-hello", .level = "c-hello", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_C,
    .title = "Hello, Robot!",
    .brief =
        "Your rescue robot has landed, but mission control can't hear it yet - its radio only works through printf.\n"
        "\n"
        "1. Radio in: print exactly Hello, World! followed by a new line.\n"
        "2. Drive to the exit, four tiles straight ahead. move() steps forward one tile, and there's a coin on the way.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // 1. Radio in: print Hello, World!\n"
        "\n"
        "    // 2. Drive to the exit: one move(); for each tile\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "printf(\"Hello, World!\\n\"); and then four lines of move(); - each one is a statement, so each needs its semicolon.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    printf(\"Hello, World!\\n\");\n"
        "\n"
        "    move();\n"
        "    move();\n"
        "    move();\n"
        "    move();\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "7 3\n"
          "#######\n"
          "#>.C.E#\n"
          "#######\n",
          "Hello, World!\n" },
    },
},

// =====================================================================
{
    .id = "robot-coins", .level = "c-variables-const", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_C,
    .title = "Coin Counter",
    .brief =
        "This corridor is full of coins and HQ wants a receipt. Drive the six tiles to the exit, then use variables to report what you found:\n"
        "\n"
        "Coins: 3\n"
        "Value: 0.75\n"
        "\n"
        "coins() tells you how many coins the robot has picked up. Store it in an int, keep the value of one coin in a constant (const double COIN_VALUE = 0.25;) and work out the total value in a double. There are two corridors with different numbers of coins, so don't just type the answer in!\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n"
        "coins()         coins you have collected\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    const double COIN_VALUE = 0.25;\n"
        "\n"
        "    // Drive the six tiles to the exit\n"
        "\n"
        "    // Store coins() in a variable, work out the value and print the receipt\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "After the six move() calls: int collected = coins(); double value = collected * COIN_VALUE; then printf(\"Coins: %d\\n\", collected); and printf(\"Value: %.2f\\n\", value);",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    const double COIN_VALUE = 0.25;\n"
        "\n"
        "    move();\n"
        "    move();\n"
        "    move();\n"
        "    move();\n"
        "    move();\n"
        "    move();\n"
        "\n"
        "    int collected = coins();\n"
        "    double value = collected * COIN_VALUE;\n"
        "\n"
        "    printf(\"Coins: %d\\n\", collected);\n"
        "    printf(\"Value: %.2f\\n\", value);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 2,
    .tests = {
        { "9 3\n"
          "#########\n"
          "#>C.C.CE#\n"
          "#########\n",
          "Coins: 3\n"
          "Value: 0.75\n" },
        { "9 3\n"
          "#########\n"
          "#>CCCCCE#\n"
          "#########\n",
          "Coins: 5\n"
          "Value: 1.25\n" },
    },
},

// =====================================================================
{
    .id = "robot-room", .level = "c-input-maths", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_C,
    .title = "Measure the Room",
    .brief =
        "Every room is a different size, so the robot is told its size as input: the width and the height in tiles, like 5 4. The robot starts in the bottom-left corner facing east, and the exit is in the top-right corner.\n"
        "\n"
        "Engineering gave you move_n(steps), which moves forward that many tiles. Read the two numbers with scanf, then:\n"
        "1. Print the room's area and how many steps the trip takes:\n"
        "Area: 20\n"
        "Steps: 7\n"
        "2. Drive there: along the bottom (width - 1 tiles), turn left, then up (height - 1 tiles).\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n"
        "move_n(steps)   move forward steps tiles\n"
        "turn_left()     turn 90 degrees left\n"
        "turn_right()    turn 90 degrees right\n",
    .config = "#define CFD_MOVE_N\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int width, height;\n"
        "\n"
        "    // Read the width and height with scanf\n"
        "\n"
        "    // Print the area and the number of steps\n"
        "\n"
        "    // Drive to the top-right corner with move_n and turn_left\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "scanf(\"%d %d\", &width, &height); The area is width * height and the trip is (width - 1) + (height - 1) steps. Drive with move_n(width - 1); turn_left(); move_n(height - 1);",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int width, height;\n"
        "    scanf(\"%d %d\", &width, &height);\n"
        "\n"
        "    printf(\"Area: %d\\n\", width * height);\n"
        "    printf(\"Steps: %d\\n\", (width - 1) + (height - 1));\n"
        "\n"
        "    move_n(width - 1);\n"
        "    turn_left();\n"
        "    move_n(height - 1);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "7 6\n"
          "#######\n"
          "#....E#\n"
          "#.....#\n"
          "#.....#\n"
          "#>....#\n"
          "#######\n"
          "5 4\n",
          "Area: 20\n"
          "Steps: 7\n" },
        { "4 8\n"
          "####\n"
          "#.E#\n"
          "#..#\n"
          "#..#\n"
          "#..#\n"
          "#..#\n"
          "#>.#\n"
          "####\n"
          "2 6\n",
          "Area: 12\n"
          "Steps: 6\n" },
        { "11 5\n"
          "###########\n"
          "#........E#\n"
          "#.........#\n"
          "#>........#\n"
          "###########\n"
          "9 3\n",
          "Area: 27\n"
          "Steps: 10\n" },
    },
},

// =====================================================================
{
    .id = "robot-signs", .level = "c-decisions-switch", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_C,
    .title = "Signposts",
    .brief =
        "Two tiles ahead is a junction with a signpost. Only one way leads to the exit, and the sign says which. Stand on the sign and read_sign() returns 'L' (turn left), 'R' (turn right) or 'S' (straight on).\n"
        "\n"
        "Drive onto the sign, switch on read_sign(), turn the right way and say where you're going:\n"
        "'L': print Turning left\n"
        "'R': print Turning right\n"
        "'S': print Going straight\n"
        "\n"
        "Then drive the last two tiles to the exit. There are three mazes, one for each sign.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n"
        "turn_left()     turn 90 degrees left\n"
        "turn_right()    turn 90 degrees right\n"
        "read_sign()     the sign you stand on: 'L', 'R' or 'S'\n",
    .config = "#define CFD_SIGNS\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    move();\n"
        "    move();\n"
        "\n"
        "    // Read the sign and use a switch to turn the right way\n"
        "\n"
        "    move();\n"
        "    move();\n"
        "    return 0;\n"
        "}\n",
    .hint = "switch (read_sign()) { case 'L': printf(\"Turning left\\n\"); turn_left(); break; ... } - remember the break at the end of each case, or the robot falls through into the next one.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    move();\n"
        "    move();\n"
        "\n"
        "    switch (read_sign()) {\n"
        "        case 'L':\n"
        "            printf(\"Turning left\\n\");\n"
        "            turn_left();\n"
        "            break;\n"
        "        case 'R':\n"
        "            printf(\"Turning right\\n\");\n"
        "            turn_right();\n"
        "            break;\n"
        "        default:\n"
        "            printf(\"Going straight\\n\");\n"
        "            break;\n"
        "    }\n"
        "\n"
        "    move();\n"
        "    move();\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "7 7\n"
          "#######\n"
          "#######\n"
          "#######\n"
          "#E.L###\n"
          "###.###\n"
          "###^###\n"
          "#######\n",
          "Turning left\n" },
        { "7 7\n"
          "#######\n"
          "#######\n"
          "#######\n"
          "###R.E#\n"
          "###.###\n"
          "###^###\n"
          "#######\n",
          "Turning right\n" },
        { "7 7\n"
          "#######\n"
          "###E###\n"
          "###.###\n"
          "###S###\n"
          "###.###\n"
          "###^###\n"
          "#######\n",
          "Going straight\n" },
    },
},

// =====================================================================
{
    .id = "robot-stairs", .level = "c-loops-2", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_C,
    .title = "The Staircase",
    .brief =
        "A staircase leads up to the exit, but every staircase has a different number of steps. Climbing one step is always the same four actions:\n"
        "\n"
        "move();  turn_left();  move();  turn_right();\n"
        "\n"
        "Repeat them until the robot is standing on the exit - at_exit() tells you when - and count the steps as you go. Then print the count, like Steps: 3.\n"
        "\n"
        "One of the staircases is seven steps high, so typing every move out gets long. The third star is for doing it with a loop.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n"
        "turn_left()     turn 90 degrees left\n"
        "turn_right()    turn 90 degrees right\n"
        "wall_ahead()    1 if a wall is right in front\n"
        "at_exit()       1 when you stand on the exit\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int steps = 0;\n"
        "\n"
        "    // Climb until at_exit() is true, counting the steps\n"
        "\n"
        "    printf(\"Steps: %d\\n\", steps);\n"
        "    return 0;\n"
        "}\n",
    .hint = "while (!at_exit()) { move(); turn_left(); move(); turn_right(); steps++; } - the ! means \"not\", so the loop runs while the robot is NOT on the exit.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int steps = 0;\n"
        "\n"
        "    while (!at_exit()) {\n"
        "        move();\n"
        "        turn_left();\n"
        "        move();\n"
        "        turn_right();\n"
        "        steps++;\n"
        "    }\n"
        "\n"
        "    printf(\"Steps: %d\\n\", steps);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "6 6\n"
          "######\n"
          "####E#\n"
          "###C.#\n"
          "##C.##\n"
          "#>.###\n"
          "######\n",
          "Steps: 3\n" },
        { "8 8\n"
          "########\n"
          "######E#\n"
          "#####C.#\n"
          "####C.##\n"
          "###C.###\n"
          "##C.####\n"
          "#>.#####\n"
          "########\n",
          "Steps: 5\n" },
        { "4 4\n"
          "####\n"
          "##E#\n"
          "#>.#\n"
          "####\n",
          "Steps: 1\n" },
        { "10 10\n"
          "##########\n"
          "########E#\n"
          "#######C.#\n"
          "######C.##\n"
          "#####C.###\n"
          "####C.####\n"
          "###C.#####\n"
          "##C.######\n"
          "#>.#######\n"
          "##########\n",
          "Steps: 7\n" },
    },
},

// =====================================================================
{
    .id = "robot-route", .level = "c-arrays-2d", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_C,
    .title = "There and Back Again",
    .brief =
        "The robot's memory chip holds the route to some buried treasure. The input gives the number of legs, then each leg's distance and the turn to make after it - L or R, and X after the last leg:\n"
        "\n"
        "3\n"
        "4 L\n"
        "2 R\n"
        "3 X\n"
        "\n"
        "1. Read the whole route into arrays: an int array for the distances and a char array for the turns.\n"
        "2. Print the total distance, like Total distance: 9\n"
        "3. Drive the route to the treasure coin at the end.\n"
        "4. Bring it home: turn around and drive the legs in reverse - last leg first - turning the opposite way at every corner. The exit is one tile behind where the robot started.\n"
        "\n"
        "Step 4 is why you keep the route in arrays: you need it again, backwards.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n"
        "turn_left()     turn 90 degrees left\n"
        "turn_right()    turn 90 degrees right\n"
        "at_exit()       1 when you stand on the exit\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int legs;\n"
        "    int dist[10];\n"
        "    char turn[10];\n"
        "\n"
        "    scanf(\"%d\", &legs);\n"
        "    // Read each leg into dist[i] and turn[i] - use \" %c\" to read the letter\n"
        "\n"
        "    // Print the total distance, drive the route, then drive it back in reverse\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Read with scanf(\"%d %c\", &dist[i], &turn[i]);. Turn around with two turn_left()s. On the way back loop i from legs - 1 down to 0: drive dist[i] tiles, then if i > 0 make the opposite of turn[i - 1] ('L' becomes turn_right). Finish with one more move().",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int legs;\n"
        "    int dist[10];\n"
        "    char turn[10];\n"
        "\n"
        "    scanf(\"%d\", &legs);\n"
        "    for (int i = 0; i < legs; i++) {\n"
        "        scanf(\"%d %c\", &dist[i], &turn[i]);\n"
        "    }\n"
        "\n"
        "    int total = 0;\n"
        "    for (int i = 0; i < legs; i++) {\n"
        "        total += dist[i];\n"
        "    }\n"
        "    printf(\"Total distance: %d\\n\", total);\n"
        "\n"
        "    // There...\n"
        "    for (int i = 0; i < legs; i++) {\n"
        "        for (int s = 0; s < dist[i]; s++) {\n"
        "            move();\n"
        "        }\n"
        "        if (turn[i] == 'L') turn_left();\n"
        "        if (turn[i] == 'R') turn_right();\n"
        "    }\n"
        "\n"
        "    // ...and back again\n"
        "    turn_left();\n"
        "    turn_left();\n"
        "    for (int i = legs - 1; i >= 0; i--) {\n"
        "        for (int s = 0; s < dist[i]; s++) {\n"
        "            move();\n"
        "        }\n"
        "        if (i > 0 && turn[i - 1] == 'L') turn_right();\n"
        "        if (i > 0 && turn[i - 1] == 'R') turn_left();\n"
        "    }\n"
        "    move();\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "11 5\n"
          "###########\n"
          "######...C#\n"
          "######.####\n"
          "#E>....####\n"
          "###########\n"
          "3\n"
          "4 L\n"
          "2 R\n"
          "3 X\n",
          "Total distance: 9\n" },
        { "8 6\n"
          "########\n"
          "#E>..###\n"
          "####.###\n"
          "####.#C#\n"
          "####...#\n"
          "########\n"
          "4\n"
          "2 R\n"
          "3 L\n"
          "2 L\n"
          "1 X\n",
          "Total distance: 8\n" },
        { "10 3\n"
          "##########\n"
          "#E>.....C#\n"
          "##########\n"
          "1\n"
          "6 X\n",
          "Total distance: 6\n" },
    },
},

// =====================================================================
{
    .id = "robot-commands", .level = "c-strings", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_C,
    .title = "Command String",
    .brief =
        "Mission control sends the route as one word of commands: F means forward one tile, L means turn left and R means turn right, for example FFLFFRF. Some operators type in lower case, so f, l and r mean the same.\n"
        "\n"
        "Read the word with scanf(\"%99s\", route), then walk along the string one character at a time until you reach the '\\0' that ends it, doing what each letter says. Finally print how many commands there were, like Commands: 7 - count them yourself as you go.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n"
        "turn_left()     turn 90 degrees left\n"
        "turn_right()    turn 90 degrees right\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    char route[100];\n"
        "    scanf(\"%99s\", route);\n"
        "\n"
        "    // Walk along route until the '\\0', obeying each letter\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "int i = 0; while (route[i] != '\\0') { switch (route[i]) { case 'F': case 'f': move(); break; ... } i++; } - two cases stacked with nothing between them share the same code. When the loop ends, i is the number of commands.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    char route[100];\n"
        "    scanf(\"%99s\", route);\n"
        "\n"
        "    int i = 0;\n"
        "    while (route[i] != '\\0') {\n"
        "        switch (route[i]) {\n"
        "            case 'F':\n"
        "            case 'f':\n"
        "                move();\n"
        "                break;\n"
        "            case 'L':\n"
        "            case 'l':\n"
        "                turn_left();\n"
        "                break;\n"
        "            case 'R':\n"
        "            case 'r':\n"
        "                turn_right();\n"
        "                break;\n"
        "        }\n"
        "        i++;\n"
        "    }\n"
        "\n"
        "    printf(\"Commands: %d\\n\", i);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "6 5\n"
          "######\n"
          "###.E#\n"
          "###.##\n"
          "#>.C##\n"
          "######\n"
          "FFLFFRF\n",
          "Commands: 7\n" },
        { "8 5\n"
          "########\n"
          "#>C.####\n"
          "###.####\n"
          "###C..E#\n"
          "########\n"
          "ffrfflfff\n",
          "Commands: 9\n" },
        { "6 5\n"
          "######\n"
          "#>.###\n"
          "##.C##\n"
          "###.E#\n"
          "######\n"
          "FRFLFRFLF\n",
          "Commands: 9\n" },
    },
},

// =====================================================================
{
    .id = "robot-toolkit", .level = "c-functions-arrays", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_C,
    .title = "Build Your Own Toolkit",
    .brief =
        "Bad news: this robot's turn_right() is broken, and there's no move_n either. Good news: you can write them yourself out of the commands that still work. Write three functions:\n"
        "\n"
        "void turn_right(void) - three left turns make one right turn\n"
        "void turn_around(void) - two left turns\n"
        "void move_n(int steps) - move() steps times (steps is a copy of the number you pass: pass by value)\n"
        "\n"
        "Then drive the maze: 5 east, turn right, 3 south to grab the coin in the dead end, turn around, 3 back north, turn right, 4 east, turn right, 4 south to the exit. The third star is for short, tidy code - which is exactly what functions are for.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n"
        "turn_left()     turn 90 degrees left\n"
        "wall_ahead()    1 if a wall is right in front\n"
        "at_exit()       1 when you stand on the exit\n",
    .config = "#define CFD_NO_TURN_RIGHT\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "// Write turn_right, turn_around and move_n here\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // Drive the route using your functions\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "void turn_right(void) { turn_left(); turn_left(); turn_left(); } and void move_n(int steps) { for (int i = 0; i < steps; i++) move(); }. Functions must be written above main (or declared there with a prototype) before main can call them.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "void turn_right(void)\n"
        "{\n"
        "    turn_left();\n"
        "    turn_left();\n"
        "    turn_left();\n"
        "}\n"
        "\n"
        "void turn_around(void)\n"
        "{\n"
        "    turn_left();\n"
        "    turn_left();\n"
        "}\n"
        "\n"
        "void move_n(int steps)\n"
        "{\n"
        "    for (int i = 0; i < steps; i++) {\n"
        "        move();\n"
        "    }\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    move_n(5);\n"
        "    turn_right();\n"
        "    move_n(3);\n"
        "    turn_around();\n"
        "    move_n(3);\n"
        "    turn_right();\n"
        "    move_n(4);\n"
        "    turn_right();\n"
        "    move_n(4);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "12 7\n"
          "############\n"
          "#>.........#\n"
          "######.###.#\n"
          "######.###.#\n"
          "######C###.#\n"
          "##########E#\n"
          "############\n",
          NULL },
    },
},

// =====================================================================
{
    .id = "robot-labyrinth", .level = NULL, .region = 0,
    .role = ROLE_BOSS, .mode = MODE_ROBOT, .lang = LANG_C,
    .title = "Boss: The Labyrinth",
    .brief =
        "The final rescue on this coast: three mazes the robot has never seen, and one program that has to escape all of them. There's no route to copy - the robot has to find its own way.\n"
        "\n"
        "Use the right-hand rule: keep your right hand on the wall and you'll always find the exit of a maze like this.\n"
        "- If there's no wall on your right, turn right and step forward.\n"
        "- Otherwise, if the way ahead is clear, step forward.\n"
        "- Otherwise, turn left.\n"
        "Repeat until at_exit(). The coins are all on the right-hand path, so you'll collect them on the way.\n"
        "\n"
        "Put one step of the rule in its own function to keep main short, and mind the battery: 2000 moves and turns at most.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n"
        "turn_left()     turn 90 degrees left\n"
        "turn_right()    turn 90 degrees right\n"
        "wall_ahead()    1 if a wall is right in front\n"
        "wall_left()     1 if a wall is on your left\n"
        "wall_right()    1 if a wall is on your right\n"
        "at_exit()       1 when you stand on the exit\n",
    .config = "#define CFD_SIDE_SENSORS\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // Escape any maze with the right-hand rule\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Write void step(void) with an if / else if / else for the three rules, then main is just while (!at_exit()) { step(); }",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "void step(void)\n"
        "{\n"
        "    if (!wall_right()) {\n"
        "        turn_right();\n"
        "        move();\n"
        "    } else if (!wall_ahead()) {\n"
        "        move();\n"
        "    } else {\n"
        "        turn_left();\n"
        "    }\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    while (!at_exit()) {\n"
        "        step();\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "13 9\n"
          "#############\n"
          "#.#.....C.#E#\n"
          "#.#.#.###.#.#\n"
          "#.#.#.#...#.#\n"
          "#.#.#C#.###.#\n"
          "#.#.#.#C....#\n"
          "#.###.#####.#\n"
          "#>....#.....#\n"
          "#############\n",
          NULL },
        { "15 11\n"
          "###############\n"
          "#...#...#....E#\n"
          "###.#.#.#.###.#\n"
          "#.#.#.#.#...#.#\n"
          "#.#.#.#####.#.#\n"
          "#.#.#.......#.#\n"
          "#.#.#########.#\n"
          "#...#.......#.#\n"
          "#.###.#####C#.#\n"
          "#>..C.#.....C.#\n"
          "###############\n",
          NULL },
        { "17 11\n"
          "#################\n"
          "#.#.......#..C.E#\n"
          "#.###.#####.#.###\n"
          "#...#.......#...#\n"
          "###.#.#########.#\n"
          "#.#.#.#.......#.#\n"
          "#.#.#.#.#####.#.#\n"
          "#...#.#.#.....#.#\n"
          "#.#####.#.#####.#\n"
          "#>...CC.#.......#\n"
          "#################\n",
          NULL },
    },
},

// =====================================================================
{
    .id = "robot-cpp", .level = "cpp-hello", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_CPP,
    .title = "Hello, C++ Robot",
    .brief =
        "The robot's software has been upgraded to C++. It still understands move(), but now it talks with cout and listens with cin.\n"
        "\n"
        "After the corridor, the input gives the corridor's length. Read it with cin, then print two lines with cout:\n"
        "\n"
        "Hello, C++!\n"
        "Corridor: 6\n"
        "\n"
        "and drive that many tiles to the exit with a loop.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "move()          step forward one tile\n"
        "turn_left()     turn 90 degrees left\n"
        "turn_right()    turn 90 degrees right\n"
        "wall_ahead()    1 if a wall is right in front\n"
        "at_exit()       1 when you stand on the exit\n",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int length;\n"
        "\n"
        "    // Read the length with cin, print the two lines with cout, then drive\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "cin >> length; then cout << \"Hello, C++!\" << endl; and cout << \"Corridor: \" << length << endl; and finally for (int i = 0; i < length; i++) move();",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int length;\n"
        "    cin >> length;\n"
        "\n"
        "    cout << \"Hello, C++!\" << endl;\n"
        "    cout << \"Corridor: \" << length << endl;\n"
        "\n"
        "    for (int i = 0; i < length; i++) {\n"
        "        move();\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "6 3\n"
          "######\n"
          "#>..E#\n"
          "######\n"
          "3\n",
          "Hello, C++!\n"
          "Corridor: 3\n" },
        { "9 3\n"
          "#########\n"
          "#>.....E#\n"
          "#########\n"
          "6\n",
          "Hello, C++!\n"
          "Corridor: 6\n" },
        { "13 3\n"
          "#############\n"
          "#>.........E#\n"
          "#############\n"
          "10\n",
          "Hello, C++!\n"
          "Corridor: 10\n" },
    },
},

// =====================================================================
{
    .id = "robot-hologram", .level = "cpp-references", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_CPP,
    .title = "The Hologram Trap",
    .brief =
        "The rover is a Robot object, and its methods drive it: rover.move(), rover.turnRight(), rover.wallAhead() and so on.\n"
        "\n"
        "The starter code has a function escape(Robot r) that drives a robot through an L-shaped corridor: forward until a wall, turn right, and keep going until the exit. Press Run & Check and watch closely. The function gets a copy of the rover - a hologram - and the hologram walks out while the real rover never moves!\n"
        "\n"
        "Fix it by passing the robot by reference, so the function drives the real one. Then the coins line will be right too.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "rover.move()          step forward one tile\n"
        "rover.turnLeft()      turn left\n"
        "rover.turnRight()     turn right\n"
        "rover.wallAhead()     true if a wall is in front\n"
        "rover.atExit()        true on the exit\n"
        "rover.coins()         coins this robot has\n",
    .config = "#define CFD_ROVER\n",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "// This gets a COPY of the robot. Make it a reference.\n"
        "void escape(Robot r)\n"
        "{\n"
        "    while (!r.atExit()) {\n"
        "        if (r.wallAhead()) {\n"
        "            r.turnRight();\n"
        "        } else {\n"
        "            r.move();\n"
        "        }\n"
        "    }\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    escape(rover);\n"
        "    cout << \"Coins: \" << rover.coins() << endl;\n"
        "    return 0;\n"
        "}\n",
    .hint = "Add one character: void escape(Robot &r). With the & the parameter is another name for rover itself instead of a copy.",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "void escape(Robot &r)\n"
        "{\n"
        "    while (!r.atExit()) {\n"
        "        if (r.wallAhead()) {\n"
        "            r.turnRight();\n"
        "        } else {\n"
        "            r.move();\n"
        "        }\n"
        "    }\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    escape(rover);\n"
        "    cout << \"Coins: \" << rover.coins() << endl;\n"
        "    return 0;\n"
        "}\n",
    .testCount = 2,
    .tests = {
        { "7 6\n"
          "#######\n"
          "#>.C..#\n"
          "#####.#\n"
          "#####C#\n"
          "#####E#\n"
          "#######\n",
          "Coins: 2\n" },
        { "6 5\n"
          "######\n"
          "##>..#\n"
          "####C#\n"
          "#E...#\n"
          "######\n",
          "Coins: 1\n" },
    },
},

// =====================================================================
{
    .id = "robot-fleet", .level = "cpp-objects", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_ROBOT, .lang = LANG_CPP,
    .title = "Robot Fleet",
    .brief =
        "Two robots this time. red and blue are two objects of the same Robot class, and each object has its own position, direction and coins, so red.move() only moves red. red is the robot at the top of the map.\n"
        "\n"
        "Write void escape(Robot &r) - forward until a wall, then turn right, until the exit - and call it on both robots. Then print each robot's coins and the fleet's total:\n"
        "\n"
        "Red: 2\n"
        "Blue: 1\n"
        "Fleet: 3\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "red.move()          step forward one tile\n"
        "red.turnLeft()      turn left\n"
        "red.turnRight()     turn right\n"
        "red.wallAhead()     true if a wall is in front\n"
        "red.atExit()        true on the exit\n"
        "red.coins()         coins this robot has\n"
        "(blue has the same methods)\n",
    .config = "#define CFD_FLEET\n",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "// Write escape(Robot &r) here\n"
        "\n"
        "int main()\n"
        "{\n"
        "    // Escape with both robots, then print their coins\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "escape is the same function as in The Hologram Trap. In main: escape(red); escape(blue); then print red.coins(), blue.coins() and their sum.",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "void escape(Robot &r)\n"
        "{\n"
        "    while (!r.atExit()) {\n"
        "        if (r.wallAhead()) {\n"
        "            r.turnRight();\n"
        "        } else {\n"
        "            r.move();\n"
        "        }\n"
        "    }\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    escape(red);\n"
        "    escape(blue);\n"
        "\n"
        "    cout << \"Red: \" << red.coins() << endl;\n"
        "    cout << \"Blue: \" << blue.coins() << endl;\n"
        "    cout << \"Fleet: \" << red.coins() + blue.coins() << endl;\n"
        "    return 0;\n"
        "}\n",
    .testCount = 2,
    .tests = {
        { "8 9\n"
          "########\n"
          "#>.C.###\n"
          "####C###\n"
          "####E###\n"
          "########\n"
          "########\n"
          "#>..C..#\n"
          "######E#\n"
          "########\n",
          "Red: 2\n"
          "Blue: 1\n"
          "Fleet: 3\n" },
        { "6 12\n"
          "######\n"
          "#>C.##\n"
          "###.##\n"
          "###.##\n"
          "#E..##\n"
          "######\n"
          "######\n"
          "#>CC.#\n"
          "####.#\n"
          "####C#\n"
          "####E#\n"
          "######\n",
          "Red: 1\n"
          "Blue: 3\n"
          "Fleet: 4\n" },
    },
},

// =====================================================================
{
    .id = "robot-rally", .level = NULL, .region = 2,
    .role = ROLE_BOSS, .mode = MODE_ROBOT, .lang = LANG_CPP,
    .title = "Boss: Robot Rally",
    .brief =
        "The C++ final: red and blue are dropped into two different mazes, and both must escape. Build a class to do the driving:\n"
        "\n"
        "class Pilot with\n"
        "- a private reference member Robot &robot (a reference member can only be set in an initialisation list),\n"
        "- a constructor Pilot(Robot &r) that sets it,\n"
        "- a method void solve() that escapes with the right-hand rule: no wall on the right? turn right and move; else if the way ahead is clear, move; else turn left.\n"
        "\n"
        "Make one Pilot for each robot and solve both mazes. When both robots are on their exits, print Both robots escaped!\n"
        "\n"
        "If your Pilot stores a Robot instead of a Robot &, it gets a hologram copy - watch for it.\n"
        "\n"
        "Collect every coin (drive over it) and end your program standing on the exit. Driving into a wall ends the run.",
    .api =
        "red.move()          step forward one tile\n"
        "red.turnLeft()      turn left\n"
        "red.turnRight()     turn right\n"
        "red.wallAhead()     true if a wall is in front\n"
        "red.atExit()        true on the exit\n"
        "red.coins()         coins this robot has\n"
        "red.wallLeft()      true if a wall is on the left\n"
        "red.wallRight()     true if a wall is on the right\n"
        "(blue has the same methods)\n",
    .config = "#define CFD_FLEET\n",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "// Write the Pilot class here\n"
        "\n"
        "int main()\n"
        "{\n"
        "    // Make a Pilot for red and one for blue, and solve both mazes\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "class Pilot { public: Pilot(Robot &r) : robot(r) { } void solve() { while (!robot.atExit()) { ... } } private: Robot &robot; };",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "class Pilot {\n"
        "public:\n"
        "    Pilot(Robot &r) : robot(r), moves(0) { }\n"
        "\n"
        "    void solve()\n"
        "    {\n"
        "        while (!robot.atExit()) {\n"
        "            if (!robot.wallRight()) {\n"
        "                robot.turnRight();\n"
        "                drive();\n"
        "            } else if (!robot.wallAhead()) {\n"
        "                drive();\n"
        "            } else {\n"
        "                robot.turnLeft();\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "\n"
        "    int getMoves() const { return moves; }\n"
        "\n"
        "private:\n"
        "    void drive()\n"
        "    {\n"
        "        robot.move();\n"
        "        moves++;\n"
        "    }\n"
        "\n"
        "    Robot &robot;\n"
        "    int moves;\n"
        "};\n"
        "\n"
        "int main()\n"
        "{\n"
        "    Pilot redPilot(red);\n"
        "    Pilot bluePilot(blue);\n"
        "    redPilot.solve();\n"
        "    bluePilot.solve();\n"
        "\n"
        "    if (red.atExit() && blue.atExit()) {\n"
        "        cout << \"Both robots escaped!\" << endl;\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .testCount = 2,
    .tests = {
        { "23 9\n"
          "#######################\n"
          "#v....#...###v..#.#...#\n"
          "#####.#.#.#####.#.#.#.#\n"
          "#...#.#.#.###...#...#.#\n"
          "#.#.#.#.#.###.#######.#\n"
          "#.#...#.#.###.........#\n"
          "#.#####.#.###########.#\n"
          "#.......#E###........E#\n"
          "#######################\n",
          "Both robots escaped!\n" },
        { "23 9\n"
          "#######################\n"
          "#v..#.....###v#.......#\n"
          "###.#.#.#.###.#######.#\n"
          "#.#.#.#.#.###.......#.#\n"
          "#.#.#.#.###########.#.#\n"
          "#...#.#...###.#.....#.#\n"
          "#.#######.###.#.#####.#\n"
          "#........E###........E#\n"
          "#######################\n",
          "Both robots escaped!\n" },
    },
},

// =====================================================================
{
    .id = "heist-tipoff", .level = "c-pointers-2", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_HEIST, .lang = LANG_C,
    .title = "The Tip-Off",
    .brief =
        "Welcome to Memory City, where every house is a memory address and the only way to get around is by following pointers.\n"
        "\n"
        "Your fence sent a tip: int *tip points at the house with the loot. But fences lie - sometimes the tip is NULL, and going to the NULL house sets off the alarm.\n"
        "\n"
        "If tip isn't NULL: go(tip) to walk there, print the loot with printf(\"Loot: %d\\n\", *tip), then grab(tip).\n"
        "If tip is NULL: print Fake tip! and stay away.\n"
        "Either way, finish with go(&hideout) - & gives you the address of your hideout.",
    .api =
        "go(p)            walk to the house p points at\n"
        "grab(p)          take the loot from the house you stand at\n"
        "int *tip         the fence's tip (may be NULL!)\n"
        "int hideout      your hideout: go(&hideout)\n",
    .config = "#define CFD_TIP\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // Check the tip, follow it if it's real, then head home\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "if (tip != NULL) { go(tip); printf(\"Loot: %d\\n\", *tip); grab(tip); } else { printf(\"Fake tip!\\n\"); } go(&hideout); - read *tip before grab() empties the house.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    if (tip != NULL) {\n"
        "        go(tip);\n"
        "        printf(\"Loot: %d\\n\", *tip);\n"
        "        grab(tip);\n"
        "    } else {\n"
        "        printf(\"Fake tip!\\n\");\n"
        "    }\n"
        "\n"
        "    go(&hideout);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "STREET 8 0 0 0 0 120 0 0 0\n"
          "TIP 4\n"
          "END\n",
          "Loot: 120\n" },
        { "STREET 8 0 75 0 0 0 0 0 0\n"
          "TIP 1\n"
          "END\n",
          "Loot: 75\n" },
        { "STREET 8 0 0 0 50 0 0 0 0\n"
          "TIP -1\n"
          "END\n",
          "Fake tip!\n" },
        { "STREET 6 0 0 0 0 0 999\n"
          "TIP 5\n"
          "END\n",
          "Loot: 999\n" },
    },
},

// =====================================================================
{
    .id = "heist-block", .level = "c-pointer-arith", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_HEIST, .lang = LANG_C,
    .title = "Walk the Block",
    .brief =
        "Tonight only some houses are unguarded: the count houses starting at the one first points to - first, first + 1, first + 2 and so on. The houses on either side have guards.\n"
        "\n"
        "Walk the block with a pointer: start at first and do p++ while p < first + count. At each house go(p), and if it has loot (*p > 0) add it to your total and grab(p). Step outside the block and the guards catch you.\n"
        "\n"
        "At the end print your haul, like Total: 450.",
    .api =
        "go(p)            walk to the house p points at\n"
        "grab(p)          take the loot from the house you stand at\n"
        "int *first       the first unguarded house\n"
        "int count        how many houses are unguarded\n",
    .config = "#define CFD_BLOCK\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int total = 0;\n"
        "\n"
        "    // Walk from first to first + count - 1 with a pointer\n"
        "\n"
        "    printf(\"Total: %d\\n\", total);\n"
        "    return 0;\n"
        "}\n",
    .hint = "for (int *p = first; p < first + count; p++) { go(p); if (*p > 0) { total += *p; grab(p); } }",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int total = 0;\n"
        "\n"
        "    for (int *p = first; p < first + count; p++) {\n"
        "        go(p);\n"
        "        if (*p > 0) {\n"
        "            total += *p;\n"
        "            grab(p);\n"
        "        }\n"
        "    }\n"
        "\n"
        "    printf(\"Total: %d\\n\", total);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "STREET 10 5 0 100 0 200 150 0 0 0 9\n"
          "BLOCK 2 4\n"
          "END\n",
          "Total: 450\n" },
        { "STREET 8 30 20 10 0 0 0 0 99\n"
          "BLOCK 0 3\n"
          "END\n",
          "Total: 60\n" },
        { "STREET 12 1 1 1 1 1 0 70 0 80 0 1 1\n"
          "BLOCK 5 5\n"
          "END\n",
          "Total: 150\n" },
    },
},

// =====================================================================
{
    .id = "heist-van", .level = "c-malloc", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_HEIST, .lang = LANG_C,
    .title = "The Getaway Van",
    .brief =
        "Big job tonight - too much loot to carry. You need a getaway van: memory from malloc, with room for one int per house in the block.\n"
        "\n"
        "1. int *van = malloc(count * sizeof(int)); and check it isn't NULL.\n"
        "2. Print the van's size: Van: 20 bytes\n"
        "3. Walk the block. At each house copy its loot into the van (van[i] = first[i]) and then grab it.\n"
        "4. drive(van, count) to take it all to the hideout.\n"
        "5. free(van). Leaving the van behind - a memory leak - gets you caught, and so does freeing it twice.",
    .api =
        "go(p)            walk to the house p points at\n"
        "grab(p)          take the loot from the house you stand at\n"
        "int *first       the first unguarded house\n"
        "int count        how many houses are unguarded\n"
        "drive(van, n)    drive the van (from malloc) to the hideout\n",
    .config = "#define CFD_BLOCK\n"
        "#define CFD_MALLOC\n"
        "#define CFD_VAN\n",
    .starterCode =
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // Get a van from malloc, load it, drive it and free it\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Print the size with printf(\"Van: %d bytes\\n\", (int)(count * sizeof(int)));. In the loop: go(first + i); van[i] = first[i]; grab(first + i);. After drive(van, count); call free(van); - once.",
    .solution =
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int *van = (int*)malloc(count * sizeof(int));\n"
        "    if (van == NULL) {\n"
        "        printf(\"No van!\\n\");\n"
        "        return 1;\n"
        "    }\n"
        "    printf(\"Van: %d bytes\\n\", (int)(count * sizeof(int)));\n"
        "\n"
        "    for (int i = 0; i < count; i++) {\n"
        "        go(first + i);\n"
        "        van[i] = first[i];\n"
        "        grab(first + i);\n"
        "    }\n"
        "\n"
        "    drive(van, count);\n"
        "    free(van);\n"
        "    van = NULL;\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "STREET 8 0 40 10 0 25 60 5 0\n"
          "BLOCK 1 5\n"
          "END\n",
          "Van: 20 bytes\n" },
        { "STREET 5 7 8 9 0 0\n"
          "BLOCK 0 3\n"
          "END\n",
          "Van: 12 bytes\n" },
        { "STREET 10 1 2 3 4 5 6 7 8 9 10\n"
          "BLOCK 2 7\n"
          "END\n",
          "Van: 28 bytes\n" },
    },
},

// =====================================================================
{
    .id = "heist-gadgets", .level = "c-scope-funcptr", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_HEIST, .lang = LANG_C,
    .title = "Gadget Belt",
    .brief =
        "The vault has a row of locked doors, and every lock needs the right gadget. Your gadgets are functions:\n"
        "pick opens 'K' keyhole locks, drill opens 'S' steel locks and blast opens 'W' wall vaults.\n"
        "\n"
        "Keep a function pointer, int (*gadget)(int), and for each door point it at the right function for lock_type(), then hand it to use(gadget). The wrong gadget sets off the alarm. Keep going while doors_left() > 0.\n"
        "\n"
        "Every gadget adds to the global variable noise. When all the doors are open, print it: Noise: 14\n"
        "(Don't declare your own noise inside main - it would shadow the global one and print 0.)",
    .api =
        "int (*gadget)(int)  a pointer to a gadget function\n"
        "pick, drill, blast   the gadgets\n"
        "lock_type()      the next lock: 'K', 'S' or 'W'\n"
        "doors_left()     how many doors are still locked\n"
        "use(gadget)      try a gadget on the next door\n"
        "int noise        global: noise you've made\n",
    .config = "#define CFD_DOORS\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int (*gadget)(int) = NULL;\n"
        "\n"
        "    // While doors are left: point gadget at the right function and use(gadget)\n"
        "\n"
        "    printf(\"Noise: %d\\n\", noise);\n"
        "    return 0;\n"
        "}\n",
    .hint = "while (doors_left() > 0) { switch (lock_type()) { case 'K': gadget = pick; break; case 'S': gadget = drill; break; case 'W': gadget = blast; break; } use(gadget); } - a function's name without brackets is its address.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int (*gadget)(int) = NULL;\n"
        "\n"
        "    while (doors_left() > 0) {\n"
        "        switch (lock_type()) {\n"
        "            case 'K': gadget = pick;  break;\n"
        "            case 'S': gadget = drill; break;\n"
        "            case 'W': gadget = blast; break;\n"
        "        }\n"
        "        use(gadget);\n"
        "    }\n"
        "\n"
        "    printf(\"Noise: %d\\n\", noise);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "DOORS KSW\n"
          "END\n",
          "Noise: 14\n" },
        { "DOORS WWKS\n"
          "END\n",
          "Noise: 24\n" },
        { "DOORS KKKK\n"
          "END\n",
          "Noise: 4\n" },
        { "DOORS S\n"
          "END\n",
          "Noise: 3\n" },
    },
},

// =====================================================================
{
    .id = "heist-estate", .level = "c-structs", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_HEIST, .lang = LANG_C,
    .title = "Floor Plans",
    .brief =
        "You've got hold of the estate's floor plans: an array of struct House, one per house, with its number, its loot and whether its alarm is on.\n"
        "\n"
        "Walk the estate with a struct House pointer - h = start, then h++ while h < start + count - and use -> to read the fields. Rob every house that has loot and no alarm: robbing a house with h->alarm switched on gets you caught. Then print:\n"
        "\n"
        "Robbed: 3\n"
        "Haul: 450",
    .api =
        "struct House {\n"
        "    int number;\n"
        "    int loot;\n"
        "    int alarm;\n"
        "    struct House *next;\n"
        "};\n"
        "struct House *start   the first house\n"
        "int count             how many houses\n"
        "rob(h)                rob the house h points at\n",
    .config = "#define CFD_ESTATE\n",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int robbed = 0;\n"
        "    int haul = 0;\n"
        "\n"
        "    // Walk the estate with a struct House pointer and rob the safe houses\n"
        "\n"
        "    printf(\"Robbed: %d\\n\", robbed);\n"
        "    printf(\"Haul: %d\\n\", haul);\n"
        "    return 0;\n"
        "}\n",
    .hint = "for (struct House *h = start; h < start + count; h++) { if (!h->alarm && h->loot > 0) { haul += h->loot; rob(h); robbed++; } } - add up h->loot before rob() empties the house.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int robbed = 0;\n"
        "    int haul = 0;\n"
        "\n"
        "    for (struct House *h = start; h < start + count; h++) {\n"
        "        if (!h->alarm && h->loot > 0) {\n"
        "            haul += h->loot;\n"
        "            rob(h);\n"
        "            robbed++;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    printf(\"Robbed: %d\\n\", robbed);\n"
        "    printf(\"Haul: %d\\n\", haul);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "ESTATE 5\n"
          "12 100 0\n"
          "14 200 1\n"
          "16 150 0\n"
          "18 0 0\n"
          "20 200 0\n"
          "END\n",
          "Robbed: 3\n"
          "Haul: 450\n" },
        { "ESTATE 4\n"
          "1 50 1\n"
          "3 60 1\n"
          "5 70 0\n"
          "7 80 1\n"
          "END\n",
          "Robbed: 1\n"
          "Haul: 70\n" },
        { "ESTATE 6\n"
          "2 10 0\n"
          "4 20 0\n"
          "6 30 0\n"
          "8 0 0\n"
          "10 40 0\n"
          "12 50 0\n"
          "END\n",
          "Robbed: 5\n"
          "Haul: 150\n" },
    },
},

// =====================================================================
{
    .id = "heist-bigheist", .level = NULL, .region = 1,
    .role = ROLE_BOSS, .mode = MODE_HEIST, .lang = LANG_C,
    .title = "Boss: The Big Heist",
    .brief =
        "The big one. The loot is hidden along a trail of houses: each house's next pointer leads to the next house in the trail, and the last house's next is NULL. The houses that aren't on the trail are decoys with their alarms on.\n"
        "\n"
        "1. Walk the trail once (h = first_clue; follow h = h->next until NULL) to count its houses.\n"
        "2. malloc an array of ints with room for that many house numbers, and check it isn't NULL.\n"
        "3. Walk the trail again: store each house's number, add up its loot, and rob(h) it.\n"
        "4. Give the fence your list with report(numbers, n), then print\n"
        "Houses: 4\n"
        "Total: 435\n"
        "5. free the list. A leak, a double free or a NULL house sets off the alarm.",
    .api =
        "struct House {\n"
        "    int number;\n"
        "    int loot;\n"
        "    int alarm;\n"
        "    struct House *next;\n"
        "};\n"
        "struct House *first_clue  the first house in the trail\n"
        "rob(h)                rob the house h points at\n"
        "report(numbers, n)    hand the fence your list (from malloc)\n",
    .config = "#define CFD_TRAIL\n"
        "#define CFD_MALLOC\n",
    .starterCode =
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // 1. Count the houses on the trail\n"
        "\n"
        "    // 2. malloc a list for their numbers\n"
        "\n"
        "    // 3. Rob the trail and fill the list\n"
        "\n"
        "    // 4. report(), print the totals, 5. free the list\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Counting: for (struct House *h = first_clue; h != NULL; h = h->next) n++;. Then int *numbers = (int*)malloc(n * sizeof(int));. In the second loop: numbers[i] = h->number; total += h->loot; rob(h); i++;",
    .solution =
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n = 0;\n"
        "    for (struct House *h = first_clue; h != NULL; h = h->next) {\n"
        "        n++;\n"
        "    }\n"
        "\n"
        "    int *numbers = (int*)malloc(n * sizeof(int));\n"
        "    if (numbers == NULL) {\n"
        "        printf(\"Out of memory\\n\");\n"
        "        return 1;\n"
        "    }\n"
        "\n"
        "    int total = 0, i = 0;\n"
        "    for (struct House *h = first_clue; h != NULL; h = h->next) {\n"
        "        numbers[i] = h->number;\n"
        "        total += h->loot;\n"
        "        rob(h);\n"
        "        i++;\n"
        "    }\n"
        "\n"
        "    report(numbers, n);\n"
        "    printf(\"Houses: %d\\n\", n);\n"
        "    printf(\"Total: %d\\n\", total);\n"
        "\n"
        "    free(numbers);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "ESTATE 8\n"
          "2 40 1\n"
          "4 0 0\n"
          "6 75 0\n"
          "8 120 1\n"
          "10 15 1\n"
          "12 300 0\n"
          "14 60 0\n"
          "16 5 1\n"
          "TRAIL 4 5 1 6 2\n"
          "END\n",
          "Houses: 4\n"
          "Total: 435\n" },
        { "ESTATE 8\n"
          "2 40 0\n"
          "4 0 1\n"
          "6 75 1\n"
          "8 120 1\n"
          "10 15 1\n"
          "12 300 1\n"
          "14 60 1\n"
          "16 5 0\n"
          "TRAIL 2 7 0\n"
          "END\n",
          "Houses: 2\n"
          "Total: 45\n" },
        { "ESTATE 8\n"
          "2 40 0\n"
          "4 0 0\n"
          "6 75 0\n"
          "8 120 0\n"
          "10 15 0\n"
          "12 300 1\n"
          "14 60 0\n"
          "16 5 1\n"
          "TRAIL 6 0 2 4 6 1 3\n"
          "END\n",
          "Houses: 6\n"
          "Total: 310\n" },
    },
},

// =====================================================================
{
    .id = "heist-crates", .level = "cpp-new-delete", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_HEIST, .lang = LANG_CPP,
    .title = "Smuggler's Crates",
    .brief =
        "The same block of houses, but this crew works in C++. Make room for the loot with new:\n"
        "\n"
        "1. int *crates = new int[count];\n"
        "2. Walk the block: go(first + i), copy the loot into crates[i], then grab it.\n"
        "3. deliver(crates, count), then print Crates: 5\n"
        "4. Free them with delete[] crates; - not delete! They came from new[], so they need delete[]. A mismatch, a missing delete or deleting twice sets off the alarm.",
    .api =
        "go(p)            walk to the house p points at\n"
        "grab(p)          take the loot from the house you stand at\n"
        "int *first       the first unguarded house\n"
        "int count        how many houses are unguarded\n"
        "deliver(c, n)    hand over the crates (from new[])\n",
    .config = "#define CFD_BLOCK\n"
        "#define CFD_NEW\n"
        "#define CFD_CRATES\n",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    // Make crates with new int[count], fill them, deliver them, then delete[] them\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "for (int i = 0; i < count; i++) { go(first + i); crates[i] = first[i]; grab(first + i); } then deliver(crates, count); and finish with delete[] crates;",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int *crates = new int[count];\n"
        "\n"
        "    for (int i = 0; i < count; i++) {\n"
        "        go(first + i);\n"
        "        crates[i] = first[i];\n"
        "        grab(first + i);\n"
        "    }\n"
        "\n"
        "    deliver(crates, count);\n"
        "    cout << \"Crates: \" << count << endl;\n"
        "\n"
        "    delete[] crates;\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "STREET 8 0 40 10 0 25 60 5 0\n"
          "BLOCK 1 5\n"
          "END\n",
          "Crates: 5\n" },
        { "STREET 5 7 8 9 0 0\n"
          "BLOCK 0 3\n"
          "END\n",
          "Crates: 3\n" },
        { "STREET 8 3 1 4 1 5 9 2 6\n"
          "BLOCK 3 4\n"
          "END\n",
          "Crates: 4\n" },
    },
},

// =====================================================================
{
    .id = "heist-chain", .level = "cpp-linked-list", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_HEIST, .lang = LANG_CPP,
    .title = "The Safehouse Chain",
    .brief =
        "Your crew left a chain of safehouses: a linked list of Nodes made with new. head points to the first one, and each node's next points to the next safehouse (nullptr at the end). node->house says which street house it is.\n"
        "\n"
        "Walk the chain from head. At each node, go(&street[node->house]) and grab the stash there. Then burn the safehouse: delete the node - but save its next pointer first, or you lose the rest of the chain! Every node must be deleted exactly once.\n"
        "\n"
        "Finally print Safehouses: 3",
    .api =
        "struct Node {\n"
        "    int house;\n"
        "    Node *next;\n"
        "};\n"
        "Node *head       the first safehouse\n"
        "int street[]     the houses\n"
        "go(p)            walk to the house p points at\n"
        "grab(p)          take the loot from the house you stand at\n",
    .config = "#define CFD_CHAIN\n"
        "#define CFD_NEW\n",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int visited = 0;\n"
        "\n"
        "    // Walk the chain from head: visit each safehouse, then delete its node\n"
        "\n"
        "    cout << \"Safehouses: \" << visited << endl;\n"
        "    return 0;\n"
        "}\n",
    .hint = "Node *cur = head; while (cur != nullptr) { go(&street[cur->house]); grab(&street[cur->house]); Node *next = cur->next; delete cur; cur = next; visited++; } - read cur->next before the delete, never after.",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int visited = 0;\n"
        "\n"
        "    Node *cur = head;\n"
        "    while (cur != nullptr) {\n"
        "        go(&street[cur->house]);\n"
        "        grab(&street[cur->house]);\n"
        "        Node *next = cur->next;\n"
        "        delete cur;\n"
        "        cur = next;\n"
        "        visited++;\n"
        "    }\n"
        "    head = nullptr;\n"
        "\n"
        "    cout << \"Safehouses: \" << visited << endl;\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "STREET 8 0 50 0 80 0 0 30 0\n"
          "CHAIN 3 3 1 6\n"
          "END\n",
          "Safehouses: 3\n" },
        { "STREET 5 10 20 30 40 50\n"
          "CHAIN 5 0 1 2 3 4\n"
          "END\n",
          "Safehouses: 5\n" },
        { "STREET 10 0 0 0 0 0 0 0 0 0 77\n"
          "CHAIN 1 9\n"
          "END\n",
          "Safehouses: 1\n" },
    },
},

// =====================================================================
{
    .id = "bugs-hello", .level = "c-hello", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_C,
    .title = "Compiler Clues",
    .brief =
        "Two bugs have crawled into Hello, World and now it won't even compile. Don't panic at the error messages - they're clues: each one names a line and says what the compiler expected. Read the first error first.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "The header for printf is stdio.h - standard input/output. And every statement ends with a semicolon.",
    .timeLimit = 120,
    .bugCount = 2,
    .bugs = {
        { "#include <stdio.h>",
          "#include <stdoi.h>",
          "The header is stdio.h. \"No such file or directory\" pointed straight at it." },
        { "    printf(\"Hello, World!\\n\");",
          "    printf(\"Hello, World!\\n\")",
          "Every statement ends with a semicolon. The compiler noticed on the next line - errors often show up just after the real mistake." },
    },
},

// =====================================================================
{
    .id = "bugs-grades", .level = "c-decisions-switch", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_C,
    .title = "Wrong Grades",
    .brief =
        "The Grade Report gives out the wrong grades: a score of exactly 90 isn't an A, and B students get two comments. Two bugs are hiding in the decisions.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "Is 90 an A? Then the check needs >=. And look at what happens at the end of case 'B'.",
    .timeLimit = 180,
    .bugCount = 2,
    .bugs = {
        { "    if (score >= 90) {",
          "    if (score > 90) {",
          "90 counts as an A, so the check needs >=, not >." },
        { "            printf(\"Good job!\\n\");\n"
          "            break;",
          "            printf(\"Good job!\\n\");",
          "Without break, case 'B' fell through into case 'C' and printed both comments." },
    },
},

// =====================================================================
{
    .id = "bugs-loops", .level = "c-loops-2", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_C,
    .title = "Off by One",
    .brief =
        "The times table is missing its last column and the total is way off. Two classic loop bugs.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "Which column does the inner loop stop at? And does total add up, or get replaced each time?",
    .timeLimit = 180,
    .bugCount = 2,
    .bugs = {
        { "        for (int col = 1; col <= n; col++) {",
          "        for (int col = 1; col < n; col++) {",
          "col has to reach n itself, so the loop needs <=. Off-by-one is the most common loop bug of all." },
        { "            total += row * col;",
          "            total = row * col;",
          "= replaces the total each time; += adds to it." },
    },
},

// =====================================================================
{
    .id = "bugs-matrix", .level = "c-arrays-2d", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_C,
    .title = "Out of Bounds",
    .brief =
        "Matrix Sums prints the grid the wrong way round and finds an extra row that isn't there. One bug reads past the end of the data - C never checks array bounds for you.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "grid[row][col] - the row always comes first. And the row-sum loop should stop before rows, not at it.",
    .timeLimit = 180,
    .bugCount = 2,
    .bugs = {
        { "            printf(\"%4d\", grid[r][c]);",
          "            printf(\"%4d\", grid[c][r]);",
          "grid[row][col]: the row comes first. Swapping them prints the wrong cells." },
        { "    printf(\"Row sums:\");\n"
          "    for (int r = 0; r < rows; r++) {",
          "    printf(\"Row sums:\");\n"
          "    for (int r = 0; r <= rows; r++) {",
          "<= rows reads one row past the end of the data." },
    },
},

// =====================================================================
{
    .id = "bugs-reverse", .level = "c-strings", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_C,
    .title = "Backwards Bugs",
    .brief =
        "Word Reverser says every word is 100 letters long and loses the first letter. Two string bugs.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "sizeof measures the whole array; strlen counts the characters before the '\\0'. And where is the last character?",
    .timeLimit = 180,
    .bugCount = 2,
    .bugs = {
        { "    int len = strlen(word);",
          "    int len = sizeof(word);",
          "sizeof gives the size of the whole array (100 bytes); strlen counts the characters before the '\\0'." },
        { "    for (int i = len - 1; i >= 0; i--) {",
          "    for (int i = len; i > 0; i--) {",
          "The last character is at len - 1 (word[len] is the '\\0'), and the loop must still run for i = 0." },
    },
},

// =====================================================================
{
    .id = "bugs-swap", .level = "c-pointers-2", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_C,
    .title = "Pointer Panic",
    .brief =
        "Sort Three doesn't sort anything. The bugs are in how the pointers are used: one changes the wrong thing, and one compares the wrong thing.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "a is an address, *a is the number at that address. Which one should swap change, and which should sortThree compare?",
    .timeLimit = 180,
    .bugCount = 2,
    .bugs = {
        { "    *a = *b;",
          "    a = b;",
          "a = b only changes the local pointer. *a = *b changes the number it points at." },
        { "    if (*b > *c) swap(b, c);",
          "    if (b > c) swap(b, c);",
          "b > c compares the two addresses, not the numbers - you need *b > *c." },
    },
},

// =====================================================================
{
    .id = "bugs-numbers", .level = "c-malloc", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_C,
    .title = "Memory Mix-up",
    .brief =
        "How Many Numbers forgets the first number when it prints them backwards, and thinks the largest of -6 is 0.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "Where should the search for the largest start? And does the reverse loop ever reach index 0?",
    .timeLimit = 180,
    .bugCount = 2,
    .bugs = {
        { "    int largest = numbers[0];",
          "    int largest = 0;",
          "Starting the search at 0 goes wrong when every number is negative. Start with the first element." },
        { "    for (int i = count - 1; i >= 0; i--) {",
          "    for (int i = count - 1; i > 0; i--) {",
          "i > 0 stops before index 0, so the first number never got printed." },
    },
},

// =====================================================================
{
    .id = "bugs-files", .level = "c-files", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_BUGS, .lang = LANG_C,
    .title = "Broken Save File",
    .brief =
        "Files can't really go wrong in a robot maze or a heist, so this level's game is Bug Hunter. The Save and Load program has three bugs: it can't save, it reads the file back too early, and it adds up the numbers wrongly. Find them before time runs out.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "Which mode opens a file for writing? What has to happen to a file before you read it back? And = or +=?",
    .timeLimit = 240,
    .bugCount = 3,
    .bugs = {
        { "    FILE *out = fopen(\"scores.txt\", \"w\");",
          "    FILE *out = fopen(\"scores.txt\", \"r\");",
          "\"r\" opens a file for reading. To save, open it with \"w\" (write)." },
        { "    fclose(out);",
          "    // fclose(out);",
          "Without fclose, the saved numbers can still be waiting in memory when you read the file back." },
        { "        total += x;",
          "        total = x;",
          "= keeps only the last number; += adds them all up." },
    },
},

// =====================================================================
{
    .id = "bugs-bank", .level = "cpp-classes", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_CPP,
    .title = "Bank Glitches",
    .brief =
        "The BankAccount class is losing customers' money: deposits make the balance go down, and withdrawals are refused when there's plenty in the account.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "Look at the body of deposit, and at the condition that decides when to refuse a withdrawal.",
    .timeLimit = 180,
    .bugCount = 2,
    .bugs = {
        { "        balance += amount;",
          "        balance -= amount;",
          "A deposit adds money: += not -=." },
        { "        if (amount > balance) {",
          "        if (amount < balance) {",
          "Refuse the withdrawal when the amount is MORE than the balance." },
    },
},

// =====================================================================
{
    .id = "bugs-deepcopy", .level = "cpp-object-memory", .region = 0,
    .role = ROLE_MAIN, .mode = MODE_BUGS, .lang = LANG_CPP,
    .title = "Shallow Waters",
    .brief =
        "Copy constructors are all about what you can't see: two objects quietly sharing memory. That makes this level a Bug Hunter round. The Student class's copy constructor makes a shallow copy, so changing s2 changes s1 too - and then both destructors delete the same name. There's a second, smaller bug as well.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "A deep copy needs its own memory: new char[strlen(other.name) + 1] and then strcpy. And which index is the first letter?",
    .timeLimit = 240,
    .bugCount = 2,
    .bugs = {
        { "        name = new char[strlen(other.name) + 1];\n"
          "        strcpy(name, other.name);",
          "        name = other.name;",
          "That copied the pointer, not the text, so both students shared one name - and both destructors deleted it. Give the copy its own memory and strcpy the text in." },
        { "    void setFirstLetter(char c) { name[0] = c; }",
          "    void setFirstLetter(char c) { name[1] = c; }",
          "The first letter is name[0] - arrays count from 0." },
    },
},

// =====================================================================
{
    .id = "bugs-scores", .level = "cpp-algorithms", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_CPP,
    .title = "Leftover Scores",
    .brief =
        "Score Cleaner says it removed nothing, the negative scores are still there, and the ranking is upside down.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "remove_if on its own only shuffles - which other function finishes the erase-remove idiom? And which way should the comparator point?",
    .timeLimit = 180,
    .bugCount = 2,
    .bugs = {
        { "    scores.erase(remove_if(scores.begin(), scores.end(), isNegative), scores.end());",
          "    remove_if(scores.begin(), scores.end(), isNegative);",
          "remove_if only moves the keepers to the front - erase is what actually shrinks the vector." },
        { "    return a > b;",
          "    return a < b;",
          "The comparator decides the order: a > b puts the highest score first." },
    },
},

// =====================================================================
{
    .id = "bugs-list", .level = "cpp-linked-list", .region = 0,
    .role = ROLE_BONUS, .mode = MODE_BUGS, .lang = LANG_CPP,
    .title = "Lost Links",
    .brief =
        "Build a List crashes as soon as it adds a second number, and inserting at the front throws the rest of the list away.\n"
        "\n"
        "Click a bug to jump to its line, fix the code, then Run & Check - when every test passes, all the bugs are squashed. Beat the clock, and fix them all on your first run for the third star.",
    .hint = "When InsertAtEnd walks to the last node, what should it check? And what must a new front node point at?",
    .timeLimit = 240,
    .bugCount = 2,
    .bugs = {
        { "    while (cur->next != nullptr) {",
          "    while (cur != nullptr) {",
          "The walk must stop ON the last node (whose next is nullptr). Walking off the end left cur as nullptr, and cur->next crashed." },
        { "    newNode->next = *ptrToHeadPtr;",
          "    newNode->next = nullptr;",
          "The new front node has to point at the old head, or the rest of the list is lost." },
    },
},

// =====================================================================
{
    .id = "factory-mega", .level = NULL, .region = 3,
    .role = ROLE_BOSS, .mode = MODE_FACTORY, .lang = LANG_CPP,
    .title = "Boss: The Mega Factory",
    .brief =
        "The warehouse is chaos. Crates arrive as lines of an item name and a quantity - negative quantities are returns - until the input runs out:\n"
        "\n"
        "bolts 30\n"
        "nuts 12\n"
        "bolts 20\n"
        "washers 5\n"
        "washers -5\n"
        "\n"
        "Use everything from the STL:\n"
        "1. Add up each item's total in a map<string, int>.\n"
        "2. Copy the map into a vector<pair<string, int>>.\n"
        "3. Remove the items whose total is 0 with the erase-remove_if idiom.\n"
        "4. Sort with a comparator: biggest quantity first, ties in alphabetical order.\n"
        "5. Print the report:\n"
        "Items: 2\n"
        "1. bolts 50\n"
        "2. nuts 12\n"
        "Total: 62",
    .starterCode =
        "#include <iostream>\n"
        "#include <map>\n"
        "#include <string>\n"
        "#include <vector>\n"
        "#include <algorithm>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    map<string, int> stock;\n"
        "    string item;\n"
        "    int quantity;\n"
        "\n"
        "    // Total the crates, then filter, sort and print the report\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "while (cin >> item >> quantity) stock[item] += quantity;. vector<pair<string, int>> items(stock.begin(), stock.end()); copies the map. The comparator takes two const pair<string, int> & and compares .second, then .first.",
    .solution =
        "#include <iostream>\n"
        "#include <map>\n"
        "#include <string>\n"
        "#include <vector>\n"
        "#include <algorithm>\n"
        "using namespace std;\n"
        "\n"
        "bool byQuantity(const pair<string, int> &a, const pair<string, int> &b)\n"
        "{\n"
        "    if (a.second != b.second) return a.second > b.second;\n"
        "    return a.first < b.first;\n"
        "}\n"
        "\n"
        "bool isEmpty(const pair<string, int> &p)\n"
        "{\n"
        "    return p.second == 0;\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    map<string, int> stock;\n"
        "    string item;\n"
        "    int quantity;\n"
        "\n"
        "    while (cin >> item >> quantity) {\n"
        "        stock[item] += quantity;\n"
        "    }\n"
        "\n"
        "    vector<pair<string, int>> items(stock.begin(), stock.end());\n"
        "    items.erase(remove_if(items.begin(), items.end(), isEmpty), items.end());\n"
        "    sort(items.begin(), items.end(), byQuantity);\n"
        "\n"
        "    int total = 0;\n"
        "    cout << \"Items: \" << items.size() << \"\\n\";\n"
        "    for (size_t i = 0; i < items.size(); i++) {\n"
        "        cout << i + 1 << \". \" << items[i].first << \" \" << items[i].second << \"\\n\";\n"
        "        total += items[i].second;\n"
        "    }\n"
        "    cout << \"Total: \" << total << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "bolts 30\n"
          "nuts 12\n"
          "bolts 20\n"
          "washers 5\n"
          "washers -5\n",
          "Items: 2\n"
          "1. bolts 50\n"
          "2. nuts 12\n"
          "Total: 62\n" },
        { "gears 3\n"
          "cogs 3\n"
          "springs 9\n",
          "Items: 3\n"
          "1. springs 9\n"
          "2. cogs 3\n"
          "3. gears 3\n"
          "Total: 15\n" },
        { "pipes 4\n"
          "pipes -4\n",
          "Items: 0\n"
          "Total: 0\n" },
        { "",
          "Items: 0\n"
          "Total: 0\n" },
    },
},

};

int MissionTableCount(void) { return (int)(sizeof(MISSIONS) / sizeof(MISSIONS[0])); }

const Mission *GetMissionTable(int index)
{
    if (index < 0 || index >= MissionTableCount()) return &MISSIONS[0];
    return &MISSIONS[index];
}
