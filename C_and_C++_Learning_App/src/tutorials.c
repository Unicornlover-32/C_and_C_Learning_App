#include "tutorials.h"
#include <stddef.h>

// ---------------------------------------------------------------------------
// Tutorial content. To add a tutorial, add a new entry to this array.
// Each project's tests feed `input` to the student's program and compare
// what it prints with `expected`.
// ---------------------------------------------------------------------------

const Tutorial TUTORIALS[] = {

// ===================================================================== 1
{
    .title   = "Your First Program",
    .summary = "What C is, how a program is laid out, and printing text with printf.",
    .pageCount = 4,
    .pages = {
        { "What is C?",
          "C is a programming language created in the early 1970s. It is fast and small, and it is still used "
          "everywhere: operating systems, game engines, microcontrollers, and even raylib, the library that draws this app.\n\n"
          "C++ was built on top of C, so almost everything you learn here also works in C++. We start with C and add C++ later.",
          NULL },
        { "The shape of a C program",
          "Every C program starts running at a function called main. The lines between the curly braces { and } "
          "are the instructions, and they run from top to bottom. The #include line at the top gives us access to printf.",
          "#include <stdio.h>   // gives us printf\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    printf(\"Hello!\\n\");\n"
          "    return 0;        // 0 means \"finished OK\"\n"
          "}" },
        { "Printing with printf",
          "printf prints text to the screen. The text goes inside double quotes. \\n is a special code that means "
          "\"start a new line\" - without it, the next thing you print carries on from the same line.\n\n"
          "Every statement in C ends with a semicolon ; - forgetting it is the most common beginner mistake, "
          "and the compiler will tell you about it.",
          "printf(\"Line one\\n\");\n"
          "printf(\"Line two\\n\");\n"
          "printf(\"Same \");\n"
          "printf(\"line\\n\");      // prints: Same line" },
        { "Compiling",
          "Your computer can't run C text directly. A compiler such as gcc turns your code into a program "
          "that can run. If the compiler finds a mistake, it prints an error with the line number - always read the first error first.\n\n"
          "When you press Run & Check in a project, this app compiles your code, runs it, and compares what it prints "
          "with what was expected.",
          "gcc hello.c -o hello\n"
          "./hello" },
    },
    .projectTitle = "Hello, World!",
    .projectBrief =
        "Write a program that prints exactly:\n\n"
        "Hello, World!\n\n"
        "That is a capital H, a comma, a space, a capital W and an exclamation mark, followed by a new line.",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // Print Hello, World! here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "", "Hello, World!\n" },
    },
},

// ===================================================================== 2
{
    .title   = "Variables and Types",
    .summary = "Storing numbers and characters, doing maths, and printing values.",
    .pageCount = 4,
    .pages = {
        { "What is a variable?",
          "A variable is a named box that holds a value. Before you use a variable in C you must declare it: "
          "say what type of value it holds, then give it a name. You can give it a starting value with =.",
          "int age = 21;          // a whole number\n"
          "float price = 4.99f;   // a decimal number\n"
          "char letter = 'A';     // a single character" },
        { "Common types",
          "int holds whole numbers such as -3, 0 or 42.\n"
          "float and double hold numbers with a decimal point. double is more precise.\n"
          "char holds one character, written in single quotes like 'x'.\n\n"
          "Names can use letters, digits and underscores, but can't start with a digit. "
          "C is case sensitive, so score and Score are two different variables.",
          NULL },
        { "Changing variables and maths",
          "After a variable is declared you can give it a new value with =. You can do maths with + - * / and %.\n\n"
          "Careful: dividing two ints gives an int, so 7 / 2 is 3, not 3.5. The % operator gives the remainder, so 7 % 2 is 1.",
          "int score = 10;\n"
          "score = score + 5;     // now 15\n"
          "score += 5;            // shortcut, now 20\n"
          "int half = 7 / 2;      // 3\n"
          "int left = 7 % 2;      // 1" },
        { "Printing variables",
          "printf uses format specifiers as placeholders. The values listed after the text fill them in, in order.\n\n"
          "%d prints an int, %f prints a float or double, %.2f prints it with 2 decimal places, and %c prints a char.",
          "int age = 21;\n"
          "float height = 1.8f;\n"
          "printf(\"Age: %d\\n\", age);\n"
          "printf(\"Height: %.2f\\n\", height);   // Height: 1.80" },
    },
    .projectTitle = "About Me",
    .projectBrief =
        "Create three variables:\n"
        "- an int called age set to 21\n"
        "- a float called height set to 1.80\n"
        "- a char called initial set to 'J'\n\n"
        "Then use printf with %d, %.2f and %c so the output is exactly:\n\n"
        "Age: 21\n"
        "Height: 1.80\n"
        "Initial: J",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    // Declare your variables here\n"
        "\n"
        "    // Print them here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "", "Age: 21\nHeight: 1.80\nInitial: J\n" },
    },
},

