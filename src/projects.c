#include "projects.h"
#include <stddef.h>

// ---------------------------------------------------------------------------
// Bigger projects, built one step at a time. To add a project, add a new entry
// to this array. Each step's `solution` must pass that step's tests, and the
// steps should build on each other so the student keeps one growing program.
// ---------------------------------------------------------------------------

static const Project PROJECTS[] = {

// ===================================================================== 1
{
    .id = "proj-temperature", .lang = LANG_C,
    .title   = "Temperature Converter",
    .summary = "Convert between Celsius and Fahrenheit, then keep going until the user quits.",
    .intro   =
        "Almost every course starts with a converter, because it uses the basics you need everywhere: "
        "reading input, doing maths, making decisions and repeating.\n\n"
        "You'll start with a one-line formula and finish with a small program that handles both units, "
        "rejects bad input and keeps running until it is told to stop.",
    .needs = "Variables, Types and Constants; Input, Maths and Type Conversion; Decisions; Loops",
    .lecture = "Intro to C",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // Step 1: read a temperature in Celsius and print it in Fahrenheit\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 3,
    .steps = {
        { "Celsius to Fahrenheit",
          "Read one temperature in Celsius (it can have a decimal point, so use a double) and print it in Fahrenheit.\n\n"
          "The formula is:  F = C * 9 / 5 + 32\n\n"
          "Print both numbers with one decimal place, like this:\n\n"
          "100.0 C = 212.0 F",
          "Declare double celsius; and read it with scanf(\"%lf\", &celsius); - note %lf for a double. "
          "Then printf(\"%.1f C = %.1f F\\n\", celsius, celsius * 9 / 5 + 32);",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    double celsius;\n"
          "    scanf(\"%lf\", &celsius);\n"
          "\n"
          "    printf(\"%.1f C = %.1f F\\n\", celsius, celsius * 9 / 5 + 32);\n"
          "\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "100\n",  "100.0 C = 212.0 F\n" },
              { "0\n",    "0.0 C = 32.0 F\n" },
              { "-40\n",  "-40.0 C = -40.0 F\n" },
              { "21.5\n", "21.5 C = 70.7 F\n" },
          } },
        { "Pick the unit",
          "Now the input starts with a letter that says which unit the number is in, for example:\n\n"
          "C 100\n"
          "F 212\n\n"
          "For C, convert to Fahrenheit like before. For F, convert to Celsius with:  C = (F - 32) * 5 / 9\n"
          "and print it as  212.0 F = 100.0 C\n\n"
          "For any other letter print  Unknown unit: X  (with the letter they typed).",
          "Read both values at once: scanf(\" %c %lf\", &unit, &value); - the space before %c skips any spaces or newlines. "
          "Then use if (unit == 'C') ... else if (unit == 'F') ... else ...",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char unit;\n"
          "    double value;\n"
          "    scanf(\" %c %lf\", &unit, &value);\n"
          "\n"
          "    if (unit == 'C') {\n"
          "        printf(\"%.1f C = %.1f F\\n\", value, value * 9 / 5 + 32);\n"
          "    } else if (unit == 'F') {\n"
          "        printf(\"%.1f F = %.1f C\\n\", value, (value - 32) * 5 / 9);\n"
          "    } else {\n"
          "        printf(\"Unknown unit: %c\\n\", unit);\n"
          "    }\n"
          "\n"
          "    return 0;\n"
          "}\n",
          5, {
              { "C 100\n", "100.0 C = 212.0 F\n" },
              { "F 212\n", "212.0 F = 100.0 C\n" },
              { "F 0\n",   "0.0 F = -17.8 C\n" },
              { "C -40\n", "-40.0 C = -40.0 F\n" },
              { "K 300\n", "Unknown unit: K\n" },
          } },
        { "Keep converting",
          "Real programs don't stop after one go. Keep reading conversions until the user types Q (or the input runs out), "
          "then print  Goodbye!\n\n"
          "Example input:\n"
          "C 100\n"
          "F 32\n"
          "Q\n\n"
          "Example output:\n"
          "100.0 C = 212.0 F\n"
          "32.0 F = 0.0 C\n"
          "Goodbye!",
          "Wrap your code in while (scanf(\" %c\", &unit) == 1) { ... } - scanf returns 1 while it reads a letter. "
          "Inside, break; if the unit is 'Q', otherwise read the number with scanf(\"%lf\", &value); and convert as before. "
          "Print Goodbye! after the loop.",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char unit;\n"
          "    double value;\n"
          "\n"
          "    while (scanf(\" %c\", &unit) == 1) {\n"
          "        if (unit == 'Q') {\n"
          "            break;\n"
          "        }\n"
          "        scanf(\"%lf\", &value);\n"
          "\n"
          "        if (unit == 'C') {\n"
          "            printf(\"%.1f C = %.1f F\\n\", value, value * 9 / 5 + 32);\n"
          "        } else if (unit == 'F') {\n"
          "            printf(\"%.1f F = %.1f C\\n\", value, (value - 32) * 5 / 9);\n"
          "        } else {\n"
          "            printf(\"Unknown unit: %c\\n\", unit);\n"
          "        }\n"
          "    }\n"
          "\n"
          "    printf(\"Goodbye!\\n\");\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "C 100\nF 32\nQ\n",  "100.0 C = 212.0 F\n32.0 F = 0.0 C\nGoodbye!\n" },
              { "Q\nC 5\n",          "Goodbye!\n" },
              { "K 5\nC 0\nQ\n",     "Unknown unit: K\n0.0 C = 32.0 F\nGoodbye!\n" },
              { "F 98.6\n",          "98.6 F = 37.0 C\nGoodbye!\n" },
          } },
    },
},

// ===================================================================== 2
{
    .id = "proj-calculator", .lang = LANG_C,
    .title   = "Simple Calculator",
    .summary = "A calculator that reads sums like 6 * 7, handles all four operators and never divides by zero.",
    .intro   =
        "A calculator is the classic second project. You'll read numbers and an operator, choose what to do with "
        "if and else, guard against a crash-worthy mistake (dividing by zero), and finally keep calculating in a loop.\n\n"
        "You'll also meet %g, a printf format that prints numbers without pointless zeros: 5 instead of 5.000000, and 2.5 instead of 2.500000.",
    .needs = "Input, Maths and Type Conversion; Decisions: if, else and switch; Loops",
    .lecture = "Intro to C",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // Step 1: read two numbers and print their sum\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Add two numbers",
          "Read two numbers (they can have decimals, so use double) and print their sum like this:\n\n"
          "2 + 3 = 5\n\n"
          "Use %g in printf to print each number - it leaves off the extra zeros, so 1.5 prints as 1.5 and 5.0 prints as 5.",
          "scanf(\"%lf %lf\", &a, &b); reads both numbers. Then printf(\"%g + %g = %g\\n\", a, b, a + b);",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    double a, b;\n"
          "    scanf(\"%lf %lf\", &a, &b);\n"
          "\n"
          "    printf(\"%g + %g = %g\\n\", a, b, a + b);\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "2 3\n",       "2 + 3 = 5\n" },
              { "1.5 2.25\n",  "1.5 + 2.25 = 3.75\n" },
              { "-4 10\n",     "-4 + 10 = 6\n" },
          } },
        { "Choose the operator",
          "Now the user types a whole sum, like  6 * 7  or  10 / 4. Read the first number, the operator character and the second number.\n\n"
          "Support + - * and / and print the result in the same style:\n\n"
          "6 * 7 = 42\n"
          "10 / 4 = 2.5\n\n"
          "If the operator is anything else, print  Unknown operator: x  (with the character they typed).",
          "scanf(\"%lf %c %lf\", &a, &op, &b); reads all three. Then check op with if (op == '+') ... else if (op == '-') ... and so on. "
          "A switch (op) with one case per operator works just as well.",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    double a, b;\n"
          "    char op;\n"
          "    scanf(\"%lf %c %lf\", &a, &op, &b);\n"
          "\n"
          "    if (op == '+') {\n"
          "        printf(\"%g + %g = %g\\n\", a, b, a + b);\n"
          "    } else if (op == '-') {\n"
          "        printf(\"%g - %g = %g\\n\", a, b, a - b);\n"
          "    } else if (op == '*') {\n"
          "        printf(\"%g * %g = %g\\n\", a, b, a * b);\n"
          "    } else if (op == '/') {\n"
          "        printf(\"%g / %g = %g\\n\", a, b, a / b);\n"
          "    } else {\n"
          "        printf(\"Unknown operator: %c\\n\", op);\n"
          "    }\n"
          "\n"
          "    return 0;\n"
          "}\n",
          5, {
              { "6 * 7\n",      "6 * 7 = 42\n" },
              { "10 / 4\n",     "10 / 4 = 2.5\n" },
              { "9 - 12\n",     "9 - 12 = -3\n" },
              { "1.5 + 2.25\n", "1.5 + 2.25 = 3.75\n" },
              { "3 x 4\n",      "Unknown operator: x\n" },
          } },
        { "Don't divide by zero",
          "Try 5 / 0 in your calculator - it prints inf, which is not a helpful answer.\n\n"
          "When the operator is / and the second number is 0, print this instead:\n\n"
          "Error: can't divide by zero\n\n"
          "Everything else should work exactly as before.",
          "Inside the op == '/' branch, add if (b == 0) { print the error } else { print the result }.",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    double a, b;\n"
          "    char op;\n"
          "    scanf(\"%lf %c %lf\", &a, &op, &b);\n"
          "\n"
          "    if (op == '+') {\n"
          "        printf(\"%g + %g = %g\\n\", a, b, a + b);\n"
          "    } else if (op == '-') {\n"
          "        printf(\"%g - %g = %g\\n\", a, b, a - b);\n"
          "    } else if (op == '*') {\n"
          "        printf(\"%g * %g = %g\\n\", a, b, a * b);\n"
          "    } else if (op == '/') {\n"
          "        if (b == 0) {\n"
          "            printf(\"Error: can't divide by zero\\n\");\n"
          "        } else {\n"
          "            printf(\"%g / %g = %g\\n\", a, b, a / b);\n"
          "        }\n"
          "    } else {\n"
          "        printf(\"Unknown operator: %c\\n\", op);\n"
          "    }\n"
          "\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "5 / 0\n",  "Error: can't divide by zero\n" },
              { "0 / 5\n",  "0 / 5 = 0\n" },
              { "10 / 4\n", "10 / 4 = 2.5\n" },
              { "8 * 0\n",  "8 * 0 = 0\n" },
          } },
        { "Keep calculating",
          "Make the calculator handle as many sums as the user types, one per line, until the input runs out.\n\n"
          "At the end, print how many sums were entered (including ones that gave an error):\n\n"
          "Calculations: 2",
          "scanf returns how many values it read, so while (scanf(\"%lf %c %lf\", &a, &op, &b) == 3) { ... } "
          "keeps going while there are sums to read. Add int count = 0; before the loop and count++; inside it.",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    double a, b;\n"
          "    char op;\n"
          "    int count = 0;\n"
          "\n"
          "    while (scanf(\"%lf %c %lf\", &a, &op, &b) == 3) {\n"
          "        count++;\n"
          "\n"
          "        if (op == '+') {\n"
          "            printf(\"%g + %g = %g\\n\", a, b, a + b);\n"
          "        } else if (op == '-') {\n"
          "            printf(\"%g - %g = %g\\n\", a, b, a - b);\n"
          "        } else if (op == '*') {\n"
          "            printf(\"%g * %g = %g\\n\", a, b, a * b);\n"
          "        } else if (op == '/') {\n"
          "            if (b == 0) {\n"
          "                printf(\"Error: can't divide by zero\\n\");\n"
          "            } else {\n"
          "                printf(\"%g / %g = %g\\n\", a, b, a / b);\n"
          "            }\n"
          "        } else {\n"
          "            printf(\"Unknown operator: %c\\n\", op);\n"
          "        }\n"
          "    }\n"
          "\n"
          "    printf(\"Calculations: %d\\n\", count);\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "1 + 1\n2 * 3\n",          "1 + 1 = 2\n2 * 3 = 6\nCalculations: 2\n" },
              { "",                        "Calculations: 0\n" },
              { "8 / 0\n8 / 2\n",          "Error: can't divide by zero\n8 / 2 = 4\nCalculations: 2\n" },
              { "7 - 10\n2 ^ 3\n0.5 * 4\n", "7 - 10 = -3\nUnknown operator: ^\n0.5 * 4 = 2\nCalculations: 3\n" },
          } },
    },
},

// ===================================================================== 3
{
    .id = "proj-guessing", .lang = LANG_C,
    .title   = "Number Guessing Game",
    .summary = "Player one picks a secret number, player two guesses it with \"too high\" and \"too low\" clues.",
    .intro   =
        "The guessing game is a programming rite of passage. The secret number is typed in first (by player one), "
        "then each guess gets a clue until the player gets it right or runs out of tries.\n\n"
        "Along the way you'll use a loop that stops early with break, a counter, and a flag variable that remembers "
        "whether the player won.",
    .needs = "Decisions: if, else and switch; Loops",
    .lecture = "Intro to C",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // Step 1: read the secret number and one guess, then give a clue\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "One guess",
          "The input is two whole numbers: first the secret number, then the player's guess.\n\n"
          "Print one line:\n"
          "Too low!   if the guess is smaller than the secret\n"
          "Too high!   if it is bigger\n"
          "Correct!   if it matches",
          "Read both with scanf(\"%d %d\", &secret, &guess); then use if (guess < secret) ... else if (guess > secret) ... else ...",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int secret, guess;\n"
          "    scanf(\"%d %d\", &secret, &guess);\n"
          "\n"
          "    if (guess < secret) {\n"
          "        printf(\"Too low!\\n\");\n"
          "    } else if (guess > secret) {\n"
          "        printf(\"Too high!\\n\");\n"
          "    } else {\n"
          "        printf(\"Correct!\\n\");\n"
          "    }\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "50\n30\n", "Too low!\n" },
              { "50\n70\n", "Too high!\n" },
              { "50\n50\n", "Correct!\n" },
          } },
        { "Keep guessing",
          "One guess isn't much of a game. After the secret, keep reading guesses and print a clue for each one.\n\n"
          "Stop as soon as the player is correct - any guesses after that are ignored. "
          "Also stop if the input runs out.",
          "Read the secret first, then loop with while (scanf(\"%d\", &guess) == 1) { ... }. "
          "Put your if/else inside the loop and add break; right after printing Correct!",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int secret, guess;\n"
          "    scanf(\"%d\", &secret);\n"
          "\n"
          "    while (scanf(\"%d\", &guess) == 1) {\n"
          "        if (guess < secret) {\n"
          "            printf(\"Too low!\\n\");\n"
          "        } else if (guess > secret) {\n"
          "            printf(\"Too high!\\n\");\n"
          "        } else {\n"
          "            printf(\"Correct!\\n\");\n"
          "            break;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "50\n25\n75\n50\n",    "Too low!\nToo high!\nCorrect!\n" },
              { "7\n7\n",              "Correct!\n" },
              { "10\n1\n2\n10\n99\n",  "Too low!\nToo low!\nCorrect!\n" },
          } },
        { "Count the guesses",
          "Count every guess the player makes. When the loop finishes, print:\n\n"
          "Guesses used: 3",
          "Add int tries = 0; before the loop and tries++; as the first thing inside it. "
          "Print the count after the loop with printf(\"Guesses used: %d\\n\", tries);",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int secret, guess;\n"
          "    int tries = 0;\n"
          "    scanf(\"%d\", &secret);\n"
          "\n"
          "    while (scanf(\"%d\", &guess) == 1) {\n"
          "        tries++;\n"
          "        if (guess < secret) {\n"
          "            printf(\"Too low!\\n\");\n"
          "        } else if (guess > secret) {\n"
          "            printf(\"Too high!\\n\");\n"
          "        } else {\n"
          "            printf(\"Correct!\\n\");\n"
          "            break;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    printf(\"Guesses used: %d\\n\", tries);\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "50\n25\n75\n50\n",    "Too low!\nToo high!\nCorrect!\nGuesses used: 3\n" },
              { "7\n7\n",              "Correct!\nGuesses used: 1\n" },
              { "10\n1\n2\n10\n99\n",  "Too low!\nToo low!\nCorrect!\nGuesses used: 3\n" },
          } },
        { "Only five tries",
          "Make the game harder: the player gets at most 5 guesses.\n\n"
          "If they guess correctly, print  Guesses used: N  like before.\n"
          "If they use all 5 guesses (or the input runs out) without getting it, print:\n\n"
          "Out of tries! The number was 50.\n\n"
          "Going further: to play on your own, set the secret with srand(time(NULL)); and secret = rand() % 100 + 1; "
          "(from <stdlib.h> and <time.h>). The tests can't predict random numbers, so here the secret is typed in.",
          "Loop with while (tries < 5 && scanf(\"%d\", &guess) == 1). Use int won = 0; and set won = 1; when they're correct. "
          "After the loop, if (won) print the guesses, else print the Out of tries message.",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int secret, guess;\n"
          "    int tries = 0;\n"
          "    int won = 0;\n"
          "    scanf(\"%d\", &secret);\n"
          "\n"
          "    while (tries < 5 && scanf(\"%d\", &guess) == 1) {\n"
          "        tries++;\n"
          "        if (guess < secret) {\n"
          "            printf(\"Too low!\\n\");\n"
          "        } else if (guess > secret) {\n"
          "            printf(\"Too high!\\n\");\n"
          "        } else {\n"
          "            printf(\"Correct!\\n\");\n"
          "            won = 1;\n"
          "            break;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    if (won) {\n"
          "        printf(\"Guesses used: %d\\n\", tries);\n"
          "    } else {\n"
          "        printf(\"Out of tries! The number was %d.\\n\", secret);\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "50\n1\n2\n3\n4\n5\n50\n", "Too low!\nToo low!\nToo low!\nToo low!\nToo low!\nOut of tries! The number was 50.\n" },
              { "50\n50\n",                "Correct!\nGuesses used: 1\n" },
              { "20\n10\n30\n20\n",        "Too low!\nToo high!\nCorrect!\nGuesses used: 3\n" },
              { "8\n1\n2\n3\n4\n8\n",      "Too low!\nToo low!\nToo low!\nToo low!\nCorrect!\nGuesses used: 5\n" },
          } },
    },
},

