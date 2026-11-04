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
    .needs = "Variables and Types, Making Decisions, Loops",
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
    .needs = "Variables and Types, Making Decisions, Loops",
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
    .needs = "Variables and Types, Making Decisions, Loops",
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
    .needs = "Loops, Functions, Arrays",
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
    .needs = "Loops, Functions, Arrays, Strings",
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
    .needs = "Loops, Functions, Arrays",
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
    .id = "proj-todo", .lang = LANG_CPP,
    .title   = "To-Do List",
    .summary = "A command-driven to-do list in C++ using strings and a vector that grows as you add tasks.",
    .intro   =
        "A to-do list is the classic first \"app\". The user types commands such as add, list and done, and the "
        "program keeps the tasks in a vector<string> - a list that grows and shrinks for you, with no fixed size.\n\n"
        "This is how many real command-line tools work: read a command word, then decide what to do with the rest of the line.",
    .needs = "Hello, C++ and Strings and Vectors (in the C++ section)",
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

};

int ProjectCount(void) { return (int)(sizeof(PROJECTS) / sizeof(PROJECTS[0])); }

const Project *GetProject(int index)
{
    if (index < 0 || index >= ProjectCount()) return &PROJECTS[0];
    return &PROJECTS[index];
}