// ===================================================================== 3
{
    .title   = "Making Decisions",
    .summary = "Reading input with scanf and choosing what to do with if and else.",
    .pageCount = 4,
    .pages = {
        { "Reading input",
          "Programs get more interesting when they react to the user. scanf reads a value typed in by the user. "
          "Notice the & before the variable name: it tells scanf where to store the value.\n\n"
          "In the projects, the app types the input in for you, so you can test with lots of different values.",
          "int number;\n"
          "scanf(\"%d\", &number);\n"
          "printf(\"You typed %d\\n\", number);" },
        { "The if statement",
          "An if statement runs a block of code only when its condition is true. The condition goes in round brackets, "
          "and the code to run goes in curly braces.",
          "if (number > 10) {\n"
          "    printf(\"That's a big number\\n\");\n"
          "}" },
        { "Comparisons",
          "==  equal to (two equals signs - a single = changes a variable!)\n"
          "!=  not equal to\n"
          "<  less than,   >  greater than\n"
          "<=  less than or equal,   >=  greater than or equal\n\n"
          "Combine conditions with && (and), || (or) and ! (not).",
          "if (age >= 13 && age <= 19) {\n"
          "    printf(\"Teenager\\n\");\n"
          "}" },
        { "else and else if",
          "else runs when the if condition is false. else if lets you check several conditions in order. "
          "C checks them from top to bottom and runs only the first one that is true.",
          "if (temp > 25) {\n"
          "    printf(\"Hot\\n\");\n"
          "} else if (temp > 15) {\n"
          "    printf(\"Warm\\n\");\n"
          "} else {\n"
          "    printf(\"Cold\\n\");\n"
          "}" },
    },
    .projectTitle = "Grade Calculator",
    .projectBrief =
        "Read a score (a whole number from 0 to 100) with scanf, then print the matching grade:\n\n"
        "90 or more:  Grade: A\n"
        "80 to 89:  Grade: B\n"
        "70 to 79:  Grade: C\n"
        "60 to 69:  Grade: D\n"
        "below 60:  Grade: F\n\n"
        "Your program is tested with several different scores. Print only the grade line, for example: Grade: B",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int score;\n"
        "    scanf(\"%d\", &score);\n"
        "\n"
        "    // Print the grade here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 6,
    .tests = {
        { "95\n", "Grade: A\n" },
        { "83\n", "Grade: B\n" },
        { "70\n", "Grade: C\n" },
        { "61\n", "Grade: D\n" },
        { "12\n", "Grade: F\n" },
        { "90\n", "Grade: A\n" },
    },
},