// ===================================================================== 4
{
    .id = "proj-gradebook", .lang = LANG_C,
    .title   = "Grade Book",
    .summary = "Store a class's test scores in an array and report grades, the average and the best and worst.",
    .intro   =
        "Teachers were some of the first people to use computers for everyday work, and a grade book is a perfect "
        "first project with arrays: you need to keep every score, because some answers (like who beat the average) "
        "can only be worked out once you have seen them all.\n\n"
        "You'll also write your own function to turn a score into a letter grade.",
    .needs = "Loops; Arrays and 2D Arrays; Functions and Pass by Value",
    .lecture = "Intro to C",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int scores[100];\n"
        "\n"
        "    // Step 1: read how many scores there are, then the scores, and print the average\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Read the scores",
          "The input starts with how many students there are (between 1 and 100), followed by each student's score (a whole number from 0 to 100).\n\n"
          "Store every score in the scores array - later steps need them all - and print the average with one decimal place:\n\n"
          "Average: 80.0",
          "Read the count with scanf(\"%d\", &count); then loop for (int i = 0; i < count; i++) { scanf(\"%d\", &scores[i]); total += scores[i]; }. "
          "Watch out: total / count with two ints drops the decimals, so use (double)total / count.",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int scores[100];\n"
          "    int count;\n"
          "    int total = 0;\n"
          "    scanf(\"%d\", &count);\n"
          "\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        scanf(\"%d\", &scores[i]);\n"
          "        total += scores[i];\n"
          "    }\n"
          "\n"
          "    double average = (double)total / count;\n"
          "    printf(\"Average: %.1f\\n\", average);\n"
          "\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "3\n70 80 90\n",       "Average: 80.0\n" },
              { "1\n55\n",             "Average: 55.0\n" },
              { "4\n59 60 89 90\n",    "Average: 74.5\n" },
              { "5\n100 92 67 81 45\n", "Average: 77.0\n" },
          } },
        { "Highest and lowest",
          "After the average, print the highest and the lowest score:\n\n"
          "Average: 80.0\n"
          "Highest: 90\n"
          "Lowest: 70",
          "Start with int highest = scores[0]; and int lowest = scores[0]; then loop over the rest of the array: "
          "if (scores[i] > highest) highest = scores[i]; and the same idea for lowest.",
          "#include <stdio.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int scores[100];\n"
          "    int count;\n"
          "    int total = 0;\n"
          "    scanf(\"%d\", &count);\n"
          "\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        scanf(\"%d\", &scores[i]);\n"
          "        total += scores[i];\n"
          "    }\n"
          "\n"
          "    int highest = scores[0];\n"
          "    int lowest = scores[0];\n"
          "    for (int i = 1; i < count; i++) {\n"
          "        if (scores[i] > highest) {\n"
          "            highest = scores[i];\n"
          "        }\n"
          "        if (scores[i] < lowest) {\n"
          "            lowest = scores[i];\n"
          "        }\n"
          "    }\n"
          "\n"
          "    double average = (double)total / count;\n"
          "    printf(\"Average: %.1f\\n\", average);\n"
          "    printf(\"Highest: %d\\n\", highest);\n"
          "    printf(\"Lowest: %d\\n\", lowest);\n"
          "\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "3\n70 80 90\n",        "Average: 80.0\nHighest: 90\nLowest: 70\n" },
              { "1\n55\n",              "Average: 55.0\nHighest: 55\nLowest: 55\n" },
              { "4\n59 60 89 90\n",     "Average: 74.5\nHighest: 90\nLowest: 59\n" },
              { "5\n100 92 67 81 45\n", "Average: 77.0\nHighest: 100\nLowest: 45\n" },
          } },
        { "Letter grades",
          "Write a function char letterGrade(int score) that returns 'A' for 90 or more, 'B' for 80-89, 'C' for 70-79, 'D' for 60-69 and 'F' below 60.\n\n"
          "Before the average, print one line per student using it:\n\n"
          "Student 1: 70 (C)\n"
          "Student 2: 80 (B)\n"
          "Student 3: 90 (A)\n"
          "Average: 80.0\n"
          "...",
          "Put the function above main. Inside it, if (score >= 90) return 'A'; then if (score >= 80) return 'B'; and so on, ending with return 'F';. "
          "Print each student with printf(\"Student %d: %d (%c)\\n\", i + 1, scores[i], letterGrade(scores[i]));",
          "#include <stdio.h>\n"
          "\n"
          "char letterGrade(int score)\n"
          "{\n"
          "    if (score >= 90) return 'A';\n"
          "    if (score >= 80) return 'B';\n"
          "    if (score >= 70) return 'C';\n"
          "    if (score >= 60) return 'D';\n"
          "    return 'F';\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int scores[100];\n"
          "    int count;\n"
          "    int total = 0;\n"
          "    scanf(\"%d\", &count);\n"
          "\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        scanf(\"%d\", &scores[i]);\n"
          "        total += scores[i];\n"
          "    }\n"
          "\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        printf(\"Student %d: %d (%c)\\n\", i + 1, scores[i], letterGrade(scores[i]));\n"
          "    }\n"
          "\n"
          "    int highest = scores[0];\n"
          "    int lowest = scores[0];\n"
          "    for (int i = 1; i < count; i++) {\n"
          "        if (scores[i] > highest) {\n"
          "            highest = scores[i];\n"
          "        }\n"
          "        if (scores[i] < lowest) {\n"
          "            lowest = scores[i];\n"
          "        }\n"
          "    }\n"
          "\n"
          "    double average = (double)total / count;\n"
          "    printf(\"Average: %.1f\\n\", average);\n"
          "    printf(\"Highest: %d\\n\", highest);\n"
          "    printf(\"Lowest: %d\\n\", lowest);\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "3\n70 80 90\n",
                "Student 1: 70 (C)\nStudent 2: 80 (B)\nStudent 3: 90 (A)\nAverage: 80.0\nHighest: 90\nLowest: 70\n" },
              { "4\n59 60 89 90\n",
                "Student 1: 59 (F)\nStudent 2: 60 (D)\nStudent 3: 89 (B)\nStudent 4: 90 (A)\nAverage: 74.5\nHighest: 90\nLowest: 59\n" },
              { "1\n100\n",
                "Student 1: 100 (A)\nAverage: 100.0\nHighest: 100\nLowest: 100\n" },
          } },
        { "Above average",
          "Finally, count how many students scored more than the average and print it on the last line:\n\n"
          "Above average: 1\n\n"
          "This is why we kept every score in an array: you can't know who beat the average until you've read them all.",
          "After working out average, loop over the array again with a counter: if (scores[i] > average) above++;",
          "#include <stdio.h>\n"
          "\n"
          "char letterGrade(int score)\n"
          "{\n"
          "    if (score >= 90) return 'A';\n"
          "    if (score >= 80) return 'B';\n"
          "    if (score >= 70) return 'C';\n"
          "    if (score >= 60) return 'D';\n"
          "    return 'F';\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int scores[100];\n"
          "    int count;\n"
          "    int total = 0;\n"
          "    scanf(\"%d\", &count);\n"
          "\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        scanf(\"%d\", &scores[i]);\n"
          "        total += scores[i];\n"
          "    }\n"
          "\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        printf(\"Student %d: %d (%c)\\n\", i + 1, scores[i], letterGrade(scores[i]));\n"
          "    }\n"
          "\n"
          "    int highest = scores[0];\n"
          "    int lowest = scores[0];\n"
          "    for (int i = 1; i < count; i++) {\n"
          "        if (scores[i] > highest) {\n"
          "            highest = scores[i];\n"
          "        }\n"
          "        if (scores[i] < lowest) {\n"
          "            lowest = scores[i];\n"
          "        }\n"
          "    }\n"
          "\n"
          "    double average = (double)total / count;\n"
          "    printf(\"Average: %.1f\\n\", average);\n"
          "    printf(\"Highest: %d\\n\", highest);\n"
          "    printf(\"Lowest: %d\\n\", lowest);\n"
          "\n"
          "    int above = 0;\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        if (scores[i] > average) {\n"
          "            above++;\n"
          "        }\n"
          "    }\n"
          "    printf(\"Above average: %d\\n\", above);\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "3\n70 80 90\n",
                "Student 1: 70 (C)\nStudent 2: 80 (B)\nStudent 3: 90 (A)\nAverage: 80.0\nHighest: 90\nLowest: 70\nAbove average: 1\n" },
              { "5\n100 92 67 81 45\n",
                "Student 1: 100 (A)\nStudent 2: 92 (A)\nStudent 3: 67 (D)\nStudent 4: 81 (B)\nStudent 5: 45 (F)\n"
                "Average: 77.0\nHighest: 100\nLowest: 45\nAbove average: 3\n" },
              { "1\n55\n",
                "Student 1: 55 (F)\nAverage: 55.0\nHighest: 55\nLowest: 55\nAbove average: 0\n" },
          } },
    },
},

// ===================================================================== 5
{
    .id = "proj-wordguess", .lang = LANG_C,
    .title   = "Word Guess",
    .summary = "Guess the hidden word one letter at a time before you run out of lives.",
    .intro   =
        "A word-guessing game (often called hangman) is a great workout for strings and arrays. "
        "The secret word is typed in first, then the player guesses one letter at a time.\n\n"
        "You'll keep track of which letters have been found, count lives, write a function that shows the "
        "word with blanks, and stop the player from wasting a turn on a letter they already tried.",
    .needs = "Loops; Arrays and 2D Arrays; Strings; Functions and Pass by Value",
    .lecture = "Intro to C",
    .starterCode =
        "#include <stdio.h>\n"
        "#include <string.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    char word[50];\n"
        "\n"
        "    // Step 1: read the secret word and print one _ per letter\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Show the blanks",
          "Read the secret word (lowercase letters only, at most 49 of them) and print one underscore and a space for each letter.\n\n"
          "For the word cat, print:\n\n"
          "_ _ _",
          "scanf(\"%49s\", word); reads the word, and strlen(word) gives its length. "
          "Loop that many times printing \"_ \", then print \"\\n\". (The space at the very end of the line doesn't matter.)",
          "#include <stdio.h>\n"
          "#include <string.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char word[50];\n"
          "    scanf(\"%49s\", word);\n"
          "    int length = strlen(word);\n"
          "\n"
          "    for (int i = 0; i < length; i++) {\n"
          "        printf(\"_ \");\n"
          "    }\n"
          "    printf(\"\\n\");\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "cat\n",    "_ _ _\n" },
              { "banana\n", "_ _ _ _ _ _\n" },
              { "a\n",      "_\n" },
          } },
        { "Guess letters",
          "After the word, the input has one guessed letter per line. For each guess:\n\n"
          "1. Print  Good guess!  if the letter is in the word, or  Nope!  if it isn't.\n"
          "2. Print the word again, showing the letters found so far and _ for the rest.\n\n"
          "Input cat, a, z gives:\n"
          "_ _ _\n"
          "Good guess!\n"
          "_ a _\n"
          "Nope!\n"
          "_ a _\n\n"
          "Write a function void showProgress(const char *word, const int found[]) to print the word, "
          "and use an int found[50] array where found[i] is 1 once letter i has been guessed.",
          "Start with int found[50] = {0}; so every letter starts hidden. Loop with while (scanf(\" %c\", &letter) == 1). "
          "Inside, check every position: if (word[i] == letter) { found[i] = 1; hit = 1; }. "
          "In showProgress, print word[i] followed by a space if found[i] is set, otherwise \"_ \".",
          "#include <stdio.h>\n"
          "#include <string.h>\n"
          "\n"
          "void showProgress(const char *word, const int found[])\n"
          "{\n"
          "    for (int i = 0; word[i] != '\\0'; i++) {\n"
          "        if (found[i]) {\n"
          "            printf(\"%c \", word[i]);\n"
          "        } else {\n"
          "            printf(\"_ \");\n"
          "        }\n"
          "    }\n"
          "    printf(\"\\n\");\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char word[50];\n"
          "    int found[50] = {0};\n"
          "    char letter;\n"
          "\n"
          "    scanf(\"%49s\", word);\n"
          "    int length = strlen(word);\n"
          "    showProgress(word, found);\n"
          "\n"
          "    while (scanf(\" %c\", &letter) == 1) {\n"
          "        int hit = 0;\n"
          "        for (int i = 0; i < length; i++) {\n"
          "            if (word[i] == letter) {\n"
          "                found[i] = 1;\n"
          "                hit = 1;\n"
          "            }\n"
          "        }\n"
          "\n"
          "        if (hit) {\n"
          "            printf(\"Good guess!\\n\");\n"
          "        } else {\n"
          "            printf(\"Nope!\\n\");\n"
          "        }\n"
          "        showProgress(word, found);\n"
          "    }\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "cat\na\nz\n",       "_ _ _\nGood guess!\n_ a _\nNope!\n_ a _\n" },
              { "banana\nn\na\n",    "_ _ _ _ _ _\nGood guess!\n_ _ n _ n _\nGood guess!\n_ a n a n a\n" },
              { "dog\n",             "_ _ _\n" },
          } },
        { "Lives, winning and losing",
          "The player has 6 lives. A wrong guess costs one, and instead of Nope! print how many are left:\n\n"
          "Nope! Lives left: 5\n\n"
          "After showing the word, if every letter has been found print  You win!  and stop.\n"
          "If the lives reach 0, stop and print  You lose! The word was dog  (with the real word).\n"
          "If the input just runs out, stop without printing anything extra.",
          "Keep int lives = 6; and loop with while (lives > 0 && scanf(\" %c\", &letter) == 1). "
          "To check for a win, count the positions where found[i] is 0 - if there are none, print You win! and break;. "
          "After the loop, if (lives == 0) print the losing message.",
          "#include <stdio.h>\n"
          "#include <string.h>\n"
          "\n"
          "void showProgress(const char *word, const int found[])\n"
          "{\n"
          "    for (int i = 0; word[i] != '\\0'; i++) {\n"
          "        if (found[i]) {\n"
          "            printf(\"%c \", word[i]);\n"
          "        } else {\n"
          "            printf(\"_ \");\n"
          "        }\n"
          "    }\n"
          "    printf(\"\\n\");\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char word[50];\n"
          "    int found[50] = {0};\n"
          "    int lives = 6;\n"
          "    char letter;\n"
          "\n"
          "    scanf(\"%49s\", word);\n"
          "    int length = strlen(word);\n"
          "    showProgress(word, found);\n"
          "\n"
          "    while (lives > 0 && scanf(\" %c\", &letter) == 1) {\n"
          "        int hit = 0;\n"
          "        for (int i = 0; i < length; i++) {\n"
          "            if (word[i] == letter) {\n"
          "                found[i] = 1;\n"
          "                hit = 1;\n"
          "            }\n"
          "        }\n"
          "\n"
          "        if (hit) {\n"
          "            printf(\"Good guess!\\n\");\n"
          "        } else {\n"
          "            lives--;\n"
          "            printf(\"Nope! Lives left: %d\\n\", lives);\n"
          "        }\n"
          "        showProgress(word, found);\n"
          "\n"
          "        int hidden = 0;\n"
          "        for (int i = 0; i < length; i++) {\n"
          "            if (!found[i]) {\n"
          "                hidden++;\n"
          "            }\n"
          "        }\n"
          "        if (hidden == 0) {\n"
          "            printf(\"You win!\\n\");\n"
          "            break;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    if (lives == 0) {\n"
          "        printf(\"You lose! The word was %s\\n\", word);\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "hi\nh\nx\ni\n",
                "_ _\nGood guess!\nh _\nNope! Lives left: 5\nh _\nGood guess!\nh i\nYou win!\n" },
              { "dog\na\nb\nc\ne\nf\nh\nd\n",
                "_ _ _\nNope! Lives left: 5\n_ _ _\nNope! Lives left: 4\n_ _ _\nNope! Lives left: 3\n_ _ _\n"
                "Nope! Lives left: 2\n_ _ _\nNope! Lives left: 1\n_ _ _\nNope! Lives left: 0\n_ _ _\n"
                "You lose! The word was dog\n" },
              { "cat\nc\na\nt\nz\n",
                "_ _ _\nGood guess!\nc _ _\nGood guess!\nc a _\nGood guess!\nc a t\nYou win!\n" },
              { "zoo\nq\n",
                "_ _ _\nNope! Lives left: 5\n_ _ _\n" },
          } },
        { "No double guesses",
          "Right now, guessing the same wrong letter twice costs two lives. Make repeated guesses free: if the letter was already guessed, print\n\n"
          "Already guessed q\n\n"
          "and skip straight to the next guess (don't show the word again).",
          "Add int guessed[26] = {0}; - one box per letter of the alphabet. letter - 'a' turns 'a' into 0, 'b' into 1 and so on. "
          "At the top of the loop: if (guessed[letter - 'a']) { print the message; continue; } then set guessed[letter - 'a'] = 1;",
          "#include <stdio.h>\n"
          "#include <string.h>\n"
          "\n"
          "void showProgress(const char *word, const int found[])\n"
          "{\n"
          "    for (int i = 0; word[i] != '\\0'; i++) {\n"
          "        if (found[i]) {\n"
          "            printf(\"%c \", word[i]);\n"
          "        } else {\n"
          "            printf(\"_ \");\n"
          "        }\n"
          "    }\n"
          "    printf(\"\\n\");\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char word[50];\n"
          "    int found[50] = {0};\n"
          "    int guessed[26] = {0};\n"
          "    int lives = 6;\n"
          "    char letter;\n"
          "\n"
          "    scanf(\"%49s\", word);\n"
          "    int length = strlen(word);\n"
          "    showProgress(word, found);\n"
          "\n"
          "    while (lives > 0 && scanf(\" %c\", &letter) == 1) {\n"
          "        if (guessed[letter - 'a']) {\n"
          "            printf(\"Already guessed %c\\n\", letter);\n"
          "            continue;\n"
          "        }\n"
          "        guessed[letter - 'a'] = 1;\n"
          "\n"
          "        int hit = 0;\n"
          "        for (int i = 0; i < length; i++) {\n"
          "            if (word[i] == letter) {\n"
          "                found[i] = 1;\n"
          "                hit = 1;\n"
          "            }\n"
          "        }\n"
          "\n"
          "        if (hit) {\n"
          "            printf(\"Good guess!\\n\");\n"
          "        } else {\n"
          "            lives--;\n"
          "            printf(\"Nope! Lives left: %d\\n\", lives);\n"
          "        }\n"
          "        showProgress(word, found);\n"
          "\n"
          "        int hidden = 0;\n"
          "        for (int i = 0; i < length; i++) {\n"
          "            if (!found[i]) {\n"
          "                hidden++;\n"
          "            }\n"
          "        }\n"
          "        if (hidden == 0) {\n"
          "            printf(\"You win!\\n\");\n"
          "            break;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    if (lives == 0) {\n"
          "        printf(\"You lose! The word was %s\\n\", word);\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "banana\na\na\nz\nn\nb\n",
                "_ _ _ _ _ _\nGood guess!\n_ a _ a _ a\nAlready guessed a\nNope! Lives left: 5\n_ a _ a _ a\n"
                "Good guess!\n_ a n a n a\nGood guess!\nb a n a n a\nYou win!\n" },
              { "zz\nq\nq\n",
                "_ _\nNope! Lives left: 5\n_ _\nAlready guessed q\n" },
              { "cat\nc\na\nt\n",
                "_ _ _\nGood guess!\nc _ _\nGood guess!\nc a _\nGood guess!\nc a t\nYou win!\n" },
          } },
    },
},

