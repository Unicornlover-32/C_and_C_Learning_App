#include "tutorials.h"
#include <stddef.h>

// ---------------------------------------------------------------------------
// C sections, following the Advanced Programming lecture notes:
//   Intro to C, then Pointers and Memory in C.
// To add a tutorial, add a new entry to this array. Set .section on the first
// tutorial of a section. Pages whose heading starts with "Extra: " go beyond
// the lecture notes. Each project's tests feed `input` to the student's
// program and compare what it prints with `expected`; `solution` must pass.
// ---------------------------------------------------------------------------

const Tutorial C_TUTORIALS[] = {

// ===================================================================== 1
{
    .id = "c-hello", .lang = LANG_C,
    .title   = "Your First Program",
    .summary = "What C is, how a program is laid out, and printing text with printf.",
    .lecture = "Intro to C",
    .section = "Intro to C",
    .sectionBlurb = "The foundations - start here.",
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
    .id = "c-variables-const", .lang = LANG_C,
    .title   = "Variables, Types and Constants",
    .summary = "Typed variables, the basic data types, uninitialised variables and const.",
    .lecture = "Intro to C",
    .pageCount = 6,
    .pages = {
        { "What is a variable?",
          "A variable is a named box that holds a value. Unlike some languages, C makes you declare the type of every "
          "variable before you use it: say what kind of value it holds, then give it a name. = gives it a starting value.",
          "int age = 25;           // a whole number\n"
          "float price = 19.99f;   // a decimal number\n"
          "double pi = 3.14159;    // a more precise decimal\n"
          "char grade = 'A';       // one character, in single quotes" },
        { "The basic types",
          "int holds whole numbers and is typically 4 bytes. short is a smaller integer and long a larger one (write L after a long literal).\n"
          "float is a single-precision decimal (4 bytes) and double is double precision (8 bytes), so prefer double when accuracy matters.\n"
          "char holds a single character.\n\n"
          "Names can use letters, digits and underscores but can't start with a digit, and C is case sensitive: score and Score are different.",
          "short small = 100;\n"
          "long big = 1000000L;\n"
          "float f = 19.99f;     // the f marks a float literal\n"
          "double d = 19.99;     // plain decimals are doubles" },
        { "Always initialise your variables",
          "If you declare a variable without a value, C does not set it to 0 for you. It holds whatever happened to be in that piece of "
          "memory already - often called a garbage value. Reading it before you give it a value is a bug, and the program may behave "
          "differently every time it runs.",
          "int var;          // NOT initialised: could be anything\n"
          "int count = 0;    // initialised: definitely 0\n"
          "\n"
          "count = count + 1;   // fine\n"
          "var = var + 1;       // bug: garbage + 1" },
        { "Printing variables",
          "printf uses format specifiers as placeholders, filled in by the values listed after the text, in order.\n\n"
          "%d prints an int, %f a float or double, %c a char and %s a string. Put a precision between % and f to choose the "
          "decimal places: %.2f prints two.",
          "int age = 25;\n"
          "float price = 19.99f;\n"
          "char grade = 'A';\n"
          "printf(\"Age: %d\\n\", age);\n"
          "printf(\"Price: %.2f\\n\", price);   // .2 shows 2 decimal places\n"
          "printf(\"Grade: %c\\n\", grade);" },
        { "Constants with const",
          "Put const in front of a declaration to make a variable unchangeable. If you later try to assign to it, "
          "the compiler stops with an error - which is exactly what you want for values like a tax rate that should never change by accident.\n\n"
          "Constants are often written in UPPER_CASE so they stand out.",
          "const double TAX_RATE = 0.08;\n"
          "const int MAX_SCORE = 100;\n"
          "\n"
          "// TAX_RATE = 0.10;   // compile error!\n"
          "printf(\"Tax rate: %.2f\\n\", TAX_RATE);" },
        { "Extra: sizeof",
          "The lecture says an int is typically 4 bytes - typically, because sizes can differ between computers. "
          "sizeof tells you the real size in bytes on the machine you are using. Print its result with %zu.\n\n"
          "You'll use sizeof a lot later: to count the items in an array and to ask for memory with malloc.",
          "printf(\"char:   %zu\\n\", sizeof(char));     // always 1\n"
          "printf(\"int:    %zu\\n\", sizeof(int));      // usually 4\n"
          "printf(\"double: %zu\\n\", sizeof(double));   // usually 8" },
    },
    .quizCount = 4,
    .quiz = {
        { "Which type stores 3.14159 most precisely?", NULL, 4,
          { "int", "float", "double", "char" }, 2,
          "double uses 8 bytes (double precision), twice as much as float, so it keeps more digits." },
        { "What does this print?", "int x;\nprintf(\"%d\\n\", x);", 4,
          { "0", "1", "Whatever value happened to be in that memory", "The word x" }, 2,
          "x was never initialised, so it holds a garbage value. Always give variables a starting value." },
        { "What happens with this code?", "const double TAX_RATE = 0.08;\nTAX_RATE = 0.10;", 4,
          { "TAX_RATE becomes 0.10", "The compiler reports an error", "It crashes when run", "The second line is skipped" }, 1,
          "A const variable can't be assigned to after it is declared - the compiler refuses." },
        { "Which format specifier prints a char?", NULL, 4,
          { "%d", "%f", "%c", "%s" }, 2,
          "%c prints a single character. %s is for whole strings." },
    },
    .projectTitle = "Shop Receipt",
    .projectBrief =
        "The starter code declares a constant tax rate, a quantity and a price. Use them to print this receipt exactly:\n\n"
        "Quantity: 3\n"
        "Price: 19.99\n"
        "Subtotal: 59.97\n"
        "Tax: 4.80\n"
        "Total: 64.77\n\n"
        "Subtotal is quantity * price, tax is subtotal * TAX_RATE and total is subtotal + tax. "
        "Print the money values with two decimal places. Store each result in its own variable - and initialise them!",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    const double TAX_RATE = 0.08;\n"
        "    int quantity = 3;\n"
        "    double price = 19.99;\n"
        "\n"
        "    // Work out the subtotal, tax and total, then print the receipt\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "double subtotal = quantity * price; double tax = subtotal * TAX_RATE; double total = subtotal + tax; "
            "then printf(\"Subtotal: %.2f\\n\", subtotal); and so on. Quantity is an int, so print it with %d.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    const double TAX_RATE = 0.08;\n"
        "    int quantity = 3;\n"
        "    double price = 19.99;\n"
        "\n"
        "    double subtotal = quantity * price;\n"
        "    double tax = subtotal * TAX_RATE;\n"
        "    double total = subtotal + tax;\n"
        "\n"
        "    printf(\"Quantity: %d\\n\", quantity);\n"
        "    printf(\"Price: %.2f\\n\", price);\n"
        "    printf(\"Subtotal: %.2f\\n\", subtotal);\n"
        "    printf(\"Tax: %.2f\\n\", tax);\n"
        "    printf(\"Total: %.2f\\n\", total);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 1,
    .tests = {
        { "", "Quantity: 3\nPrice: 19.99\nSubtotal: 59.97\nTax: 4.80\nTotal: 64.77\n" },
    },
},

// ===================================================================== 3
{
    .id = "c-input-maths", .lang = LANG_C,
    .title   = "Input, Maths and Type Conversion",
    .summary = "Reading input with scanf, arithmetic operators, ++ and +=, integer division and casting.",
    .lecture = "Intro to C",
    .pageCount = 7,
    .pages = {
        { "Reading input with scanf",
          "scanf reads from the keyboard into a variable. It uses the same format specifiers as printf - but note %lf for a double - "
          "and needs an & before each variable. & means \"the address of\": scanf needs to know where the variable lives so it can fill it in. "
          "You'll find out exactly why in the Pointers tutorial.\n\n"
          "A space before %c (\" %c\") skips spaces and the Enter key left over from earlier input.",
          "int age;\n"
          "float salary;\n"
          "char initial;\n"
          "scanf(\"%d\", &age);\n"
          "scanf(\"%f\", &salary);\n"
          "scanf(\" %c\", &initial);   // space skips whitespace" },
        { "Did the input work?",
          "scanf returns how many values it managed to read. If the user types abc when you asked for a number, it returns 0 and leaves "
          "your variable untouched. Checking the return value makes your program safe against bad input.\n\n"
          "(Visual Studio may complain about scanf and suggest scanf_s - the check works the same way with either.)",
          "printf(\"Enter your age: \");\n"
          "if (scanf(\"%d\", &age) != 1) {\n"
          "    printf(\"Invalid input for age.\\n\");\n"
          "    return 1;   // a non-zero return means something went wrong\n"
          "}" },
        { "Arithmetic",
          "C has + - * / and % (the remainder, or modulo).\n\n"
          "Careful: when both sides of / are ints, the answer is an int and the decimal part is simply cut off (truncated). "
          "10 / 3 is 3. Use floats or doubles when you need the decimals.",
          "int a = 10, b = 3;\n"
          "printf(\"Division: %d\\n\", a / b);    // 3 (truncated)\n"
          "printf(\"Remainder: %d\\n\", a % b);   // 1\n"
          "\n"
          "float x = 10.0, y = 3.0;\n"
          "printf(\"Float division: %.2f\\n\", x / y);   // 3.33" },
        { "Increment and compound assignment",
          "count++ adds 1 and count-- subtracts 1. You'll see these all the time in loops.\n\n"
          "Compound assignment is a shortcut for changing a variable using its own value: num += 5 means num = num + 5, "
          "and there are -=, *=, /= and %= too.",
          "int count = 5;\n"
          "count++;      // 6\n"
          "count--;      // 5\n"
          "\n"
          "int num = 10;\n"
          "num += 5;     // num = num + 5, now 15\n"
          "num *= 2;     // num = num * 2, now 30" },
        { "Type conversion",
          "C converts between types automatically when you mix them: int + float gives a float.\n\n"
          "A cast forces a conversion: write the type in brackets before the value. (float)7 / 2 turns 7 into 7.0 first, so the division "
          "keeps the decimals. Casting a float to an int truncates - it does not round - so (int)3.9 is 3.",
          "float result1 = 5 + 2.5;       // 7.50 (5 became a float)\n"
          "int result2 = 7 / 2;           // 3 (truncated)\n"
          "float result3 = (float)7 / 2;  // 3.50\n"
          "int result4 = (int)3.9;        // 3, not 4!" },
        { "Extra: Where the cast goes matters",
          "A cast only applies to the value right after it. (float)(7 / 2) is 3.00, because the int division 7 / 2 = 3 happens first "
          "and only then is 3 turned into a float.\n\n"
          "To round instead of truncate, use round() from <math.h> (on Linux add -lm when compiling), or add 0.5 before casting a positive number.",
          "float a = (float)7 / 2;     // 3.50 - cast first, then divide\n"
          "float b = (float)(7 / 2);   // 3.00 - divide first, then cast\n"
          "\n"
          "int r = (int)(3.9 + 0.5);   // 4 - rounded" },
        { "Extra: Prompts that appear late",
          "The lecture notes mention that on some online compilers the \"Enter ...\" prompts only show up at the end. That's because output "
          "is buffered: it is collected and printed in chunks. Ending a prompt with \\n, or calling fflush(stdout); after it, prints it straight away.\n\n"
          "The projects in this app don't print prompts, because the tests compare everything your program prints.",
          "printf(\"Enter your age: \");\n"
          "fflush(stdout);           // show the prompt now\n"
          "scanf(\"%d\", &age);" },
    },
    .quizCount = 4,
    .quiz = {
        { "What does (float)7 / 2 give?", NULL, 4,
          { "3", "3.5", "4", "3.0" }, 1,
          "The cast turns 7 into 7.0 before dividing, so the division keeps the decimal part: 3.5." },
        { "scanf(\"%d\", &n) reads one number successfully. What does it return?", NULL, 4,
          { "0", "The number it read", "1", "-1" }, 2,
          "scanf returns how many values it read - 1 here. Comparing it with 1 is how you check the input was valid." },
        { "What is 17 % 5?", NULL, 4,
          { "3", "2", "3.4", "12" }, 1,
          "17 divided by 5 is 3 remainder 2, and % gives the remainder." },
        { "What is x after this code?", "int x = 4;\nx *= 3;\nx++;", 4,
          { "12", "13", "8", "7" }, 1,
          "x *= 3 makes it 12, then x++ adds one: 13." },
    },
    .projectTitle = "Time Converter",
    .projectBrief =
        "Read a number of seconds and print it as hours, minutes and seconds, then as minutes with two decimal places. For 3725:\n\n"
        "3725 seconds is 1 hours, 2 minutes and 5 seconds\n"
        "In minutes: 62.08\n\n"
        "If the input isn't a number, print  Invalid input  and stop (return 1).\n\n"
        "Use integer division and % for the first line, and a cast for the second - 3725 / 60 with two ints would lose the decimals.",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int total;\n"
        "\n"
        "    // Read total, checking that scanf worked\n"
        "\n"
        "    // Work out the hours, minutes and seconds and print them\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "if (scanf(\"%d\", &total) != 1) { printf(\"Invalid input\\n\"); return 1; } "
            "Then hours = total / 3600; minutes = (total % 3600) / 60; seconds = total % 60; "
            "and the decimal minutes are (double)total / 60, printed with %.2f.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int total;\n"
        "\n"
        "    if (scanf(\"%d\", &total) != 1) {\n"
        "        printf(\"Invalid input\\n\");\n"
        "        return 1;\n"
        "    }\n"
        "\n"
        "    int hours = total / 3600;\n"
        "    int minutes = (total % 3600) / 60;\n"
        "    int seconds = total % 60;\n"
        "\n"
        "    printf(\"%d seconds is %d hours, %d minutes and %d seconds\\n\", total, hours, minutes, seconds);\n"
        "    printf(\"In minutes: %.2f\\n\", (double)total / 60);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 5,
    .tests = {
        { "3725\n", "3725 seconds is 1 hours, 2 minutes and 5 seconds\nIn minutes: 62.08\n" },
        { "59\n",   "59 seconds is 0 hours, 0 minutes and 59 seconds\nIn minutes: 0.98\n" },
        { "7200\n", "7200 seconds is 2 hours, 0 minutes and 0 seconds\nIn minutes: 120.00\n" },
        { "90061\n","90061 seconds is 25 hours, 1 minutes and 1 seconds\nIn minutes: 1501.02\n" },
        { "abc\n",  "Invalid input\n" },
    },
},

// ===================================================================== 4
{
    .id = "c-decisions-switch", .lang = LANG_C,
    .title   = "Decisions: if, else and switch",
    .summary = "if / else if / else, comparisons, logical operators && || !, and the switch statement.",
    .lecture = "Intro to C",
    .pageCount = 6,
    .pages = {
        { "The if statement",
          "if runs a block of code only when its condition is true. The condition goes in brackets, the code in curly braces.\n\n"
          "Comparisons: == equal, != not equal, < less, > greater, <= and >=. Careful: = stores a value and == compares. "
          "Writing if (x = 5) is a classic bug.",
          "int number = 95;\n"
          "if (number >= 90) {\n"
          "    printf(\"Number is at least 90\\n\");\n"
          "}" },
        { "else if and else",
          "Chain more checks with else if, and catch everything left over with else. They are checked from the top down "
          "and only the first true branch runs, so put the strictest condition first.",
          "if (score >= 90) {\n"
          "    printf(\"Grade: A - Excellent!\\n\");\n"
          "} else if (score >= 80) {\n"
          "    printf(\"Grade: B - Good!\\n\");\n"
          "} else if (score >= 70) {\n"
          "    printf(\"Grade: C - Satisfactory\\n\");\n"
          "} else {\n"
          "    printf(\"Grade: F - Failed\\n\");\n"
          "}" },
        { "Logical operators",
          "Combine conditions with && (and: both must be true), || (or: at least one is true) and ! (not: flips true and false).\n\n"
          "In C, 0 means false and any other number means true.",
          "int age = 10;\n"
          "if (age >= 18 && age <= 65) {\n"
          "    printf(\"You are of working age.\\n\");\n"
          "} else if (age < 18 || age > 65) {\n"
          "    printf(\"You are not of working age.\\n\");\n"
          "}" },
        { "The switch statement",
          "switch checks one variable against a list of specific values. Each case ends with break; - otherwise the code carries "
          "on into the next case. Stacking cases with no code between them lets several values share one block (here, upper and lower case). "
          "default runs when no case matches.",
          "switch (grade) {\n"
          "    case 'A':\n"
          "    case 'a':\n"
          "        printf(\"Excellent performance!\\n\");\n"
          "        break;\n"
          "    case 'B':\n"
          "    case 'b':\n"
          "        printf(\"Good job!\\n\");\n"
          "        break;\n"
          "    default:\n"
          "        printf(\"Invalid grade entered\\n\");\n"
          "}" },
        { "Extra: Forgetting break",
          "If you leave out a break, C \"falls through\" and runs the next case's code too, even though its value didn't match. "
          "Sometimes that's on purpose (like stacking 'A' and 'a'), but usually it's a bug.\n\n"
          "switch only works with whole-number types such as int and char - not with strings or doubles.",
          "switch (2) {\n"
          "    case 1: printf(\"one \");\n"
          "    case 2: printf(\"two \");     // matches here...\n"
          "    case 3: printf(\"three \");   // ...and falls into here\n"
          "}\n"
          "// prints: two three" },
        { "Extra: The conditional operator",
          "condition ? a : b is a tiny if/else that gives back a value: a when the condition is true, b when it is false. "
          "It is handy for short choices inside printf. You'll see it in the lecture's linked list code: list.contains(10) ? \"Yes\" : \"No\".",
          "int n = 7;\n"
          "printf(\"%d is %s\\n\", n, n % 2 == 0 ? \"even\" : \"odd\");\n"
          "// prints: 7 is odd" },
    },
    .quizCount = 4,
    .quiz = {
        { "Which operator checks whether two values are equal?", NULL, 4,
          { "=", "==", "!=", "=>" }, 1,
          "== compares. A single = assigns a value." },
        { "What does this print when age is 70?", "if (age >= 18 && age <= 65)\n    printf(\"working\");\nelse\n    printf(\"not working\");", 4,
          { "working", "not working", "both", "nothing" }, 1,
          "&& needs both sides to be true. 70 <= 65 is false, so the else runs." },
        { "What happens if a case has no break?", NULL, 4,
          { "A compile error", "The program stops", "It carries on into the next case's code", "The switch starts again" }, 2,
          "Without break, execution falls through into the next case." },
        { "When does the default part of a switch run?", NULL, 4,
          { "Always, at the end", "When no case matches", "Only for the value 0", "Never" }, 1,
          "default catches every value that none of the cases handled." },
    },
    .projectTitle = "Grade Report",
    .projectBrief =
        "Read a score from 0 to 100. First use if / else if / else to work out the letter grade:\n"
        "90 or more: A,  80-89: B,  70-79: C,  60-69: D,  below 60: F\n\n"
        "Print it as  Grade: B  and then use a switch on the letter to print the matching comment from the lecture:\n\n"
        "A: Excellent performance!\n"
        "B: Good job!\n"
        "C: Satisfactory\n"
        "D: You can do better\n"
        "F: Needs improvement\n\n"
        "So 83 prints:\n"
        "Grade: B\n"
        "Good job!",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int score;\n"
        "    char grade;\n"
        "    scanf(\"%d\", &score);\n"
        "\n"
        "    // 1. Use if / else if / else to set grade to 'A', 'B', 'C', 'D' or 'F'\n"
        "\n"
        "    // 2. Print the grade, then use a switch to print the comment\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "if (score >= 90) grade = 'A'; else if (score >= 80) grade = 'B'; ... else grade = 'F'; "
            "Then printf(\"Grade: %c\\n\", grade); and switch (grade) { case 'A': printf(...); break; ... }",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int score;\n"
        "    char grade;\n"
        "    scanf(\"%d\", &score);\n"
        "\n"
        "    if (score >= 90) {\n"
        "        grade = 'A';\n"
        "    } else if (score >= 80) {\n"
        "        grade = 'B';\n"
        "    } else if (score >= 70) {\n"
        "        grade = 'C';\n"
        "    } else if (score >= 60) {\n"
        "        grade = 'D';\n"
        "    } else {\n"
        "        grade = 'F';\n"
        "    }\n"
        "\n"
        "    printf(\"Grade: %c\\n\", grade);\n"
        "\n"
        "    switch (grade) {\n"
        "        case 'A':\n"
        "            printf(\"Excellent performance!\\n\");\n"
        "            break;\n"
        "        case 'B':\n"
        "            printf(\"Good job!\\n\");\n"
        "            break;\n"
        "        case 'C':\n"
        "            printf(\"Satisfactory\\n\");\n"
        "            break;\n"
        "        case 'D':\n"
        "            printf(\"You can do better\\n\");\n"
        "            break;\n"
        "        default:\n"
        "            printf(\"Needs improvement\\n\");\n"
        "    }\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 6,
    .tests = {
        { "95\n", "Grade: A\nExcellent performance!\n" },
        { "83\n", "Grade: B\nGood job!\n" },
        { "70\n", "Grade: C\nSatisfactory\n" },
        { "61\n", "Grade: D\nYou can do better\n" },
        { "12\n", "Grade: F\nNeeds improvement\n" },
        { "90\n", "Grade: A\nExcellent performance!\n" },
    },
},

// ===================================================================== 5
{
    .id = "c-loops-2", .lang = LANG_C,
    .title   = "Loops: while, do-while and for",
    .summary = "Repeating code with while, do-while and for, break and continue, and nested loops.",
    .lecture = "Intro to C",
    .pageCount = 6,
    .pages = {
        { "The while loop",
          "A while loop checks its condition first, and keeps running its block as long as the condition stays true. "
          "Something inside the loop must eventually make the condition false, or it never stops.",
          "int count = 1;\n"
          "while (count <= 5) {\n"
          "    printf(\"Count: %d\\n\", count);\n"
          "    count++;\n"
          "}" },
        { "The do-while loop",
          "A do-while loop checks its condition at the end, so the body always runs at least once - even if the condition is false "
          "from the start. That makes it a natural fit for \"ask, then ask again if the answer was bad\". Note the semicolon after while (...).",
          "int number = 3;\n"
          "do {\n"
          "    printf(\"number: %d\\n\", number);\n"
          "    number--;\n"
          "} while (number != 0);" },
        { "The for loop",
          "A for loop puts the three parts of a counting loop on one line: the start, the condition, and the step that runs after each round. "
          "It is the usual choice when you know how many times to repeat.",
          "for (int i = 1; i <= 10; i++) {\n"
          "    printf(\"%d \", i);       // 1 2 3 ... 10\n"
          "}\n"
          "\n"
          "for (int i = 10; i >= 1; i--) {\n"
          "    printf(\"%d \", i);       // counting down\n"
          "}" },
        { "break and continue",
          "break leaves the loop straight away. while (1) loops forever, so it needs a break to stop.\n"
          "continue skips the rest of this round and jumps to the next one.",
          "int i = 1;\n"
          "while (1) {               // infinite loop\n"
          "    if (i == 5) break;     // ...until i is 5\n"
          "    i++;\n"
          "}\n"
          "\n"
          "for (int i = 1; i <= 10; i++) {\n"
          "    if (i % 2 == 0) continue;   // skip even numbers\n"
          "    printf(\"%d \", i);           // 1 3 5 7 9\n"
          "}" },
        { "Nested loops",
          "A loop inside a loop runs completely for every round of the outer loop. Rows and columns are the classic use: the outer loop picks "
          "the row and the inner loop prints each column. %3d prints a number padded to 3 characters wide, which lines the columns up.",
          "for (int row = 1; row <= 5; row++) {\n"
          "    for (int col = 1; col <= 5; col++) {\n"
          "        printf(\"%3d \", row * col);\n"
          "    }\n"
          "    printf(\"\\n\");   // new line after each row\n"
          "}" },
        { "Extra: Loops that read input",
          "scanf returns how many values it read, so it can control a loop: keep going while it reads a number, and stop when the input runs out "
          "or isn't a number. Another common pattern is a sentinel - a special value such as 0 or -1 that means \"stop\".",
          "int n, total = 0;\n"
          "while (scanf(\"%d\", &n) == 1) {\n"
          "    total += n;\n"
          "}\n"
          "printf(\"Total: %d\\n\", total);" },
    },
    .quizCount = 4,
    .quiz = {
        { "How many times does this print Hi?", "for (int i = 0; i < 3; i++) {\n    printf(\"Hi\\n\");\n}", 4,
          { "2", "3", "4", "Forever" }, 1,
          "i takes the values 0, 1 and 2 - three rounds. When i reaches 3, i < 3 is false." },
        { "How many times does the body run?", "int n = 0;\ndo {\n    printf(\"x\");\n} while (n > 5);", 4,
          { "0", "1", "5", "Forever" }, 1,
          "A do-while checks its condition after the body, so the body always runs at least once." },
        { "What does continue do?", NULL, 4,
          { "Ends the loop", "Skips to the next round of the loop", "Restarts the program", "Pauses the loop" }, 1,
          "continue skips the rest of the current round. break is the one that leaves the loop." },
        { "What does %3d do in printf?", NULL, 4,
          { "Prints 3 numbers", "Prints the number 3 times", "Pads the number to 3 characters wide", "Divides by 3" }, 2,
          "The 3 is a minimum width, so 7 prints as \"  7\". It keeps table columns lined up." },
    },
    .projectTitle = "Times Table",
    .projectBrief =
        "Read a size n from 1 to 12. If the number is outside that range, print  Please enter 1 to 12  and read another one - "
        "keep asking until it is valid (a do-while loop is perfect for this).\n\n"
        "Then print an n x n multiplication table, printing each entry with %4d and a new line after each row, "
        "followed by the total of every entry. For 3:\n\n"
        "   1   2   3\n"
        "   2   4   6\n"
        "   3   6   9\n"
        "Total: 36",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "\n"
        "    // Read n, asking again until it is between 1 and 12\n"
        "\n"
        "    // Print the table with nested loops, and the total\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "do { scanf(\"%d\", &n); if (n < 1 || n > 12) printf(\"Please enter 1 to 12\\n\"); } while (n < 1 || n > 12); "
            "Then for (int row = 1; row <= n; row++) { for (int col = 1; col <= n; col++) { printf(\"%4d\", row * col); total += row * col; } printf(\"\\n\"); }",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int n;\n"
        "\n"
        "    do {\n"
        "        scanf(\"%d\", &n);\n"
        "        if (n < 1 || n > 12) {\n"
        "            printf(\"Please enter 1 to 12\\n\");\n"
        "        }\n"
        "    } while (n < 1 || n > 12);\n"
        "\n"
        "    int total = 0;\n"
        "    for (int row = 1; row <= n; row++) {\n"
        "        for (int col = 1; col <= n; col++) {\n"
        "            printf(\"%4d\", row * col);\n"
        "            total += row * col;\n"
        "        }\n"
        "        printf(\"\\n\");\n"
        "    }\n"
        "    printf(\"Total: %d\\n\", total);\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "3\n",        "   1   2   3\n   2   4   6\n   3   6   9\nTotal: 36\n" },
        { "1\n",        "   1\nTotal: 1\n" },
        { "0\n15\n2\n", "Please enter 1 to 12\nPlease enter 1 to 12\n   1   2\n   2   4\nTotal: 9\n" },
        { "4\n",        "   1   2   3   4\n   2   4   6   8\n   3   6   9  12\n   4   8  12  16\nTotal: 100\n" },
    },
},

// ===================================================================== 6
{
    .id = "c-arrays-2d", .lang = LANG_C,
    .title   = "Arrays and 2D Arrays",
    .summary = "Storing many values of one type, sizeof to count elements, finding a maximum, and grids.",
    .lecture = "Intro to C",
    .pageCount = 6,
    .pages = {
        { "What is an array?",
          "An array stores many values of the same type, one after another in memory (contiguous memory). "
          "You choose the size when you declare it, and reach each element by its index in square brackets. "
          "The index starts at 0, so the last element of a 5-element array is index 4.",
          "int numbers[5] = {10, 20, 30, 40, 50};\n"
          "\n"
          "printf(\"First: %d\\n\", numbers[0]);   // 10\n"
          "printf(\"Fifth: %d\\n\", numbers[4]);   // 50\n"
          "numbers[1] = 25;                       // change the second" },
        { "Looping over an array",
          "A for loop counting from 0 up to (but not including) the size visits every element. "
          "This is how you print, add up or search an array.",
          "int sum = 0;\n"
          "for (int i = 0; i < 5; i++) {\n"
          "    printf(\"numbers[%d] = %d\\n\", i, numbers[i]);\n"
          "    sum += numbers[i];\n"
          "}\n"
          "printf(\"Sum: %d\\n\", sum);" },
        { "Partial initialisation and sizeof",
          "If you give fewer values than the size, the rest are automatically set to 0. "
          "Leave the size out and the compiler counts the values for you.\n\n"
          "sizeof(values) is the size of the whole array in bytes and sizeof(values[0]) the size of one element, so dividing them gives the number of elements.",
          "int scores[10] = {85, 92, 78};       // scores[3] to [9] are 0\n"
          "\n"
          "int values[] = {45, 12, 89, 34, 67, 23};\n"
          "int size = sizeof(values) / sizeof(values[0]);   // 6" },
        { "Finding the maximum",
          "Start by assuming the first element is the biggest, then check every other element and replace your answer whenever you find a bigger one. "
          "The same idea, with < instead of >, finds the minimum.",
          "int max = values[0];\n"
          "for (int i = 1; i < size; i++) {\n"
          "    if (values[i] > max) {\n"
          "        max = values[i];\n"
          "    }\n"
          "}\n"
          "printf(\"Maximum value: %d\\n\", max);   // 89" },
        { "2D arrays",
          "A 2D array is a grid: matrix[3][4] has 3 rows of 4 columns, and matrix[row][col] picks one cell. "
          "Nested loops visit every cell - the outer loop over the rows, the inner one over the columns. "
          "The lecture's grade book stores grades[student][exam] the same way.",
          "int matrix[3][4] = {\n"
          "    {1, 2, 3, 4},\n"
          "    {5, 6, 7, 8},\n"
          "    {9, 10, 11, 12}\n"
          "};\n"
          "for (int row = 0; row < 3; row++) {\n"
          "    for (int col = 0; col < 4; col++) {\n"
          "        printf(\"%3d \", matrix[row][col]);\n"
          "    }\n"
          "    printf(\"\\n\");\n"
          "}" },
        { "Extra: Going out of bounds",
          "C does not check your indexes. numbers[5] on a 5-element array reads or writes whatever memory comes next - "
          "this is undefined behaviour and can give wrong answers, corrupt other variables or crash the program.\n\n"
          "Always make sure loops stop at size - 1. Using a variable such as size (or sizeof) instead of typing the number keeps loops and arrays in step.",
          "int a[5];\n"
          "for (int i = 0; i <= 5; i++) {   // BUG: <= runs i = 5 too\n"
          "    a[i] = 0;                    // writes past the end\n"
          "}" },
    },
    .quizCount = 4,
    .quiz = {
        { "For int a[4]; what is the index of the last element?", NULL, 4,
          { "4", "3", "0", "5" }, 1,
          "Indexes start at 0, so a 4-element array uses 0, 1, 2 and 3." },
        { "What is scores[5] here?", "int scores[10] = {85, 92, 78};", 4,
          { "78", "Garbage", "0", "An error" }, 2,
          "Elements you don't give a value to in a partial initialisation are set to 0." },
        { "What does sizeof(values) / sizeof(values[0]) give?", NULL, 4,
          { "The size in bytes", "The number of elements", "The largest value", "The last index" }, 1,
          "Total bytes divided by bytes per element is the number of elements." },
        { "What is matrix[1][2]?", "int matrix[3][4] = {\n    {1, 2, 3, 4},\n    {5, 6, 7, 8},\n    {9, 10, 11, 12}\n};", 4,
          { "2", "6", "7", "10" }, 2,
          "Row 1 is the second row {5, 6, 7, 8}, and column 2 is its third value: 7." },
    },
    .projectTitle = "Matrix Sums",
    .projectBrief =
        "Read the number of rows and columns (each from 1 to 10), then that many whole numbers, row by row, into a 2D array.\n\n"
        "Print the matrix with each number formatted with %4d and a new line after each row. "
        "Then print the sum of each row and the sum of each column, separated by spaces. For\n"
        "2 3\n"
        "1 2 3\n"
        "4 5 6\n\n"
        "the output is:\n"
        "   1   2   3\n"
        "   4   5   6\n"
        "Row sums: 6 15\n"
        "Column sums: 5 7 9",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int grid[10][10];\n"
        "    int rows, cols;\n"
        "    scanf(\"%d %d\", &rows, &cols);\n"
        "\n"
        "    // Read the numbers into grid, then print the matrix and the sums\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Read with nested loops: scanf(\"%d\", &grid[r][c]);. For the row sums, loop over r and add up grid[r][c] for every c. "
            "For the column sums swap the loops: loop over c on the outside and add grid[r][c] for every r.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int grid[10][10];\n"
        "    int rows, cols;\n"
        "    scanf(\"%d %d\", &rows, &cols);\n"
        "\n"
        "    for (int r = 0; r < rows; r++) {\n"
        "        for (int c = 0; c < cols; c++) {\n"
        "            scanf(\"%d\", &grid[r][c]);\n"
        "        }\n"
        "    }\n"
        "\n"
        "    for (int r = 0; r < rows; r++) {\n"
        "        for (int c = 0; c < cols; c++) {\n"
        "            printf(\"%4d\", grid[r][c]);\n"
        "        }\n"
        "        printf(\"\\n\");\n"
        "    }\n"
        "\n"
        "    printf(\"Row sums:\");\n"
        "    for (int r = 0; r < rows; r++) {\n"
        "        int sum = 0;\n"
        "        for (int c = 0; c < cols; c++) {\n"
        "            sum += grid[r][c];\n"
        "        }\n"
        "        printf(\" %d\", sum);\n"
        "    }\n"
        "    printf(\"\\n\");\n"
        "\n"
        "    printf(\"Column sums:\");\n"
        "    for (int c = 0; c < cols; c++) {\n"
        "        int sum = 0;\n"
        "        for (int r = 0; r < rows; r++) {\n"
        "            sum += grid[r][c];\n"
        "        }\n"
        "        printf(\" %d\", sum);\n"
        "    }\n"
        "    printf(\"\\n\");\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "2 3\n1 2 3\n4 5 6\n",               "   1   2   3\n   4   5   6\nRow sums: 6 15\nColumn sums: 5 7 9\n" },
        { "3 4\n1 2 3 4\n5 6 7 8\n9 10 11 12\n", "   1   2   3   4\n   5   6   7   8\n   9  10  11  12\nRow sums: 10 26 42\nColumn sums: 15 18 21 24\n" },
        { "1 1\n5\n",                           "   5\nRow sums: 5\nColumn sums: 5\n" },
        { "2 2\n-1 0\n4 -4\n",                  "  -1   0\n   4  -4\nRow sums: -1 0\nColumn sums: 3 -4\n" },
    },
},

// ===================================================================== 7
{
    .id = "c-strings", .lang = LANG_C,
    .title   = "Strings",
    .summary = "Text in C: char arrays, the null terminator, counting by hand and string.h.",
    .lecture = "Intro to C",
    .pageCount = 6,
    .pages = {
        { "Strings are char arrays",
          "C has no special string type. A string is an array of char that ends with a hidden '\\0' character "
          "(called the null terminator), which marks where the text stops. printf prints a string with %s.",
          "char name[] = \"Sam\";   // stored as 'S' 'a' 'm' '\\0'\n"
          "printf(\"%s\\n\", name);   // Sam" },
        { "Building a string by hand",
          "char name[20] = \"Alice\"; really holds 'A' 'l' 'i' 'c' 'e' '\\0', then unused space up to index 19 - the compiler adds the '\\0' for you. "
          "If you build the array yourself, character by character, you must add the '\\0' yourself. Forget it and it isn't a string, just an array of characters: "
          "printf(\"%s\") keeps reading past the end into random memory until it happens to hit a zero byte.",
          "char name[20] = \"Alice\";   // '\\0' added for you\n"
          "\n"
          "char city[30] = {'N', 'e', 'w', ' ', 'Y', 'o', 'r', 'k', '\\0'};\n"
          "printf(\"%s\\n\", city);   // New York" },
        { "Why the null terminator matters",
          "A C string has no stored length anywhere - the '\\0' is the length marker. Every %s in printf works the same way: start at index 0, "
          "keep going character by character, and stop at '\\0'. So to find the length by hand, count until you reach it.",
          "int len = 0;\n"
          "while (name[len] != '\\0') {\n"
          "    len++;\n"
          "}\n"
          "printf(\"Length: %d\\n\", len);   // 5 for Alice" },
        { "Reading words",
          "scanf with %s reads one word: it stops at the first space. No & is needed, because an array's name "
          "already tells scanf where it lives. Make the array big enough for the longest word plus the '\\0'.",
          "char word[50];\n"
          "scanf(\"%s\", word);\n"
          "printf(\"Hello, %s!\\n\", word);" },
        { "Extra: string.h",
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
    .id = "c-functions-arrays", .lang = LANG_C,
    .title   = "Functions and Pass by Value",
    .summary = "Writing functions, prototypes, passing arrays to functions and why ordinary arguments are copies.",
    .lecture = "Intro to C",
    .pageCount = 6,
    .pages = {
        { "Functions organise your code",
          "A function is a named block of code you can call whenever you need it. Functions break a big program into small pieces "
          "that are easy to read, test and reuse.\n\n"
          "A function has a return type (or void for none), a name, and parameters in brackets. return sends a value back to the caller.",
          "int add(int a, int b) {\n"
          "    return a + b;\n"
          "}\n"
          "\n"
          "void printGreeting() {\n"
          "    printf(\"Hello! Welcome to C programming.\\n\");\n"
          "}" },
        { "Calling functions",
          "Call a function by its name with values in brackets. The returned value can be stored, printed or used straight away, "
          "and a function can be called as many times as you like.",
          "int sum = add(5, 3);\n"
          "printf(\"5 + 3 = %d\\n\", sum);\n"
          "printGreeting();\n"
          "printf(\"10 + 20 = %d\\n\", add(10, 20));" },
        { "Function prototypes",
          "C reads your file from top to bottom, so a function must be declared before it is used. "
          "A prototype is the first line of the function followed by a semicolon. Put prototypes at the top and you can write the full "
          "functions in any order, even after main. Header (.h) files are mostly lists of prototypes.",
          "double calculateAverage(int arr[], int size);   // prototype\n"
          "\n"
          "int main() {\n"
          "    ...uses calculateAverage...\n"
          "}\n"
          "\n"
          "double calculateAverage(int arr[], int size) {   // the real thing\n"
          "    ...\n"
          "}" },
        { "Passing arrays to functions",
          "When you pass an array, the function does not get its own copy - it works directly on the caller's array. "
          "So a function can read an array (like calculateAverage) or change it (like doubleAll), and the change is visible back in main.\n\n"
          "The function can't tell how big the array is, so pass the size as well.",
          "void doubleAll(int arr[], int size) {\n"
          "    for (int i = 0; i < size; i++) {\n"
          "        arr[i] = arr[i] * 2;   // changes the original\n"
          "    }\n"
          "}\n"
          "\n"
          "int numbers[] = {1, 2, 3, 4, 5};\n"
          "doubleAll(numbers, 5);         // numbers is now 2 4 6 8 10" },
        { "Pass by value",
          "Ordinary variables like int, double and char are passed by value: the function receives a copy. "
          "Whatever it does to its parameter has no effect on the caller's variable. That's why add(5, 3) can never accidentally change anything in main.",
          "void tryToChange(int x) {\n"
          "    x = 999;                 // only changes the copy\n"
          "}\n"
          "\n"
          "int value = 5;\n"
          "tryToChange(value);\n"
          "printf(\"%d\\n\", value);     // still 5" },
        { "Looking ahead: changing the caller's variable",
          "So how could a function such as swap change two variables in main? There are two ways, and the lectures cover both:\n\n"
          "In C you pass a pointer - the variable's address - and the function changes what lives there (Pointers tutorial).\n"
          "In C++ you can also pass by reference with &, which makes the parameter another name for the original (References tutorial).",
          "void swap(int *a, int *b);   // C: pass by pointer, call swap(&x, &y)\n"
          "void swap(int &a, int &b);   // C++: pass by reference, call swap(x, y)" },
    },
    .quizCount = 4,
    .quiz = {
        { "What does add(2, 3) give?", "int add(int a, int b) {\n    return a + b;\n}", 4,
          { "23", "6", "5", "An error" }, 2,
          "a is 2 and b is 3, and the function returns a + b." },
        { "What does this print?", "void tryToChange(int x) { x = 999; }\n\nint value = 5;\ntryToChange(value);\nprintf(\"%d\", value);", 4,
          { "999", "5", "0", "An error" }, 1,
          "value is passed by value, so the function only changed its own copy." },
        { "After doubleAll(numbers, 5), what has happened to numbers in main?", NULL, 4,
          { "Nothing - the function had a copy", "Every element has been doubled", "Only the first element changed", "It was deleted" }, 1,
          "Arrays are not copied when passed: the function works on the original array." },
        { "Why do array functions usually take a size parameter too?", NULL, 4,
          { "Arrays must always be passed in pairs", "The function can't tell how many elements the array has", "To make it faster", "C requires two parameters" }, 1,
          "Inside the function, the array parameter doesn't know its own length, so you pass it in." },
    },
    .projectTitle = "Array Toolkit",
    .projectBrief =
        "main is written for you. It reads how many numbers there are (1 to 100) and the numbers, then calls three functions you need to write:\n\n"
        "printArray(arr, size) prints the numbers separated by spaces, then a new line.\n"
        "doubleAll(arr, size) doubles every number in the array.\n"
        "calculateAverage(arr, size) returns the average as a double.\n\n"
        "For  5  then  1 2 3 4 5  the output is:\n"
        "Before: 1 2 3 4 5\n"
        "After: 2 4 6 8 10\n"
        "Average: 6.00",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "void printArray(int arr[], int size)\n"
        "{\n"
        "    // Print each number followed by a space, then a new line\n"
        "}\n"
        "\n"
        "void doubleAll(int arr[], int size)\n"
        "{\n"
        "    // Double every element\n"
        "}\n"
        "\n"
        "double calculateAverage(int arr[], int size)\n"
        "{\n"
        "    // Add them up and return the average\n"
        "    return 0;\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int numbers[100];\n"
        "    int size;\n"
        "    scanf(\"%d\", &size);\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        scanf(\"%d\", &numbers[i]);\n"
        "    }\n"
        "\n"
        "    printf(\"Before: \");\n"
        "    printArray(numbers, size);\n"
        "    doubleAll(numbers, size);\n"
        "    printf(\"After: \");\n"
        "    printArray(numbers, size);\n"
        "    printf(\"Average: %.2f\\n\", calculateAverage(numbers, size));\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "printArray: for (int i = 0; i < size; i++) printf(\"%d \", arr[i]); then printf(\"\\n\");. "
            "calculateAverage: add the elements into int sum, then return (double)sum / size; - without the cast the decimals are lost.",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "void printArray(int arr[], int size)\n"
        "{\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        printf(\"%d \", arr[i]);\n"
        "    }\n"
        "    printf(\"\\n\");\n"
        "}\n"
        "\n"
        "void doubleAll(int arr[], int size)\n"
        "{\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        arr[i] = arr[i] * 2;\n"
        "    }\n"
        "}\n"
        "\n"
        "double calculateAverage(int arr[], int size)\n"
        "{\n"
        "    int sum = 0;\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        sum += arr[i];\n"
        "    }\n"
        "    return (double)sum / size;\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int numbers[100];\n"
        "    int size;\n"
        "    scanf(\"%d\", &size);\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        scanf(\"%d\", &numbers[i]);\n"
        "    }\n"
        "\n"
        "    printf(\"Before: \");\n"
        "    printArray(numbers, size);\n"
        "    doubleAll(numbers, size);\n"
        "    printf(\"After: \");\n"
        "    printArray(numbers, size);\n"
        "    printf(\"Average: %.2f\\n\", calculateAverage(numbers, size));\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "5\n1 2 3 4 5\n",   "Before: 1 2 3 4 5\nAfter: 2 4 6 8 10\nAverage: 6.00\n" },
        { "1\n7\n",           "Before: 7\nAfter: 14\nAverage: 14.00\n" },
        { "4\n1 2 2 2\n",     "Before: 1 2 2 2\nAfter: 2 4 4 4\nAverage: 3.50\n" },
        { "3\n-5 0 6\n",      "Before: -5 0 6\nAfter: -10 0 12\nAverage: 0.67\n" },
    },
},

// ===================================================================== 9
{
    .id = "c-pointers-2", .lang = LANG_C,
    .title   = "Pointers",
    .summary = "Memory addresses, & and *, pointers to pointers, pass by pointer and NULL.",
    .lecture = "Pointers",
    .section = "Pointers and Memory in C",
    .sectionBlurb = "Addresses, pointer arithmetic, malloc and free.",
    .pageCount = 7,
    .pages = {
        { "Memory and addresses",
          "A computer stores data in memory slots, and every slot has its own unique address - like house numbers on a street. "
          "When you declare a variable, it gets a slot, and changing the variable really means changing the contents of that slot.\n\n"
          "The lecture draws memory like this (addresses on the left of each box, contents on the right):",
          "Addr  Content     Addr  Content     Addr  Content\n"
          "1000  i: 37       1001  j: 46       1002  k: 58\n"
          "1004  a[0]: 'a'   1005  a[1]: 'b'   1006  a[2]: '\\0'\n"
          "1008  ptr: 1001                     <- ptr holds j's address" },
        { "What is a pointer?",
          "A pointer is a variable that stores the address of another variable. It does NOT store the value itself - it \"points at\" "
          "where the value lives. Declare one by putting * between the type and the name: int* ptr is a pointer to an int.",
          "int i = 5;\n"
          "int* ptr;      // a pointer to an int (no address in it yet)\n"
          "ptr = &i;      // store the address of i in ptr\n"
          "*ptr = 10;     // change what ptr points to: i is now 10" },
        { "The twin operators & and *",
          "& is the address-of operator: &i gives the address of i.\n"
          "* is the dereference operator: *ptr means \"the thing ptr points to\". You can read it or assign to it.\n\n"
          "printf prints an address with %p. The exact number changes every run - what matters is which variable it belongs to.",
          "int i = 5;\n"
          "int* ptr = &i;\n"
          "printf(\"i = %d\\n\", i);          // i = 5\n"
          "printf(\"*ptr = %d\\n\", *ptr);    // *ptr = 5\n"
          "printf(\"ptr = %p\\n\", (void*)ptr);   // an address like 0x7ffe..." },
        { "Following the changes",
          "A pointer can be moved to point at a different variable. Work through this lecture example line by line, keeping track of i, j, ptr and *ptr:\n\n"
          "After ptr = &i; *ptr = 3; i is 3. Then ptr = &j; moves the pointer, so *ptr = -2; changes j. At the end i is 3 and j is -2.",
          "int i = 5, j = 10;\n"
          "int* ptr;\n"
          "ptr = &i;      // ptr -> i\n"
          "*ptr = 3;      // i = 3\n"
          "ptr = &j;      // ptr -> j\n"
          "*ptr = -2;     // j = -2" },
        { "Pointers to pointers",
          "A pointer is a variable too, so it has its own address - and a pointer to a pointer (int**) can store it. "
          "*pptr is the pointer it points to, and **pptr follows both arrows to the int at the end.",
          "int i = 5, j = 10;\n"
          "int* ptr = &i;\n"
          "int** pptr = &ptr;   // pptr -> ptr -> i\n"
          "**pptr = 7;          // i = 7\n"
          "*pptr = &j;          // changes ptr: now ptr -> j\n"
          "*ptr = 9;            // j = 9" },
        { "Pass by pointer",
          "C passes arguments by value, so f(i) can never change i. But if you pass the address, &i, the function gets a pointer "
          "and can change the variable through it. This is how scanf fills in your variables - and how swap works.",
          "void f(int j)    { j = 5; }      // f(i): i stays 3\n"
          "void g(int *ptr) { *ptr = 5; }   // g(&i): i becomes 5\n"
          "\n"
          "void swap(int *x, int *y) {\n"
          "    int z = *x;\n"
          "    *x = *y;\n"
          "    *y = z;\n"
          "}" },
        { "NULL and nullptr",
          "An uninitialised pointer can contain any address, and using it is a serious bug. Set a pointer that doesn't point at anything yet to NULL "
          "(in C) or nullptr (in C++), and check before you dereference it.\n\n"
          "Pros of pointers: efficiency and convenience. Cons: they are error-prone and hard to debug - so be careful.",
          "int *ptr = NULL;        // C   (C++: int *ptr = nullptr;)\n"
          "...\n"
          "if (ptr != NULL) {\n"
          "    printf(\"%d\\n\", *ptr);   // only safe when it points somewhere\n"
          "}" },
    },
    .quizCount = 4,
    .quiz = {
        { "What does &x give you?", NULL, 4,
          { "The value of x", "The address of x", "x times 2", "A copy of x" }, 1,
          "& is the address-of operator." },
        { "What does this print?", "int x = 4;\nint *p = &x;\n*p = 9;\nprintf(\"%d\\n\", x);", 4,
          { "4", "An address", "9", "An error" }, 2,
          "p points at x, so *p = 9 changes x itself." },
        { "What are i and j at the end?", "int i = 2, j = 8;\nint *ptr = &i;\n*ptr = 7;\nptr = &j;\n*ptr = -25;", 4,
          { "i = 7, j = -25", "i = -25, j = 8", "i = 2, j = -25", "i = 7, j = 8" }, 0,
          "*ptr = 7 changes i. Then ptr moves to j, so *ptr = -25 changes j." },
        { "pptr = &ptr and ptr = &i. What is **pptr = 12 the same as?", NULL, 4,
          { "ptr = 12", "pptr = 12", "i = 12", "Nothing - it is an error" }, 2,
          "*pptr is ptr, and *ptr is i, so **pptr is i." },
    },
    .projectTitle = "Sort Three",
    .projectBrief =
        "Write two functions that work through pointers:\n\n"
        "swap(int *a, int *b) exchanges the two values.\n"
        "sortThree(int *a, int *b, int *c) uses swap so that afterwards *a <= *b <= *c.\n\n"
        "main is written: it reads three numbers, prints them, calls sortThree(&x, &y, &z) and prints them again. For 3 1 2:\n\n"
        "Before: 3 1 2\n"
        "After: 1 2 3",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "void swap(int *a, int *b)\n"
        "{\n"
        "    // Exchange *a and *b\n"
        "}\n"
        "\n"
        "void sortThree(int *a, int *b, int *c)\n"
        "{\n"
        "    // Use swap so that *a <= *b <= *c\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int x, y, z;\n"
        "    scanf(\"%d %d %d\", &x, &y, &z);\n"
        "    printf(\"Before: %d %d %d\\n\", x, y, z);\n"
        "    sortThree(&x, &y, &z);\n"
        "    printf(\"After: %d %d %d\\n\", x, y, z);\n"
        "    return 0;\n"
        "}\n",
    .hint = "swap: int temp = *a; *a = *b; *b = temp;. sortThree needs three compare-and-swaps: "
            "if (*a > *b) swap(a, b); if (*b > *c) swap(b, c); if (*a > *b) swap(a, b); - a, b and c are already pointers, so pass them without &.",
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
        "void sortThree(int *a, int *b, int *c)\n"
        "{\n"
        "    if (*a > *b) swap(a, b);\n"
        "    if (*b > *c) swap(b, c);\n"
        "    if (*a > *b) swap(a, b);\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int x, y, z;\n"
        "    scanf(\"%d %d %d\", &x, &y, &z);\n"
        "    printf(\"Before: %d %d %d\\n\", x, y, z);\n"
        "    sortThree(&x, &y, &z);\n"
        "    printf(\"After: %d %d %d\\n\", x, y, z);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 5,
    .tests = {
        { "3 1 2\n",    "Before: 3 1 2\nAfter: 1 2 3\n" },
        { "1 2 3\n",    "Before: 1 2 3\nAfter: 1 2 3\n" },
        { "9 5 1\n",    "Before: 9 5 1\nAfter: 1 5 9\n" },
        { "4 4 -2\n",   "Before: 4 4 -2\nAfter: -2 4 4\n" },
        { "2 8 5\n",    "Before: 2 8 5\nAfter: 2 5 8\n" },
    },
},

// ===================================================================== 10
{
    .id = "c-pointer-arith", .lang = LANG_C,
    .title   = "Pointer Arithmetic and Arrays",
    .summary = "Moving pointers with + and -, why a[i] is *(a + i), and walking arrays with pointers.",
    .lecture = "Pointers",
    .pageCount = 7,
    .pages = {
        { "Adding to a pointer",
          "ptr + 1 is the next memory location and ptr - 1 the previous one. ptr++ and ptr += 2 move the pointer itself.\n\n"
          "Only adding and subtracting make sense: ptr * 2 and ptr / 2 are invalid operations and won't compile.",
          "float a[4];\n"
          "float* ptr = &(a[2]);   // ptr -> a[2]\n"
          "*ptr = 3.14;            // a[2] = 3.14\n"
          "ptr++;                  // ptr -> a[3]\n"
          "*ptr = 9.0;             // a[3] = 9.0" },
        { "Walking up and down an array",
          "Carrying on from the last page: ptr - 3 jumps from a[3] back to a[0], and += 2 moves forward to a[2] again.\n\n"
          "At the end of the lecture example: a[0] = 6.0, a[1] is still unset, a[2] = 7.0 and a[3] = 9.0.",
          "ptr = ptr - 3;          // ptr -> a[0]\n"
          "*ptr = 6.0;             // a[0] = 6.0\n"
          "ptr += 2;               // ptr -> a[2]\n"
          "*ptr = 7.0;             // a[2] = 7.0 (replaces 3.14)" },
        { "An array name is an address",
          "The name of an array is the address of its first element, so a means &a[0]. That's why a[2] and *(a + 2) are exactly the same thing, "
          "and ptr = &(a[2]) can be written ptr = a + 2.\n\n"
          "The difference: a is a constant address (you can't do a++), while ptr is a pointer variable you can move.",
          "a[2]          is the same as   *(a + 2)\n"
          "ptr = &(a[2]) is the same as   ptr = a + 2\n"
          "\n"
          "int arr[] = {4, 6, 7};\n"
          "int *p = arr;          // no & needed: arr is already an address\n"
          "printf(\"%d\\n\", *(p + 1));   // 6" },
        { "Steps are sized by type",
          "A double takes more memory than a char, so how far does ptr + 1 move? The compiler adjusts automatically: + and - move by "
          "the size of the type the pointer points to. With 8-byte doubles starting at address 1000, ptr + 1 is 1008, not 1001.\n\n"
          "So ptr + 1 always means \"the next element\", whatever the type.",
          "double a[3] = {37.9, 1.23, 3.14};\n"
          "double *ptr = a;       // say a starts at address 1000\n"
          "ptr + 1;               // 1000 + sizeof(double) = 1008 -> a[1]\n"
          "ptr + 2;               // 1016 -> a[2]" },
        { "Looping with a pointer",
          "Lecture exercise: output the integers of an array using pointer arithmetic. Start a pointer at the array and move it along, "
          "stopping when it reaches one past the end (arr + len). Functions often take a pointer plus a count in place of an array.",
          "int arr[] = {4, 6, 7, 8, 9};\n"
          "int len = 5;\n"
          "for (int *ptr = arr; ptr < arr + len; ptr++) {\n"
          "    printf(\"%d \", *ptr);\n"
          "}\n"
          "\n"
          "int sum(int *ptr, int numValues);   // sum(arr, 5) is 34" },
        { "Stopping at a 0",
          "Another lecture exercise: add up array elements until you reach a 0. The loop has no counter at all - it just moves the pointer "
          "and checks what it points at. C strings work the same way, stopping at the '\\0' character.",
          "int a[] = {4, 6, 7, 8, 9, 0};\n"
          "int sum = 0;\n"
          "for (int *ptr = a; *ptr != 0; ptr++) {\n"
          "    sum += *ptr;\n"
          "}\n"
          "printf(\"%d\\n\", sum);   // 34" },
        { "Extra: Const pointers",
          "When a function only needs to read through a pointer, mark it const: const int *p lets you move p but not change *p. "
          "The compiler then stops you modifying the caller's data by accident, and anyone reading the prototype knows the function is read-only.\n\n"
          "Pointers can also be compared with <, == and so on when they point into the same array, and subtracted to count the elements between them.",
          "int sum(const int *ptr, int n) {\n"
          "    int total = 0;\n"
          "    for (const int *end = ptr + n; ptr < end; ptr++) {\n"
          "        total += *ptr;    // reading is fine\n"
          "        // *ptr = 0;      // compile error: *ptr is const\n"
          "    }\n"
          "    return total;\n"
          "}" },
    },
    .quizCount = 4,
    .quiz = {
        { "arr[3] is the same as which expression?", NULL, 4,
          { "*arr + 3", "&arr + 3", "*(arr + 3)", "arr * 3" }, 2,
          "Add 3 to the address first, then dereference: *(arr + 3)." },
        { "Which of these is NOT a valid pointer operation?", NULL, 4,
          { "ptr + 1", "ptr - 1", "ptr++", "ptr * 2" }, 3,
          "You can only add to or subtract from a pointer. Multiplying or dividing one is invalid." },
        { "What does this print?", "int a[] = {10, 20, 30, 40};\nint *p = a + 1;\np += 2;\nprintf(\"%d\", *p);", 4,
          { "20", "30", "40", "An address" }, 2,
          "p starts at a[1], and += 2 moves it to a[3], which holds 40." },
        { "double *p points at address 1000 and a double is 8 bytes. What address is p + 1?", NULL, 4,
          { "1001", "1004", "1008", "1800" }, 2,
          "Pointer arithmetic moves by the size of the type: 1000 + 8." },
    },
    .projectTitle = "Pointer Walk",
    .projectBrief =
        "Read whole numbers into an array until you read a 0 (at most 100 numbers; the 0 isn't stored). "
        "Then, using pointers rather than [ ] indexes:\n\n"
        "print the count, print the numbers forwards and then backwards, and print their sum using a function int sum(int *ptr, int numValues).\n\n"
        "For  4 6 7 0  the output is:\n"
        "Count: 3\n"
        "Forward: 4 6 7\n"
        "Backward: 7 6 4\n"
        "Sum: 17",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int sum(int *ptr, int numValues)\n"
        "{\n"
        "    // Add up numValues values using pointer arithmetic\n"
        "    return 0;\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int numbers[100];\n"
        "    int count = 0;\n"
        "    int value;\n"
        "\n"
        "    // Read values into numbers until you read a 0\n"
        "\n"
        "    // Print Count, Forward, Backward and Sum using pointers\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Forward: for (int *p = numbers; p < numbers + count; p++) printf(\" %d\", *p);. "
            "Backward: start at the last element, int *p = numbers + count - 1, and loop while p >= numbers with p--. "
            "In sum, loop numValues times doing total += *ptr; ptr++;",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int sum(int *ptr, int numValues)\n"
        "{\n"
        "    int total = 0;\n"
        "    for (int i = 0; i < numValues; i++) {\n"
        "        total += *ptr;\n"
        "        ptr++;\n"
        "    }\n"
        "    return total;\n"
        "}\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int numbers[100];\n"
        "    int count = 0;\n"
        "    int value;\n"
        "\n"
        "    while (count < 100 && scanf(\"%d\", &value) == 1 && value != 0) {\n"
        "        *(numbers + count) = value;\n"
        "        count++;\n"
        "    }\n"
        "\n"
        "    printf(\"Count: %d\\n\", count);\n"
        "\n"
        "    printf(\"Forward:\");\n"
        "    for (int *p = numbers; p < numbers + count; p++) {\n"
        "        printf(\" %d\", *p);\n"
        "    }\n"
        "    printf(\"\\n\");\n"
        "\n"
        "    printf(\"Backward:\");\n"
        "    for (int *p = numbers + count - 1; p >= numbers; p--) {\n"
        "        printf(\" %d\", *p);\n"
        "    }\n"
        "    printf(\"\\n\");\n"
        "\n"
        "    printf(\"Sum: %d\\n\", sum(numbers, count));\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "4 6 7 0\n",        "Count: 3\nForward: 4 6 7\nBackward: 7 6 4\nSum: 17\n" },
        { "4 6 7 8 9 0\n",    "Count: 5\nForward: 4 6 7 8 9\nBackward: 9 8 7 6 4\nSum: 34\n" },
        { "-3 0 5 5\n",       "Count: 1\nForward: -3\nBackward: -3\nSum: -3\n" },
        { "0\n",              "Count: 0\nForward:\nBackward:\nSum: 0\n" },
    },
},

// ===================================================================== 11
{
    .id = "c-malloc", .lang = LANG_C,
    .title   = "Memory Management in C",
    .summary = "Scope and lifetime, dangling pointers, the stack and the heap, malloc and free.",
    .lecture = "Memory Management",
    .pageCount = 7,
    .pages = {
        { "Scope and memory",
          "Whenever you declare a variable, memory is allocated for it. That memory is freed for reuse when the variable goes out of scope - "
          "when the program leaves the curly braces it was declared in. After that, the memory is no longer guaranteed to hold the value.",
          "int main(void) {\n"
          "    if (1) {\n"
          "        int x = 5;\n"
          "    }\n"
          "    // x is out of scope here: its memory can be reused\n"
          "}" },
        { "Dangling pointers",
          "A pointer that still holds the address of memory that has been freed is a dangling pointer. Its contents are undefined - "
          "you might see 5, garbage, or a crash.\n\n"
          "The classic mistake is returning the address of a local variable: x disappears as soon as the function returns.",
          "int* getPtrToFive(void) {\n"
          "    int x = 5;\n"
          "    return &x;     // WRONG: x dies when the function ends\n"
          "}\n"
          "\n"
          "int *p = getPtrToFive();\n"
          "printf(\"%d\\n\", *p);   // ??? undefined" },
        { "The stack and the heap",
          "Ordinary local variables live on the stack, and are freed automatically when they go out of scope.\n\n"
          "Memory you ask for yourself comes from the heap. It stays allocated until you hand it back - even after the function that asked for it returns. "
          "In C you ask with malloc and hand it back with free (C++ uses new and delete).",
          "int x = 5;                            // stack: freed automatically\n"
          "int *p = (int*)malloc(sizeof(int));   // heap: yours until free(p)" },
        { "malloc and free",
          "malloc(bytes) from <stdlib.h> returns a pointer to that many bytes of heap memory - use sizeof to get the size right. "
          "If there isn't enough memory it returns NULL, so always check. When you're done, free gives the memory back.",
          "#include <stdlib.h>\n"
          "\n"
          "int *ptr = (int*)malloc(sizeof(int));\n"
          "if (ptr == NULL) {\n"
          "    printf(\"Memory allocation failed!\\n\");\n"
          "    return 1;\n"
          "}\n"
          "*ptr = 42;\n"
          "printf(\"Value: %d\\n\", *ptr);\n"
          "free(ptr);" },
        { "Dynamic arrays",
          "An array declared normally needs its size fixed when you write the code. With malloc you can choose the size while the program runs - "
          "for example from the user's input. Ask for size * sizeof(int) bytes, then use the pointer exactly like an array.",
          "int size;\n"
          "scanf(\"%d\", &size);\n"
          "int *arr = (int*)malloc(size * sizeof(int));\n"
          "if (arr == NULL) return 1;\n"
          "for (int i = 0; i < size; i++) {\n"
          "    arr[i] = i * 10;\n"
          "}\n"
          "free(arr);" },
        { "The rules to remember",
          "1. Always check if malloc returns NULL - allocation can fail.\n"
          "2. Call free once for every successful malloc. Memory you never free is a memory leak.\n"
          "3. Don't use memory after freeing it - that is undefined behaviour.\n"
          "4. Set a pointer to NULL after freeing it, so a mistaken second use is easy to spot.\n"
          "5. Only free memory that came from malloc - never the address of an ordinary variable.",
          "free(arr);\n"
          "arr = NULL;      // good practice\n"
          "\n"
          "int x = 5;\n"
          "free(&x);        // WRONG: x was not malloc'd" },
        { "Extra: calloc and realloc",
          "calloc(count, size) is like malloc but sets every byte to zero. realloc(ptr, newBytes) resizes an existing block - "
          "growing an array without losing what's in it. realloc may move the data and returns the new address (or NULL on failure), "
          "so store it in a temporary pointer first.",
          "int *nums = (int*)calloc(5, sizeof(int));   // five zeros\n"
          "\n"
          "int *bigger = (int*)realloc(nums, 10 * sizeof(int));\n"
          "if (bigger != NULL) {\n"
          "    nums = bigger;     // now room for 10, first 5 kept\n"
          "}\n"
          "free(nums);" },
    },
    .quizCount = 4,
    .quiz = {
        { "What does malloc return when it can't allocate the memory?", NULL, 4,
          { "0 bytes", "NULL", "-1", "It stops the program" }, 1,
          "malloc returns NULL on failure, which is why you check it before using the pointer." },
        { "What is wrong with this function?", "int* getPtrToFive(void) {\n    int x = 5;\n    return &x;\n}", 4,
          { "Nothing", "It should return x", "x goes out of scope, so the pointer dangles", "It leaks memory" }, 2,
          "x is a local on the stack and is gone once the function returns." },
        { "What is a memory leak?", NULL, 4,
          { "Reading past the end of an array", "Heap memory that is never freed", "A pointer set to NULL", "Using too many variables" }, 1,
          "Memory you allocate and never free stays used until the program ends." },
        { "How many bytes does malloc(10 * sizeof(int)) ask for when an int is 4 bytes?", NULL, 4,
          { "10", "14", "40", "4" }, 2,
          "10 ints of 4 bytes each is 40 bytes." },
    },
    .projectTitle = "How Many Numbers?",
    .projectBrief =
        "This is the lecture's exercise, in C. Read how many numbers there will be, allocate exactly enough memory for them with malloc, "
        "then read them in.\n\n"
        "Print them in order, then in reverse, then the largest one, and free the memory. For  3  then  7 4 9:\n\n"
        "You entered: 7 4 9\n"
        "Reversed: 9 4 7\n"
        "Largest: 9\n\n"
        "If the count is 0 or less, print  Nothing to store  and don't call malloc. If malloc returns NULL, print  Out of memory  and return 1.",
    .starterCode =
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int count;\n"
        "    scanf(\"%d\", &count);\n"
        "\n"
        "    // Allocate an int array of size count with malloc (check for NULL)\n"
        "\n"
        "    // Read the numbers, print them, then free the memory\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "int *numbers = (int*)malloc(count * sizeof(int)); then if (numbers == NULL) { printf(\"Out of memory\\n\"); return 1; }. "
            "Use numbers[i] like a normal array. Finish with free(numbers);",
    .solution =
        "#include <stdio.h>\n"
        "#include <stdlib.h>\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    int count;\n"
        "    scanf(\"%d\", &count);\n"
        "\n"
        "    if (count <= 0) {\n"
        "        printf(\"Nothing to store\\n\");\n"
        "        return 0;\n"
        "    }\n"
        "\n"
        "    int *numbers = (int*)malloc(count * sizeof(int));\n"
        "    if (numbers == NULL) {\n"
        "        printf(\"Out of memory\\n\");\n"
        "        return 1;\n"
        "    }\n"
        "\n"
        "    for (int i = 0; i < count; i++) {\n"
        "        scanf(\"%d\", &numbers[i]);\n"
        "    }\n"
        "\n"
        "    printf(\"You entered:\");\n"
        "    for (int i = 0; i < count; i++) {\n"
        "        printf(\" %d\", numbers[i]);\n"
        "    }\n"
        "    printf(\"\\n\");\n"
        "\n"
        "    printf(\"Reversed:\");\n"
        "    for (int i = count - 1; i >= 0; i--) {\n"
        "        printf(\" %d\", numbers[i]);\n"
        "    }\n"
        "    printf(\"\\n\");\n"
        "\n"
        "    int largest = numbers[0];\n"
        "    for (int i = 1; i < count; i++) {\n"
        "        if (numbers[i] > largest) {\n"
        "            largest = numbers[i];\n"
        "        }\n"
        "    }\n"
        "    printf(\"Largest: %d\\n\", largest);\n"
        "\n"
        "    free(numbers);\n"
        "    numbers = NULL;\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "3\n7 4 9\n",         "You entered: 7 4 9\nReversed: 9 4 7\nLargest: 9\n" },
        { "1\n-6\n",            "You entered: -6\nReversed: -6\nLargest: -6\n" },
        { "0\n",                "Nothing to store\n" },
        { "6\n5 3 8 8 1 2\n",   "You entered: 5 3 8 8 1 2\nReversed: 2 1 8 8 3 5\nLargest: 8\n" },
    },
},

// ===================================================================== 12
{
    .id = "c-scope-funcptr", .lang = LANG_C,
    .title   = "Globals and Function Pointers",
    .summary = "Global and local variables, shadowing, static locals and calling functions through pointers.",
    .lecture = "Memory Management",
    .pageCount = 6,
    .pages = {
        { "Global variables",
          "A variable declared outside every function is global: it exists for the whole run of the program and can be used in any function below it. "
          "Locals, declared inside a function or block, exist only there.\n\n"
          "Globals are convenient but easy to misuse - any function can change them - so use them sparingly.",
          "int glVar = 10;          // global\n"
          "\n"
          "void change(void) {\n"
          "    glVar = 20;          // changes the global\n"
          "}\n"
          "\n"
          "int main(void) {\n"
          "    change();\n"
          "    printf(\"%d\\n\", glVar);   // 20\n"
          "}" },
        { "Shadowing",
          "If a function declares a local variable with the same name as a global, the local one hides (shadows) the global inside that function. "
          "The compiler creates a new local glVar and only changes that - the global is untouched. This is a common source of confusing bugs.",
          "int glVar = 10;\n"
          "\n"
          "void change(void) {\n"
          "    int glVar;           // a NEW local variable\n"
          "    glVar = 20;          // changes the local one only\n"
          "}\n"
          "// after change(), the global glVar is still 10" },
        { "Extra: Static local variables",
          "A local variable marked static is created once and keeps its value between calls, but can still only be used inside its function. "
          "It's a safer alternative to a global when one function needs to remember something, such as how many times it has been called.",
          "int nextTicket(void) {\n"
          "    static int count = 0;   // set to 0 only once\n"
          "    count++;\n"
          "    return count;\n"
          "}\n"
          "// nextTicket() returns 1, then 2, then 3..." },
        { "Function pointers",
          "Functions live in memory too, so you can store a function's address in a function pointer and call the function through it. "
          "The pointer's declaration must match the function's return type and parameter types. "
          "You can assign and call function pointers, but you can't do arithmetic on them.",
          "int Add(int num1, int num2) {\n"
          "    return num1 + num2;\n"
          "}\n"
          "\n"
          "int (*functionPtr)(int, int);   // pointer to a function taking 2 ints\n"
          "functionPtr = Add;\n"
          "int result = functionPtr(20, 10);   // 30" },
        { "Choosing a function while running",
          "Function pointers let a program decide which function to call while it runs - for example from the user's input - "
          "without a long if/else every time it is called. You can even keep them in an array.",
          "int Add(int a, int b) { return a + b; }\n"
          "int Sub(int a, int b) { return a - b; }\n"
          "\n"
          "int (*op)(int, int) = NULL;\n"
          "if (choice == '+') op = Add;\n"
          "if (choice == '-') op = Sub;\n"
          "if (op != NULL) printf(\"%d\\n\", op(7, 3));" },
        { "Extra: Callbacks with qsort",
          "A function you hand to another function to call is a callback. The standard qsort from <stdlib.h> sorts any array, and you pass a "
          "comparison function that returns a negative number, 0 or a positive number. C++'s sort with a comparator, in the STL tutorials, is the same idea.",
          "int compareInts(const void *a, const void *b) {\n"
          "    return *(const int*)a - *(const int*)b;\n"
          "}\n"
          "\n"
          "int nums[] = {5, 2, 9, 1};\n"
          "qsort(nums, 4, sizeof(int), compareInts);   // 1 2 5 9" },
    },
    .quizCount = 3,
    .quiz = {
        { "What does main print?", "int glVar = 10;\nvoid change(void) { int glVar; glVar = 20; }\n\nint main(void) {\n    change();\n    printf(\"%d\", glVar);\n}", 4,
          { "20", "10", "0", "Garbage" }, 1,
          "change declares its own local glVar, which shadows the global. The global is still 10." },
        { "Which declares a pointer to int Add(int, int)?", NULL, 4,
          { "int *Add(int, int);", "int (*fp)(int, int);", "int fp(int*, int*);", "pointer<int> fp;" }, 1,
          "The brackets around *fp make it a pointer to a function, rather than a function returning int*." },
        { "Which operation is allowed on a function pointer?", NULL, 4,
          { "fp + 1", "fp++", "Calling it: fp(2, 3)", "fp * 2" }, 2,
          "Function pointers can be assigned and called, but not used in arithmetic." },
    },
    .projectTitle = "Function Pointer Calculator",
    .projectBrief =
        "The four functions add, subtract, multiply and divide are written for you. Each one increments the global variable calls.\n\n"
        "In main, read sums like  6 * 7  until the input runs out. For each one, pick the right function by storing it in the function pointer "
        "operation, then call it through the pointer and print the result with %g:\n\n"
        "6 * 7 = 42\n\n"
        "If the operator is unknown, print  Unknown operator: x . For division by 0, print  Error: divide by zero  without calling divide. "
        "At the end, print the global count:  Calls: 1",
    .starterCode =
        "#include <stdio.h>\n"
        "\n"
        "int calls = 0;   // global: how many times an operation ran\n"
        "\n"
        "double add(double a, double b)      { calls++; return a + b; }\n"
        "double subtract(double a, double b) { calls++; return a - b; }\n"
        "double multiply(double a, double b) { calls++; return a * b; }\n"
        "double divide(double a, double b)   { calls++; return a / b; }\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    double a, b;\n"
        "    char op;\n"
        "    double (*operation)(double, double);\n"
        "\n"
        "    // Read sums until the input runs out, choose operation, call it\n"
        "\n"
        "    printf(\"Calls: %d\\n\", calls);\n"
        "    return 0;\n"
        "}\n",
    .hint = "while (scanf(\"%lf %c %lf\", &a, &op, &b) == 3) { operation = NULL; if (op == '+') operation = add; ... } "
            "Check the divide-by-zero case before calling. Then if (operation == NULL) print the unknown message, "
            "else printf(\"%g %c %g = %g\\n\", a, op, b, operation(a, b));",
    .solution =
        "#include <stdio.h>\n"
        "\n"
        "int calls = 0;   // global: how many times an operation ran\n"
        "\n"
        "double add(double a, double b)      { calls++; return a + b; }\n"
        "double subtract(double a, double b) { calls++; return a - b; }\n"
        "double multiply(double a, double b) { calls++; return a * b; }\n"
        "double divide(double a, double b)   { calls++; return a / b; }\n"
        "\n"
        "int main(void)\n"
        "{\n"
        "    double a, b;\n"
        "    char op;\n"
        "    double (*operation)(double, double);\n"
        "\n"
        "    while (scanf(\"%lf %c %lf\", &a, &op, &b) == 3) {\n"
        "        operation = NULL;\n"
        "        switch (op) {\n"
        "            case '+': operation = add; break;\n"
        "            case '-': operation = subtract; break;\n"
        "            case '*': operation = multiply; break;\n"
        "            case '/': operation = divide; break;\n"
        "        }\n"
        "\n"
        "        if (operation == NULL) {\n"
        "            printf(\"Unknown operator: %c\\n\", op);\n"
        "        } else if (op == '/' && b == 0) {\n"
        "            printf(\"Error: divide by zero\\n\");\n"
        "        } else {\n"
        "            printf(\"%g %c %g = %g\\n\", a, op, b, operation(a, b));\n"
        "        }\n"
        "    }\n"
        "\n"
        "    printf(\"Calls: %d\\n\", calls);\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "6 * 7\n",                    "6 * 7 = 42\nCalls: 1\n" },
        { "1 + 2\n10 / 4\n9 - 12\n",    "1 + 2 = 3\n10 / 4 = 2.5\n9 - 12 = -3\nCalls: 3\n" },
        { "5 / 0\n3 x 4\n",             "Error: divide by zero\nUnknown operator: x\nCalls: 0\n" },
        { "",                           "Calls: 0\n" },
    },
},

// ===================================================================== 13
{
    .id = "c-structs", .lang = LANG_C,
    .title   = "Structs",
    .summary = "Grouping related data into your own types, and arrays of them.",
    .lecture = "Classes",
    .pageCount = 5,
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
        { "Extra: Pointers to structs",
          "With a pointer to a struct, reach its fields with -> instead of the dot: p->level means (*p).level. "
          "You'll use this all the time - with structs made by malloc, with linked list nodes, and with C++ objects, which the Classes lecture reaches the same way. "
          "(The Classes lecture also explains that C++ classes grew out of C structs.)",
          "#include <stdlib.h>\n"
          "\n"
          "struct Player *p = (struct Player*)malloc(sizeof(struct Player));\n"
          "if (p != NULL) {\n"
          "    p->level = 3;      // same as (*p).level = 3\n"
          "    p->health = 100;\n"
          "    free(p);\n"
          "}" },
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

// ===================================================================== 14
{
    .id = "c-files", .lang = LANG_C,
    .title   = "Files (extra topic)",
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