// ===================================================================== 4
{
    .title   = "Loops",
    .summary = "Repeating code with for and while loops, and keeping a running total.",
    .pageCount = 4,
    .pages = {
        { "Why loops?",
          "A loop repeats code. Instead of writing printf 100 times, you write it once inside a loop "
          "and tell C how many times to repeat it. Loops are one of the most useful tools in programming.",
          NULL },
        { "The for loop",
          "A for loop has three parts in its brackets, separated by semicolons: a start (int i = 1), "
          "a condition checked before every round (i <= 5), and a step that runs after every round (i++ adds 1 to i).",
          "for (int i = 1; i <= 5; i++) {\n"
          "    printf(\"%d\\n\", i);\n"
          "}\n"
          "// prints 1 2 3 4 5, one per line" },
        { "The while loop",
          "A while loop keeps repeating as long as its condition is true. Use it when you don't know in advance "
          "how many rounds you need. Make sure something inside the loop changes, or it will run forever!",
          "int n = 3;\n"
          "while (n > 0) {\n"
          "    printf(\"%d...\\n\", n);\n"
          "    n--;               // take 1 away from n\n"
          "}\n"
          "printf(\"Liftoff!\\n\");" },
        { "Adding things up",
          "A very common pattern is a running total: start a variable at 0 and add to it each time round the loop.",
          "int total = 0;\n"
          "for (int i = 1; i <= 4; i++) {\n"
          "    total += i;\n"
          "}\n"
          "printf(\"Total: %d\\n\", total);   // Total: 10" },
    },
    .projectTitle = "Count and Sum",
    .projectBrief =
        "Read a whole number n with scanf. Print every number from 1 up to n, each on its own line. "
        "After the loop, print the sum of all those numbers.\n\n"
        "For an input of 4 the output should be:\n\n"
        "1\n2\n3\n4\nTotal: 10",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    // Write your loop here\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "5\n",  "1\n2\n3\n4\n5\nTotal: 15\n" },
        { "1\n",  "1\nTotal: 1\n" },
        { "10\n", "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\nTotal: 55\n" },
    },
},

// ===================================================================== 5
{
    .title   = "Functions",
    .summary = "Writing your own reusable functions with parameters and return values.",
    .pageCount = 4,
    .pages = {
        { "What is a function?",
          "A function is a named, reusable block of code. You've already used two: main and printf. "
          "Writing your own functions lets you split a big problem into small pieces and avoid repeating yourself.",
          NULL },
        { "Writing a function",
          "A function has a return type, a name, parameters in brackets, and a body in curly braces. "
          "return sends a value back to the code that called the function.",
          "int add(int a, int b)\n"
          "{\n"
          "    return a + b;\n"
          "}" },
        { "Calling a function",
          "To use a function, write its name followed by the values to pass in. "
          "The function must be written (or declared) above the place where you use it.",
          "int add(int a, int b)\n"
          "{\n"
          "    return a + b;\n"
          "}\n"
          "\n"
          "int main(void)\n"
          "{\n"
          "    int result = add(2, 3);\n"
          "    printf(\"%d\\n\", result);   // 5\n"
          "    return 0;\n"
          "}" },
        { "void and yes/no functions",
          "Some functions just do something and don't hand back a value. Their return type is void.\n\n"
          "Functions can also answer yes/no questions by returning an int: 1 means yes (true) and 0 means no (false).",
          "void greet(void)\n"
          "{\n"
          "    printf(\"Hi there!\\n\");\n"
          "}\n"
          "\n"
          "int is_positive(int x)\n"
          "{\n"
          "    return x > 0;   // 1 if true, 0 if false\n"
          "}" },
    },
    .projectTitle = "Square and Even",
    .projectBrief =
        "Write two functions:\n"
        "- int square(int x) returns x multiplied by itself\n"
        "- int is_even(int x) returns 1 if x is even and 0 if it is odd (hint: use %)\n\n"
        "main reads a number, then uses your functions to print its square and whether it is even.\n\n"
        "For an input of 4 the output should be:\n\n"
        "Square: 16\n"
        "Even: yes",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "// Write square() here\n"
        "\n"
        "// Write is_even() here\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    // Print:  Square: <n squared>\n"
        "    // Then:   Even: yes   (or Even: no)\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "4\n",  "Square: 16\nEven: yes\n" },
        { "7\n",  "Square: 49\nEven: no\n" },
        { "0\n",  "Square: 0\nEven: yes\n" },
        { "11\n", "Square: 121\nEven: no\n" },
    },
},

};

const int TUTORIAL_COUNT = (int)(sizeof(TUTORIALS) / sizeof(TUTORIALS[0]));