// ===================================================================== 6
{
    .id = "proj-tictactoe", .lang = LANG_C,
    .title   = "Tic-Tac-Toe",
    .summary = "A two-player game on a 3x3 grid, with move checking and a winner.",
    .intro   =
        "Noughts and crosses is the game everyone builds when they first learn 2D arrays. The board is a 3x3 grid of "
        "characters, the players take turns typing a row and a column, and the program has to spot illegal moves and a winner.\n\n"
        "It's the biggest project so far, so it is split into small steps. Take them one at a time.",
    .needs = "Loops; Arrays and 2D Arrays; Functions and Pass by Value",
    .lecture = "Intro to C",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    char board[3][3];\n"
        "\n"
        "    // Step 1: fill the board with '.' and print it\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Draw the board",
          "The board is a 2D array: char board[3][3]; - 3 rows with 3 squares each. board[0][0] is the top left, board[2][2] the bottom right.\n\n"
          "Fill every square with '.' (meaning empty), then write a function void printBoard(char board[3][3]) "
          "that prints one row per line with a space between the squares:\n\n"
          ". . .\n"
          ". . .\n"
          ". . .",
          "Use two loops, one inside the other: for (int r = 0; r < 3; r++) for (int c = 0; c < 3; c++) board[r][c] = '.'; "
          "In printBoard, print each row with printf(\"%c %c %c\\n\", board[r][0], board[r][1], board[r][2]);",
          "#include <stdio.h>\n"
          "\n"
          "void printBoard(char board[3][3])\n"
          "{\n"
          "    for (int r = 0; r < 3; r++) {\n"
          "        printf(\"%c %c %c\\n\", board[r][0], board[r][1], board[r][2]);\n"
          "    }\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char board[3][3];\n"
          "    for (int r = 0; r < 3; r++) {\n"
          "        for (int c = 0; c < 3; c++) {\n"
          "            board[r][c] = '.';\n"
          "        }\n"
          "    }\n"
          "\n"
          "    printBoard(board);\n"
          "\n"
          "    return 0;\n"
          "}\n",
          1, {
              { "", ". . .\n. . .\n. . .\n" },
          } },
        { "Take turns",
          "Now read the moves. Each move is a row and a column from 1 to 3, like  2 3  for the middle row, right column.\n\n"
          "X always goes first, then O, then X and so on. Put each player's letter on the board, swap players, and keep going "
          "until 9 moves have been made or the input runs out. Then print the board.\n\n"
          "Input  1 1  then  2 2  gives:\n"
          "X . .\n"
          ". O .\n"
          ". . .",
          "Keep char player = 'X'; and int moves = 0;. Loop with while (moves < 9 && scanf(\"%d %d\", &row, &col) == 2). "
          "Players count from 1 but arrays from 0, so use board[row - 1][col - 1] = player;. "
          "Swap with if (player == 'X') player = 'O'; else player = 'X';",
          "#include <stdio.h>\n"
          "\n"
          "void printBoard(char board[3][3])\n"
          "{\n"
          "    for (int r = 0; r < 3; r++) {\n"
          "        printf(\"%c %c %c\\n\", board[r][0], board[r][1], board[r][2]);\n"
          "    }\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char board[3][3];\n"
          "    for (int r = 0; r < 3; r++) {\n"
          "        for (int c = 0; c < 3; c++) {\n"
          "            board[r][c] = '.';\n"
          "        }\n"
          "    }\n"
          "\n"
          "    char player = 'X';\n"
          "    int moves = 0;\n"
          "    int row, col;\n"
          "\n"
          "    while (moves < 9 && scanf(\"%d %d\", &row, &col) == 2) {\n"
          "        board[row - 1][col - 1] = player;\n"
          "        moves++;\n"
          "\n"
          "        if (player == 'X') {\n"
          "            player = 'O';\n"
          "        } else {\n"
          "            player = 'X';\n"
          "        }\n"
          "    }\n"
          "\n"
          "    printBoard(board);\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "1 1\n2 2\n",     "X . .\n. O .\n. . .\n" },
              { "",               ". . .\n. . .\n. . .\n" },
              { "3 3\n1 3\n2 1\n", ". . O\nX . .\n. . X\n" },
          } },
        { "Reject bad moves",
          "Two kinds of move must be refused. The same player then tries again (don't swap players, and it doesn't count as a move):\n\n"
          "If the row or column isn't between 1 and 3, print  Invalid move: 0 5\n"
          "If the square isn't empty, print  Square taken: 1 1\n\n"
          "Without this check, a move like  4 4  writes outside the array - a classic C bug that can crash the program.",
          "At the top of the loop, check if (row < 1 || row > 3 || col < 1 || col > 3) { print; continue; }. "
          "Then check if (board[row - 1][col - 1] != '.') { print; continue; }. continue; jumps straight to the next loop turn.",
          "#include <stdio.h>\n"
          "\n"
          "void printBoard(char board[3][3])\n"
          "{\n"
          "    for (int r = 0; r < 3; r++) {\n"
          "        printf(\"%c %c %c\\n\", board[r][0], board[r][1], board[r][2]);\n"
          "    }\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char board[3][3];\n"
          "    for (int r = 0; r < 3; r++) {\n"
          "        for (int c = 0; c < 3; c++) {\n"
          "            board[r][c] = '.';\n"
          "        }\n"
          "    }\n"
          "\n"
          "    char player = 'X';\n"
          "    int moves = 0;\n"
          "    int row, col;\n"
          "\n"
          "    while (moves < 9 && scanf(\"%d %d\", &row, &col) == 2) {\n"
          "        if (row < 1 || row > 3 || col < 1 || col > 3) {\n"
          "            printf(\"Invalid move: %d %d\\n\", row, col);\n"
          "            continue;\n"
          "        }\n"
          "        if (board[row - 1][col - 1] != '.') {\n"
          "            printf(\"Square taken: %d %d\\n\", row, col);\n"
          "            continue;\n"
          "        }\n"
          "\n"
          "        board[row - 1][col - 1] = player;\n"
          "        moves++;\n"
          "\n"
          "        if (player == 'X') {\n"
          "            player = 'O';\n"
          "        } else {\n"
          "            player = 'X';\n"
          "        }\n"
          "    }\n"
          "\n"
          "    printBoard(board);\n"
          "\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "1 1\n1 1\n2 2\n", "Square taken: 1 1\nX . .\n. O .\n. . .\n" },
              { "0 5\n2 2\n",      "Invalid move: 0 5\n. . .\n. X .\n. . .\n" },
              { "4 4\n3 1\n3 1\n1 3\n", "Invalid move: 4 4\nSquare taken: 3 1\n. . O\n. . .\nX . .\n" },
              { "1 1\n2 2\n",      "X . .\n. O .\n. . .\n" },
          } },
        { "Find the winner",
          "Write a function int hasWon(char board[3][3], char p) that returns 1 if player p has three in a row - "
          "across any row, down any column, or along either diagonal - and 0 otherwise.\n\n"
          "After every move, check if that player has won. If so, stop reading moves. At the end, print the board, then one of:\n\n"
          "X wins!   (or O wins!)\n"
          "It's a draw!   when all 9 squares are full with no winner\n"
          "No winner yet.   when the input ran out first",
          "In hasWon, loop i from 0 to 2 and check row i (board[i][0], board[i][1], board[i][2]) and column i (board[0][i], board[1][i], board[2][i]). "
          "Then check the two diagonals. Keep char winner = ' '; in main and set it before you break; out of the loop.",
          "#include <stdio.h>\n"
          "\n"
          "void printBoard(char board[3][3])\n"
          "{\n"
          "    for (int r = 0; r < 3; r++) {\n"
          "        printf(\"%c %c %c\\n\", board[r][0], board[r][1], board[r][2]);\n"
          "    }\n"
          "}\n"
          "\n"
          "int hasWon(char board[3][3], char p)\n"
          "{\n"
          "    for (int i = 0; i < 3; i++) {\n"
          "        if (board[i][0] == p && board[i][1] == p && board[i][2] == p) return 1;   // row\n"
          "        if (board[0][i] == p && board[1][i] == p && board[2][i] == p) return 1;   // column\n"
          "    }\n"
          "    if (board[0][0] == p && board[1][1] == p && board[2][2] == p) return 1;\n"
          "    if (board[0][2] == p && board[1][1] == p && board[2][0] == p) return 1;\n"
          "    return 0;\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    char board[3][3];\n"
          "    for (int r = 0; r < 3; r++) {\n"
          "        for (int c = 0; c < 3; c++) {\n"
          "            board[r][c] = '.';\n"
          "        }\n"
          "    }\n"
          "\n"
          "    char player = 'X';\n"
          "    char winner = ' ';\n"
          "    int moves = 0;\n"
          "    int row, col;\n"
          "\n"
          "    while (moves < 9 && scanf(\"%d %d\", &row, &col) == 2) {\n"
          "        if (row < 1 || row > 3 || col < 1 || col > 3) {\n"
          "            printf(\"Invalid move: %d %d\\n\", row, col);\n"
          "            continue;\n"
          "        }\n"
          "        if (board[row - 1][col - 1] != '.') {\n"
          "            printf(\"Square taken: %d %d\\n\", row, col);\n"
          "            continue;\n"
          "        }\n"
          "\n"
          "        board[row - 1][col - 1] = player;\n"
          "        moves++;\n"
          "\n"
          "        if (hasWon(board, player)) {\n"
          "            winner = player;\n"
          "            break;\n"
          "        }\n"
          "\n"
          "        if (player == 'X') {\n"
          "            player = 'O';\n"
          "        } else {\n"
          "            player = 'X';\n"
          "        }\n"
          "    }\n"
          "\n"
          "    printBoard(board);\n"
          "    if (winner != ' ') {\n"
          "        printf(\"%c wins!\\n\", winner);\n"
          "    } else if (moves == 9) {\n"
          "        printf(\"It's a draw!\\n\");\n"
          "    } else {\n"
          "        printf(\"No winner yet.\\n\");\n"
          "    }\n"
          "\n"
          "    return 0;\n"
          "}\n",
          6, {
              { "1 1\n2 1\n1 2\n2 2\n1 3\n",
                "X X X\nO O .\n. . .\nX wins!\n" },
              { "1 1\n1 2\n3 3\n2 2\n2 1\n3 2\n",
                "X O .\nX O .\n. O X\nO wins!\n" },
              { "1 1\n1 2\n1 3\n2 2\n2 1\n2 3\n3 2\n3 1\n3 3\n",
                "X O X\nX O O\nO X X\nIt's a draw!\n" },
              { "1 1\n1 2\n2 2\n2 2\n1 3\n3 3\n",
                "Square taken: 2 2\nX O O\n. X .\n. . X\nX wins!\n" },
              { "2 2\n",
                ". . .\n. X .\n. . .\nNo winner yet.\n" },
              { "1 1\n2 1\n1 2\n2 2\n1 3\n3 3\n",
                "X X X\nO O .\n. . .\nX wins!\n" },
          } },
    },
},

// ===================================================================== 7
{
    .id = "proj-dynlist", .lang = LANG_C,
    .title   = "Dynamic Number List",
    .summary = "A list of numbers on the heap that grows with realloc and shrinks when you remove values.",
    .intro   =
        "Arrays in C have a fixed size, but real programs rarely know in advance how much data is coming. "
        "In this project you'll keep a list of numbers in heap memory from malloc, walk it with pointer arithmetic, "
        "pass pointers to functions so they can hand back several answers, and resize it with realloc as numbers are added and removed.\n\n"
        "It brings together both of the Pointers and Memory Management lectures.",
    .needs = "Pointers, Pointer Arithmetic and Arrays, Memory Management in C",
    .lecture = "Pointers, Memory Management",
    .starterCode =
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int count;\n"
        "    scanf(\"%d\", &count);\n"
        "\n"
        "    // Step 1: allocate count ints with malloc, read them in and print them\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Allocate and fill",
          "The input starts with how many numbers there are (at least 1), followed by the numbers.\n\n"
          "Allocate exactly enough memory with malloc. If it returns NULL, print  Out of memory  and return 1. "
          "Read the numbers into the memory, print them, then free it:\n\n"
          "List: 4 6 7",
          "int *list = (int*)malloc(count * sizeof(int)); - then use list[i] like a normal array. "
          "Print \"List:\" and then \" %d\" for each number, so there's a space before each one. Finish with free(list);",
          "#include <stdio.h>\n"
          "#include <stdlib.h>\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int count;\n"
          "    scanf(\"%d\", &count);\n"
          "\n"
          "    int *list = (int*)malloc(count * sizeof(int));\n"
          "    if (list == NULL) {\n"
          "        printf(\"Out of memory\\n\");\n"
          "        return 1;\n"
          "    }\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        scanf(\"%d\", &list[i]);\n"
          "    }\n"
          "\n"
          "    printf(\"List:\");\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        printf(\" %d\", list[i]);\n"
          "    }\n"
          "    printf(\"\\n\");\n"
          "\n"
          "    free(list);\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "3\n4 6 7\n",      "List: 4 6 7\n" },
              { "1\n9\n",          "List: 9\n" },
              { "5\n5 4 3 2 1\n",  "List: 5 4 3 2 1\n" },
          } },
        { "Pointer functions",
          "Move the work into functions that take a pointer to the first number and a count, and walk the list with pointer arithmetic:\n\n"
          "void printList(const int *start, int count) prints the List: line.\n"
          "int sum(const int *ptr, int count) returns the total.\n"
          "void minMax(const int *ptr, int count, int *min, int *max) hands back two answers through pointers.\n\n"
          "After the list, print:\n"
          "Sum: 17\n"
          "Min: 4\n"
          "Max: 7",
          "In printList, loop for (const int *p = start; p < start + count; p++). "
          "In minMax, start with *min = *ptr; *max = *ptr; and compare every other value. Call it as minMax(list, count, &min, &max);",
          "#include <stdio.h>\n"
          "#include <stdlib.h>\n"
          "\n"
          "void printList(const int *start, int count)\n"
          "{\n"
          "    printf(\"List:\");\n"
          "    for (const int *p = start; p < start + count; p++) {\n"
          "        printf(\" %d\", *p);\n"
          "    }\n"
          "    printf(\"\\n\");\n"
          "}\n"
          "\n"
          "int sum(const int *ptr, int count)\n"
          "{\n"
          "    int total = 0;\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        total += *(ptr + i);\n"
          "    }\n"
          "    return total;\n"
          "}\n"
          "\n"
          "void minMax(const int *ptr, int count, int *min, int *max)\n"
          "{\n"
          "    *min = *ptr;\n"
          "    *max = *ptr;\n"
          "    for (const int *p = ptr + 1; p < ptr + count; p++) {\n"
          "        if (*p < *min) *min = *p;\n"
          "        if (*p > *max) *max = *p;\n"
          "    }\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int count;\n"
          "    scanf(\"%d\", &count);\n"
          "\n"
          "    int *list = (int*)malloc(count * sizeof(int));\n"
          "    if (list == NULL) {\n"
          "        printf(\"Out of memory\\n\");\n"
          "        return 1;\n"
          "    }\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        scanf(\"%d\", &list[i]);\n"
          "    }\n"
          "    printList(list, count);\n"
          "\n"
          "    int min, max;\n"
          "    minMax(list, count, &min, &max);\n"
          "    printf(\"Sum: %d\\n\", sum(list, count));\n"
          "    printf(\"Min: %d\\n\", min);\n"
          "    printf(\"Max: %d\\n\", max);\n"
          "\n"
          "    free(list);\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "3\n4 6 7\n",       "List: 4 6 7\nSum: 17\nMin: 4\nMax: 7\n" },
              { "1\n-3\n",          "List: -3\nSum: -3\nMin: -3\nMax: -3\n" },
              { "5\n5 -4 3 12 1\n", "List: 5 -4 3 12 1\nSum: 17\nMin: -4\nMax: 12\n" },
          } },
        { "Grow with realloc",
          "After the starting numbers, read commands until the input runs out:\n\n"
          "add 9 - make the memory one int bigger with realloc, store 9 at the end and print  Added 9\n"
          "print - print the List: line again\n\n"
          "Print the first List: line straight after reading the numbers, as before, and move the Sum, Min and Max lines to the very end. "
          "If realloc returns NULL, print  Out of memory  and stop reading commands (your old memory is still valid - free it).",
          "Read each command word with scanf(\"%19s\", command) and compare with strcmp (add #include <string.h>). "
          "For add: int *bigger = (int*)realloc(list, (count + 1) * sizeof(int)); if it isn't NULL, list = bigger; list[count] = value; count++;",
          "#include <stdio.h>\n"
          "#include <stdlib.h>\n"
          "#include <string.h>\n"
          "\n"
          "void printList(const int *start, int count)\n"
          "{\n"
          "    printf(\"List:\");\n"
          "    for (const int *p = start; p < start + count; p++) {\n"
          "        printf(\" %d\", *p);\n"
          "    }\n"
          "    printf(\"\\n\");\n"
          "}\n"
          "\n"
          "int sum(const int *ptr, int count)\n"
          "{\n"
          "    int total = 0;\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        total += *(ptr + i);\n"
          "    }\n"
          "    return total;\n"
          "}\n"
          "\n"
          "void minMax(const int *ptr, int count, int *min, int *max)\n"
          "{\n"
          "    *min = *ptr;\n"
          "    *max = *ptr;\n"
          "    for (const int *p = ptr + 1; p < ptr + count; p++) {\n"
          "        if (*p < *min) *min = *p;\n"
          "        if (*p > *max) *max = *p;\n"
          "    }\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int count;\n"
          "    scanf(\"%d\", &count);\n"
          "\n"
          "    int *list = (int*)malloc(count * sizeof(int));\n"
          "    if (list == NULL) {\n"
          "        printf(\"Out of memory\\n\");\n"
          "        return 1;\n"
          "    }\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        scanf(\"%d\", &list[i]);\n"
          "    }\n"
          "    printList(list, count);\n"
          "\n"
          "    char command[20];\n"
          "    int value;\n"
          "    while (scanf(\"%19s\", command) == 1) {\n"
          "        if (strcmp(command, \"add\") == 0) {\n"
          "            scanf(\"%d\", &value);\n"
          "            int *bigger = (int*)realloc(list, (count + 1) * sizeof(int));\n"
          "            if (bigger == NULL) {\n"
          "                printf(\"Out of memory\\n\");\n"
          "                break;\n"
          "            }\n"
          "            list = bigger;\n"
          "            list[count] = value;\n"
          "            count++;\n"
          "            printf(\"Added %d\\n\", value);\n"
          "        } else if (strcmp(command, \"print\") == 0) {\n"
          "            printList(list, count);\n"
          "        }\n"
          "    }\n"
          "\n"
          "    int min, max;\n"
          "    minMax(list, count, &min, &max);\n"
          "    printf(\"Sum: %d\\n\", sum(list, count));\n"
          "    printf(\"Min: %d\\n\", min);\n"
          "    printf(\"Max: %d\\n\", max);\n"
          "\n"
          "    free(list);\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "2\n4 6\nadd 9\nprint\n",  "List: 4 6\nAdded 9\nList: 4 6 9\nSum: 19\nMin: 4\nMax: 9\n" },
              { "1\n5\nadd -2\nadd 8\n",   "List: 5\nAdded -2\nAdded 8\nSum: 11\nMin: -2\nMax: 8\n" },
              { "3\n4 6 7\n",              "List: 4 6 7\nSum: 17\nMin: 4\nMax: 7\n" },
          } },
        { "Remove values",
          "Add one more command:\n\n"
          "remove 3 - delete every 3 from the list and print how many went:  Removed 2\n\n"
          "Shuffle the numbers you keep towards the front using two pointers (one reading, one writing), then shrink the memory with realloc. "
          "If the list becomes empty, free it and set the pointer to NULL instead - realloc(NULL, size) acts like malloc, so add still works afterwards.\n\n"
          "At the end, if the list is empty print  List is empty  instead of the Sum, Min and Max lines.",
          "int *write = list; for (int *read = list; read < list + count; read++) { if (*read != value) { *write = *read; write++; } } "
          "The number kept is write - list. Pointer subtraction counts the elements between two pointers.",
          "#include <stdio.h>\n"
          "#include <stdlib.h>\n"
          "#include <string.h>\n"
          "\n"
          "void printList(const int *start, int count)\n"
          "{\n"
          "    printf(\"List:\");\n"
          "    for (const int *p = start; p < start + count; p++) {\n"
          "        printf(\" %d\", *p);\n"
          "    }\n"
          "    printf(\"\\n\");\n"
          "}\n"
          "\n"
          "int sum(const int *ptr, int count)\n"
          "{\n"
          "    int total = 0;\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        total += *(ptr + i);\n"
          "    }\n"
          "    return total;\n"
          "}\n"
          "\n"
          "void minMax(const int *ptr, int count, int *min, int *max)\n"
          "{\n"
          "    *min = *ptr;\n"
          "    *max = *ptr;\n"
          "    for (const int *p = ptr + 1; p < ptr + count; p++) {\n"
          "        if (*p < *min) *min = *p;\n"
          "        if (*p > *max) *max = *p;\n"
          "    }\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int count;\n"
          "    scanf(\"%d\", &count);\n"
          "\n"
          "    int *list = (int*)malloc(count * sizeof(int));\n"
          "    if (list == NULL) {\n"
          "        printf(\"Out of memory\\n\");\n"
          "        return 1;\n"
          "    }\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        scanf(\"%d\", &list[i]);\n"
          "    }\n"
          "    printList(list, count);\n"
          "\n"
          "    char command[20];\n"
          "    int value;\n"
          "    while (scanf(\"%19s\", command) == 1) {\n"
          "        if (strcmp(command, \"add\") == 0) {\n"
          "            scanf(\"%d\", &value);\n"
          "            int *bigger = (int*)realloc(list, (count + 1) * sizeof(int));\n"
          "            if (bigger == NULL) {\n"
          "                printf(\"Out of memory\\n\");\n"
          "                break;\n"
          "            }\n"
          "            list = bigger;\n"
          "            list[count] = value;\n"
          "            count++;\n"
          "            printf(\"Added %d\\n\", value);\n"
          "        } else if (strcmp(command, \"remove\") == 0) {\n"
          "            scanf(\"%d\", &value);\n"
          "            int *write = list;\n"
          "            for (int *read = list; read < list + count; read++) {\n"
          "                if (*read != value) {\n"
          "                    *write = *read;\n"
          "                    write++;\n"
          "                }\n"
          "            }\n"
          "            int kept = (int)(write - list);\n"
          "            printf(\"Removed %d\\n\", count - kept);\n"
          "            count = kept;\n"
          "            if (count == 0) {\n"
          "                free(list);\n"
          "                list = NULL;\n"
          "            } else {\n"
          "                int *smaller = (int*)realloc(list, count * sizeof(int));\n"
          "                if (smaller != NULL) {\n"
          "                    list = smaller;\n"
          "                }\n"
          "            }\n"
          "        } else if (strcmp(command, \"print\") == 0) {\n"
          "            printList(list, count);\n"
          "        }\n"
          "    }\n"
          "\n"
          "    if (count == 0) {\n"
          "        printf(\"List is empty\\n\");\n"
          "    } else {\n"
          "        int min, max;\n"
          "        minMax(list, count, &min, &max);\n"
          "        printf(\"Sum: %d\\n\", sum(list, count));\n"
          "        printf(\"Min: %d\\n\", min);\n"
          "        printf(\"Max: %d\\n\", max);\n"
          "    }\n"
          "\n"
          "    free(list);\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "4\n3 1 3 2\nremove 3\nprint\n",      "List: 3 1 3 2\nRemoved 2\nList: 1 2\nSum: 3\nMin: 1\nMax: 2\n" },
              { "2\n7 7\nremove 7\nprint\nadd 4\n",    "List: 7 7\nRemoved 2\nList:\nAdded 4\nSum: 4\nMin: 4\nMax: 4\n" },
              { "1\n5\nremove 9\nremove 5\n",          "List: 5\nRemoved 0\nRemoved 1\nList is empty\n" },
              { "2\n4 6\nadd 9\nremove 4\n",           "List: 4 6\nAdded 9\nRemoved 1\nSum: 15\nMin: 6\nMax: 9\n" },
          } },
    },
},

