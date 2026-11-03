#include "tutorials.h"
#include <stddef.h>

// ---------------------------------------------------------------------------
// C section. To add a tutorial, add a new entry to this array.
// Each project's tests feed `input` to the student's program and compare
// what it prints with `expected`. `solution` must pass every test.
// ---------------------------------------------------------------------------

const Tutorial C_TUTORIALS[] = {

// ===================================================================== 1
{
    .id = "c-hello", .lang = LANG_C,
    .title   = "Your First Program",
    .summary = "What C is, how a program is laid out, and printing text with printf.",
    .pageCount = 4,
    .pages = {
        { "What is C?",
          "C is a programming language created in the early 1970s. It is fast and small, and it is still used "
          "everywhere: operating systems, game engines, microcontrollers, and even raylib, the library that draws this app.\n\n"
          "C++ was built on top of C, so almost everything you learn here also works in C++. We start with C and move to C++ later.",
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
    .quizCount = 3,
    .quiz = {
        { "Where does every C program start running?", NULL, 4,
          { "At the first line of the file", "In the function called main", "Inside printf", "At the #include line" }, 1,
          "C always starts at main, no matter where main sits in the file." },
        { "What does \\n do inside printf's text?", NULL, 4,
          { "Prints the letter n", "Ends the program", "Starts a new line", "Prints a backslash" }, 2,
          "\\n is the newline code. Without it, the next output continues on the same line." },
        { "What is missing from this line?", "printf(\"Hi\\n\")", 4,
          { "Curly braces", "A semicolon at the end", "An #include", "Nothing - it is fine" }, 1,
          "Every statement in C ends with a semicolon." },
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
    .hint = "Use printf with the text in double quotes and finish it with \\n, like printf(\"Hello, World!\\n\"); - don't forget the semicolon.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    printf(\"Hello, World!\\n\");\n"
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
    .id = "c-variables", .lang = LANG_C,
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
    .quizCount = 3,
    .quiz = {
        { "Which type would you use to store 3.75?", NULL, 4,
          { "int", "char", "float", "void" }, 2,
          "float (or double) holds numbers with a decimal point. An int would drop the .75." },
        { "What is the value of x?", "int x = 7 / 2;", 4,
          { "3.5", "3", "4", "1" }, 1,
          "Dividing two ints gives an int: the decimal part is simply dropped, so 7 / 2 is 3." },
        { "Which format specifier prints an int?", NULL, 4,
          { "%c", "%f", "%s", "%d" }, 3,
          "%d is for ints, %f for floats and doubles, %c for a single char." },
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
    .hint = "Declare each one as type name = value; for example int age = 21; Then print with printf(\"Age: %d\\n\", age); and do the same for the other two using %.2f and %c.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int age = 21;\n"
        "    float height = 1.80f;\n"
        "    char initial = 'J';\n"
        "\n"
        "    printf(\"Age: %d\\n\", age);\n"
        "    printf(\"Height: %.2f\\n\", height);\n"
        "    printf(\"Initial: %c\\n\", initial);\n"
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
    .id = "c-decisions", .lang = LANG_C,
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
    .quizCount = 3,
    .quiz = {
        { "Which operator checks whether two values are equal?", NULL, 4,
          { "=", "==", "!=", "=>" }, 1,
          "== compares. A single = assigns a new value, which is a classic bug inside an if." },
        { "What does this print when x is 5?",
          "if (x > 10) {\n    printf(\"big\\n\");\n} else {\n    printf(\"small\\n\");\n}", 4,
          { "big", "small", "big and small", "Nothing" }, 1,
          "5 > 10 is false, so the else block runs." },
        { "What does && mean in a condition?", NULL, 4,
          { "or", "not", "and", "equals" }, 2,
          "a && b is true only when both a and b are true. || means or, ! means not." },
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
    .hint = "Check from the top grade down: if (score >= 90) for A, then else if (score >= 80) for B, and so on, with a final else for F. Only the first true branch runs.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int score;\n"
        "    scanf(\"%d\", &score);\n"
        "\n"
        "    if (score >= 90) {\n"
        "        printf(\"Grade: A\\n\");\n"
        "    } else if (score >= 80) {\n"
        "        printf(\"Grade: B\\n\");\n"
        "    } else if (score >= 70) {\n"
        "        printf(\"Grade: C\\n\");\n"
        "    } else if (score >= 60) {\n"
        "        printf(\"Grade: D\\n\");\n"
        "    } else {\n"
        "        printf(\"Grade: F\\n\");\n"
        "    }\n"
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
    .id = "c-loops", .lang = LANG_C,
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
    .quizCount = 3,
    .quiz = {
        { "How many times does this loop run?", "for (int i = 0; i < 3; i++) {\n    printf(\"Hi\\n\");\n}", 4,
          { "2", "3", "4", "Forever" }, 1,
          "i takes the values 0, 1 and 2. When i becomes 3, i < 3 is false and the loop stops." },
        { "What does i++ do?", NULL, 4,
          { "Doubles i", "Sets i back to 0", "Adds 1 to i", "Prints i" }, 2,
          "i++ is short for i = i + 1. i-- takes 1 away." },
        { "When is a while loop a better fit than a for loop?", NULL, 4,
          { "When you don't know in advance how many rounds you need", "When you want the program to run faster",
            "When counting from 1 to 10", "Never - they are always worse" }, 0,
          "for suits a known count. while suits \"keep going until something happens\"." },
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
    .hint = "Use for (int i = 1; i <= n; i++) - note the <= so that n itself is included. Add i to a total inside the loop, and print the total after the loop ends.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    int total = 0;\n"
        "    for (int i = 1; i <= n; i++) {\n"
        "        printf(\"%d\\n\", i);\n"
        "        total += i;\n"
        "    }\n"
        "    printf(\"Total: %d\\n\", total);\n"
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
    .id = "c-functions", .lang = LANG_C,
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
    .quizCount = 3,
    .quiz = {
        { "What does return do inside a function?", NULL, 4,
          { "Restarts the function", "Sends a value back to the code that called it", "Prints the value", "Deletes the function" }, 1,
          "return ends the function and hands its value back to the caller." },
        { "What does add(2, 3) give?", "int add(int a, int b)\n{\n    return a + b;\n}", 4,
          { "23", "6", "5", "An error" }, 2,
          "a becomes 2 and b becomes 3, so the function returns 2 + 3." },
        { "What does the void in void greet(void) mean at the start?", NULL, 4,
          { "The function returns nothing", "The function is empty", "The function is broken", "The function returns 0" }, 0,
          "A void return type means the function does a job but hands no value back." },
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
    .hint = "square returns x * x. is_even returns x % 2 == 0 (the remainder after dividing by 2 is 0 for even numbers). In main, put the two Even printf lines in an if/else on is_even(n).",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int square(int x)\n"
        "{\n"
        "    return x * x;\n"
        "}\n"
        "\n"
        "int is_even(int x)\n"
        "{\n"
        "    return x % 2 == 0;\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    printf(\"Square: %d\\n\", square(n));\n"
        "    if (is_even(n)) {\n"
        "        printf(\"Even: yes\\n\");\n"
        "    } else {\n"
        "        printf(\"Even: no\\n\");\n"
        "    }\n"
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

// ===================================================================== 6
{
    .id = "c-arrays", .lang = LANG_C,
    .title   = "Arrays",
    .summary = "Storing many values under one name and looping over them.",
    .pageCount = 4,
    .pages = {
        { "What is an array?",
          "An array stores many values of the same type under one name. You choose how many slots it has "
          "when you declare it, and each value sits in a numbered slot.",
          "int scores[5] = { 90, 72, 85, 64, 99 };\n"
          "float prices[3];      // 3 slots, not filled in yet" },
        { "Indexes start at 0",
          "The first element is scores[0] and the last of 5 is scores[4]. You read and change elements with square brackets.\n\n"
          "Using scores[5] goes past the end. C won't stop you, but your program may crash or print garbage, so always stay inside the array.",
          "printf(\"%d\\n\", scores[0]);   // 90\n"
          "scores[1] = 75;              // change the second one\n"
          "printf(\"%d\\n\", scores[4]);   // 99, the last one" },
        { "Looping over an array",
          "Arrays and for loops go together: let i run from 0 up to (but not including) the size, "
          "and use i as the index. You can read input straight into an array the same way.",
          "for (int i = 0; i < 5; i++) {\n"
          "    printf(\"%d\\n\", scores[i]);\n"
          "}\n"
          "\n"
          "int nums[3];\n"
          "for (int i = 0; i < 3; i++) {\n"
          "    scanf(\"%d\", &nums[i]);\n"
          "}" },
        { "Finding the biggest",
          "To find the largest value, assume the first element is the biggest, then compare each of the others with it. "
          "Starting at 0 instead would break when every number is negative.",
          "int max = scores[0];\n"
          "for (int i = 1; i < 5; i++) {\n"
          "    if (scores[i] > max) {\n"
          "        max = scores[i];\n"
          "    }\n"
          "}" },
    },
    .quizCount = 3,
    .quiz = {
        { "For int a[4]; what is the index of the last element?", NULL, 4,
          { "4", "3", "0", "5" }, 1,
          "Indexes start at 0, so 4 slots are numbered 0, 1, 2 and 3." },
        { "What does this print?", "int a[3] = { 5, 8, 2 };\nprintf(\"%d\\n\", a[1]);", 4,
          { "5", "2", "8", "An error" }, 2,
          "a[0] is 5, a[1] is 8 and a[2] is 2." },
        { "Can one array hold an int and a float together?", NULL, 4,
          { "Yes, any mix is fine", "Only if it has 2 slots", "Only inside a loop", "No - every element has the same type" }, 3,
          "An array has one type for all its elements. Structs (coming soon) let you mix types." },
    },
    .projectTitle = "Array Stats",
    .projectBrief =
        "The first input is n, how many numbers follow (at most 100). Then come n whole numbers.\n\n"
        "Read them into an array, then print their sum, the biggest and the smallest.\n\n"
        "For the input 5 then 3 9 1 7 5 the output should be:\n\n"
        "Sum: 25\n"
        "Max: 9\n"
        "Min: 1\n\n"
        "Watch out: some tests use negative numbers.",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "    int nums[100];\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    // Read n numbers into nums\n"
        "\n"
        "    // Work out and print Sum, Max and Min\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Read with for (int i = 0; i < n; i++) scanf(\"%d\", &nums[i]); Then start max and min at nums[0] (not at 0, or negative numbers break it) and compare every element in a second loop.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "    int nums[100];\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        scanf(\"%d\", &nums[i]);\n"
        "    }\n"
        "\n"
        "    int sum = 0;\n"
        "    int max = nums[0];\n"
        "    int min = nums[0];\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        sum += nums[i];\n"
        "        if (nums[i] > max) max = nums[i];\n"
        "        if (nums[i] < min) min = nums[i];\n"
        "    }\n"
        "\n"
        "    printf(\"Sum: %d\\n\", sum);\n"
        "    printf(\"Max: %d\\n\", max);\n"
        "    printf(\"Min: %d\\n\", min);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "5\n3 9 1 7 5\n",           "Sum: 25\nMax: 9\nMin: 1\n" },
        { "1\n42\n",                  "Sum: 42\nMax: 42\nMin: 42\n" },
        { "4\n-5 -2 -9 -1\n",         "Sum: -17\nMax: -1\nMin: -9\n" },
        { "6\n10 20 30 40 50 60\n",   "Sum: 210\nMax: 60\nMin: 10\n" },
    },
},

// ===================================================================== 7
{
    .id = "c-strings", .lang = LANG_C,
    .title   = "Strings",
    .summary = "Text in C: char arrays, the null terminator and string.h.",
    .pageCount = 4,
    .pages = {
        { "Strings are char arrays",
          "C has no special string type. A string is an array of char that ends with a hidden '\\0' character "
          "(called the null terminator), which marks where the text stops. printf prints a string with %s.",
          "char name[] = \"Sam\";   // stored as 'S' 'a' 'm' '\\0'\n"
          "printf(\"%s\\n\", name);   // Sam" },
        { "Reading words",
          "scanf with %s reads one word: it stops at the first space. No & is needed, because an array's name "
          "already tells scanf where it lives. Make the array big enough for the longest word plus the '\\0'.",
          "char word[50];\n"
          "scanf(\"%s\", word);\n"
          "printf(\"Hello, %s!\\n\", word);" },
        { "string.h",
          "#include <string.h> gives useful string functions:\n"
          "strlen counts the characters (not counting the '\\0').\n"
          "strcmp compares two strings and gives 0 when they are equal - you can't compare strings with ==.\n"
          "strcpy copies one string into another.",
          "#include <string.h>\n"
          "\n"
          "int len = strlen(word);\n"
          "if (strcmp(word, \"yes\") == 0) {\n"
          "    printf(\"You said yes\\n\");\n"
          "}" },
        { "One character at a time",
          "Because a string is an array, you can loop over it and look at each char. "
          "Either loop until you reach the '\\0', or loop up to strlen.",
          "for (int i = 0; word[i] != '\\0'; i++) {\n"
          "    printf(\"%c\\n\", word[i]);\n"
          "}" },
    },
    .quizCount = 3,
    .quiz = {
        { "Which character marks the end of a C string?", NULL, 4,
          { "'\\n'", "' '", "'\\0'", "'.'" }, 2,
          "'\\0', the null terminator, tells every string function where the text stops." },
        { "What does strlen(\"cat\") return?", NULL, 4,
          { "3", "4", "2", "0" }, 0,
          "strlen counts the visible characters and does not count the '\\0'." },
        { "How do you check that two strings a and b are equal?", NULL, 4,
          { "a == b", "a = b", "strlen(a) == strlen(b)", "strcmp(a, b) == 0" }, 3,
          "== compares where the arrays live in memory, not their text. strcmp compares the text." },
    },
    .projectTitle = "Word Reverser",
    .projectBrief =
        "Read one word (at most 99 letters) with scanf. Print its length, then the word written backwards.\n\n"
        "For the input hello the output should be:\n\n"
        "Length: 5\n"
        "Reversed: olleh",
    .starterCode =
        "#include <stdio.h>\n"
        "#include <string.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    char word[100];\n"
        "    scanf(\"%s\", word);\n"
        "\n"
        "    // Print Length: ...\n"
        "\n"
        "    // Print Reversed: ...\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "int len = strlen(word); gives the length. For the reverse, print \"Reversed: \" first, then loop from i = len - 1 down to 0 printing word[i] with %c, and finish with a \\n.",
    .solution =
        "#include <stdio.h>\n"
        "#include <string.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    char word[100];\n"
        "    scanf(\"%s\", word);\n"
        "\n"
        "    int len = strlen(word);\n"
        "    printf(\"Length: %d\\n\", len);\n"
        "\n"
        "    printf(\"Reversed: \");\n"
        "    for (int i = len - 1; i >= 0; i--) {\n"
        "        printf(\"%c\", word[i]);\n"
        "    }\n"
        "    printf(\"\\n\");\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "hello\n",       "Length: 5\nReversed: olleh\n" },
        { "a\n",           "Length: 1\nReversed: a\n" },
        { "racecar\n",     "Length: 7\nReversed: racecar\n" },
        { "Programming\n", "Length: 11\nReversed: gnimmargorP\n" },
    },
},

// ===================================================================== 8
{
    .id = "c-pointers", .lang = LANG_C,
    .title   = "Pointers",
    .summary = "Memory addresses, the & and * operators, and changing variables through a pointer.",
    .pageCount = 4,
    .pages = {
        { "Memory addresses",
          "Every variable lives somewhere in your computer's memory, and that place has an address - a number. "
          "The & operator gives you a variable's address. That is why scanf needed &: it needs to know where to put the value.",
          "int x = 10;\n"
          "printf(\"%p\\n\", (void *)&x);   // something like 0x7ffd5c3a" },
        { "What is a pointer?",
          "A pointer is a variable that holds an address. int *p means \"p points to an int\". "
          "Putting * in front of a pointer follows it to the value it points at, so you can read or change that value.",
          "int x = 10;\n"
          "int *p = &x;           // p holds the address of x\n"
          "printf(\"%d\\n\", *p);   // 10\n"
          "*p = 20;               // x is now 20" },
        { "Why pointers? Changing a caller's variable",
          "When you pass a variable to a function, the function gets a copy. Changing the copy does nothing to the original. "
          "Pass the variable's address instead, and the function can change the original through the pointer.",
          "void add_one(int *n)\n"
          "{\n"
          "    *n = *n + 1;\n"
          "}\n"
          "\n"
          "int score = 5;\n"
          "add_one(&score);       // score is now 6" },
        { "Pointers and arrays",
          "An array's name works like a pointer to its first element, which is how functions receive arrays. "
          "NULL is a special value meaning \"points to nothing\" - never follow a NULL pointer with *, or the program crashes.",
          "void print_all(int *arr, int count)\n"
          "{\n"
          "    for (int i = 0; i < count; i++) {\n"
          "        printf(\"%d\\n\", arr[i]);\n"
          "    }\n"
          "}" },
    },
    .quizCount = 3,
    .quiz = {
        { "What does &x give you?", NULL, 4,
          { "The value of x", "The address of x", "x times 2", "A copy of x" }, 1,
          "& is the address-of operator. * goes the other way, from an address to the value." },
        { "What does this print?", "int x = 4;\nint *p = &x;\n*p = 9;\nprintf(\"%d\\n\", x);", 4,
          { "4", "An address", "9", "An error" }, 2,
          "p points at x, so *p = 9 changes x itself." },
        { "Why would you pass &score to a function instead of score?", NULL, 4,
          { "So the function can change score", "Because ints can't be passed", "To print its address", "It makes no difference" }, 0,
          "With the address, the function can reach the original variable rather than a copy." },
    },
    .projectTitle = "Swap",
    .projectBrief =
        "Finish the function void swap(int *a, int *b) so that it swaps the two values its pointers point to. "
        "main is already written: it reads two numbers, prints them, calls swap(&x, &y) and prints them again.\n\n"
        "For the input 3 7 the output should be:\n\n"
        "Before: 3 7\n"
        "After: 7 3",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "void swap(int *a, int *b)\n"
        "{\n"
        "    // Swap the values that a and b point to\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int x, y;\n"
        "    scanf(\"%d %d\", &x, &y);\n"
        "\n"
        "    printf(\"Before: %d %d\\n\", x, y);\n"
        "    swap(&x, &y);\n"
        "    printf(\"After: %d %d\\n\", x, y);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "You need a temporary variable to hold one value while you overwrite it: int temp = *a; then *a = *b; then *b = temp;",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "void swap(int *a, int *b)\n"
        "{\n"
        "    int temp = *a;\n"
        "    *a = *b;\n"
        "    *b = temp;\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int x, y;\n"
        "    scanf(\"%d %d\", &x, &y);\n"
        "\n"
        "    printf(\"Before: %d %d\\n\", x, y);\n"
        "    swap(&x, &y);\n"
        "    printf(\"After: %d %d\\n\", x, y);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "3 7\n",   "Before: 3 7\nAfter: 7 3\n" },
        { "10 -4\n", "Before: 10 -4\nAfter: -4 10\n" },
        { "5 5\n",   "Before: 5 5\nAfter: 5 5\n" },
        { "0 99\n",  "Before: 0 99\nAfter: 99 0\n" },
    },
},

// ===================================================================== 9
{
    .id = "c-structs", .lang = LANG_C,
    .title   = "Structs",
    .summary = "Grouping related data into your own types, and arrays of them.",
    .pageCount = 4,
    .pages = {
        { "Grouping data",
          "A struct groups related variables into one new type. A game player might have a name, a level and health - "
          "a struct keeps them together, so you pass around one thing instead of three.",
          "struct Player {\n"
          "    char name[20];\n"
          "    int level;\n"
          "    float health;\n"
          "};" },
        { "Creating and using one",
          "Declare a variable of the struct type, then use a dot to reach each of its fields.",
          "struct Player p;\n"
          "p.level = 3;\n"
          "p.health = 97.5f;\n"
          "strcpy(p.name, \"Ada\");   // needs string.h\n"
          "printf(\"%s is level %d\\n\", p.name, p.level);" },
        { "typedef",
          "typedef gives a type a shorter name, so you can skip writing struct every time. "
          "You can also fill in a struct in one go with braces, in the same order as its fields.",
          "typedef struct {\n"
          "    char name[20];\n"
          "    int age;\n"
          "} Person;\n"
          "\n"
          "Person a = { \"Sam\", 30 };" },
        { "Arrays of structs",
          "Structs and arrays work well together: a list of students, items or enemies. "
          "people[i].name means \"the name field of element i\".",
          "Person people[3];\n"
          "for (int i = 0; i < 3; i++) {\n"
          "    scanf(\"%s %d\", people[i].name, &people[i].age);\n"
          "}\n"
          "printf(\"%s\\n\", people[0].name);" },
    },
    .quizCount = 3,
    .quiz = {
        { "How do you get the level field of a struct variable p?", NULL, 4,
          { "p[level]", "level(p)", "p.level", "p::level" }, 2,
          "A dot reaches into a struct: p.level, p.name and so on." },
        { "What does typedef do?", NULL, 4,
          { "Gives a type a new, shorter name", "Deletes a struct", "Makes a variable constant", "Prints a struct" }, 0,
          "typedef struct { ... } Person; lets you write Person instead of struct something." },
        { "Can a struct contain an array, like char name[20]?", NULL, 4,
          { "No, only single values", "Yes", "Only arrays of int", "Only in C++" }, 1,
          "Fields can be any type, including arrays and even other structs." },
    },
    .projectTitle = "Student Records",
    .projectBrief =
        "The first input is n (at most 20). Then come n lines, each with a name (one word) and a whole-number score.\n\n"
        "Store them in an array of the Student struct, then print the student with the highest score "
        "(if there is a tie, the one that came first) and the average score to 1 decimal place.\n\n"
        "For this input:\n3\nLiam 72\nMaya 91\nNoah 71\n\n"
        "the output should be:\n\n"
        "Top: Maya 91\n"
        "Average: 78.0",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "typedef struct {\n"
        "    char name[30];\n"
        "    int score;\n"
        "} Student;\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "    Student students[20];\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    // Read each student's name and score\n"
        "\n"
        "    // Find the top student and the average\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Read each one with scanf(\"%s %d\", students[i].name, &students[i].score); Keep the index of the best student and only replace it when a score is strictly greater (>), so ties keep the first. For the average, divide as a float: total / (float)n, and print with %.1f.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "typedef struct {\n"
        "    char name[30];\n"
        "    int score;\n"
        "} Student;\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "    Student students[20];\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        scanf(\"%s %d\", students[i].name, &students[i].score);\n"
        "    }\n"
        "\n"
        "    int best = 0;\n"
        "    int total = 0;\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        total += students[i].score;\n"
        "        if (students[i].score > students[best].score) {\n"
        "            best = i;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    printf(\"Top: %s %d\\n\", students[best].name, students[best].score);\n"
        "    printf(\"Average: %.1f\\n\", total / (float)n);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "3\nLiam 72\nMaya 91\nNoah 71\n",   "Top: Maya 91\nAverage: 78.0\n" },
        { "1\nZoe 50\n",                      "Top: Zoe 50\nAverage: 50.0\n" },
        { "4\nAl 10\nBo 20\nCy 30\nDi 26\n",  "Top: Cy 30\nAverage: 21.5\n" },
        { "2\nAmy 88\nBen 88\n",              "Top: Amy 88\nAverage: 88.0\n" },
    },
},

// ===================================================================== 10
{
    .id = "c-files", .lang = LANG_C,
    .title   = "Files",
    .summary = "Saving data to a file and loading it back with fopen, fprintf and fscanf.",
    .pageCount = 4,
    .pages = {
        { "Why files?",
          "Variables disappear when a program ends. Files let a program save data and load it the next time it runs - "
          "high scores, settings, documents and save games all work this way.",
          NULL },
        { "Opening and writing",
          "fopen opens a file and gives you a FILE pointer. The second argument is the mode: \"w\" writes (and wipes "
          "what was there), \"a\" adds to the end, \"r\" reads. If it fails, fopen returns NULL, so always check. "
          "fprintf works like printf but writes to the file. fclose closes it when you are done.",
          "FILE *f = fopen(\"notes.txt\", \"w\");\n"
          "if (f == NULL) {\n"
          "    printf(\"Could not open the file\\n\");\n"
          "    return 1;\n"
          "}\n"
          "fprintf(f, \"Hello, file!\\n\");\n"
          "fclose(f);" },
        { "Reading",
          "Open with \"r\" and use fscanf, which works like scanf but reads from the file. fscanf returns how many "
          "values it read, so a loop can keep going until it stops returning 1.",
          "FILE *f = fopen(\"numbers.txt\", \"r\");\n"
          "int x;\n"
          "while (fscanf(f, \"%d\", &x) == 1) {\n"
          "    printf(\"Read %d\\n\", x);\n"
          "}\n"
          "fclose(f);" },
        { "Save, then load",
          "A program can write a file, close it, then open it again and read it back - exactly what a game does when "
          "you save and then load. Always fclose after writing, or some of the data may not reach the file yet.",
          NULL },
    },
    .quizCount = 3,
    .quiz = {
        { "Which mode adds to the end of a file without wiping it?", NULL, 4,
          { "\"r\"", "\"w\"", "\"a\"", "\"x\"" }, 2,
          "\"a\" is append. \"w\" starts the file again from empty, and \"r\" only reads." },
        { "What does fopen return when it can't open the file?", NULL, 4,
          { "NULL", "-1", "An empty file", "0 bytes" }, 0,
          "Always compare the result with NULL before using it." },
        { "Why call fclose after writing?", NULL, 4,
          { "To delete the file", "It is optional decoration", "To print the file", "To make sure everything is written and release the file" }, 3,
          "Writes are often held in memory first. fclose pushes them to the file and frees it." },
    },
    .projectTitle = "Save and Load",
    .projectBrief =
        "Read n, then n whole numbers.\n\n"
        "1. Write the numbers to a file called scores.txt, one per line, then close it.\n"
        "2. Open scores.txt again for reading, read the numbers back, and count and add them up.\n"
        "3. Print what you loaded.\n\n"
        "For the input 3 then 4 5 6 the output should be:\n\n"
        "Loaded 3 numbers\n"
        "Total: 15",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    // 1. Open scores.txt for writing and write each number\n"
        "\n"
        "    // 2. Open scores.txt for reading, count and add them up\n"
        "\n"
        "    // 3. Print Loaded ... numbers and Total: ...\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Writing: FILE *f = fopen(\"scores.txt\", \"w\"); then a loop that does scanf(\"%d\", &x); fprintf(f, \"%d\\n\", x); then fclose(f). Reading: fopen with \"r\", then while (fscanf(f, \"%d\", &x) == 1) { count++; total += x; } and fclose.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n, x;\n"
        "    scanf(\"%d\", &n);\n"
        "\n"
        "    FILE *out = fopen(\"scores.txt\", \"w\");\n"
        "    if (out == NULL) {\n"
        "        printf(\"Could not open scores.txt\\n\");\n"
        "        return 1;\n"
        "    }\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        scanf(\"%d\", &x);\n"
        "        fprintf(out, \"%d\\n\", x);\n"
        "    }\n"
        "    fclose(out);\n"
        "\n"
        "    FILE *in = fopen(\"scores.txt\", \"r\");\n"
        "    if (in == NULL) {\n"
        "        printf(\"Could not open scores.txt\\n\");\n"
        "        return 1;\n"
        "    }\n"
        "    int count = 0, total = 0;\n"
        "    while (fscanf(in, \"%d\", &x) == 1) {\n"
        "        count++;\n"
        "        total += x;\n"
        "    }\n"
        "    fclose(in);\n"
        "\n"
        "    printf(\"Loaded %d numbers\\n\", count);\n"
        "    printf(\"Total: %d\\n\", total);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "3\n4 5 6\n",       "Loaded 3 numbers\nTotal: 15\n" },
        { "1\n100\n",         "Loaded 1 numbers\nTotal: 100\n" },
        { "5\n1 2 3 4 5\n",   "Loaded 5 numbers\nTotal: 15\n" },
        { "0\n",              "Loaded 0 numbers\nTotal: 0\n" },
    },
},

};

const int C_TUTORIAL_COUNT = (int)(sizeof(C_TUTORIALS) / sizeof(C_TUTORIALS[0]));