// ===================================================================== 8
{
    .id = "proj-geometry", .lang = LANG_CPP,
    .title   = "Geometry Vectors",
    .summary = "The lecture's Point and Vector classes, grown into a small geometry tool with methods and constructors.",
    .intro   =
        "The Classes lecture builds up a geometric vector - a start point and an end point - from four loose doubles into a pair of classes. "
        "In this project you'll write those classes yourself and keep improving them: methods to move vectors around, constructors and private data "
        "with getters, and finally functions that change a vector through a reference and through a pointer.\n\n"
        "Every step reads commands such as  move 1 2  and prints the vector after each one.",
    .needs = "Classes and Objects, Constructors and Access Control, Pass by Reference",
    .lecture = "Classes",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "// Step 1: write a Point class (x, y and a print method)\n"
        "// and a Vector class (start and end Points and a print method)\n"
        "\n"
        "int main()\n"
        "{\n"
        "    // Read the start and end points, then print the vector\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Points and vectors",
          "Write the two classes from the lecture, with public fields:\n\n"
          "Point has doubles x and y, and print() prints (x,y).\n"
          "Vector has Points start and end, and print() prints the start, then  -> , then the end, then a new line.\n\n"
          "Read four numbers - start x and y, then end x and y - and print the vector:\n\n"
          "Vector: (1.2,0.4) -> (2,1.6)",
          "Vector's print can just call start.print(); then cout << \" -> \"; then end.print(); and cout << \"\\n\";. "
          "In main, read straight into the fields: cin >> vec.start.x >> vec.start.y >> vec.end.x >> vec.end.y;",
          "#include <iostream>\n"
          "using namespace std;\n"
          "\n"
          "class Point {\n"
          "public:\n"
          "    double x, y;\n"
          "\n"
          "    void print()\n"
          "    {\n"
          "        cout << \"(\" << x << \",\" << y << \")\";\n"
          "    }\n"
          "};\n"
          "\n"
          "class Vector {\n"
          "public:\n"
          "    Point start, end;\n"
          "\n"
          "    void print()\n"
          "    {\n"
          "        start.print();\n"
          "        cout << \" -> \";\n"
          "        end.print();\n"
          "        cout << \"\\n\";\n"
          "    }\n"
          "};\n"
          "\n"
          "int main()\n"
          "{\n"
          "    Vector vec;\n"
          "    cin >> vec.start.x >> vec.start.y >> vec.end.x >> vec.end.y;\n"
          "    cout << \"Vector: \";\n"
          "    vec.print();\n"
          "    return 0;\n"
          "}\n",
          2, {
              { "1.2 0.4 2.0 1.6\n", "Vector: (1.2,0.4) -> (2,1.6)\n" },
              { "0 0 3 4\n",         "Vector: (0,0) -> (3,4)\n" },
          } },
        { "Move it with methods",
          "Give Point and Vector an offset(offsetX, offsetY) method like the lecture's - Vector's just offsets both of its points.\n\n"
          "After printing the vector, read commands until the input runs out. For  move 1 1.5  offset the vector and print it again:\n\n"
          "Moved: (2.2,1.9) -> (3,3.1)",
          "Read with string command; while (cin >> command) { if (command == \"move\") { double dx, dy; cin >> dx >> dy; ... } }. "
          "Add #include <string>. Vector::offset is start.offset(offsetX, offsetY); end.offset(offsetX, offsetY);",
          "#include <iostream>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "class Point {\n"
          "public:\n"
          "    double x, y;\n"
          "\n"
          "    void offset(double offsetX, double offsetY)\n"
          "    {\n"
          "        x += offsetX;\n"
          "        y += offsetY;\n"
          "    }\n"
          "\n"
          "    void print()\n"
          "    {\n"
          "        cout << \"(\" << x << \",\" << y << \")\";\n"
          "    }\n"
          "};\n"
          "\n"
          "class Vector {\n"
          "public:\n"
          "    Point start, end;\n"
          "\n"
          "    void offset(double offsetX, double offsetY)\n"
          "    {\n"
          "        start.offset(offsetX, offsetY);\n"
          "        end.offset(offsetX, offsetY);\n"
          "    }\n"
          "\n"
          "    void print()\n"
          "    {\n"
          "        start.print();\n"
          "        cout << \" -> \";\n"
          "        end.print();\n"
          "        cout << \"\\n\";\n"
          "    }\n"
          "};\n"
          "\n"
          "int main()\n"
          "{\n"
          "    Vector vec;\n"
          "    cin >> vec.start.x >> vec.start.y >> vec.end.x >> vec.end.y;\n"
          "    cout << \"Vector: \";\n"
          "    vec.print();\n"
          "\n"
          "    string command;\n"
          "    while (cin >> command) {\n"
          "        if (command == \"move\") {\n"
          "            double dx, dy;\n"
          "            cin >> dx >> dy;\n"
          "            vec.offset(dx, dy);\n"
          "            cout << \"Moved: \";\n"
          "            vec.print();\n"
          "        }\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "1.2 0.4 2.0 1.6\nmove 1 1.5\n",   "Vector: (1.2,0.4) -> (2,1.6)\nMoved: (2.2,1.9) -> (3,3.1)\n" },
              { "0 0 3 4\nmove 1 0\nmove 0 -2\n",  "Vector: (0,0) -> (3,4)\nMoved: (1,0) -> (4,4)\nMoved: (1,-2) -> (4,2)\n" },
              { "5 5 6 6\n",                        "Vector: (5,5) -> (6,6)\n" },
          } },
        { "Constructors and private data",
          "Tidy the classes up the way the second half of the lecture does:\n\n"
          "Make every field private. Give Point a default constructor (0, 0), a constructor Point(double nx, double ny) using an initialisation list, and getters getX() and getY().\n"
          "Give Vector a constructor Vector(Point s, Point e) and a method length() returning the distance from start to end.\n\n"
          "Build the vector with the constructor in main, and at the very end print its length with two decimal places:\n\n"
          "Length: 1.44",
          "Point(double nx, double ny) : x(nx), y(ny) { }. Vector(Point s, Point e) : start(s), end(e) { }. "
          "length() is sqrt(dx * dx + dy * dy) from <cmath>, where dx = end.getX() - start.getX(). "
          "Print with cout << fixed << setprecision(2) from <iomanip> - do it last, because fixed changes how every later double prints.",
          "#include <iostream>\n"
          "#include <iomanip>\n"
          "#include <cmath>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "class Point {\n"
          "private:\n"
          "    double x, y;\n"
          "\n"
          "public:\n"
          "    Point() : x(0), y(0) { }\n"
          "    Point(double nx, double ny) : x(nx), y(ny) { }\n"
          "\n"
          "    double getX() { return x; }\n"
          "    double getY() { return y; }\n"
          "\n"
          "    void offset(double offsetX, double offsetY)\n"
          "    {\n"
          "        x += offsetX;\n"
          "        y += offsetY;\n"
          "    }\n"
          "\n"
          "    void print()\n"
          "    {\n"
          "        cout << \"(\" << x << \",\" << y << \")\";\n"
          "    }\n"
          "};\n"
          "\n"
          "class Vector {\n"
          "private:\n"
          "    Point start, end;\n"
          "\n"
          "public:\n"
          "    Vector(Point s, Point e) : start(s), end(e) { }\n"
          "\n"
          "    double length()\n"
          "    {\n"
          "        double dx = end.getX() - start.getX();\n"
          "        double dy = end.getY() - start.getY();\n"
          "        return sqrt(dx * dx + dy * dy);\n"
          "    }\n"
          "\n"
          "    void offset(double offsetX, double offsetY)\n"
          "    {\n"
          "        start.offset(offsetX, offsetY);\n"
          "        end.offset(offsetX, offsetY);\n"
          "    }\n"
          "\n"
          "    void print()\n"
          "    {\n"
          "        start.print();\n"
          "        cout << \" -> \";\n"
          "        end.print();\n"
          "        cout << \"\\n\";\n"
          "    }\n"
          "};\n"
          "\n"
          "int main()\n"
          "{\n"
          "    double x0, y0, x1, y1;\n"
          "    cin >> x0 >> y0 >> x1 >> y1;\n"
          "    Vector vec(Point(x0, y0), Point(x1, y1));\n"
          "    cout << \"Vector: \";\n"
          "    vec.print();\n"
          "\n"
          "    string command;\n"
          "    while (cin >> command) {\n"
          "        if (command == \"move\") {\n"
          "            double dx, dy;\n"
          "            cin >> dx >> dy;\n"
          "            vec.offset(dx, dy);\n"
          "            cout << \"Moved: \";\n"
          "            vec.print();\n"
          "        }\n"
          "    }\n"
          "\n"
          "    cout << fixed << setprecision(2) << \"Length: \" << vec.length() << \"\\n\";\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "1.2 0.4 2.0 1.6\nmove 1 1.5\n",   "Vector: (1.2,0.4) -> (2,1.6)\nMoved: (2.2,1.9) -> (3,3.1)\nLength: 1.44\n" },
              { "0 0 3 4\nmove 1 0\n",             "Vector: (0,0) -> (3,4)\nMoved: (1,0) -> (4,4)\nLength: 5.00\n" },
              { "2 2 2 2\n",                        "Vector: (2,2) -> (2,2)\nLength: 0.00\n" },
          } },
        { "By reference and by pointer",
          "The lecture shows that objects can be changed through a reference or through a pointer. Write two ordinary functions (not methods), "
          "plus the Vector methods they need - getStart(), getEnd() and setEnd(Point p):\n\n"
          "void scale(Vector &v, double factor) keeps the start and moves the end so the vector is factor times as long.\n"
          "void moveToOrigin(Vector *v) moves the whole vector so it starts at (0,0) - use -> on the pointer.\n\n"
          "Add the commands  scale 2  (prints  Scaled: ...) and  reset  (prints  Reset: ...).",
          "New end x = start x + (end x - start x) * factor, and the same for y. "
          "In moveToOrigin: Point s = v->getStart(); v->offset(-s.getX(), -s.getY());. Call them with scale(vec, f); and moveToOrigin(&vec);",
          "#include <iostream>\n"
          "#include <iomanip>\n"
          "#include <cmath>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "class Point {\n"
          "private:\n"
          "    double x, y;\n"
          "\n"
          "public:\n"
          "    Point() : x(0), y(0) { }\n"
          "    Point(double nx, double ny) : x(nx), y(ny) { }\n"
          "\n"
          "    double getX() { return x; }\n"
          "    double getY() { return y; }\n"
          "\n"
          "    void offset(double offsetX, double offsetY)\n"
          "    {\n"
          "        x += offsetX;\n"
          "        y += offsetY;\n"
          "    }\n"
          "\n"
          "    void print()\n"
          "    {\n"
          "        cout << \"(\" << x << \",\" << y << \")\";\n"
          "    }\n"
          "};\n"
          "\n"
          "class Vector {\n"
          "private:\n"
          "    Point start, end;\n"
          "\n"
          "public:\n"
          "    Vector(Point s, Point e) : start(s), end(e) { }\n"
          "\n"
          "    Point getStart() { return start; }\n"
          "    Point getEnd() { return end; }\n"
          "    void setEnd(Point p) { end = p; }\n"
          "\n"
          "    double length()\n"
          "    {\n"
          "        double dx = end.getX() - start.getX();\n"
          "        double dy = end.getY() - start.getY();\n"
          "        return sqrt(dx * dx + dy * dy);\n"
          "    }\n"
          "\n"
          "    void offset(double offsetX, double offsetY)\n"
          "    {\n"
          "        start.offset(offsetX, offsetY);\n"
          "        end.offset(offsetX, offsetY);\n"
          "    }\n"
          "\n"
          "    void print()\n"
          "    {\n"
          "        start.print();\n"
          "        cout << \" -> \";\n"
          "        end.print();\n"
          "        cout << \"\\n\";\n"
          "    }\n"
          "};\n"
          "\n"
          "void scale(Vector &v, double factor)\n"
          "{\n"
          "    Point s = v.getStart();\n"
          "    Point e = v.getEnd();\n"
          "    v.setEnd(Point(s.getX() + (e.getX() - s.getX()) * factor,\n"
          "                   s.getY() + (e.getY() - s.getY()) * factor));\n"
          "}\n"
          "\n"
          "void moveToOrigin(Vector *v)\n"
          "{\n"
          "    Point s = v->getStart();\n"
          "    v->offset(-s.getX(), -s.getY());\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    double x0, y0, x1, y1;\n"
          "    cin >> x0 >> y0 >> x1 >> y1;\n"
          "    Vector vec(Point(x0, y0), Point(x1, y1));\n"
          "    cout << \"Vector: \";\n"
          "    vec.print();\n"
          "\n"
          "    string command;\n"
          "    while (cin >> command) {\n"
          "        if (command == \"move\") {\n"
          "            double dx, dy;\n"
          "            cin >> dx >> dy;\n"
          "            vec.offset(dx, dy);\n"
          "            cout << \"Moved: \";\n"
          "            vec.print();\n"
          "        } else if (command == \"scale\") {\n"
          "            double factor;\n"
          "            cin >> factor;\n"
          "            scale(vec, factor);\n"
          "            cout << \"Scaled: \";\n"
          "            vec.print();\n"
          "        } else if (command == \"reset\") {\n"
          "            moveToOrigin(&vec);\n"
          "            cout << \"Reset: \";\n"
          "            vec.print();\n"
          "        }\n"
          "    }\n"
          "\n"
          "    cout << fixed << setprecision(2) << \"Length: \" << vec.length() << \"\\n\";\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "1 1 4 5\nscale 2\nreset\nmove 1 0\n", "Vector: (1,1) -> (4,5)\nScaled: (1,1) -> (7,9)\nReset: (0,0) -> (6,8)\nMoved: (1,0) -> (7,8)\nLength: 10.00\n" },
              { "2 3 4 3\nscale 0.5\n",                "Vector: (2,3) -> (4,3)\nScaled: (2,3) -> (3,3)\nLength: 1.00\n" },
              { "0 0 3 4\nmove 1 1\n",                  "Vector: (0,0) -> (3,4)\nMoved: (1,1) -> (4,5)\nLength: 5.00\n" },
          } },
    },
},

// ===================================================================== 9
{
    .id = "proj-linkedlist", .lang = LANG_CPP,
    .title   = "Linked List Manager",
    .summary = "A command-driven linked list: insert, delete, search, then a LinkedList class with deep copies.",
    .intro   =
        "This project follows the lecture on Pointers in C++ and Linked Lists from start to finish. You'll begin with a Node struct and "
        "free functions that take a Node**, add inserting at the front, deleting and searching, and end by turning it into the lecture's "
        "LinkedList class - with a destructor, a copy constructor and an assignment operator that copy every node.\n\n"
        "The program reads commands such as  end 10,  front 5,  delete 20  and  print.",
    .needs = "Linked Lists, Objects, Pointers and Destructors",
    .lecture = "Pointers in C++ and Linked Lists",
    .starterCode =
        "#include <iostream>\n"
        "#include <string>\n"
        "using namespace std;\n"
        "\n"
        "struct Node {\n"
        "    int data;\n"
        "    Node* next;\n"
        "};\n"
        "\n"
        "// Step 1: write these two functions\n"
        "//   void InsertAtEnd(Node** ptrToHeadPtr, int newData)\n"
        "//   void PrintList(Node* headPtr)\n"
        "\n"
        "int main()\n"
        "{\n"
        "    Node* head = nullptr;\n"
        "    string command;\n"
        "    int value;\n"
        "\n"
        "    while (cin >> command) {\n"
        "        // end X adds X at the end, print prints the list\n"
        "    }\n"
        "\n"
        "    // free every node before the program ends\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Insert at the end",
          "Write InsertAtEnd(Node** ptrToHeadPtr, int newData) and PrintList(Node* headPtr) from the lecture. Then read commands until the input runs out:\n\n"
          "end 10 - add 10 at the end of the list\n"
          "print - print  List:  followed by each value with a space before it, like  List: 10 20\n\n"
          "Before main returns, walk the list and delete every node.",
          "Call InsertAtEnd(&head, value); so the function can change head. In PrintList, cout << \"List:\"; then cout << \" \" << cur->data; for each node and finish with endl. "
          "To free the list: while (head != nullptr) { Node* temp = head; head = head->next; delete temp; }",
          "#include <iostream>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "struct Node {\n"
          "    int data;\n"
          "    Node* next;\n"
          "};\n"
          "\n"
          "void InsertAtEnd(Node** ptrToHeadPtr, int newData)\n"
          "{\n"
          "    Node* newNode = new Node();\n"
          "    newNode->data = newData;\n"
          "    newNode->next = nullptr;\n"
          "    if (*ptrToHeadPtr == nullptr) {\n"
          "        *ptrToHeadPtr = newNode;\n"
          "        return;\n"
          "    }\n"
          "    Node* cur = *ptrToHeadPtr;\n"
          "    while (cur->next != nullptr) {\n"
          "        cur = cur->next;\n"
          "    }\n"
          "    cur->next = newNode;\n"
          "}\n"
          "\n"
          "void PrintList(Node* headPtr)\n"
          "{\n"
          "    cout << \"List:\";\n"
          "    for (Node* cur = headPtr; cur != nullptr; cur = cur->next) {\n"
          "        cout << \" \" << cur->data;\n"
          "    }\n"
          "    cout << endl;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    Node* head = nullptr;\n"
          "    string command;\n"
          "    int value;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"end\") {\n"
          "            cin >> value;\n"
          "            InsertAtEnd(&head, value);\n"
          "        } else if (command == \"print\") {\n"
          "            PrintList(head);\n"
          "        }\n"
          "    }\n"
          "\n"
          "    while (head != nullptr) {\n"
          "        Node* temp = head;\n"
          "        head = head->next;\n"
          "        delete temp;\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "end 10\nend 20\nend 30\nprint\n", "List: 10 20 30\n" },
              { "print\nend 5\nprint\n",           "List:\nList: 5\n" },
              { "end 1\nprint\nend 2\nprint\n",    "List: 1\nList: 1 2\n" },
          } },
        { "Front and delete",
          "Add two more commands, each with its own function:\n\n"
          "front 5 - InsertAtBeginning(&head, 5) puts 5 at the start.\n"
          "delete 20 - DeleteWithValue(&head, 20) removes the first node holding 20. "
          "If the list is empty it prints  List is empty.  and if the value isn't there it prints  Value not found in the list.\n\n"
          "Remember to delete the node you unlink.",
          "InsertAtBeginning: newNode->next = *ptrToHeadPtr; *ptrToHeadPtr = newNode;. "
          "DeleteWithValue: handle the empty list, then the head (move *ptrToHeadPtr along), then walk with prev and cur and link prev->next = cur->next;",
          "#include <iostream>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "struct Node {\n"
          "    int data;\n"
          "    Node* next;\n"
          "};\n"
          "\n"
          "void InsertAtEnd(Node** ptrToHeadPtr, int newData)\n"
          "{\n"
          "    Node* newNode = new Node();\n"
          "    newNode->data = newData;\n"
          "    newNode->next = nullptr;\n"
          "    if (*ptrToHeadPtr == nullptr) {\n"
          "        *ptrToHeadPtr = newNode;\n"
          "        return;\n"
          "    }\n"
          "    Node* cur = *ptrToHeadPtr;\n"
          "    while (cur->next != nullptr) {\n"
          "        cur = cur->next;\n"
          "    }\n"
          "    cur->next = newNode;\n"
          "}\n"
          "\n"
          "void InsertAtBeginning(Node** ptrToHeadPtr, int newData)\n"
          "{\n"
          "    Node* newNode = new Node();\n"
          "    newNode->data = newData;\n"
          "    newNode->next = *ptrToHeadPtr;\n"
          "    *ptrToHeadPtr = newNode;\n"
          "}\n"
          "\n"
          "void DeleteWithValue(Node** ptrToHeadPtr, int value)\n"
          "{\n"
          "    if (*ptrToHeadPtr == nullptr) {\n"
          "        cout << \"List is empty.\" << endl;\n"
          "        return;\n"
          "    }\n"
          "    Node* cur = *ptrToHeadPtr;\n"
          "    if (cur->data == value) {\n"
          "        *ptrToHeadPtr = cur->next;\n"
          "        delete cur;\n"
          "        return;\n"
          "    }\n"
          "    Node* prev = nullptr;\n"
          "    while (cur != nullptr && cur->data != value) {\n"
          "        prev = cur;\n"
          "        cur = cur->next;\n"
          "    }\n"
          "    if (cur == nullptr) {\n"
          "        cout << \"Value not found in the list.\" << endl;\n"
          "        return;\n"
          "    }\n"
          "    prev->next = cur->next;\n"
          "    delete cur;\n"
          "}\n"
          "\n"
          "void PrintList(Node* headPtr)\n"
          "{\n"
          "    cout << \"List:\";\n"
          "    for (Node* cur = headPtr; cur != nullptr; cur = cur->next) {\n"
          "        cout << \" \" << cur->data;\n"
          "    }\n"
          "    cout << endl;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    Node* head = nullptr;\n"
          "    string command;\n"
          "    int value;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"end\") {\n"
          "            cin >> value;\n"
          "            InsertAtEnd(&head, value);\n"
          "        } else if (command == \"front\") {\n"
          "            cin >> value;\n"
          "            InsertAtBeginning(&head, value);\n"
          "        } else if (command == \"delete\") {\n"
          "            cin >> value;\n"
          "            DeleteWithValue(&head, value);\n"
          "        } else if (command == \"print\") {\n"
          "            PrintList(head);\n"
          "        }\n"
          "    }\n"
          "\n"
          "    while (head != nullptr) {\n"
          "        Node* temp = head;\n"
          "        head = head->next;\n"
          "        delete temp;\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "end 10\nend 20\nend 30\nfront 5\nprint\ndelete 20\nprint\n", "List: 5 10 20 30\nList: 5 10 30\n" },
              { "delete 1\nend 1\ndelete 9\nprint\n",                         "List is empty.\nValue not found in the list.\nList: 1\n" },
              { "front 3\nfront 2\nfront 1\ndelete 1\nprint\n",                "List: 2 3\n" },
              { "end 7\nend 7\ndelete 7\nprint\n",                             "List: 7\n" },
          } },
        { "Searching and counting",
          "Add two functions that traverse the list without changing it, and the commands that use them:\n\n"
          "contains 10 - prints  List contains 10: Yes  or  ...: No\n"
          "size - prints  List size: 3",
          "bool Contains(Node* headPtr, int value) loops like PrintList and returns true as soon as it finds the value. "
          "int GetSize(Node* headPtr) counts the nodes. For Yes/No: cout << (Contains(head, value) ? \"Yes\" : \"No\");",
          "#include <iostream>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "struct Node {\n"
          "    int data;\n"
          "    Node* next;\n"
          "};\n"
          "\n"
          "void InsertAtEnd(Node** ptrToHeadPtr, int newData)\n"
          "{\n"
          "    Node* newNode = new Node();\n"
          "    newNode->data = newData;\n"
          "    newNode->next = nullptr;\n"
          "    if (*ptrToHeadPtr == nullptr) {\n"
          "        *ptrToHeadPtr = newNode;\n"
          "        return;\n"
          "    }\n"
          "    Node* cur = *ptrToHeadPtr;\n"
          "    while (cur->next != nullptr) {\n"
          "        cur = cur->next;\n"
          "    }\n"
          "    cur->next = newNode;\n"
          "}\n"
          "\n"
          "void InsertAtBeginning(Node** ptrToHeadPtr, int newData)\n"
          "{\n"
          "    Node* newNode = new Node();\n"
          "    newNode->data = newData;\n"
          "    newNode->next = *ptrToHeadPtr;\n"
          "    *ptrToHeadPtr = newNode;\n"
          "}\n"
          "\n"
          "void DeleteWithValue(Node** ptrToHeadPtr, int value)\n"
          "{\n"
          "    if (*ptrToHeadPtr == nullptr) {\n"
          "        cout << \"List is empty.\" << endl;\n"
          "        return;\n"
          "    }\n"
          "    Node* cur = *ptrToHeadPtr;\n"
          "    if (cur->data == value) {\n"
          "        *ptrToHeadPtr = cur->next;\n"
          "        delete cur;\n"
          "        return;\n"
          "    }\n"
          "    Node* prev = nullptr;\n"
          "    while (cur != nullptr && cur->data != value) {\n"
          "        prev = cur;\n"
          "        cur = cur->next;\n"
          "    }\n"
          "    if (cur == nullptr) {\n"
          "        cout << \"Value not found in the list.\" << endl;\n"
          "        return;\n"
          "    }\n"
          "    prev->next = cur->next;\n"
          "    delete cur;\n"
          "}\n"
          "\n"
          "bool Contains(Node* headPtr, int value)\n"
          "{\n"
          "    for (Node* cur = headPtr; cur != nullptr; cur = cur->next) {\n"
          "        if (cur->data == value) {\n"
          "            return true;\n"
          "        }\n"
          "    }\n"
          "    return false;\n"
          "}\n"
          "\n"
          "int GetSize(Node* headPtr)\n"
          "{\n"
          "    int count = 0;\n"
          "    for (Node* cur = headPtr; cur != nullptr; cur = cur->next) {\n"
          "        count++;\n"
          "    }\n"
          "    return count;\n"
          "}\n"
          "\n"
          "void PrintList(Node* headPtr)\n"
          "{\n"
          "    cout << \"List:\";\n"
          "    for (Node* cur = headPtr; cur != nullptr; cur = cur->next) {\n"
          "        cout << \" \" << cur->data;\n"
          "    }\n"
          "    cout << endl;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    Node* head = nullptr;\n"
          "    string command;\n"
          "    int value;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"end\") {\n"
          "            cin >> value;\n"
          "            InsertAtEnd(&head, value);\n"
          "        } else if (command == \"front\") {\n"
          "            cin >> value;\n"
          "            InsertAtBeginning(&head, value);\n"
          "        } else if (command == \"delete\") {\n"
          "            cin >> value;\n"
          "            DeleteWithValue(&head, value);\n"
          "        } else if (command == \"contains\") {\n"
          "            cin >> value;\n"
          "            cout << \"List contains \" << value << \": \" << (Contains(head, value) ? \"Yes\" : \"No\") << endl;\n"
          "        } else if (command == \"size\") {\n"
          "            cout << \"List size: \" << GetSize(head) << endl;\n"
          "        } else if (command == \"print\") {\n"
          "            PrintList(head);\n"
          "        }\n"
          "    }\n"
          "\n"
          "    while (head != nullptr) {\n"
          "        Node* temp = head;\n"
          "        head = head->next;\n"
          "        delete temp;\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "end 10\nend 20\ncontains 10\ncontains 25\nsize\n", "List contains 10: Yes\nList contains 25: No\nList size: 2\n" },
              { "size\ncontains 1\n",                                  "List size: 0\nList contains 1: No\n" },
              { "front 4\nend 5\ndelete 4\nsize\nprint\n",             "List size: 1\nList: 5\n" },
          } },
        { "A LinkedList class",
          "Finish the way the lecture does: wrap everything in a class LinkedList with head as a private field, and the functions as methods "
          "(insertAtEnd, insertAtBeginning, deleteWithValue, contains, getSize, printList and clear). The destructor calls clear().\n\n"
          "Because the class owns memory, write a copy constructor and an assignment operator that copy every node - the rule of three. "
          "Test them with three new commands, using a second list called backup:\n\n"
          "save - backup = list; then print  Saved\n"
          "restore - list = backup; then print  Restored\n"
          "clear - empty the list and print  Cleared",
          "Copy constructor: head = nullptr; then walk other's nodes calling insertAtEnd(cur->data). "
          "Assignment: if (this != &other) { clear(); ...copy the nodes...; } return *this;. "
          "Without them, backup and list would share nodes, and deleting from one would leave the other pointing at freed memory.",
          "#include <iostream>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "struct Node {\n"
          "    int data;\n"
          "    Node* next;\n"
          "};\n"
          "\n"
          "class LinkedList {\n"
          "private:\n"
          "    Node* head;\n"
          "\n"
          "    void copyFrom(const LinkedList& other)\n"
          "    {\n"
          "        for (Node* cur = other.head; cur != nullptr; cur = cur->next) {\n"
          "            insertAtEnd(cur->data);\n"
          "        }\n"
          "    }\n"
          "\n"
          "public:\n"
          "    LinkedList() { head = nullptr; }\n"
          "    ~LinkedList() { clear(); }\n"
          "\n"
          "    LinkedList(const LinkedList& other)\n"
          "    {\n"
          "        head = nullptr;\n"
          "        copyFrom(other);\n"
          "    }\n"
          "\n"
          "    LinkedList& operator=(const LinkedList& other)\n"
          "    {\n"
          "        if (this != &other) {\n"
          "            clear();\n"
          "            copyFrom(other);\n"
          "        }\n"
          "        return *this;\n"
          "    }\n"
          "\n"
          "    void insertAtEnd(int newData)\n"
          "    {\n"
          "        Node* newNode = new Node();\n"
          "        newNode->data = newData;\n"
          "        newNode->next = nullptr;\n"
          "        if (head == nullptr) {\n"
          "            head = newNode;\n"
          "            return;\n"
          "        }\n"
          "        Node* cur = head;\n"
          "        while (cur->next != nullptr) {\n"
          "            cur = cur->next;\n"
          "        }\n"
          "        cur->next = newNode;\n"
          "    }\n"
          "\n"
          "    void insertAtBeginning(int newData)\n"
          "    {\n"
          "        Node* newNode = new Node();\n"
          "        newNode->data = newData;\n"
          "        newNode->next = head;\n"
          "        head = newNode;\n"
          "    }\n"
          "\n"
          "    void deleteWithValue(int value)\n"
          "    {\n"
          "        if (head == nullptr) {\n"
          "            cout << \"List is empty.\" << endl;\n"
          "            return;\n"
          "        }\n"
          "        if (head->data == value) {\n"
          "            Node* temp = head;\n"
          "            head = head->next;\n"
          "            delete temp;\n"
          "            return;\n"
          "        }\n"
          "        Node* cur = head;\n"
          "        Node* prev = nullptr;\n"
          "        while (cur != nullptr && cur->data != value) {\n"
          "            prev = cur;\n"
          "            cur = cur->next;\n"
          "        }\n"
          "        if (cur == nullptr) {\n"
          "            cout << \"Value not found in the list.\" << endl;\n"
          "            return;\n"
          "        }\n"
          "        prev->next = cur->next;\n"
          "        delete cur;\n"
          "    }\n"
          "\n"
          "    bool contains(int value)\n"
          "    {\n"
          "        for (Node* cur = head; cur != nullptr; cur = cur->next) {\n"
          "            if (cur->data == value) {\n"
          "                return true;\n"
          "            }\n"
          "        }\n"
          "        return false;\n"
          "    }\n"
          "\n"
          "    int getSize()\n"
          "    {\n"
          "        int count = 0;\n"
          "        for (Node* cur = head; cur != nullptr; cur = cur->next) {\n"
          "            count++;\n"
          "        }\n"
          "        return count;\n"
          "    }\n"
          "\n"
          "    void printList()\n"
          "    {\n"
          "        cout << \"List:\";\n"
          "        for (Node* cur = head; cur != nullptr; cur = cur->next) {\n"
          "            cout << \" \" << cur->data;\n"
          "        }\n"
          "        cout << endl;\n"
          "    }\n"
          "\n"
          "    void clear()\n"
          "    {\n"
          "        Node* cur = head;\n"
          "        while (cur != nullptr) {\n"
          "            Node* temp = cur;\n"
          "            cur = cur->next;\n"
          "            delete temp;\n"
          "        }\n"
          "        head = nullptr;\n"
          "    }\n"
          "};\n"
          "\n"
          "int main()\n"
          "{\n"
          "    LinkedList list;\n"
          "    LinkedList backup;\n"
          "    string command;\n"
          "    int value;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"end\") {\n"
          "            cin >> value;\n"
          "            list.insertAtEnd(value);\n"
          "        } else if (command == \"front\") {\n"
          "            cin >> value;\n"
          "            list.insertAtBeginning(value);\n"
          "        } else if (command == \"delete\") {\n"
          "            cin >> value;\n"
          "            list.deleteWithValue(value);\n"
          "        } else if (command == \"contains\") {\n"
          "            cin >> value;\n"
          "            cout << \"List contains \" << value << \": \" << (list.contains(value) ? \"Yes\" : \"No\") << endl;\n"
          "        } else if (command == \"size\") {\n"
          "            cout << \"List size: \" << list.getSize() << endl;\n"
          "        } else if (command == \"print\") {\n"
          "            list.printList();\n"
          "        } else if (command == \"save\") {\n"
          "            backup = list;\n"
          "            cout << \"Saved\" << endl;\n"
          "        } else if (command == \"restore\") {\n"
          "            list = backup;\n"
          "            cout << \"Restored\" << endl;\n"
          "        } else if (command == \"clear\") {\n"
          "            list.clear();\n"
          "            cout << \"Cleared\" << endl;\n"
          "        }\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "end 1\nend 2\nsave\ndelete 1\nprint\nrestore\nprint\n", "Saved\nList: 2\nRestored\nList: 1 2\n" },
              { "end 5\nsave\nclear\nprint\nsize\nrestore\nprint\n",     "Saved\nCleared\nList:\nList size: 0\nRestored\nList: 5\n" },
              { "front 3\nfront 2\nsave\nend 9\nrestore\ncontains 9\n",  "Saved\nRestored\nList contains 9: No\n" },
              { "end 10\nend 20\nend 30\nfront 5\ndelete 20\nprint\n",   "List: 5 10 30\n" },
          } },
    },
},

// ===================================================================== 10
{
    .id = "proj-clock", .lang = LANG_CPP,
    .title   = "Digital Clock",
    .summary = "The lecture's Clock class: ticking, printing with operator<<, prefix and postfix ++, + and comparisons.",
    .intro   =
        "The Operator Overloading lecture designs a Clock class that can be used like this: cout << c1 << endl; c1++; c1 = ++c2;. "
        "In this project you build that clock one operator at a time, then go further with + to add minutes and == and < to compare times.\n\n"
        "Watch out for the difference between prefix and postfix ++ - it's the heart of the lecture.",
    .needs = "Constructors and Access Control, Function and Operator Overloading",
    .lecture = "Operator Overloading",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "// Step 1: write the Clock class - a constructor, tick() and print()\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int h, m, n;\n"
        "    cin >> h >> m >> n;\n"
        "\n"
        "    // Make a Clock, print it, tick it n times and print it again\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Ticking clock",
          "Write a Clock class with private int fields hour and min, and:\n\n"
          "a constructor Clock(int h = 12, int m = 0) - noon by default - using an initialisation list,\n"
          "tick(), which moves on one minute (59 minutes rolls over to the next hour, and hour 23 rolls over to 0),\n"
          "print(), which prints the time as HH:MM with leading zeros, like 03:20.\n\n"
          "Read the hour, minute and a number of ticks n. Print the start time, tick n times and print it again:\n\n"
          "Start: 10:58\n"
          "After 3 ticks: 11:01",
          "tick() is the lecture's: min = (min + 1) % 60; if (min == 0) hour = (hour + 1) % 24;. "
          "For the leading zero: if (hour < 10) cout << '0'; before printing the hour, and the same for min.",
          "#include <iostream>\n"
          "using namespace std;\n"
          "\n"
          "class Clock {\n"
          "public:\n"
          "    Clock(int h = 12, int m = 0) : hour(h), min(m) { }\n"
          "\n"
          "    void tick()\n"
          "    {\n"
          "        min = (min + 1) % 60;\n"
          "        if (min == 0) {\n"
          "            hour = (hour + 1) % 24;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    void print()\n"
          "    {\n"
          "        if (hour < 10) cout << '0';\n"
          "        cout << hour << \":\";\n"
          "        if (min < 10) cout << '0';\n"
          "        cout << min;\n"
          "    }\n"
          "\n"
          "private:\n"
          "    int hour, min;\n"
          "};\n"
          "\n"
          "int main()\n"
          "{\n"
          "    int h, m, n;\n"
          "    cin >> h >> m >> n;\n"
          "\n"
          "    Clock c(h, m);\n"
          "    cout << \"Start: \";\n"
          "    c.print();\n"
          "    cout << \"\\n\";\n"
          "    for (int i = 0; i < n; i++) {\n"
          "        c.tick();\n"
          "    }\n"
          "    cout << \"After \" << n << \" ticks: \";\n"
          "    c.print();\n"
          "    cout << \"\\n\";\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "10 58 3\n", "Start: 10:58\nAfter 3 ticks: 11:01\n" },
              { "23 59 2\n", "Start: 23:59\nAfter 2 ticks: 00:01\n" },
              { "3 5 0\n",   "Start: 03:05\nAfter 0 ticks: 03:05\n" },
          } },
        { "Printing with <<",
          "Replace print() with a friend operator<< so a clock can be printed with cout << c, just like the lecture's Clock. It must return the stream.\n\n"
          "Also make a clock with the default constructor and print it first:\n\n"
          "Default: 12:00\n"
          "Start: 10:58\n"
          "After 3 ticks: 11:01",
          "Inside the class: friend ostream& operator<<(ostream& os, const Clock& c);. "
          "Outside: ostream& operator<<(ostream& os, const Clock& c) { if (c.hour < 10) os << '0'; ... return os; } - use os, not cout.",
          "#include <iostream>\n"
          "using namespace std;\n"
          "\n"
          "class Clock {\n"
          "    friend ostream& operator<<(ostream& os, const Clock& c);\n"
          "\n"
          "public:\n"
          "    Clock(int h = 12, int m = 0) : hour(h), min(m) { }\n"
          "\n"
          "    void tick()\n"
          "    {\n"
          "        min = (min + 1) % 60;\n"
          "        if (min == 0) {\n"
          "            hour = (hour + 1) % 24;\n"
          "        }\n"
          "    }\n"
          "\n"
          "private:\n"
          "    int hour, min;\n"
          "};\n"
          "\n"
          "ostream& operator<<(ostream& os, const Clock& c)\n"
          "{\n"
          "    if (c.hour < 10) os << '0';\n"
          "    os << c.hour << \":\";\n"
          "    if (c.min < 10) os << '0';\n"
          "    os << c.min;\n"
          "    return os;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    int h, m, n;\n"
          "    cin >> h >> m >> n;\n"
          "\n"
          "    Clock def;\n"
          "    Clock c(h, m);\n"
          "    cout << \"Default: \" << def << \"\\n\";\n"
          "    cout << \"Start: \" << c << \"\\n\";\n"
          "    for (int i = 0; i < n; i++) {\n"
          "        c.tick();\n"
          "    }\n"
          "    cout << \"After \" << n << \" ticks: \" << c << \"\\n\";\n"
          "    return 0;\n"
          "}\n",
          2, {
              { "10 58 3\n", "Default: 12:00\nStart: 10:58\nAfter 3 ticks: 11:01\n" },
              { "23 59 2\n", "Default: 12:00\nStart: 23:59\nAfter 2 ticks: 00:01\n" },
          } },
        { "Prefix and postfix ++",
          "Overload both forms of ++ exactly as the lecture explains: prefix ++c ticks and returns the clock itself; postfix c++ "
          "saves a copy, ticks, and returns the copy. Then, after the ticks, add:\n\n"
          "Clock a = c++;  and print  Postfix: got 10:59, clock now 11:00\n"
          "Clock b = ++c;  and print  Prefix: got 11:01, clock now 11:01",
          "Clock operator++() { tick(); return *this; } and Clock operator++(int) { Clock c = *this; tick(); return c; }. "
          "The unnamed int is the signal for postfix.",
          "#include <iostream>\n"
          "using namespace std;\n"
          "\n"
          "class Clock {\n"
          "    friend ostream& operator<<(ostream& os, const Clock& c);\n"
          "\n"
          "public:\n"
          "    Clock(int h = 12, int m = 0) : hour(h), min(m) { }\n"
          "\n"
          "    void tick()\n"
          "    {\n"
          "        min = (min + 1) % 60;\n"
          "        if (min == 0) {\n"
          "            hour = (hour + 1) % 24;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    Clock operator++()      // prefix\n"
          "    {\n"
          "        tick();\n"
          "        return *this;\n"
          "    }\n"
          "\n"
          "    Clock operator++(int)   // postfix\n"
          "    {\n"
          "        Clock c = *this;\n"
          "        tick();\n"
          "        return c;\n"
          "    }\n"
          "\n"
          "private:\n"
          "    int hour, min;\n"
          "};\n"
          "\n"
          "ostream& operator<<(ostream& os, const Clock& c)\n"
          "{\n"
          "    if (c.hour < 10) os << '0';\n"
          "    os << c.hour << \":\";\n"
          "    if (c.min < 10) os << '0';\n"
          "    os << c.min;\n"
          "    return os;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    int h, m, n;\n"
          "    cin >> h >> m >> n;\n"
          "\n"
          "    Clock def;\n"
          "    Clock c(h, m);\n"
          "    cout << \"Default: \" << def << \"\\n\";\n"
          "    cout << \"Start: \" << c << \"\\n\";\n"
          "    for (int i = 0; i < n; i++) {\n"
          "        c.tick();\n"
          "    }\n"
          "    cout << \"After \" << n << \" ticks: \" << c << \"\\n\";\n"
          "\n"
          "    Clock a = c++;\n"
          "    cout << \"Postfix: got \" << a << \", clock now \" << c << \"\\n\";\n"
          "    Clock b = ++c;\n"
          "    cout << \"Prefix: got \" << b << \", clock now \" << c << \"\\n\";\n"
          "    return 0;\n"
          "}\n",
          2, {
              { "10 58 1\n", "Default: 12:00\nStart: 10:58\nAfter 1 ticks: 10:59\nPostfix: got 10:59, clock now 11:00\nPrefix: got 11:01, clock now 11:01\n" },
              { "23 59 0\n", "Default: 12:00\nStart: 23:59\nAfter 0 ticks: 23:59\nPostfix: got 23:59, clock now 00:00\nPrefix: got 00:01, clock now 00:01\n" },
          } },
        { "Adding and comparing",
          "Go beyond the lecture with three more operators, all marked const because they don't change the clock:\n\n"
          "c + k returns a new Clock k minutes later (c itself doesn't change).\n"
          "c == other is true when both show the same time.\n"
          "c < other is true when c's time is earlier in the day.\n\n"
          "After the ++ lines, read k and then another time h2 m2, and print:\n"
          "In 90 minutes: 12:31\n"
          "Same as 11:01: yes\n"
          "Earlier than 11:01: no",
          "Clock operator+(int minutes) const { Clock result = *this; for (...) result.tick(); return result; }. "
          "For <, compare hour * 60 + min on both sides. Print other with your operator<< so it gets its leading zeros.",
          "#include <iostream>\n"
          "using namespace std;\n"
          "\n"
          "class Clock {\n"
          "    friend ostream& operator<<(ostream& os, const Clock& c);\n"
          "\n"
          "public:\n"
          "    Clock(int h = 12, int m = 0) : hour(h), min(m) { }\n"
          "\n"
          "    void tick()\n"
          "    {\n"
          "        min = (min + 1) % 60;\n"
          "        if (min == 0) {\n"
          "            hour = (hour + 1) % 24;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    Clock operator++()      // prefix\n"
          "    {\n"
          "        tick();\n"
          "        return *this;\n"
          "    }\n"
          "\n"
          "    Clock operator++(int)   // postfix\n"
          "    {\n"
          "        Clock c = *this;\n"
          "        tick();\n"
          "        return c;\n"
          "    }\n"
          "\n"
          "    Clock operator+(int minutes) const\n"
          "    {\n"
          "        Clock result = *this;\n"
          "        for (int i = 0; i < minutes; i++) {\n"
          "            result.tick();\n"
          "        }\n"
          "        return result;\n"
          "    }\n"
          "\n"
          "    bool operator==(const Clock& other) const\n"
          "    {\n"
          "        return hour == other.hour && min == other.min;\n"
          "    }\n"
          "\n"
          "    bool operator<(const Clock& other) const\n"
          "    {\n"
          "        return hour * 60 + min < other.hour * 60 + other.min;\n"
          "    }\n"
          "\n"
          "private:\n"
          "    int hour, min;\n"
          "};\n"
          "\n"
          "ostream& operator<<(ostream& os, const Clock& c)\n"
          "{\n"
          "    if (c.hour < 10) os << '0';\n"
          "    os << c.hour << \":\";\n"
          "    if (c.min < 10) os << '0';\n"
          "    os << c.min;\n"
          "    return os;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    int h, m, n;\n"
          "    cin >> h >> m >> n;\n"
          "\n"
          "    Clock def;\n"
          "    Clock c(h, m);\n"
          "    cout << \"Default: \" << def << \"\\n\";\n"
          "    cout << \"Start: \" << c << \"\\n\";\n"
          "    for (int i = 0; i < n; i++) {\n"
          "        c.tick();\n"
          "    }\n"
          "    cout << \"After \" << n << \" ticks: \" << c << \"\\n\";\n"
          "\n"
          "    Clock a = c++;\n"
          "    cout << \"Postfix: got \" << a << \", clock now \" << c << \"\\n\";\n"
          "    Clock b = ++c;\n"
          "    cout << \"Prefix: got \" << b << \", clock now \" << c << \"\\n\";\n"
          "\n"
          "    int k, h2, m2;\n"
          "    cin >> k >> h2 >> m2;\n"
          "    Clock other(h2, m2);\n"
          "    cout << \"In \" << k << \" minutes: \" << c + k << \"\\n\";\n"
          "    cout << \"Same as \" << other << \": \" << (c == other ? \"yes\" : \"no\") << \"\\n\";\n"
          "    cout << \"Earlier than \" << other << \": \" << (c < other ? \"yes\" : \"no\") << \"\\n\";\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "10 58 1\n90\n11 1\n",
                "Default: 12:00\nStart: 10:58\nAfter 1 ticks: 10:59\nPostfix: got 10:59, clock now 11:00\nPrefix: got 11:01, clock now 11:01\n"
                "In 90 minutes: 12:31\nSame as 11:01: yes\nEarlier than 11:01: no\n" },
              { "23 59 2\n5\n0 30\n",
                "Default: 12:00\nStart: 23:59\nAfter 2 ticks: 00:01\nPostfix: got 00:01, clock now 00:02\nPrefix: got 00:03, clock now 00:03\n"
                "In 5 minutes: 00:08\nSame as 00:30: no\nEarlier than 00:30: yes\n" },
              { "3 20 0\n0\n2 0\n",
                "Default: 12:00\nStart: 03:20\nAfter 0 ticks: 03:20\nPostfix: got 03:20, clock now 03:21\nPrefix: got 03:22, clock now 03:22\n"
                "In 0 minutes: 03:22\nSame as 02:00: no\nEarlier than 02:00: no\n" },
          } },
    },
},

// ===================================================================== 11
{
    .id = "proj-stack", .lang = LANG_CPP,
    .title   = "Generic Stack",
    .summary = "Build a stack for ints, turn it into a class template, then drive it with a function template.",
    .intro   =
        "Templates let you write one piece of code that works for many types. In this project you write a stack (last in, first out), "
        "first just for ints, then as a class template that works for ints, words and decimals alike - and finally a function template so the "
        "code that reads commands is written only once.\n\n"
        "The stack keeps its items in a std::vector, so you'll also practise push_back, back, pop_back and size.",
    .needs = "Templates, Strings, Vectors and Iterators",
    .lecture = "Templates and STL",
    .starterCode =
        "#include <iostream>\n"
        "#include <string>\n"
        "#include <vector>\n"
        "using namespace std;\n"
        "\n"
        "// Step 1: write an IntStack class that keeps its items in a vector<int>\n"
        "\n"
        "int main()\n"
        "{\n"
        "    string command;\n"
        "    while (cin >> command) {\n"
        "        // push X, pop, top and size\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 3,
    .steps = {
        { "A stack of ints",
          "Write a class IntStack with a private vector<int> items and the methods push(int), pop(), top(), empty() and size().\n\n"
          "Then read commands until the input runs out:\n"
          "push 5 - put 5 on top\n"
          "pop - print  Popped 5  and remove the top item\n"
          "top - print  Top: 5\n"
          "size - print  Size: 1\n\n"
          "If pop or top is used on an empty stack, print  Stack is empty  instead.",
          "The top of the stack is the end of the vector: push is items.push_back(x), top is items.back() and pop is items.pop_back(). "
          "Always check empty() before calling top() or pop() - calling back() on an empty vector is undefined behaviour.",
          "#include <iostream>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "using namespace std;\n"
          "\n"
          "class IntStack {\n"
          "private:\n"
          "    vector<int> items;\n"
          "public:\n"
          "    void push(int item) { items.push_back(item); }\n"
          "    void pop() { items.pop_back(); }\n"
          "    int top() const { return items.back(); }\n"
          "    bool empty() const { return items.empty(); }\n"
          "    int size() const { return (int)items.size(); }\n"
          "};\n"
          "\n"
          "int main()\n"
          "{\n"
          "    IntStack stack;\n"
          "    string command;\n"
          "    while (cin >> command) {\n"
          "        if (command == \"push\") {\n"
          "            int item;\n"
          "            cin >> item;\n"
          "            stack.push(item);\n"
          "        } else if (command == \"pop\") {\n"
          "            if (stack.empty()) {\n"
          "                cout << \"Stack is empty\\n\";\n"
          "            } else {\n"
          "                cout << \"Popped \" << stack.top() << \"\\n\";\n"
          "                stack.pop();\n"
          "            }\n"
          "        } else if (command == \"top\") {\n"
          "            if (stack.empty()) {\n"
          "                cout << \"Stack is empty\\n\";\n"
          "            } else {\n"
          "                cout << \"Top: \" << stack.top() << \"\\n\";\n"
          "            }\n"
          "        } else if (command == \"size\") {\n"
          "            cout << \"Size: \" << stack.size() << \"\\n\";\n"
          "        }\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "push 5\npush 7\ntop\npop\nsize\npop\npop\n", "Top: 7\nPopped 7\nSize: 1\nPopped 5\nStack is empty\n" },
              { "top\nsize\n",                                 "Stack is empty\nSize: 0\n" },
              { "push 1\npush 2\npush 3\npop\npop\ntop\n",     "Popped 3\nPopped 2\nTop: 1\n" },
          } },
        { "Make it a template",
          "Turn IntStack into template <typename T> class Stack, so it can hold any type.\n\n"
          "The input now starts with the type to use: int or string. Make a Stack<int> or a Stack<string> and run the same commands on it. "
          "For any other type, print  Unknown type: char  and stop.\n\n"
          "For now it's fine to have the command loop twice - once for each type. The next step fixes that.",
          "Replace int with T inside the class: vector<T> items; void push(const T& item); T top() const;. "
          "In main: string type; cin >> type; if (type == \"int\") { Stack<int> stack; ...loop... } else if (type == \"string\") { Stack<string> stack; ...loop... }",
          "#include <iostream>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "using namespace std;\n"
          "\n"
          "template <typename T>\n"
          "class Stack {\n"
          "private:\n"
          "    vector<T> items;\n"
          "public:\n"
          "    void push(const T& item) { items.push_back(item); }\n"
          "    void pop() { items.pop_back(); }\n"
          "    T top() const { return items.back(); }\n"
          "    bool empty() const { return items.empty(); }\n"
          "    int size() const { return (int)items.size(); }\n"
          "};\n"
          "\n"
          "int main()\n"
          "{\n"
          "    string type, command;\n"
          "    cin >> type;\n"
          "\n"
          "    if (type == \"int\") {\n"
          "        Stack<int> stack;\n"
          "        while (cin >> command) {\n"
          "            if (command == \"push\") {\n"
          "                int item;\n"
          "                cin >> item;\n"
          "                stack.push(item);\n"
          "            } else if (command == \"pop\") {\n"
          "                if (stack.empty()) cout << \"Stack is empty\\n\";\n"
          "                else { cout << \"Popped \" << stack.top() << \"\\n\"; stack.pop(); }\n"
          "            } else if (command == \"top\") {\n"
          "                if (stack.empty()) cout << \"Stack is empty\\n\";\n"
          "                else cout << \"Top: \" << stack.top() << \"\\n\";\n"
          "            } else if (command == \"size\") {\n"
          "                cout << \"Size: \" << stack.size() << \"\\n\";\n"
          "            }\n"
          "        }\n"
          "    } else if (type == \"string\") {\n"
          "        Stack<string> stack;\n"
          "        while (cin >> command) {\n"
          "            if (command == \"push\") {\n"
          "                string item;\n"
          "                cin >> item;\n"
          "                stack.push(item);\n"
          "            } else if (command == \"pop\") {\n"
          "                if (stack.empty()) cout << \"Stack is empty\\n\";\n"
          "                else { cout << \"Popped \" << stack.top() << \"\\n\"; stack.pop(); }\n"
          "            } else if (command == \"top\") {\n"
          "                if (stack.empty()) cout << \"Stack is empty\\n\";\n"
          "                else cout << \"Top: \" << stack.top() << \"\\n\";\n"
          "            } else if (command == \"size\") {\n"
          "                cout << \"Size: \" << stack.size() << \"\\n\";\n"
          "            }\n"
          "        }\n"
          "    } else {\n"
          "        cout << \"Unknown type: \" << type << \"\\n\";\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "int\npush 5\ntop\npop\npop\n",                      "Top: 5\nPopped 5\nStack is empty\n" },
              { "string\npush apple\npush pear\npop\nsize\ntop\n",   "Popped pear\nSize: 1\nTop: apple\n" },
              { "char\npush x\n",                                    "Unknown type: char\n" },
          } },
        { "A function template",
          "Remove the duplicated loop by moving it into a function template: template <typename T> void runCommands(Stack<T> &stack). "
          "main then just makes the right kind of stack and calls runCommands(stack).\n\n"
          "With the duplication gone, adding types is easy: support double as well. And add a print method and command that shows the whole stack, top first:\n\n"
          "Stack (top first): 3 2 1\n\n"
          "(or  Stack (top first): empty  when there's nothing in it).",
          "Inside runCommands, read items with T item; cin >> item; - it works for every T that cin can read. "
          "In print, loop from items.size() - 1 down to 0 (use an int, not size_t, so the loop can stop below 0).",
          "#include <iostream>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "using namespace std;\n"
          "\n"
          "template <typename T>\n"
          "class Stack {\n"
          "private:\n"
          "    vector<T> items;\n"
          "public:\n"
          "    void push(const T& item) { items.push_back(item); }\n"
          "    void pop() { items.pop_back(); }\n"
          "    T top() const { return items.back(); }\n"
          "    bool empty() const { return items.empty(); }\n"
          "    int size() const { return (int)items.size(); }\n"
          "\n"
          "    void print() const\n"
          "    {\n"
          "        cout << \"Stack (top first):\";\n"
          "        if (items.empty()) {\n"
          "            cout << \" empty\";\n"
          "        }\n"
          "        for (int i = (int)items.size() - 1; i >= 0; i--) {\n"
          "            cout << \" \" << items[i];\n"
          "        }\n"
          "        cout << \"\\n\";\n"
          "    }\n"
          "};\n"
          "\n"
          "template <typename T>\n"
          "void runCommands(Stack<T> &stack)\n"
          "{\n"
          "    string command;\n"
          "    while (cin >> command) {\n"
          "        if (command == \"push\") {\n"
          "            T item;\n"
          "            cin >> item;\n"
          "            stack.push(item);\n"
          "        } else if (command == \"pop\") {\n"
          "            if (stack.empty()) cout << \"Stack is empty\\n\";\n"
          "            else { cout << \"Popped \" << stack.top() << \"\\n\"; stack.pop(); }\n"
          "        } else if (command == \"top\") {\n"
          "            if (stack.empty()) cout << \"Stack is empty\\n\";\n"
          "            else cout << \"Top: \" << stack.top() << \"\\n\";\n"
          "        } else if (command == \"size\") {\n"
          "            cout << \"Size: \" << stack.size() << \"\\n\";\n"
          "        } else if (command == \"print\") {\n"
          "            stack.print();\n"
          "        }\n"
          "    }\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    string type;\n"
          "    cin >> type;\n"
          "\n"
          "    if (type == \"int\") {\n"
          "        Stack<int> stack;\n"
          "        runCommands(stack);\n"
          "    } else if (type == \"double\") {\n"
          "        Stack<double> stack;\n"
          "        runCommands(stack);\n"
          "    } else if (type == \"string\") {\n"
          "        Stack<string> stack;\n"
          "        runCommands(stack);\n"
          "    } else {\n"
          "        cout << \"Unknown type: \" << type << \"\\n\";\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "int\npush 1\npush 2\npush 3\nprint\npop\ntop\nsize\n", "Stack (top first): 3 2 1\nPopped 3\nTop: 2\nSize: 2\n" },
              { "double\npush 1.5\npush 2.25\npop\nprint\n",            "Popped 2.25\nStack (top first): 1.5\n" },
              { "string\npop\npush hi\nprint\n",                        "Stack is empty\nStack (top first): hi\n" },
              { "string\nprint\n",                                      "Stack (top first): empty\n" },
          } },
    },
},

// ===================================================================== 12
{
    .id = "proj-todo", .lang = LANG_CPP,
    .title   = "To-Do List",
    .summary = "A command-driven to-do list in C++ using strings and a vector that grows as you add tasks.",
    .intro   =
        "A to-do list is the classic first \"app\". The user types commands such as add, list and done, and the "
        "program keeps the tasks in a vector<string> - a list that grows and shrinks for you, with no fixed size.\n\n"
        "This is how many real command-line tools work: read a command word, then decide what to do with the rest of the line.",
    .needs = "Hello, C++; Strings, Vectors and Iterators",
    .lecture = "Templates and STL",
    .starterCode =
        "#include <iostream>\n"
        "#include <string>\n"
        "#include <vector>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    vector<string> tasks;\n"
        "\n"
        "    // Step 1: read commands - add puts a task on the list, list prints them\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 3,
    .steps = {
        { "Add and list",
          "Read commands until the input runs out. Support two of them:\n\n"
          "add Buy milk   - put \"Buy milk\" on the list and print  Added: Buy milk\n"
          "list   - print every task numbered from 1, like  1. Buy milk\n\n"
          "If the list is empty, list prints  Nothing to do!",
          "Loop with while (cin >> command). For add, the task is the rest of the line: cin >> ws; skips the space, then getline(cin, text); reads it. "
          "tasks.push_back(text); adds it. For list, loop for (size_t i = 0; i < tasks.size(); i++) and print i + 1.",
          "#include <iostream>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "using namespace std;\n"
          "\n"
          "int main()\n"
          "{\n"
          "    vector<string> tasks;\n"
          "    string command;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"add\") {\n"
          "            string text;\n"
          "            cin >> ws;\n"
          "            getline(cin, text);\n"
          "            tasks.push_back(text);\n"
          "            cout << \"Added: \" << text << \"\\n\";\n"
          "        } else if (command == \"list\") {\n"
          "            if (tasks.empty()) {\n"
          "                cout << \"Nothing to do!\\n\";\n"
          "            }\n"
          "            for (size_t i = 0; i < tasks.size(); i++) {\n"
          "                cout << i + 1 << \". \" << tasks[i] << \"\\n\";\n"
          "            }\n"
          "        }\n"
          "    }\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "add Buy milk\nadd Walk the dog\nlist\n",
                "Added: Buy milk\nAdded: Walk the dog\n1. Buy milk\n2. Walk the dog\n" },
              { "list\n",
                "Nothing to do!\n" },
              { "add Homework\nlist\nadd Tidy room\nlist\n",
                "Added: Homework\n1. Homework\nAdded: Tidy room\n1. Homework\n2. Tidy room\n" },
          } },
        { "Tick tasks off",
          "Add a done command with a task number, like  done 2. It removes that task from the list and prints\n\n"
          "Done: Walk the dog\n\n"
          "The tasks after it move up, so the old number 3 becomes number 2.\n"
          "If there is no task with that number, print  No task 5",
          "Read the number with int n; cin >> n;. Check if (n < 1 || n > (int)tasks.size()) first. "
          "Otherwise print tasks[n - 1] and remove it with tasks.erase(tasks.begin() + (n - 1));",
          "#include <iostream>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "using namespace std;\n"
          "\n"
          "int main()\n"
          "{\n"
          "    vector<string> tasks;\n"
          "    string command;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"add\") {\n"
          "            string text;\n"
          "            cin >> ws;\n"
          "            getline(cin, text);\n"
          "            tasks.push_back(text);\n"
          "            cout << \"Added: \" << text << \"\\n\";\n"
          "        } else if (command == \"list\") {\n"
          "            if (tasks.empty()) {\n"
          "                cout << \"Nothing to do!\\n\";\n"
          "            }\n"
          "            for (size_t i = 0; i < tasks.size(); i++) {\n"
          "                cout << i + 1 << \". \" << tasks[i] << \"\\n\";\n"
          "            }\n"
          "        } else if (command == \"done\") {\n"
          "            int n;\n"
          "            cin >> n;\n"
          "            if (n < 1 || n > (int)tasks.size()) {\n"
          "                cout << \"No task \" << n << \"\\n\";\n"
          "            } else {\n"
          "                cout << \"Done: \" << tasks[n - 1] << \"\\n\";\n"
          "                tasks.erase(tasks.begin() + (n - 1));\n"
          "            }\n"
          "        }\n"
          "    }\n"
          "\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "add A\nadd B\nadd C\ndone 2\nlist\n", "Added: A\nAdded: B\nAdded: C\nDone: B\n1. A\n2. C\n" },
              { "add Only\ndone 5\n",                  "Added: Only\nNo task 5\n" },
              { "add X\ndone 1\nlist\ndone 1\n",       "Added: X\nDone: X\nNothing to do!\nNo task 1\n" },
          } },
        { "Quit and summary",
          "Finish the app:\n\n"
          "quit   - stop reading commands straight away.\n"
          "Any other word - print  Unknown command: jump  and ignore the rest of that line.\n\n"
          "When the program ends (after quit, or when the input runs out), print how many tasks are left:\n\n"
          "Tasks left: 1",
          "For quit, use break;. For unknown commands, add a final else that prints the message and then reads the rest of the line "
          "into a throwaway string with getline(cin, rest);. Print tasks.size() after the loop.",
          "#include <iostream>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "using namespace std;\n"
          "\n"
          "int main()\n"
          "{\n"
          "    vector<string> tasks;\n"
          "    string command;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"add\") {\n"
          "            string text;\n"
          "            cin >> ws;\n"
          "            getline(cin, text);\n"
          "            tasks.push_back(text);\n"
          "            cout << \"Added: \" << text << \"\\n\";\n"
          "        } else if (command == \"list\") {\n"
          "            if (tasks.empty()) {\n"
          "                cout << \"Nothing to do!\\n\";\n"
          "            }\n"
          "            for (size_t i = 0; i < tasks.size(); i++) {\n"
          "                cout << i + 1 << \". \" << tasks[i] << \"\\n\";\n"
          "            }\n"
          "        } else if (command == \"done\") {\n"
          "            int n;\n"
          "            cin >> n;\n"
          "            if (n < 1 || n > (int)tasks.size()) {\n"
          "                cout << \"No task \" << n << \"\\n\";\n"
          "            } else {\n"
          "                cout << \"Done: \" << tasks[n - 1] << \"\\n\";\n"
          "                tasks.erase(tasks.begin() + (n - 1));\n"
          "            }\n"
          "        } else if (command == \"quit\") {\n"
          "            break;\n"
          "        } else {\n"
          "            cout << \"Unknown command: \" << command << \"\\n\";\n"
          "            string rest;\n"
          "            getline(cin, rest);\n"
          "        }\n"
          "    }\n"
          "\n"
          "    cout << \"Tasks left: \" << tasks.size() << \"\\n\";\n"
          "    return 0;\n"
          "}\n",
          4, {
              { "add Read\nquit\nadd Ignored\n",        "Added: Read\nTasks left: 1\n" },
              { "jump around\nlist\n",                  "Unknown command: jump\nNothing to do!\nTasks left: 0\n" },
              { "add Homework\nadd Dishes\ndone 1\n",   "Added: Homework\nAdded: Dishes\nDone: Homework\nTasks left: 1\n" },
              { "",                                     "Tasks left: 0\n" },
          } },
    },
},

// ===================================================================== 13
{
    .id = "proj-rankings", .lang = LANG_CPP,
    .title   = "Student Rankings",
    .summary = "A vector of student records, ranked with sort and a comparator and cleaned up with erase-remove_if.",
    .intro   =
        "The Templates and STL lecture shows that vectors can hold objects, that sort takes a comparison function, and that the "
        "erase-remove idiom with a predicate is the way to delete matching items. This project uses all three on a class list.\n\n"
        "You'll store student records in a vector, rank them by score, remove the students who were absent and finish with some statistics.",
    .needs = "Strings, Vectors and Iterators, STL Algorithms",
    .lecture = "Templates and STL",
    .starterCode =
        "#include <iostream>\n"
        "#include <string>\n"
        "#include <vector>\n"
        "using namespace std;\n"
        "\n"
        "// Step 1: a struct Student with a name and a score\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int n;\n"
        "    cin >> n;\n"
        "\n"
        "    // Read n students into a vector<Student> and print them\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Store the students",
          "Write a struct Student with a string name and an int score. Read how many students there are, then each student's name and score, "
          "and push_back each one into a vector<Student>.\n\n"
          "Print them in the order they were entered, numbered from 1, using an iterator loop:\n\n"
          "1. Ann 72\n"
          "2. Ben 91",
          "Read with Student s; cin >> s.name >> s.score; students.push_back(s);. "
          "Loop with for (vector<Student>::iterator it = students.begin(); it != students.end(); ++it) and use it->name and it->score.",
          "#include <iostream>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "using namespace std;\n"
          "\n"
          "struct Student {\n"
          "    string name;\n"
          "    int score;\n"
          "};\n"
          "\n"
          "int main()\n"
          "{\n"
          "    int n;\n"
          "    cin >> n;\n"
          "    vector<Student> students;\n"
          "    for (int i = 0; i < n; i++) {\n"
          "        Student s;\n"
          "        cin >> s.name >> s.score;\n"
          "        students.push_back(s);\n"
          "    }\n"
          "\n"
          "    int rank = 1;\n"
          "    for (vector<Student>::iterator it = students.begin(); it != students.end(); ++it) {\n"
          "        cout << rank << \". \" << it->name << \" \" << it->score << \"\\n\";\n"
          "        rank++;\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          2, {
              { "3\nAnn 72\nBen 91\nCal 35\n", "1. Ann 72\n2. Ben 91\n3. Cal 35\n" },
              { "1\nZoe 50\n",                 "1. Zoe 50\n" },
          } },
        { "Rank them",
          "Before printing, sort the students with sort and a comparator function byRank: the highest score comes first, "
          "and students with the same score are in alphabetical order of name.\n\n"
          "1. Ben 91\n"
          "2. Ann 72\n"
          "3. Cal 35",
          "bool byRank(const Student &a, const Student &b) { if (a.score != b.score) return a.score > b.score; return a.name < b.name; } "
          "then sort(students.begin(), students.end(), byRank); - #include <algorithm>.",
          "#include <iostream>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "#include <algorithm>\n"
          "using namespace std;\n"
          "\n"
          "struct Student {\n"
          "    string name;\n"
          "    int score;\n"
          "};\n"
          "\n"
          "bool byRank(const Student &a, const Student &b)\n"
          "{\n"
          "    if (a.score != b.score) {\n"
          "        return a.score > b.score;\n"
          "    }\n"
          "    return a.name < b.name;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    int n;\n"
          "    cin >> n;\n"
          "    vector<Student> students;\n"
          "    for (int i = 0; i < n; i++) {\n"
          "        Student s;\n"
          "        cin >> s.name >> s.score;\n"
          "        students.push_back(s);\n"
          "    }\n"
          "\n"
          "    sort(students.begin(), students.end(), byRank);\n"
          "\n"
          "    int rank = 1;\n"
          "    for (vector<Student>::iterator it = students.begin(); it != students.end(); ++it) {\n"
          "        cout << rank << \". \" << it->name << \" \" << it->score << \"\\n\";\n"
          "        rank++;\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "3\nAnn 72\nBen 91\nCal 35\n", "1. Ben 91\n2. Ann 72\n3. Cal 35\n" },
              { "3\nZed 80\nAmy 80\nBo 90\n",  "1. Bo 90\n2. Amy 80\n3. Zed 80\n" },
              { "1\nZoe 50\n",                 "1. Zoe 50\n" },
          } },
        { "Drop the absentees",
          "A score of -1 means the student was absent. Before sorting, remove every absent student with the erase-remove_if idiom and a predicate "
          "function isAbsent. Print how many were removed first:\n\n"
          "Absent: 1\n"
          "1. Ben 91\n"
          "...\n\n"
          "If nobody is left, print  No students  after the Absent line.",
          "size_t before = students.size(); students.erase(remove_if(students.begin(), students.end(), isAbsent), students.end());. "
          "The number removed is before - students.size().",
          "#include <iostream>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "#include <algorithm>\n"
          "using namespace std;\n"
          "\n"
          "struct Student {\n"
          "    string name;\n"
          "    int score;\n"
          "};\n"
          "\n"
          "bool byRank(const Student &a, const Student &b)\n"
          "{\n"
          "    if (a.score != b.score) {\n"
          "        return a.score > b.score;\n"
          "    }\n"
          "    return a.name < b.name;\n"
          "}\n"
          "\n"
          "bool isAbsent(const Student &s)\n"
          "{\n"
          "    return s.score == -1;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    int n;\n"
          "    cin >> n;\n"
          "    vector<Student> students;\n"
          "    for (int i = 0; i < n; i++) {\n"
          "        Student s;\n"
          "        cin >> s.name >> s.score;\n"
          "        students.push_back(s);\n"
          "    }\n"
          "\n"
          "    size_t before = students.size();\n"
          "    students.erase(remove_if(students.begin(), students.end(), isAbsent), students.end());\n"
          "    cout << \"Absent: \" << before - students.size() << \"\\n\";\n"
          "    if (students.empty()) {\n"
          "        cout << \"No students\\n\";\n"
          "        return 0;\n"
          "    }\n"
          "\n"
          "    sort(students.begin(), students.end(), byRank);\n"
          "\n"
          "    int rank = 1;\n"
          "    for (vector<Student>::iterator it = students.begin(); it != students.end(); ++it) {\n"
          "        cout << rank << \". \" << it->name << \" \" << it->score << \"\\n\";\n"
          "        rank++;\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "4\nAnn 72\nBen -1\nCal 35\nDee -1\n", "Absent: 2\n1. Ann 72\n2. Cal 35\n" },
              { "3\nAnn 72\nBen 91\nCal 35\n",          "Absent: 0\n1. Ben 91\n2. Ann 72\n3. Cal 35\n" },
              { "2\nX -1\nY -1\n",                      "Absent: 2\nNo students\n" },
          } },
        { "Statistics",
          "After the ranking, print three more lines:\n\n"
          "Average: 53.5   (one decimal place)\n"
          "Top: Ann   (the first student after sorting)\n"
          "Passed: 1 of 2   (a pass is 40 or more - count them with count_if and a predicate hasPassed)",
          "Add up the scores with a range-based for: for (const Student &s : students) total += s.score;. "
          "Print the average with cout << fixed << setprecision(1) (#include <iomanip>). students.front().name is the top student.",
          "#include <iostream>\n"
          "#include <iomanip>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "#include <algorithm>\n"
          "using namespace std;\n"
          "\n"
          "struct Student {\n"
          "    string name;\n"
          "    int score;\n"
          "};\n"
          "\n"
          "bool byRank(const Student &a, const Student &b)\n"
          "{\n"
          "    if (a.score != b.score) {\n"
          "        return a.score > b.score;\n"
          "    }\n"
          "    return a.name < b.name;\n"
          "}\n"
          "\n"
          "bool isAbsent(const Student &s)\n"
          "{\n"
          "    return s.score == -1;\n"
          "}\n"
          "\n"
          "bool hasPassed(const Student &s)\n"
          "{\n"
          "    return s.score >= 40;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    int n;\n"
          "    cin >> n;\n"
          "    vector<Student> students;\n"
          "    for (int i = 0; i < n; i++) {\n"
          "        Student s;\n"
          "        cin >> s.name >> s.score;\n"
          "        students.push_back(s);\n"
          "    }\n"
          "\n"
          "    size_t before = students.size();\n"
          "    students.erase(remove_if(students.begin(), students.end(), isAbsent), students.end());\n"
          "    cout << \"Absent: \" << before - students.size() << \"\\n\";\n"
          "    if (students.empty()) {\n"
          "        cout << \"No students\\n\";\n"
          "        return 0;\n"
          "    }\n"
          "\n"
          "    sort(students.begin(), students.end(), byRank);\n"
          "\n"
          "    int rank = 1;\n"
          "    for (vector<Student>::iterator it = students.begin(); it != students.end(); ++it) {\n"
          "        cout << rank << \". \" << it->name << \" \" << it->score << \"\\n\";\n"
          "        rank++;\n"
          "    }\n"
          "\n"
          "    int total = 0;\n"
          "    for (const Student &s : students) {\n"
          "        total += s.score;\n"
          "    }\n"
          "    cout << fixed << setprecision(1) << \"Average: \" << (double)total / students.size() << \"\\n\";\n"
          "    cout << \"Top: \" << students.front().name << \"\\n\";\n"
          "    cout << \"Passed: \" << count_if(students.begin(), students.end(), hasPassed)\n"
          "         << \" of \" << students.size() << \"\\n\";\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "4\nAnn 72\nBen -1\nCal 35\nDee -1\n", "Absent: 2\n1. Ann 72\n2. Cal 35\nAverage: 53.5\nTop: Ann\nPassed: 1 of 2\n" },
              { "3\nZed 80\nAmy 80\nBo 90\n",          "Absent: 0\n1. Bo 90\n2. Amy 80\n3. Zed 80\nAverage: 83.3\nTop: Bo\nPassed: 3 of 3\n" },
              { "1\nX -1\n",                            "Absent: 1\nNo students\n" },
          } },
    },
},

// ===================================================================== 14
{
    .id = "proj-wordfreq", .lang = LANG_CPP,
    .title   = "Word Frequency Counter",
    .summary = "Count words with a std::map, clean up punctuation, find the most common word and answer look-ups.",
    .intro   =
        "The STL Maps lecture ends with an exercise that counts how often each digit appears. This project does the same for words in a piece of text - "
        "the kind of thing search engines and spell checkers do all the time.\n\n"
        "A map<string, int> does the heavy lifting: it keeps one count per word and keeps the words sorted for you. "
        "The last step looks words up safely, without falling into the [] trap.",
    .needs = "Strings, Vectors and Iterators, std::map",
    .lecture = "STL Lists and Maps",
    .starterCode =
        "#include <iostream>\n"
        "#include <map>\n"
        "#include <string>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    map<string, int> freq;\n"
        "    string word;\n"
        "\n"
        "    // Step 1: read every word, count it, then print the counts\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 4,
    .steps = {
        { "Count the words",
          "Read words until the input runs out and count each one in a map<string, int>. Then print every word and its count, one per line:\n\n"
          "cat: 1\n"
          "dog: 1\n"
          "the: 2\n\n"
          "The map keeps its keys sorted, so the words come out in alphabetical order without any extra work.",
          "while (cin >> word) { freq[word]++; } - a new key starts at 0, so ++ works for new words too. "
          "Print with for (map<string, int>::iterator it = freq.begin(); it != freq.end(); ++it) cout << it->first << \": \" << it->second << \"\\n\";",
          "#include <iostream>\n"
          "#include <map>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "int main()\n"
          "{\n"
          "    map<string, int> freq;\n"
          "    string word;\n"
          "\n"
          "    while (cin >> word) {\n"
          "        freq[word]++;\n"
          "    }\n"
          "\n"
          "    for (map<string, int>::iterator it = freq.begin(); it != freq.end(); ++it) {\n"
          "        cout << it->first << \": \" << it->second << \"\\n\";\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          2, {
              { "the cat the dog\n", "cat: 1\ndog: 1\nthe: 2\n" },
              { "b a b a b\n",       "a: 2\nb: 3\n" },
          } },
        { "Clean the words",
          "Right now The, the and the, are three different words. Write a function string clean(const string &word) that keeps only the letters, "
          "turned into lowercase. Count the cleaned word instead - and skip it if nothing is left (like 42 or a lone -).",
          "Loop over the characters with for (char ch : word). isalpha and tolower come from <cctype>; "
          "pass them (unsigned char)ch to be safe. Build the result with result += (char)tolower((unsigned char)ch);",
          "#include <iostream>\n"
          "#include <map>\n"
          "#include <string>\n"
          "#include <cctype>\n"
          "using namespace std;\n"
          "\n"
          "string clean(const string &word)\n"
          "{\n"
          "    string result;\n"
          "    for (char ch : word) {\n"
          "        if (isalpha((unsigned char)ch)) {\n"
          "            result += (char)tolower((unsigned char)ch);\n"
          "        }\n"
          "    }\n"
          "    return result;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    map<string, int> freq;\n"
          "    string word;\n"
          "\n"
          "    while (cin >> word) {\n"
          "        string w = clean(word);\n"
          "        if (!w.empty()) {\n"
          "            freq[w]++;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    for (map<string, int>::iterator it = freq.begin(); it != freq.end(); ++it) {\n"
          "        cout << it->first << \": \" << it->second << \"\\n\";\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "The cat, the DOG!\n",           "cat: 1\ndog: 1\nthe: 2\n" },
              { "Hello hello HELLO... 42 -\n",   "hello: 3\n" },
              { "the cat the dog\n",             "cat: 1\ndog: 1\nthe: 2\n" },
          } },
        { "Totals and the winner",
          "After the counts, print three summary lines:\n\n"
          "Total words: 4\n"
          "Unique words: 3\n"
          "Most common: the (2)\n\n"
          "If two words tie for most common, pick the one that comes first alphabetically. If there were no words at all, leave out the Most common line.",
          "Count the total as you read. freq.size() is the number of unique words. "
          "While printing the map, remember the best word: only replace it when a count is bigger (not equal) - because the map is in alphabetical order, ties keep the earlier word.",
          "#include <iostream>\n"
          "#include <map>\n"
          "#include <string>\n"
          "#include <cctype>\n"
          "using namespace std;\n"
          "\n"
          "string clean(const string &word)\n"
          "{\n"
          "    string result;\n"
          "    for (char ch : word) {\n"
          "        if (isalpha((unsigned char)ch)) {\n"
          "            result += (char)tolower((unsigned char)ch);\n"
          "        }\n"
          "    }\n"
          "    return result;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    map<string, int> freq;\n"
          "    string word;\n"
          "    int total = 0;\n"
          "\n"
          "    while (cin >> word) {\n"
          "        string w = clean(word);\n"
          "        if (!w.empty()) {\n"
          "            freq[w]++;\n"
          "            total++;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    string best;\n"
          "    int bestCount = 0;\n"
          "    for (map<string, int>::iterator it = freq.begin(); it != freq.end(); ++it) {\n"
          "        cout << it->first << \": \" << it->second << \"\\n\";\n"
          "        if (it->second > bestCount) {\n"
          "            best = it->first;\n"
          "            bestCount = it->second;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    cout << \"Total words: \" << total << \"\\n\";\n"
          "    cout << \"Unique words: \" << freq.size() << \"\\n\";\n"
          "    if (bestCount > 0) {\n"
          "        cout << \"Most common: \" << best << \" (\" << bestCount << \")\\n\";\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "the cat the dog\n",  "cat: 1\ndog: 1\nthe: 2\nTotal words: 4\nUnique words: 3\nMost common: the (2)\n" },
              { "b a b a\n",          "a: 2\nb: 2\nTotal words: 4\nUnique words: 2\nMost common: a (2)\n" },
              { "123 !!\n",           "Total words: 0\nUnique words: 0\n" },
          } },
        { "Look-ups",
          "Now the text can be followed by a # on its own, and every word after the # is a look-up instead of part of the text. "
          "Answer the look-ups first, before the counts, each on its own line:\n\n"
          "Look up cat: 1\n"
          "Look up zebra: not found\n\n"
          "Use find (or count) for the look-ups - not freq[word]. Reading a missing key with [] adds it to the map with a count of 0, "
          "which would put zebra: 0 into your counts!",
          "Store the cleaned look-up words in a vector<string> while reading (switch to look-up mode when the raw word is \"#\"). "
          "Then for each one: map<string, int>::iterator it = freq.find(q); if (it == freq.end()) ... else ... it->second.",
          "#include <iostream>\n"
          "#include <map>\n"
          "#include <string>\n"
          "#include <vector>\n"
          "#include <cctype>\n"
          "using namespace std;\n"
          "\n"
          "string clean(const string &word)\n"
          "{\n"
          "    string result;\n"
          "    for (char ch : word) {\n"
          "        if (isalpha((unsigned char)ch)) {\n"
          "            result += (char)tolower((unsigned char)ch);\n"
          "        }\n"
          "    }\n"
          "    return result;\n"
          "}\n"
          "\n"
          "int main()\n"
          "{\n"
          "    map<string, int> freq;\n"
          "    vector<string> queries;\n"
          "    bool asking = false;\n"
          "    string word;\n"
          "    int total = 0;\n"
          "\n"
          "    while (cin >> word) {\n"
          "        if (word == \"#\") {\n"
          "            asking = true;\n"
          "            continue;\n"
          "        }\n"
          "        string w = clean(word);\n"
          "        if (w.empty()) {\n"
          "            continue;\n"
          "        }\n"
          "        if (asking) {\n"
          "            queries.push_back(w);\n"
          "        } else {\n"
          "            freq[w]++;\n"
          "            total++;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    for (const string &q : queries) {\n"
          "        map<string, int>::iterator it = freq.find(q);\n"
          "        if (it == freq.end()) {\n"
          "            cout << \"Look up \" << q << \": not found\\n\";\n"
          "        } else {\n"
          "            cout << \"Look up \" << q << \": \" << it->second << \"\\n\";\n"
          "        }\n"
          "    }\n"
          "\n"
          "    string best;\n"
          "    int bestCount = 0;\n"
          "    for (map<string, int>::iterator it = freq.begin(); it != freq.end(); ++it) {\n"
          "        cout << it->first << \": \" << it->second << \"\\n\";\n"
          "        if (it->second > bestCount) {\n"
          "            best = it->first;\n"
          "            bestCount = it->second;\n"
          "        }\n"
          "    }\n"
          "\n"
          "    cout << \"Total words: \" << total << \"\\n\";\n"
          "    cout << \"Unique words: \" << freq.size() << \"\\n\";\n"
          "    if (bestCount > 0) {\n"
          "        cout << \"Most common: \" << best << \" (\" << bestCount << \")\\n\";\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "a b a # a zebra\n",        "Look up a: 2\nLook up zebra: not found\na: 2\nb: 1\nTotal words: 3\nUnique words: 2\nMost common: a (2)\n" },
              { "The cat. # CAT dog\n",     "Look up cat: 1\nLook up dog: not found\ncat: 1\nthe: 1\nTotal words: 2\nUnique words: 2\nMost common: cat (1)\n" },
              { "the cat the dog\n",        "cat: 1\ndog: 1\nthe: 2\nTotal words: 4\nUnique words: 3\nMost common: the (2)\n" },
          } },
    },
},

// ===================================================================== 15
{
    .id = "proj-urlshort", .lang = LANG_CPP,
    .title   = "URL Shortener",
    .summary = "The lecture's map example: short keys for long URLs, with duplicate checks, listing and deleting.",
    .intro   =
        "The STL Maps lecture shows a URL shortener: each long web address gets a short key, and a map<string, string> turns the key back into the address. "
        "Here you build a fuller version driven by commands.\n\n"
        "The lecture makes random keys with rand(). Tests can't predict random numbers, so this version numbers the keys instead: s1, s2, s3 and so on. "
        "You'll use [], find, count, erase and an iterator loop, and keep two maps in step with each other.",
    .needs = "std::map",
    .lecture = "STL Lists and Maps",
    .starterCode =
        "#include <iostream>\n"
        "#include <map>\n"
        "#include <string>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    map<string, string> urlMapping;   // short key -> long URL\n"
        "    int nextId = 1;\n"
        "    string command;\n"
        "\n"
        "    while (cin >> command) {\n"
        "        // shorten URL and open KEY\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .stepCount = 3,
    .steps = {
        { "Shorten and open",
          "Read commands until the input runs out:\n\n"
          "shorten https://a.com - make the key \"s\" followed by nextId (s1 for the first one), store the URL in urlMapping under that key, "
          "add 1 to nextId and print  s1 -> https://a.com\n"
          "open s1 - print  Original URL: https://a.com  or, if there's no such key,  Short URL not found.",
          "Build the key with string key = \"s\" + to_string(nextId);. For open, check urlMapping.count(key) (or use find) before reading the value - "
          "the lecture does exactly this.",
          "#include <iostream>\n"
          "#include <map>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "int main()\n"
          "{\n"
          "    map<string, string> urlMapping;   // short key -> long URL\n"
          "    int nextId = 1;\n"
          "    string command;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"shorten\") {\n"
          "            string longUrl;\n"
          "            cin >> longUrl;\n"
          "            string key = \"s\" + to_string(nextId);\n"
          "            nextId++;\n"
          "            urlMapping[key] = longUrl;\n"
          "            cout << key << \" -> \" << longUrl << \"\\n\";\n"
          "        } else if (command == \"open\") {\n"
          "            string key;\n"
          "            cin >> key;\n"
          "            if (urlMapping.count(key)) {\n"
          "                cout << \"Original URL: \" << urlMapping[key] << \"\\n\";\n"
          "            } else {\n"
          "                cout << \"Short URL not found.\\n\";\n"
          "            }\n"
          "        }\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          2, {
              { "shorten https://a.com\nshorten https://b.org/x\nopen s2\nopen s9\n",
                "s1 -> https://a.com\ns2 -> https://b.org/x\nOriginal URL: https://b.org/x\nShort URL not found.\n" },
              { "open s1\n", "Short URL not found.\n" },
          } },
        { "No duplicates",
          "Shortening the same URL twice shouldn't use up a new key. Keep a second map<string, string> longToShort going the other way, "
          "from URL to key. If the URL is already in it, print\n\n"
          "Already shortened: s1 -> https://a.com\n\n"
          "and don't make a new key.",
          "Check if (longToShort.count(longUrl) > 0) before making a key. When you do make one, store it in both maps: "
          "urlMapping[key] = longUrl; and longToShort[longUrl] = key;",
          "#include <iostream>\n"
          "#include <map>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "int main()\n"
          "{\n"
          "    map<string, string> urlMapping;    // short key -> long URL\n"
          "    map<string, string> longToShort;   // long URL -> short key\n"
          "    int nextId = 1;\n"
          "    string command;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"shorten\") {\n"
          "            string longUrl;\n"
          "            cin >> longUrl;\n"
          "            if (longToShort.count(longUrl) > 0) {\n"
          "                cout << \"Already shortened: \" << longToShort[longUrl] << \" -> \" << longUrl << \"\\n\";\n"
          "            } else {\n"
          "                string key = \"s\" + to_string(nextId);\n"
          "                nextId++;\n"
          "                urlMapping[key] = longUrl;\n"
          "                longToShort[longUrl] = key;\n"
          "                cout << key << \" -> \" << longUrl << \"\\n\";\n"
          "            }\n"
          "        } else if (command == \"open\") {\n"
          "            string key;\n"
          "            cin >> key;\n"
          "            if (urlMapping.count(key)) {\n"
          "                cout << \"Original URL: \" << urlMapping[key] << \"\\n\";\n"
          "            } else {\n"
          "                cout << \"Short URL not found.\\n\";\n"
          "            }\n"
          "        }\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          2, {
              { "shorten https://a.com\nshorten https://a.com\nshorten https://b.com\n",
                "s1 -> https://a.com\nAlready shortened: s1 -> https://a.com\ns2 -> https://b.com\n" },
              { "shorten x.ie\nopen s1\nshorten x.ie\nopen s2\n",
                "s1 -> x.ie\nOriginal URL: x.ie\nAlready shortened: s1 -> x.ie\nShort URL not found.\n" },
          } },
        { "List and delete",
          "Two more commands:\n\n"
          "list - print  URL Mappings:  and then every entry as key:url, like the lecture's loop\n"
          "delete s1 - remove the key from both maps and print  Deleted s1  (or  Short URL not found.)\n\n"
          "Notice the order list prints in once you have ten or more keys: s1, s10, s11, s2... The map sorts its string keys alphabetically, "
          "character by character - not by number.",
          "For delete, use map<string, string>::iterator it = urlMapping.find(key);. If it's found, erase it->second from longToShort first, "
          "then urlMapping.erase(it); - it is invalid after that erase.",
          "#include <iostream>\n"
          "#include <map>\n"
          "#include <string>\n"
          "using namespace std;\n"
          "\n"
          "int main()\n"
          "{\n"
          "    map<string, string> urlMapping;    // short key -> long URL\n"
          "    map<string, string> longToShort;   // long URL -> short key\n"
          "    int nextId = 1;\n"
          "    string command;\n"
          "\n"
          "    while (cin >> command) {\n"
          "        if (command == \"shorten\") {\n"
          "            string longUrl;\n"
          "            cin >> longUrl;\n"
          "            if (longToShort.count(longUrl) > 0) {\n"
          "                cout << \"Already shortened: \" << longToShort[longUrl] << \" -> \" << longUrl << \"\\n\";\n"
          "            } else {\n"
          "                string key = \"s\" + to_string(nextId);\n"
          "                nextId++;\n"
          "                urlMapping[key] = longUrl;\n"
          "                longToShort[longUrl] = key;\n"
          "                cout << key << \" -> \" << longUrl << \"\\n\";\n"
          "            }\n"
          "        } else if (command == \"open\") {\n"
          "            string key;\n"
          "            cin >> key;\n"
          "            if (urlMapping.count(key)) {\n"
          "                cout << \"Original URL: \" << urlMapping[key] << \"\\n\";\n"
          "            } else {\n"
          "                cout << \"Short URL not found.\\n\";\n"
          "            }\n"
          "        } else if (command == \"delete\") {\n"
          "            string key;\n"
          "            cin >> key;\n"
          "            map<string, string>::iterator it = urlMapping.find(key);\n"
          "            if (it == urlMapping.end()) {\n"
          "                cout << \"Short URL not found.\\n\";\n"
          "            } else {\n"
          "                longToShort.erase(it->second);\n"
          "                urlMapping.erase(it);\n"
          "                cout << \"Deleted \" << key << \"\\n\";\n"
          "            }\n"
          "        } else if (command == \"list\") {\n"
          "            cout << \"URL Mappings:\\n\";\n"
          "            for (map<string, string>::iterator it = urlMapping.begin(); it != urlMapping.end(); ++it) {\n"
          "                cout << it->first << ':' << it->second << \"\\n\";\n"
          "            }\n"
          "        }\n"
          "    }\n"
          "    return 0;\n"
          "}\n",
          3, {
              { "shorten https://a.com\ndelete s1\nopen s1\nshorten https://a.com\nlist\n",
                "s1 -> https://a.com\nDeleted s1\nShort URL not found.\ns2 -> https://a.com\nURL Mappings:\ns2:https://a.com\n" },
              { "delete s4\nlist\n", "Short URL not found.\nURL Mappings:\n" },
              { "shorten a\nshorten b\nshorten c\nshorten d\nshorten e\nshorten f\nshorten g\nshorten h\nshorten i\nshorten j\nshorten k\nlist\n",
                "s1 -> a\ns2 -> b\ns3 -> c\ns4 -> d\ns5 -> e\ns6 -> f\ns7 -> g\ns8 -> h\ns9 -> i\ns10 -> j\ns11 -> k\n"
                "URL Mappings:\ns1:a\ns10:j\ns11:k\ns2:b\ns3:c\ns4:d\ns5:e\ns6:f\ns7:g\ns8:h\ns9:i\n" },
          } },
    },
},

};

int ProjectCount(void) { return (int)(sizeof(PROJECTS) / sizeof(PROJECTS[0])); }

const Project *GetProject(int index)
{
    if (index < 0 || index >= ProjectCount()) return &PROJECTS[0];
    return &PROJECTS[index];
}
