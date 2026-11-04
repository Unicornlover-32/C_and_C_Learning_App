#include "tutorials.h"
#include <stddef.h>

// ---------------------------------------------------------------------------
// C++ sections, following the lecture notes: C++ and Classes, then Templates
// and the STL. Projects here are compiled with g++ (-std=c++17).
// ---------------------------------------------------------------------------

const Tutorial CPP_TUTORIALS[] = {

// ===================================================================== 1
{
    .id = "cpp-hello", .lang = LANG_CPP,
    .title   = "Hello, C++",
    .summary = "From C to C++: printing with cout and reading with cin.",
    .section = "C++ and Classes",
    .sectionBlurb = "References, new and delete, classes, linked lists and operators.",
    .pageCount = 4,
    .pages = {
        { "From C to C++",
          "C++ started life as \"C with classes\". Nearly all the C you know still works, but C++ adds easier text, "
          "ready-made containers, classes and much more.\n\n"
          "C++ files usually end in .cpp and are compiled with g++ instead of gcc. This app takes care of that for you.",
          NULL },
        { "Printing with cout",
          "#include <iostream> gives you std::cout for printing. Send things to it with <<, one after another. "
          "There are no format specifiers - C++ works out each value's type by itself.",
          "#include <iostream>\n"
          "\n"
          "int main()\n"
          "{\n"
          "    int age = 21;\n"
          "    std::cout << \"Age: \" << age << \"\\n\";\n"
          "    return 0;\n"
          "}" },
        { "Reading with cin",
          "std::cin reads input. Use >> to read into a variable - again no & and no format specifier. "
          "You can read several values in one line.",
          "int a, b;\n"
          "std::cin >> a >> b;\n"
          "std::cout << \"Sum: \" << a + b << \"\\n\";" },
        { "using namespace std",
          "std:: means \"from the standard library\". Writing using namespace std; near the top lets you leave it off. "
          "That is fine for small programs like these; big projects usually keep the std:: to avoid name clashes.",
          "#include <iostream>\n"
          "using namespace std;\n"
          "\n"
          "int main()\n"
          "{\n"
          "    cout << \"No std:: needed\\n\";\n"
          "    return 0;\n"
          "}" },
    },
    .quizCount = 3,
    .quiz = {
        { "Which header gives you std::cout?", NULL, 4,
          { "<stdio.h>", "<cout>", "<iostream>", "<string>" }, 2,
          "<iostream> holds cout and cin. <stdio.h> is C's printf header." },
        { "Which operator sends values to cout?", NULL, 4,
          { "<<", ">>", "->", "::" }, 0,
          "<< points towards cout: values flow into the output. >> is used with cin." },
        { "What does the std:: in std::cout mean?", NULL, 4,
          { "Standard deviation", "It is a pointer", "It is a comment", "It comes from the standard library" }, 3,
          "std is the namespace that holds the C++ standard library." },
    },
    .projectTitle = "Rectangle",
    .projectBrief =
        "Read a rectangle's width and height (whole numbers) with cin. Use cout to print its area and perimeter.\n\n"
        "For the input 4 5 the output should be:\n\n"
        "Area: 20\n"
        "Perimeter: 18",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int width, height;\n"
        "\n"
        "    // Read width and height with cin\n"
        "\n"
        "    // Print the area and perimeter with cout\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "cin >> width >> height; reads both. Area is width * height, perimeter is 2 * (width + height). Print with cout << \"Area: \" << ... << \"\\n\";",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int width, height;\n"
        "    cin >> width >> height;\n"
        "\n"
        "    cout << \"Area: \" << width * height << \"\\n\";\n"
        "    cout << \"Perimeter: \" << 2 * (width + height) << \"\\n\";\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "4 5\n",  "Area: 20\nPerimeter: 18\n" },
        { "1 1\n",  "Area: 1\nPerimeter: 4\n" },
        { "10 3\n", "Area: 30\nPerimeter: 26\n" },
        { "7 7\n",  "Area: 49\nPerimeter: 28\n" },
    },
},

// ===================================================================== 2
{
    .id = "cpp-references", .lang = LANG_CPP,
    .title   = "Pass by Reference",
    .summary = "References with &, value vs reference vs pointer, swap, and const references.",
    .lecture = "Intro to C, Pointers",
    .pageCount = 7,
    .pages = {
        { "Recap: pass by value",
          "Ordinary arguments are passed by value: the function gets a copy, so changing the parameter does nothing to the caller's variable.",
          "void addOneByValue(int x) {\n"
          "    x = x + 1;      // changes the copy\n"
          "}\n"
          "\n"
          "int a = 5;\n"
          "addOneByValue(a);   // a is still 5" },
        { "Pass by reference",
          "C++ adds pass by reference. Put & after the parameter's type and the function does not get a copy: the parameter becomes another "
          "name for the caller's actual variable, so any change is a change to the original. C has no references - there you use pointers.",
          "void addOneByReference(int& x) {\n"
          "    x = x + 1;      // changes the caller's variable\n"
          "}\n"
          "\n"
          "int a = 5;\n"
          "addOneByReference(a);   // a is now 6" },
        { "Why use pass by reference?",
          "1. To modify the caller's variable - a swap function is impossible with pass by value.\n"
          "2. To avoid copying large data. Passing a big object by value copies the whole thing; a reference doesn't.\n"
          "3. To return several values. A function can only return one value, but it can change several reference parameters.",
          "void swap(int &a, int &b) {\n"
          "    int temp = a;\n"
          "    a = b;\n"
          "    b = temp;\n"
          "}\n"
          "\n"
          "void minMax(int x, int y, int &smallest, int &largest);" },
        { "Reference or pointer?",
          "Both let a function change the caller's variable, and they are very similar. Pass by pointer needs &a at the call and *x inside; "
          "pass by reference looks like ordinary code and is seen as neater.\n\n"
          "The big difference: a pointer can be nullptr, a reference cannot. Use references unless you need to allow \"nothing\".",
          "void swap(int* x, int* y) { int z = *x; *x = *y; *y = z; }\n"
          "void swap(int& x, int& y) { int z = x;  x = y;   y = z;  }\n"
          "\n"
          "swap(&a, &b);   // calls the pointer version\n"
          "swap(a, b);     // calls the reference version" },
        { "const references",
          "When a function only needs to read a large object, pass it as a const reference: you avoid the copy, and the compiler stops the function "
          "from changing it. The lecture's advice is that reference parameters should generally be const unless the function must modify them.",
          "void printName(const string &name) {\n"
          "    cout << name << \"\\n\";   // reading is fine\n"
          "    // name = \"x\";          // compile error: it's const\n"
          "}" },
        { "Value vs reference at a glance",
          "Pass by value: a copy is passed. Syntax: void f(int x). The function cannot change the caller's variable. "
          "Use it when the function only needs to read a small value.\n\n"
          "Pass by reference: a reference to the original is passed. Syntax: void f(int &x). The function can change the caller's variable. "
          "Use it when the function needs to modify the variable, or to avoid copying large data.",
          NULL },
        { "Extra: Reference variables",
          "References aren't only for parameters. int &r = x; makes r another name for x. A reference must be set up when it is declared, "
          "and it can never be changed to refer to a different variable later - unlike a pointer, which can be moved.",
          "int x = 1, y = 2;\n"
          "int &r = x;     // r is another name for x\n"
          "r = 10;         // x is now 10\n"
          "r = y;          // copies y's value into x - it does NOT move r\n"
          "// int &bad;    // error: a reference must be initialised" },
    },
    .quizCount = 4,
    .quiz = {
        { "What is a after this?", "void addOne(int& x) { x = x + 1; }\n\nint a = 5;\naddOne(a);", 4,
          { "5", "6", "0", "It doesn't compile" }, 1,
          "x is a reference to a, so the function changes a itself." },
        { "Which one can be null?", NULL, 4,
          { "A reference", "A pointer", "Both", "Neither" }, 1,
          "A pointer can be nullptr. A reference must always refer to a real variable." },
        { "Why pass a big object as const Object &obj?", NULL, 4,
          { "So the function can change it", "To avoid copying it while preventing changes", "It is required for objects", "To make a copy" }, 1,
          "A reference avoids the copy and const stops the function modifying it." },
        { "How do you call void swap(int* x, int* y) on a and b?", NULL, 4,
          { "swap(a, b)", "swap(*a, *b)", "swap(&a, &b)", "swap(a&, b&)" }, 2,
          "The pointer version needs the addresses of a and b." },
    },
    .projectTitle = "Stats by Reference",
    .projectBrief =
        "Write two functions that hand back their results through reference parameters:\n\n"
        "void stats(const int arr[], int size, int &smallest, int &largest, double &average)\n"
        "void swap(int &a, int &b)\n\n"
        "main (already written) reads how many numbers there are and the numbers, prints the stats, then swaps the first and last numbers and prints the array. "
        "For  4  then  3 9 1 6:\n\n"
        "Min: 1\n"
        "Max: 9\n"
        "Average: 4.75\n"
        "Swapped: 6 9 1 3",
    .starterCode =
        "#include <iostream>\n"
        "#include <iomanip>\n"
        "using namespace std;\n"
        "\n"
        "void stats(const int arr[], int size, int &smallest, int &largest, double &average)\n"
        "{\n"
        "    // Set smallest, largest and average\n"
        "}\n"
        "\n"
        "void swap(int &a, int &b)\n"
        "{\n"
        "    // Exchange a and b\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int numbers[100];\n"
        "    int size;\n"
        "    cin >> size;\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        cin >> numbers[i];\n"
        "    }\n"
        "\n"
        "    int smallest = 0, largest = 0;\n"
        "    double average = 0;\n"
        "    stats(numbers, size, smallest, largest, average);\n"
        "\n"
        "    cout << \"Min: \" << smallest << \"\\n\";\n"
        "    cout << \"Max: \" << largest << \"\\n\";\n"
        "    cout << fixed << setprecision(2) << \"Average: \" << average << \"\\n\";\n"
        "\n"
        "    swap(numbers[0], numbers[size - 1]);\n"
        "    cout << \"Swapped:\";\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        cout << \" \" << numbers[i];\n"
        "    }\n"
        "    cout << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .hint = "In stats, start with smallest = largest = arr[0]; and a running total, then loop over the array. "
            "Set average = (double)total / size;. Because the parameters are references, assigning to them sets main's variables.",
    .solution =
        "#include <iostream>\n"
        "#include <iomanip>\n"
        "using namespace std;\n"
        "\n"
        "void stats(const int arr[], int size, int &smallest, int &largest, double &average)\n"
        "{\n"
        "    smallest = arr[0];\n"
        "    largest = arr[0];\n"
        "    int total = 0;\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        if (arr[i] < smallest) smallest = arr[i];\n"
        "        if (arr[i] > largest) largest = arr[i];\n"
        "        total += arr[i];\n"
        "    }\n"
        "    average = (double)total / size;\n"
        "}\n"
        "\n"
        "void swap(int &a, int &b)\n"
        "{\n"
        "    int temp = a;\n"
        "    a = b;\n"
        "    b = temp;\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int numbers[100];\n"
        "    int size;\n"
        "    cin >> size;\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        cin >> numbers[i];\n"
        "    }\n"
        "\n"
        "    int smallest = 0, largest = 0;\n"
        "    double average = 0;\n"
        "    stats(numbers, size, smallest, largest, average);\n"
        "\n"
        "    cout << \"Min: \" << smallest << \"\\n\";\n"
        "    cout << \"Max: \" << largest << \"\\n\";\n"
        "    cout << fixed << setprecision(2) << \"Average: \" << average << \"\\n\";\n"
        "\n"
        "    swap(numbers[0], numbers[size - 1]);\n"
        "    cout << \"Swapped:\";\n"
        "    for (int i = 0; i < size; i++) {\n"
        "        cout << \" \" << numbers[i];\n"
        "    }\n"
        "    cout << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "4\n3 9 1 6\n",     "Min: 1\nMax: 9\nAverage: 4.75\nSwapped: 6 9 1 3\n" },
        { "1\n5\n",           "Min: 5\nMax: 5\nAverage: 5.00\nSwapped: 5\n" },
        { "3\n-2 -8 -1\n",    "Min: -8\nMax: -1\nAverage: -3.67\nSwapped: -1 -8 -2\n" },
        { "2\n10 20\n",       "Min: 10\nMax: 20\nAverage: 15.00\nSwapped: 20 10\n" },
    },
},

// ===================================================================== 3
{
    .id = "cpp-new-delete", .lang = LANG_CPP,
    .title   = "new and delete",
    .summary = "Heap memory in C++, memory leaks, the three delete mistakes, new[] and 2D arrays.",
    .lecture = "Memory Management",
    .pageCount = 8,
    .pages = {
        { "The problem with locals",
          "Remember the Memory Management lecture's problem task: write a function that returns a pointer to memory containing 5. "
          "Returning the address of a local variable gives a dangling pointer, because the local goes out of scope when the function returns.",
          "int* getPtrToFive() {\n"
          "    int x = 5;\n"
          "    return &x;      // WRONG: x is gone after return\n"
          "}" },
        { "The new operator",
          "new allocates memory that stays allocated until you de-allocate it yourself, and returns a pointer to it. "
          "The type tells it how much memory to allocate. You can give a starting value in brackets or braces.\n\n"
          "Memory from new lives on the heap; ordinary local variables live on the stack.",
          "int *x = new int;       // uninitialised int on the heap\n"
          "int *y = new int(5);    // initialised to 5\n"
          "int *z = new int{5};    // also 5\n"
          "int *arr = new int[5];  // an array of 5 ints (see later)" },
        { "delete",
          "delete hands memory from new back. Now the lecture's task works: the int created with new survives after the function returns, "
          "and main deletes it when it has finished with it.",
          "int *getPtrToNewInt() {\n"
          "    int *x = new int;\n"
          "    *x = 5;\n"
          "    return x;           // fine: heap memory outlives the function\n"
          "}\n"
          "\n"
          "int *p = getPtrToNewInt();\n"
          "cout << *p << endl;     // 5\n"
          "delete p;" },
        { "Memory leaks",
          "If you never delete memory from new, your program wastes it. Memory your program allocated but can no longer free is a memory leak.\n\n"
          "Here each round allocates a new int and the pointer to the old one is lost. Adding delete p after the loop does not fix it - "
          "it only frees the last one. The fix is to delete inside the loop.",
          "int *p;\n"
          "for (int i = 0; i < 3; ++i) {\n"
          "    p = getPtrToFive();   // the previous int is now lost\n"
          "    cout << *p << endl;\n"
          "    delete p;             // the fix: free each one\n"
          "}" },
        { "Three delete mistakes",
          "Don't use memory after deleting it - print *x before the delete, not after.\n"
          "Don't delete the same memory twice.\n"
          "Only delete memory that was allocated with new - never the address of an ordinary variable.",
          "int *x = getPtrToFive();\n"
          "delete x;\n"
          "cout << *x;       // WRONG: used after delete\n"
          "delete x;         // WRONG: deleted twice\n"
          "\n"
          "int y = 5;\n"
          "delete &y;        // WRONG: y wasn't made with new" },
        { "Arrays with new[]",
          "Arrays declared on the stack (int arr[SIZE]) need a constant size in standard C++. new[] can allocate an array whose size you only "
          "find out while the program runs. De-allocate it with delete[] - the square brackets matter.",
          "int numItems;\n"
          "cin >> numItems;\n"
          "// int arr[numItems];          // not allowed in standard C++\n"
          "int *arr = new int[numItems];  // allowed\n"
          "for (int i = 0; i < numItems; ++i) {\n"
          "    cin >> arr[i];\n"
          "}\n"
          "delete[] arr;" },
        { "2D arrays with new",
          "A dynamic 2D array is an array of pointers, one per row, where each row is its own array. "
          "Allocate the array of row pointers first, then each row. Free them in the opposite order: every row, then the array of pointers.",
          "int** arr2D = new int*[rows];\n"
          "for (int row = 0; row < rows; row++) {\n"
          "    arr2D[row] = new int[cols];\n"
          "}\n"
          "arr2D[1][2] = 7;                // use it like a normal 2D array\n"
          "for (int row = 0; row < rows; row++) {\n"
          "    delete[] arr2D[row];\n"
          "}\n"
          "delete[] arr2D;" },
        { "Extra: Smart pointers",
          "Modern C++ code rarely calls delete by hand. A std::unique_ptr from <memory> owns heap memory and deletes it automatically when it goes "
          "out of scope, so leaks and double deletes can't happen. For dynamic arrays, std::vector (in the STL tutorials) does the same job.\n\n"
          "It's still important to understand new and delete - smart pointers and vectors use them inside.",
          "#include <memory>\n"
          "\n"
          "unique_ptr<int> p = make_unique<int>(5);\n"
          "cout << *p << endl;   // 5\n"
          "// no delete needed: freed when p goes out of scope" },
    },
    .quizCount = 4,
    .quiz = {
        { "Memory from int *p = new int[10]; is freed with...", NULL, 4,
          { "delete p;", "delete[] p;", "free(p);", "It frees itself" }, 1,
          "Arrays allocated with new[] must be freed with delete[]." },
        { "Does adding delete p; after this loop fix the leak?", "for (int i = 0; i < 3; ++i) {\n    p = getPtrToFive();\n}", 4,
          { "Yes", "No - only the last int is freed", "No - delete can't be used on p", "Yes, but only in C" }, 1,
          "The first two ints were lost when p was overwritten, so they can never be freed." },
        { "Where does memory from new come from?", NULL, 4,
          { "The stack", "The heap", "The CPU", "The hard drive" }, 1,
          "new allocates on the heap; local variables are on the stack." },
        { "What is wrong here?", "int x = 5;\nint *xPtr = &x;\ndelete xPtr;", 4,
          { "Nothing", "x must be deleted with delete[]", "x wasn't allocated with new, so it must not be deleted", "xPtr should be NULL" }, 2,
          "Only memory from new may be deleted." },
    },
    .projectTitle = "Dynamic Grid",
    .projectBrief =
        "This follows the lecture's 2D array example. Read the number of rows and columns, then:\n\n"
        "1. Allocate a 2D array with new: an int** with one new int[cols] per row.\n"
        "2. Fill it with 1, 2, 3, ... going along each row in turn.\n"
        "3. Print it, one row per line with a space between the numbers, then print the total.\n"
        "4. Free every row with delete[], then the array of rows.\n\n"
        "For  3 4  the output is:\n"
        "1 2 3 4\n"
        "5 6 7 8\n"
        "9 10 11 12\n"
        "Total: 78",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int rows, cols;\n"
        "    cin >> rows >> cols;\n"
        "\n"
        "    // 1. Allocate int** arr2D with new\n"
        "\n"
        "    // 2. Fill it, 3. print it and the total\n"
        "\n"
        "    // 4. Free the memory\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "int** arr2D = new int*[rows]; then for each row arr2D[row] = new int[cols];. Fill with int counter = 1; ... arr2D[row][col] = counter++;. "
            "Print with cout << arr2D[row][col] << \" \"; (trailing spaces are fine). Free with delete[] arr2D[row]; in a loop and then delete[] arr2D;",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int rows, cols;\n"
        "    cin >> rows >> cols;\n"
        "\n"
        "    int** arr2D = new int*[rows];\n"
        "    for (int row = 0; row < rows; row++) {\n"
        "        arr2D[row] = new int[cols];\n"
        "    }\n"
        "\n"
        "    int counter = 1;\n"
        "    for (int row = 0; row < rows; row++) {\n"
        "        for (int col = 0; col < cols; col++) {\n"
        "            arr2D[row][col] = counter++;\n"
        "        }\n"
        "    }\n"
        "\n"
        "    int total = 0;\n"
        "    for (int row = 0; row < rows; row++) {\n"
        "        for (int col = 0; col < cols; col++) {\n"
        "            cout << arr2D[row][col] << \" \";\n"
        "            total += arr2D[row][col];\n"
        "        }\n"
        "        cout << endl;\n"
        "    }\n"
        "    cout << \"Total: \" << total << endl;\n"
        "\n"
        "    for (int row = 0; row < rows; row++) {\n"
        "        delete[] arr2D[row];\n"
        "    }\n"
        "    delete[] arr2D;\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "3 4\n", "1 2 3 4\n5 6 7 8\n9 10 11 12\nTotal: 78\n" },
        { "1 1\n", "1\nTotal: 1\n" },
        { "2 5\n", "1 2 3 4 5\n6 7 8 9 10\nTotal: 55\n" },
    },
},

// ===================================================================== 4
{
    .id = "cpp-objects", .lang = LANG_CPP,
    .title   = "Classes and Objects",
    .summary = "Grouping data into classes, instances, fields, passing objects around and methods.",
    .lecture = "Classes",
    .pageCount = 8,
    .pages = {
        { "Why classes?",
          "The lecture's example: a geometric vector has a start point and an end point, each with an x and y. "
          "With plain variables that's four doubles, and every function needs all four passed separately - "
          "easy to get in the wrong order (the lecture's own printVector call does exactly that!).",
          "void printVector(double x0, double x1, double y0, double y1);\n"
          "\n"
          "double xStart = 1.2, xEnd = 2.0;\n"
          "double yStart = 0.4, yEnd = 1.6;\n"
          "printVector(xStart, xEnd, yStart, yEnd);" },
        { "Defining a class",
          "A class is a user-defined datatype that groups related pieces of information. The pieces are called fields (or members). "
          "public: means code outside the class can use them.\n\n"
          "Don't forget the semicolon after the closing brace - leaving it out causes cryptic errors.",
          "class Vector {\n"
          "public:\n"
          "    double xStart;\n"
          "    double xEnd;\n"
          "    double yStart;\n"
          "    double yEnd;\n"
          "};    // <- this semicolon!" },
        { "Instances and fields",
          "An instance (or object) is one occurrence of a class, and each instance has its own set of field values. "
          "To represent two vectors, declare two instances. Use instance.fieldName to read or set a field.",
          "Vector vector1;\n"
          "Vector vector2;\n"
          "\n"
          "vector2.xStart = 10;\n"
          "vector1.xEnd = 12;     // vector1 and vector2 are separate" },
        { "Classes inside classes",
          "Fields can have any type - including other classes. A vector is really two points, so a Point class makes the design clearer. "
          "Reach nested fields with more dots. Assigning one instance to another copies all of its fields.",
          "class Point {\n"
          "public:\n"
          "    double x, y;\n"
          "};\n"
          "class Vector {\n"
          "public:\n"
          "    Point start, end;\n"
          "};\n"
          "\n"
          "Vector vec1;\n"
          "vec1.start.x = 3.0;\n"
          "Vector vec2;\n"
          "vec2.start = vec1.start;   // copies x and y" },
        { "Passing objects to functions",
          "Objects are passed by value by default, so the function gets a copy and its changes are lost. Pass by reference (&) to change the original. "
          "You can also pass a pointer: then use -> instead of . to reach the fields.",
          "void offsetPoint(Point p, double x, double y)  { p.x += x; }   // does nothing\n"
          "void offsetPoint(Point &p, double x, double y) { p.x += x; }   // works\n"
          "void offsetPoint(Point *p, double x, double y) { p->x += x; }  // works\n"
          "\n"
          "offsetPoint(pt, 1.0, 2.0);    // the reference version\n"
          "offsetPoint(&pt, 1.0, 2.0);   // the pointer version" },
        { "Methods",
          "Functions like printVector and offsetVector belong closely to one class, so C++ lets you put them inside it. A function that is part of a class is a method. "
          "You call it on an instance - vec.print() - and it can use that instance's fields directly. Methods can call the methods of fields too.",
          "class Point {\n"
          "public:\n"
          "    double x, y;\n"
          "    void offset(double offsetX, double offsetY) {\n"
          "        x += offsetX;\n"
          "        y += offsetY;\n"
          "    }\n"
          "    void print() {\n"
          "        cout << \"(\" << x << \",\" << y << \")\";\n"
          "    }\n"
          "};" },
        { "Methods of a nested class",
          "A Vector's methods can simply ask its two Points to do the work. Calling vec1.print() and vec2.print() runs the same code, but each call "
          "uses the fields of the instance it was called on.",
          "class Vector {\n"
          "public:\n"
          "    Point start, end;\n"
          "    void offset(double offsetX, double offsetY) {\n"
          "        start.offset(offsetX, offsetY);\n"
          "        end.offset(offsetX, offsetY);\n"
          "    }\n"
          "    void print() {\n"
          "        start.print();\n"
          "        cout << \" -> \";\n"
          "        end.print();\n"
          "        cout << endl;\n"
          "    }\n"
          "};" },
        { "Implementing methods separately",
          "Just like function prototypes, you can declare methods in the class and write them later. Usually the class goes in a header file "
          "(vector.h) and the method bodies in a .cpp file. ClassName:: says which class's method is being implemented.",
          "// vector.h\n"
          "class Point {\n"
          "public:\n"
          "    double x, y;\n"
          "    void offset(double offsetX, double offsetY);\n"
          "};\n"
          "\n"
          "// vector.cpp\n"
          "#include \"vector.h\"\n"
          "void Point::offset(double offsetX, double offsetY) {\n"
          "    x += offsetX; y += offsetY;\n"
          "}" },
    },
    .quizCount = 4,
    .quiz = {
        { "What is missing that can cause cryptic errors?", "class Point {\npublic:\n    double x, y;\n}", 4,
          { "A constructor", "A semicolon after the }", "private:", "A method" }, 1,
          "A class definition must end with };" },
        { "After vec2.start = vec1.start; what happens if you change vec2.start.x?", NULL, 4,
          { "vec1.start.x changes too", "vec1 is unaffected - the fields were copied", "It doesn't compile", "Both are deleted" }, 1,
          "Assigning one instance to another copies the field values." },
        { "Does this change p in main?", "void offsetPoint(Point p, double x, double y) {\n    p.x += x;\n}", 4,
          { "Yes", "No - p is a copy", "Only p.x", "Only if x is positive" }, 1,
          "The Point is passed by value. Use Point &p to change the original." },
        { "p is a Point*. How do you set its x field?", NULL, 4,
          { "p.x = 3;", "p->x = 3;", "*p.x = 3;", "p::x = 3;" }, 1,
          "-> reaches a field through a pointer. (*p).x = 3; also works." },
    },
    .projectTitle = "Moving Points",
    .projectBrief =
        "Finish the Point class from the lecture by writing its two methods:\n\n"
        "offset(offsetX, offsetY) adds the offsets to x and y.\n"
        "print() prints the point as (x,y) - no spaces and no new line.\n\n"
        "main is written. It reads a starting point, then pairs of offsets until the input runs out, and prints the point after every move. "
        "For  3 4  then  1 2  then  -0.5 0:\n\n"
        "Start: (3,4)\n"
        "Moved to: (4,6)\n"
        "Moved to: (3.5,6)",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "class Point {\n"
        "public:\n"
        "    double x, y;\n"
        "\n"
        "    void offset(double offsetX, double offsetY)\n"
        "    {\n"
        "        // Move the point\n"
        "    }\n"
        "\n"
        "    void print()\n"
        "    {\n"
        "        // Print (x,y)\n"
        "    }\n"
        "};\n"
        "\n"
        "int main()\n"
        "{\n"
        "    Point p;\n"
        "    cin >> p.x >> p.y;\n"
        "    cout << \"Start: \";\n"
        "    p.print();\n"
        "    cout << \"\\n\";\n"
        "\n"
        "    double dx, dy;\n"
        "    while (cin >> dx >> dy) {\n"
        "        p.offset(dx, dy);\n"
        "        cout << \"Moved to: \";\n"
        "        p.print();\n"
        "        cout << \"\\n\";\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .hint = "Inside a method you can use the fields directly: x += offsetX; y += offsetY;. "
            "print is cout << \"(\" << x << \",\" << y << \")\"; - cout prints 4.0 as 4 and 3.5 as 3.5 by itself.",
    .solution =
        "#include <iostream>\n"
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
        "int main()\n"
        "{\n"
        "    Point p;\n"
        "    cin >> p.x >> p.y;\n"
        "    cout << \"Start: \";\n"
        "    p.print();\n"
        "    cout << \"\\n\";\n"
        "\n"
        "    double dx, dy;\n"
        "    while (cin >> dx >> dy) {\n"
        "        p.offset(dx, dy);\n"
        "        cout << \"Moved to: \";\n"
        "        p.print();\n"
        "        cout << \"\\n\";\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "3 4\n1 2\n-0.5 0\n",  "Start: (3,4)\nMoved to: (4,6)\nMoved to: (3.5,6)\n" },
        { "0 0\n",               "Start: (0,0)\n" },
        { "1.2 0.4\n1 1.5\n",    "Start: (1.2,0.4)\nMoved to: (2.2,1.9)\n" },
    },
},

// ===================================================================== 5
{
    .id = "cpp-classes", .lang = LANG_CPP,
    .title   = "Constructors and Access Control",
    .summary = "Constructors, initialisation lists, public and private, getters and setters, this and static.",
    .lecture = "Classes",
    .pageCount = 8,
    .pages = {
        { "Constructors",
          "Setting every field by hand after creating an object is tedious. A constructor is a method that runs automatically when an instance is created. "
          "It has the same name as the class and no return type.",
          "class Point {\n"
          "public:\n"
          "    double x, y;\n"
          "    Point() {\n"
          "        x = 0.0; y = 0.0;\n"
          "        cout << \"Point instance created\" << endl;\n"
          "    }\n"
          "};\n"
          "\n"
          "Point p;   // prints: Point instance created" },
        { "Constructors with parameters",
          "Constructors can take parameters, and a class can have several constructors as long as their parameter lists differ. "
          "C++ picks the one that matches how you create the object.",
          "Point() { x = 0.0; y = 0.0; }\n"
          "Point(double nx, double ny) { x = nx; y = ny; }\n"
          "\n"
          "Point p;          // default constructor: (0, 0)\n"
          "Point q(2.0, 3.0);   // 2-parameter constructor: (2, 3)" },
        { "Initialisation lists",
          "An initialisation list sets fields before the constructor's body runs. It goes after a colon, between the brackets and the {. "
          "Use either round brackets or braces for each field. It is the preferred way to set fields, and the only way for const and reference fields.",
          "Point() : x(0), y(0) { }\n"
          "\n"
          "Point(double nx, double ny) : x(nx), y{ny} {\n"
          "    cout << \"2-parameter constructor\" << endl;\n"
          "}" },
        { "public and private",
          "Access modifiers decide where fields and methods can be used from. public members can be accessed from anywhere. "
          "private members can only be accessed inside the class's own methods. Keeping fields private stops other code putting the object into a bad state.",
          "class Point {\n"
          "private:\n"
          "    double x, y;\n"
          "public:\n"
          "    Point(double nx, double ny) { x = nx; y = ny; }\n"
          "};\n"
          "\n"
          "Point p(2.0, 3.0);\n"
          "p.x = 5.0;     // not allowed: x is private" },
        { "Getters and setters",
          "Use getters to give read-only access to private fields, and setters when outside code should be allowed to change them. "
          "A setter can check the new value and refuse a bad one, which is the whole point of making the field private.",
          "class Point {\n"
          "private:\n"
          "    double x, y;\n"
          "public:\n"
          "    double getX() { return x; }\n"
          "    double getY() { return y; }\n"
          "    bool setX(double px) {\n"
          "        if (px < 0) return false;   // refuse bad values\n"
          "        x = px;\n"
          "        return true;\n"
          "    }\n"
          "};" },
        { "class vs struct",
          "Structs are a carry-over from C. In C++ they are essentially the same as classes, with one difference: "
          "a class's members are private by default and a struct's are public by default. Many people use struct for simple bundles of data "
          "and class for anything with methods and rules.",
          "class Point  { double x, y; };   // x and y are private\n"
          "struct Point { double x, y; };   // x and y are public" },
        { "The this pointer",
          "Inside a method, this is a pointer to the instance the method was called on. this->x is the same as x, but it's useful "
          "when a parameter has the same name as a field.",
          "class Point {\n"
          "public:\n"
          "    double x, y;\n"
          "    Point(double x, double y) {\n"
          "        this->x = x;     // the field x = the parameter x\n"
          "        this->y = y;\n"
          "    }\n"
          "};" },
        { "Static members",
          "A static member belongs to the class itself, not to any one instance, so there is only one copy shared by every object. "
          "Use ClassName:: to reach it. A static method has no this pointer, so it can't use the non-static members.",
          "class Maths {\n"
          "public:\n"
          "    static float PI;\n"
          "    static float Mul(Point a, Point b) {\n"
          "        return a.x * b.x + a.y * b.y;\n"
          "    }\n"
          "};\n"
          "float Maths::PI = 3.14;   // defined once, outside the class\n"
          "\n"
          "float result = Maths::Mul(q, q);" },
    },
    .quizCount = 4,
    .quiz = {
        { "When does a constructor run?", NULL, 4,
          { "When the object is created", "When the program ends", "Every time a method is called", "Only if you call it by name" }, 0,
          "A constructor runs automatically as each instance is created." },
        { "What is the default access of members in a class?", NULL, 4,
          { "public", "private", "protected", "static" }, 1,
          "class members are private by default; struct members are public by default." },
        { "Where can private members be used?", NULL, 4,
          { "Anywhere", "Only in main", "Only inside the class's own methods", "Only in other classes" }, 2,
          "Outside code has to go through public methods such as getters and setters." },
        { "Why can't a static method use a non-static field?", NULL, 4,
          { "Static methods can't have code", "It isn't called on any particular instance, so there's no this", "Fields are always static", "It can - there's no restriction" }, 1,
          "A static method belongs to the class, so there is no object whose fields it could use." },
    },
    .projectTitle = "Bank Account",
    .projectBrief =
        "Finish the BankAccount class:\n"
        "- the constructor sets the starting balance\n"
        "- deposit(amount) adds to the balance\n"
        "- withdraw(amount) takes money out and returns true, but if there isn't enough money it changes nothing and returns false\n"
        "- getBalance() returns the balance\n\n"
        "main is already written. It reads a starting balance, then commands such as d 50 (deposit 50) or w 30 (withdraw 30), "
        "prints Declined when a withdrawal is refused, and prints the final balance.\n\n"
        "For the input 20 then w 50 the output should be:\n\n"
        "Declined\n"
        "Balance: 20",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "class BankAccount {\n"
        "public:\n"
        "    BankAccount(int start)\n"
        "    {\n"
        "        // Set the starting balance\n"
        "    }\n"
        "\n"
        "    void deposit(int amount)\n"
        "    {\n"
        "    }\n"
        "\n"
        "    bool withdraw(int amount)\n"
        "    {\n"
        "        // Return false if there isn't enough money\n"
        "        return true;\n"
        "    }\n"
        "\n"
        "    int getBalance()\n"
        "    {\n"
        "        return 0;\n"
        "    }\n"
        "\n"
        "private:\n"
        "    int balance;\n"
        "};\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int start;\n"
        "    cin >> start;\n"
        "    BankAccount account(start);\n"
        "\n"
        "    char cmd;\n"
        "    int amount;\n"
        "    while (cin >> cmd >> amount) {\n"
        "        if (cmd == 'd') {\n"
        "            account.deposit(amount);\n"
        "        } else if (cmd == 'w') {\n"
        "            if (!account.withdraw(amount)) {\n"
        "                cout << \"Declined\\n\";\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "\n"
        "    cout << \"Balance: \" << account.getBalance() << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .hint = "Constructor: balance = start; deposit: balance += amount; withdraw: if (amount > balance) return false; otherwise balance -= amount; and return true. getBalance: return balance;",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "class BankAccount {\n"
        "public:\n"
        "    BankAccount(int start)\n"
        "    {\n"
        "        balance = start;\n"
        "    }\n"
        "\n"
        "    void deposit(int amount)\n"
        "    {\n"
        "        balance += amount;\n"
        "    }\n"
        "\n"
        "    bool withdraw(int amount)\n"
        "    {\n"
        "        if (amount > balance) {\n"
        "            return false;\n"
        "        }\n"
        "        balance -= amount;\n"
        "        return true;\n"
        "    }\n"
        "\n"
        "    int getBalance()\n"
        "    {\n"
        "        return balance;\n"
        "    }\n"
        "\n"
        "private:\n"
        "    int balance;\n"
        "};\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int start;\n"
        "    cin >> start;\n"
        "    BankAccount account(start);\n"
        "\n"
        "    char cmd;\n"
        "    int amount;\n"
        "    while (cin >> cmd >> amount) {\n"
        "        if (cmd == 'd') {\n"
        "            account.deposit(amount);\n"
        "        } else if (cmd == 'w') {\n"
        "            if (!account.withdraw(amount)) {\n"
        "                cout << \"Declined\\n\";\n"
        "            }\n"
        "        }\n"
        "    }\n"
        "\n"
        "    cout << \"Balance: \" << account.getBalance() << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "100\nd 50\nw 30\n",            "Balance: 120\n" },
        { "20\nw 50\n",                   "Declined\nBalance: 20\n" },
        { "0\nd 10\nd 5\nw 15\nw 1\n",    "Declined\nBalance: 0\n" },
        { "500\n",                        "Balance: 500\n" },
    },
},

// ===================================================================== 6
{
    .id = "cpp-object-memory", .lang = LANG_CPP,
    .title   = "Objects, Pointers and Destructors",
    .summary = "Pointers to objects, new for objects and arrays of objects, destructors and copy constructors.",
    .lecture = "Classes",
    .pageCount = 8,
    .pages = {
        { "Pointers to objects",
          "A pointer can point to an object just like it points to an int. Declaring Point* creates no instance - it only stores an address. "
          "When you have a pointer, use -> to reach the fields and methods.",
          "Point p;\n"
          "Point* pPtr;      // a pointer: no instance created\n"
          "pPtr = &p;\n"
          "\n"
          "cout << p.x << \" \" << p.y;\n"
          "cout << pPtr->x << \" \" << pPtr->y;   // the same values\n"
          "pPtr->x = 10;    // changes p.x" },
        { "Creating objects with new",
          "new creates an instance on the heap, runs its constructor and returns a pointer to it. As with any memory from new, "
          "the object lives until you delete it - and delete runs the object's destructor first.",
          "Point* p = new Point();     // instance created, p points to it\n"
          "p->x = 10;\n"
          "cout << p->x << \" \" << p->y;\n"
          "\n"
          "Point* q = new Point(2.0, 3.0);\n"
          "delete p;\n"
          "delete q;" },
        { "Arrays of objects",
          "An array of objects creates every instance at once, so the class needs a default constructor (one with no parameters). "
          "With new[] the array can have a size chosen at run time - free it with delete[].",
          "Point points[20];              // 20 Points, default constructor\n"
          "for (int i = 0; i < 20; i++) {\n"
          "    points[i].x = i;\n"
          "}\n"
          "\n"
          "Point* more = new Point[30];\n"
          "more[0].x = 5;\n"
          "delete[] more;" },
        { "Arrays of pointers",
          "An array of pointers holds addresses, not objects, so each object can be created separately - with any constructor you like. "
          "Each one made with new must be deleted on its own. With new Point*[30], free the array of pointers with delete[] as well.",
          "Point* pointPointers[20];\n"
          "for (int i = 0; i < 20; i++) {\n"
          "    pointPointers[i] = new Point(i);\n"
          "}\n"
          "pointPointers[3]->Print();\n"
          "for (int i = 0; i < 20; i++) {\n"
          "    delete pointPointers[i];\n"
          "}" },
        { "Destructors",
          "A destructor is called automatically when an object goes out of scope (or is deleted). It's named ~ClassName and takes no parameters. "
          "Its job is cleaning up - above all, deleting memory the object allocated. Without one, this class leaks 20000 Points every time an instance disappears.",
          "class TwentyThousandPoints {\n"
          "    Point* points;\n"
          "public:\n"
          "    TwentyThousandPoints() {\n"
          "        points = new Point[20000];\n"
          "    }\n"
          "    ~TwentyThousandPoints() {\n"
          "        delete[] points;     // without this: a leak\n"
          "    }\n"
          "};" },
        { "Copy constructors: the problem",
          "Point r = q; creates r as a copy of q using the copy constructor. The default one copies each field. That's fine for numbers, "
          "but for a pointer field it copies the address - so both objects share the same memory. Change one and you change the other.",
          "class MITStudent {\n"
          "public:\n"
          "    int studentID;\n"
          "    char *name;\n"
          "};\n"
          "\n"
          "MITStudent student2 = student1;   // copies the pointer\n"
          "student2.name[0] = 'b';\n"
          "cout << student1.name;            // boo - student1 changed too!" },
        { "Copy constructors: the fix",
          "Write your own copy constructor that makes a deep copy: allocate new memory and copy the contents, not just the address. "
          "The lecture uses strdup, which mallocs a copy of a C string (Visual Studio calls it _strdup). "
          "The parameter is a reference - usually a const one.",
          "MITStudent(const MITStudent &o) {\n"
          "    studentID = o.studentID;\n"
          "    name = strdup(o.name);   // a new copy of the text\n"
          "}\n"
          "\n"
          "MITStudent student2 = student1;\n"
          "student2.name[0] = 'b';\n"
          "cout << student1.name;       // foo - unaffected" },
        { "Extra: The rule of three",
          "If a class needs a destructor, it almost always needs a copy constructor and a copy assignment operator too - that's the rule of three. "
          "Assignment (a = b; on existing objects) is separate from copying: it must free the old memory, copy the new, and handle a = a. "
          "The lecture's LinkedList class does exactly this.",
          "Student& operator=(const Student& other) {\n"
          "    if (this != &other) {          // a = a would delete our own data\n"
          "        delete[] name;\n"
          "        name = new char[strlen(other.name) + 1];\n"
          "        strcpy(name, other.name);\n"
          "    }\n"
          "    return *this;\n"
          "}" },
    },
    .quizCount = 4,
    .quiz = {
        { "Point* pPtr; - how many Point instances does this create?", NULL, 4,
          { "One", "None", "Two", "It depends on the constructor" }, 1,
          "It only declares a pointer. An instance needs Point p; or new Point()." },
        { "When is a destructor called?", NULL, 4,
          { "When the object is created", "When the object goes out of scope or is deleted", "Whenever a method finishes", "Only if you call it" }, 1,
          "Destructors run automatically at the end of an object's life." },
        { "The default copy constructor copies a char* field. What happens?", NULL, 4,
          { "The text is copied", "Both objects point to the same text", "The field is set to NULL", "It doesn't compile" }, 1,
          "Only the address is copied - a shallow copy. A custom copy constructor makes a deep copy." },
        { "How do you free Point* pts = new Point[30];?", NULL, 4,
          { "delete pts;", "delete[] pts;", "for each i: delete pts[i];", "pts.free();" }, 1,
          "Memory from new[] is freed with delete[]." },
    },
    .projectTitle = "Deep Copy",
    .projectBrief =
        "The Student class stores its name in memory it allocates with new char[]. Write three things:\n\n"
        "1. The constructor Student(int id, const char *n): allocate strlen(n) + 1 chars and strcpy the name in.\n"
        "2. A copy constructor that makes a deep copy - its own new memory with the same text.\n"
        "3. A destructor that frees the name with delete[].\n\n"
        "main (already written) reads an id, a name and a letter, copies the student, changes the copy's first letter and prints both. "
        "For  98 foo b  the output must be:\n\n"
        "s1: 98 foo\n"
        "s2: 98 boo\n\n"
        "With the default (shallow) copy, s1 would wrongly show boo too.",
    .starterCode =
        "#include <iostream>\n"
        "#include <cstring>\n"
        "using namespace std;\n"
        "\n"
        "class Student {\n"
        "public:\n"
        "    int id;\n"
        "    char *name;\n"
        "\n"
        "    Student(int newId, const char *n)\n"
        "    {\n"
        "        id = newId;\n"
        "        name = nullptr;   // allocate memory and copy n into it\n"
        "    }\n"
        "\n"
        "    // Write the copy constructor: Student(const Student &other)\n"
        "\n"
        "    // Write the destructor: ~Student()\n"
        "\n"
        "    void setFirstLetter(char c) { name[0] = c; }\n"
        "    void print() { cout << id << \" \" << name << \"\\n\"; }\n"
        "};\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int id;\n"
        "    char buffer[100];\n"
        "    char letter;\n"
        "    cin >> id >> buffer >> letter;\n"
        "\n"
        "    Student s1(id, buffer);\n"
        "    Student s2 = s1;          // uses the copy constructor\n"
        "    s2.setFirstLetter(letter);\n"
        "\n"
        "    cout << \"s1: \";\n"
        "    s1.print();\n"
        "    cout << \"s2: \";\n"
        "    s2.print();\n"
        "    return 0;\n"
        "}\n",
    .hint = "Constructor: name = new char[strlen(n) + 1]; strcpy(name, n);. "
            "Copy constructor: Student(const Student &other) { id = other.id; name = new char[strlen(other.name) + 1]; strcpy(name, other.name); }. "
            "Destructor: ~Student() { delete[] name; }",
    .solution =
        "#include <iostream>\n"
        "#include <cstring>\n"
        "using namespace std;\n"
        "\n"
        "class Student {\n"
        "public:\n"
        "    int id;\n"
        "    char *name;\n"
        "\n"
        "    Student(int newId, const char *n)\n"
        "    {\n"
        "        id = newId;\n"
        "        name = new char[strlen(n) + 1];\n"
        "        strcpy(name, n);\n"
        "    }\n"
        "\n"
        "    Student(const Student &other)\n"
        "    {\n"
        "        id = other.id;\n"
        "        name = new char[strlen(other.name) + 1];\n"
        "        strcpy(name, other.name);\n"
        "    }\n"
        "\n"
        "    ~Student()\n"
        "    {\n"
        "        delete[] name;\n"
        "    }\n"
        "\n"
        "    void setFirstLetter(char c) { name[0] = c; }\n"
        "    void print() { cout << id << \" \" << name << \"\\n\"; }\n"
        "};\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int id;\n"
        "    char buffer[100];\n"
        "    char letter;\n"
        "    cin >> id >> buffer >> letter;\n"
        "\n"
        "    Student s1(id, buffer);\n"
        "    Student s2 = s1;          // uses the copy constructor\n"
        "    s2.setFirstLetter(letter);\n"
        "\n"
        "    cout << \"s1: \";\n"
        "    s1.print();\n"
        "    cout << \"s2: \";\n"
        "    s2.print();\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "98 foo b\n",   "s1: 98 foo\ns2: 98 boo\n" },
        { "1 Alice M\n",  "s1: 1 Alice\ns2: 1 Mlice\n" },
        { "42 x y\n",     "s1: 42 x\ns2: 42 y\n" },
    },
},

// ===================================================================== 7
{
    .id = "cpp-linked-list", .lang = LANG_CPP,
    .title   = "Linked Lists",
    .summary = "Building a singly linked list with Node structs and pointers: insert, traverse, delete and a class version.",
    .lecture = "Pointers in C++ and Linked Lists",
    .pageCount = 8,
    .pages = {
        { "What is a linked list?",
          "A linked list is a dynamic data structure made of nodes. Each node holds some data and a pointer to the next node. "
          "The list is reached through a head pointer to the first node, and the last node's next pointer is nullptr.\n\n"
          "Linked lists grow one node at a time with new, and inserting or deleting just means re-linking pointers.",
          "head -> [10 | next] -> [20 | next] -> [30 | nullptr]" },
        { "The Node struct",
          "Each node stores an integer (data) and a pointer to the next node (next). A struct is a good fit: it's just public data.\n\n"
          "(The lecture notes switch between the names next and nextPtr - pick one and use it everywhere. These tutorials use next.)",
          "#include <iostream>\n"
          "using namespace std;\n"
          "\n"
          "struct Node {\n"
          "    int data;      // the actual information\n"
          "    Node* next;    // the next node, or nullptr at the end\n"
          "};" },
        { "Inserting at the end",
          "Create a node with new, then walk from the head to the last node and link the new node onto it. If the list is empty, the new node becomes the head.\n\n"
          "Why Node**? The function may need to change main's head pointer itself, so it is passed a pointer to that pointer: call it with InsertAtEnd(&head, 10).",
          "void InsertAtEnd(Node** ptrToHeadPtr, int newData) {\n"
          "    Node* newNode = new Node();\n"
          "    newNode->data = newData;\n"
          "    newNode->next = nullptr;\n"
          "    if (*ptrToHeadPtr == nullptr) {\n"
          "        *ptrToHeadPtr = newNode;     // empty list: new head\n"
          "        return;\n"
          "    }\n"
          "    Node* cur = *ptrToHeadPtr;\n"
          "    while (cur->next != nullptr) cur = cur->next;\n"
          "    cur->next = newNode;\n"
          "}" },
        { "Traversing the list",
          "To visit every node, start a pointer at the head and keep following next until it becomes nullptr. "
          "Printing, counting (getSize) and searching (contains) all use this same loop.",
          "void PrintList(Node* headPtr) {\n"
          "    Node* cur = headPtr;\n"
          "    while (cur != nullptr) {\n"
          "        cout << cur->data << \" \";\n"
          "        cur = cur->next;\n"
          "    }\n"
          "    cout << endl;\n"
          "}\n"
          "\n"
          "Node* head = nullptr;     // start with an empty list\n"
          "InsertAtEnd(&head, 10);\n"
          "PrintList(head);           // 10" },
        { "Inserting at the beginning",
          "Inserting at the front is even easier, and needs no walking at all: point the new node at the old head, then make it the head. "
          "This is one of the big advantages over an array, where everything would have to shift up.",
          "void InsertAtBeginning(Node** ptrToHeadPtr, int newData) {\n"
          "    Node* newNode = new Node();\n"
          "    newNode->data = newData;\n"
          "    newNode->next = *ptrToHeadPtr;\n"
          "    *ptrToHeadPtr = newNode;\n"
          "}" },
        { "Deleting a value",
          "To delete the first node holding a value: if it's the head, move the head along. Otherwise walk the list remembering the previous node, "
          "then link prev straight to the node after the one being removed. Always delete the removed node, or it leaks.",
          "Node* cur = *ptrToHeadPtr;\n"
          "if (cur != nullptr && cur->data == value) {\n"
          "    *ptrToHeadPtr = cur->next;   // change head\n"
          "    delete cur;\n"
          "    return;\n"
          "}\n"
          "Node* prev = nullptr;\n"
          "while (cur != nullptr && cur->data != value) {\n"
          "    prev = cur;\n"
          "    cur = cur->next;\n"
          "}\n"
          "if (cur == nullptr) { cout << \"Value not found in the list.\" << endl; return; }\n"
          "prev->next = cur->next;          // unlink\n"
          "delete cur;" },
        { "A LinkedList class",
          "The lecture finishes by wrapping it all in a class. head becomes a private field, the functions become methods (no more Node**), "
          "and a destructor deletes every node automatically. Because the class owns memory, it also has a copy constructor and an assignment "
          "operator that copy the nodes - the rule of three.",
          "class LinkedList {\n"
          "private:\n"
          "    Node* head;\n"
          "public:\n"
          "    LinkedList() { head = nullptr; }\n"
          "    ~LinkedList() { clear(); }\n"
          "    void clear() {\n"
          "        Node* cur = head;\n"
          "        while (cur != nullptr) {\n"
          "            Node* temp = cur;\n"
          "            cur = cur->next;\n"
          "            delete temp;\n"
          "        }\n"
          "        head = nullptr;\n"
          "    }\n"
          "};" },
        { "Extra: List or array?",
          "Linked list: inserting or removing at the front is instant, and it never needs to be resized - but there's no list[5]: reaching the 5th item "
          "means walking past the first four, and each node costs an extra pointer.\n\n"
          "Array or vector: instant access by index and compact in memory, but inserting near the front shifts everything along.\n\n"
          "The STL already has std::list, a doubly linked list where each node also points back to the previous one (STL Lists tutorial).",
          NULL },
    },
    .quizCount = 4,
    .quiz = {
        { "What marks the end of a singly linked list?", NULL, 4,
          { "A node with data 0", "The last node's next is nullptr", "The head pointer", "A special End node" }, 1,
          "The last node points to nothing: nullptr." },
        { "Why does InsertAtEnd take a Node** instead of a Node*?", NULL, 4,
          { "It's faster", "So it can change the caller's head pointer when the list is empty", "Nodes must always be passed twice", "To delete the list" }, 1,
          "With a Node* it would only change its own copy of head." },
        { "What happens if DeleteWithValue unlinks a node but never deletes it?", NULL, 4,
          { "Nothing", "The list breaks", "A memory leak", "The node is deleted automatically" }, 2,
          "The node is still allocated but nothing points to it any more." },
        { "head is 5 -> 10 -> 30. What does inserting 1 at the beginning do?", NULL, 4,
          { "1 -> 5 -> 10 -> 30", "5 -> 10 -> 30 -> 1", "1 -> 30", "5 -> 1 -> 10 -> 30" }, 0,
          "The new node points at the old head and becomes the new head." },
    },
    .projectTitle = "Build a List",
    .projectBrief =
        "Using the Node struct, write InsertAtEnd, InsertAtBeginning, DeleteWithValue and PrintList from the lecture. main is written.\n\n"
        "The input is some numbers ended by a 0 (inserted at the end), then a value to delete, then a value to insert at the front. "
        "For  10 20 30 0 20 5:\n\n"
        "List: 10 20 30\n"
        "After deleting 20: 10 30\n"
        "After inserting 5: 5 10 30\n\n"
        "If the value to delete isn't there, DeleteWithValue prints  Value not found in the list.  first. If the list is empty it prints  List is empty.",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "struct Node {\n"
        "    int data;\n"
        "    Node* next;\n"
        "};\n"
        "\n"
        "void InsertAtEnd(Node** ptrToHeadPtr, int newData)\n"
        "{\n"
        "}\n"
        "\n"
        "void InsertAtBeginning(Node** ptrToHeadPtr, int newData)\n"
        "{\n"
        "}\n"
        "\n"
        "void DeleteWithValue(Node** ptrToHeadPtr, int value)\n"
        "{\n"
        "}\n"
        "\n"
        "void PrintList(Node* headPtr)\n"
        "{\n"
        "    // Print each value followed by a space, then endl\n"
        "    cout << endl;\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    Node* head = nullptr;\n"
        "    int value;\n"
        "    while (cin >> value && value != 0) {\n"
        "        InsertAtEnd(&head, value);\n"
        "    }\n"
        "    cout << \"List: \";\n"
        "    PrintList(head);\n"
        "\n"
        "    int toDelete, toInsert;\n"
        "    cin >> toDelete >> toInsert;\n"
        "    DeleteWithValue(&head, toDelete);\n"
        "    cout << \"After deleting \" << toDelete << \": \";\n"
        "    PrintList(head);\n"
        "    InsertAtBeginning(&head, toInsert);\n"
        "    cout << \"After inserting \" << toInsert << \": \";\n"
        "    PrintList(head);\n"
        "\n"
        "    while (head != nullptr) {   // free every node\n"
        "        Node* temp = head;\n"
        "        head = head->next;\n"
        "        delete temp;\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .hint = "Follow the lesson pages closely - the code is almost exactly the lecture's. Remember to set newNode->next in both insert functions, "
            "and in DeleteWithValue check for an empty list first, then the head, then walk with prev and cur.",
    .solution =
        "#include <iostream>\n"
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
        "    Node* cur = headPtr;\n"
        "    while (cur != nullptr) {\n"
        "        cout << cur->data << \" \";\n"
        "        cur = cur->next;\n"
        "    }\n"
        "    cout << endl;\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    Node* head = nullptr;\n"
        "    int value;\n"
        "    while (cin >> value && value != 0) {\n"
        "        InsertAtEnd(&head, value);\n"
        "    }\n"
        "    cout << \"List: \";\n"
        "    PrintList(head);\n"
        "\n"
        "    int toDelete, toInsert;\n"
        "    cin >> toDelete >> toInsert;\n"
        "    DeleteWithValue(&head, toDelete);\n"
        "    cout << \"After deleting \" << toDelete << \": \";\n"
        "    PrintList(head);\n"
        "    InsertAtBeginning(&head, toInsert);\n"
        "    cout << \"After inserting \" << toInsert << \": \";\n"
        "    PrintList(head);\n"
        "\n"
        "    while (head != nullptr) {   // free every node\n"
        "        Node* temp = head;\n"
        "        head = head->next;\n"
        "        delete temp;\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .testCount = 5,
    .tests = {
        { "10 20 30 0 20 5\n", "List: 10 20 30\nAfter deleting 20: 10 30\nAfter inserting 5: 5 10 30\n" },
        { "10 20 30 0 10 1\n", "List: 10 20 30\nAfter deleting 10: 20 30\nAfter inserting 1: 1 20 30\n" },
        { "4 8 0 99 2\n",      "List: 4 8\nValue not found in the list.\nAfter deleting 99: 4 8\nAfter inserting 2: 2 4 8\n" },
        { "0 3 7\n",           "List:\nList is empty.\nAfter deleting 3:\nAfter inserting 7: 7\n" },
        { "1 2 3 0 3 3\n",     "List: 1 2 3\nAfter deleting 3: 1 2\nAfter inserting 3: 3 1 2\n" },
    },
},

// ===================================================================== 8
{
    .id = "cpp-overloading", .lang = LANG_CPP,
    .title   = "Function and Operator Overloading",
    .summary = "Several functions with one name, operator+ for your own classes, operator<< and prefix/postfix ++.",
    .lecture = "Operator Overloading",
    .pageCount = 7,
    .pages = {
        { "Function overloading",
          "C++ lets you have several functions (or methods) with the same name in the same scope, as long as their parameter lists differ "
          "in the types or the number of parameters. The compiler picks the version that matches the arguments.\n\n"
          "You can't overload functions that differ only by their return type.",
          "class printData {\n"
          "public:\n"
          "    void print(int i)    { cout << \"Printing int: \" << i << endl; }\n"
          "    void print(double f) { cout << \"Printing float: \" << f << endl; }\n"
          "    void print(const char* c) { cout << \"Printing character: \" << c << endl; }\n"
          "};\n"
          "\n"
          "pd.print(5);           // Printing int: 5\n"
          "pd.print(500.263);     // Printing float: 500.263\n"
          "pd.print(\"Hello C++\"); // Printing character: Hello C++" },
        { "Operator overloading",
          "You can redefine most of C++'s built-in operators so they work with your own types. An overloaded operator is a function with a special name: "
          "the keyword operator followed by the symbol. Like any function it has a return type and parameters.\n\n"
          "This declares + for adding two Box objects and returning a new Box:",
          "Box operator+(const Box& b);              // as a member\n"
          "Box operator+(const Box& a, const Box& b); // as a non-member" },
        { "A member operator+",
          "As a member function, operator+ is called on the left-hand Box, with the right-hand Box as its parameter. "
          "box1 + box2 really means box1.operator+(box2), and this-> is the left-hand box.",
          "class Box {\n"
          "public:\n"
          "    double getVolume() { return length * breadth * height; }\n"
          "    Box operator+(const Box& b) {\n"
          "        Box box;\n"
          "        box.length = this->length + b.length;\n"
          "        box.breadth = this->breadth + b.breadth;\n"
          "        box.height = this->height + b.height;\n"
          "        return box;\n"
          "    }\n"
          "private:\n"
          "    double length, breadth, height;\n"
          "};\n"
          "\n"
          "Box3 = Box1 + Box2;" },
        { "Printing with operator<<",
          "cout << c works for your class if you write operator<<. It must be a non-member, because the left side is the ostream, not your object. "
          "Declaring it a friend inside the class lets it use private fields. Return the stream so calls can be chained: cout << a << \" \" << b.",
          "class Clock {\n"
          "    friend ostream& operator<<(ostream& os, const Clock& c);\n"
          "    int hour, min;\n"
          "};\n"
          "\n"
          "ostream& operator<<(ostream& os, const Clock& c) {\n"
          "    if (c.hour < 10) os << '0';      // pad with a 0\n"
          "    os << c.hour << \":\";\n"
          "    if (c.min < 10) os << '0';\n"
          "    os << c.min;\n"
          "    return os;\n"
          "}" },
        { "Prefix ++",
          "For ++c2 the object must increment itself first and then be the result, so prefix ++ calls tick() and returns *this. "
          "c1 = ++c2; is the same as c1 = c2.operator++();.",
          "void Clock::tick() {            // advance by 1 minute\n"
          "    min = (min + 1) % 60;\n"
          "    if (min == 0) hour = (hour + 1) % 24;\n"
          "}\n"
          "\n"
          "Clock Clock::operator++() {     // prefix form\n"
          "    tick();          // increment\n"
          "    return *this;    // return itself\n"
          "}" },
        { "Postfix ++",
          "c3++ must change the object but give back the value from before. C++ tells the two apart by giving postfix ++ a dummy int parameter - "
          "it means nothing and doesn't even need a name. So save a copy, tick, and return the copy.",
          "Clock Clock::operator++(int) {  // postfix form\n"
          "    Clock c = *this;   // save the object\n"
          "    tick();            // increment it\n"
          "    return c;          // return it as it was\n"
          "}\n"
          "\n"
          "Clock c3(10, 59);\n"
          "Clock c1 = c3++;     // c1 is 10:59, c3 is 11:00" },
        { "Extra: Comparison operators",
          "== and < are common operators to overload. Mark them const because they only read the objects. "
          "The STL relies on them: sort uses < to order your objects, find uses ==, and a std::map needs < on its keys.",
          "class Box {\n"
          "public:\n"
          "    bool operator==(const Box& b) const {\n"
          "        return length == b.length && breadth == b.breadth && height == b.height;\n"
          "    }\n"
          "    bool operator<(const Box& b) const {\n"
          "        return getVolume() < b.getVolume();\n"
          "    }\n"
          "};" },
    },
    .quizCount = 4,
    .quiz = {
        { "Can you overload two functions that differ only by return type?", NULL, 4,
          { "Yes", "No", "Only for methods", "Only in C" }, 1,
          "The parameter lists must differ in type or number - the return type alone isn't enough." },
        { "Which declaration is the postfix ++?", NULL, 4,
          { "Clock operator++();", "Clock operator++(int);", "Clock operator+(int);", "Clock ++operator();" }, 1,
          "The dummy int parameter marks the postfix version." },
        { "What should operator<< return?", NULL, 4,
          { "void", "The object being printed", "A reference to the ostream", "An int" }, 2,
          "Returning the ostream lets you chain: cout << a << b." },
        { "c is 03:20. What does this print?", "Clock d = c++;\ncout << d << \" \" << c;", 4,
          { "03:20 03:21", "03:21 03:21", "03:21 03:20", "03:20 03:20" }, 0,
          "Postfix ++ returns the old value, then the clock has moved on." },
    },
    .projectTitle = "Box Arithmetic",
    .projectBrief =
        "Finish the lecture's Box class:\n\n"
        "getVolume() returns length * breadth * height.\n"
        "operator+ returns a new Box whose sides are the sums of the two boxes' sides.\n"
        "operator== returns true when all three sides match.\n"
        "operator<< (a friend) prints a box as  Box(6x7x5).\n\n"
        "main is written: it reads two boxes and prints them, their sum and whether they're the same. For  6 7 5 12 13 10:\n\n"
        "Box1: Box(6x7x5) volume 210\n"
        "Box2: Box(12x13x10) volume 1560\n"
        "Box3: Box(18x20x15) volume 5400\n"
        "Same size: no",
    .starterCode =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "class Box {\n"
        "    friend ostream& operator<<(ostream& os, const Box& b);\n"
        "public:\n"
        "    Box(double l = 0, double b = 0, double h = 0) : length(l), breadth(b), height(h) { }\n"
        "\n"
        "    double getVolume() const\n"
        "    {\n"
        "        return 0;\n"
        "    }\n"
        "\n"
        "    Box operator+(const Box& b) const\n"
        "    {\n"
        "        return Box();   // add the two boxes' sides\n"
        "    }\n"
        "\n"
        "    bool operator==(const Box& b) const\n"
        "    {\n"
        "        return false;   // true when all three sides match\n"
        "    }\n"
        "\n"
        "private:\n"
        "    double length, breadth, height;\n"
        "};\n"
        "\n"
        "ostream& operator<<(ostream& os, const Box& b)\n"
        "{\n"
        "    // Print Box(lengthxbreadthxheight)\n"
        "    return os;\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    double l, b, h;\n"
        "    cin >> l >> b >> h;\n"
        "    Box box1(l, b, h);\n"
        "    cin >> l >> b >> h;\n"
        "    Box box2(l, b, h);\n"
        "    Box box3 = box1 + box2;\n"
        "\n"
        "    cout << \"Box1: \" << box1 << \" volume \" << box1.getVolume() << \"\\n\";\n"
        "    cout << \"Box2: \" << box2 << \" volume \" << box2.getVolume() << \"\\n\";\n"
        "    cout << \"Box3: \" << box3 << \" volume \" << box3.getVolume() << \"\\n\";\n"
        "    cout << \"Same size: \" << (box1 == box2 ? \"yes\" : \"no\") << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .hint = "Box operator+(const Box& b) const { return Box(length + b.length, breadth + b.breadth, height + b.height); } "
            "and bool operator==(const Box& b) const { return length == b.length && ... }. "
            "In operator<<: os << \"Box(\" << b.length << \"x\" << b.breadth << \"x\" << b.height << \")\";",
    .solution =
        "#include <iostream>\n"
        "using namespace std;\n"
        "\n"
        "class Box {\n"
        "    friend ostream& operator<<(ostream& os, const Box& b);\n"
        "public:\n"
        "    Box(double l = 0, double b = 0, double h = 0) : length(l), breadth(b), height(h) { }\n"
        "\n"
        "    double getVolume() const\n"
        "    {\n"
        "        return length * breadth * height;\n"
        "    }\n"
        "\n"
        "    Box operator+(const Box& b) const\n"
        "    {\n"
        "        Box box;\n"
        "        box.length = this->length + b.length;\n"
        "        box.breadth = this->breadth + b.breadth;\n"
        "        box.height = this->height + b.height;\n"
        "        return box;\n"
        "    }\n"
        "\n"
        "    bool operator==(const Box& b) const\n"
        "    {\n"
        "        return length == b.length && breadth == b.breadth && height == b.height;\n"
        "    }\n"
        "\n"
        "private:\n"
        "    double length, breadth, height;\n"
        "};\n"
        "\n"
        "ostream& operator<<(ostream& os, const Box& b)\n"
        "{\n"
        "    os << \"Box(\" << b.length << \"x\" << b.breadth << \"x\" << b.height << \")\";\n"
        "    return os;\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    double l, b, h;\n"
        "    cin >> l >> b >> h;\n"
        "    Box box1(l, b, h);\n"
        "    cin >> l >> b >> h;\n"
        "    Box box2(l, b, h);\n"
        "    Box box3 = box1 + box2;\n"
        "\n"
        "    cout << \"Box1: \" << box1 << \" volume \" << box1.getVolume() << \"\\n\";\n"
        "    cout << \"Box2: \" << box2 << \" volume \" << box2.getVolume() << \"\\n\";\n"
        "    cout << \"Box3: \" << box3 << \" volume \" << box3.getVolume() << \"\\n\";\n"
        "    cout << \"Same size: \" << (box1 == box2 ? \"yes\" : \"no\") << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "6 7 5 12 13 10\n", "Box1: Box(6x7x5) volume 210\nBox2: Box(12x13x10) volume 1560\nBox3: Box(18x20x15) volume 5400\nSame size: no\n" },
        { "2 2 2 2 2 2\n",    "Box1: Box(2x2x2) volume 8\nBox2: Box(2x2x2) volume 8\nBox3: Box(4x4x4) volume 64\nSame size: yes\n" },
        { "1.5 2 4 0.5 1 1\n","Box1: Box(1.5x2x4) volume 12\nBox2: Box(0.5x1x1) volume 0.5\nBox3: Box(2x3x5) volume 30\nSame size: no\n" },
    },
},

// ===================================================================== 9
{
    .id = "cpp-templates", .lang = LANG_CPP,
    .title   = "Templates",
    .summary = "Function templates, class templates, methods outside the class and several type parameters.",
    .lecture = "Templates and STL",
    .section = "Templates and the STL",
    .sectionBlurb = "Generic code and the Standard Template Library.",
    .pageCount = 7,
    .pages = {
        { "One algorithm, many types",
          "Say you write a function that adds up an array of ints. Later you need the same thing for floats, so you copy it and change the types. "
          "If the requirements change, every copy has to be fixed. Templates remove that duplication: you write the algorithm once, "
          "and it works for many datatypes.",
          "int sum(int const* pArray, int const ELEMENTS) {\n"
          "    int sum = 0;\n"
          "    for (int index = 0; index < ELEMENTS; index++) {\n"
          "        sum += pArray[index];\n"
          "    }\n"
          "    return sum;\n"
          "}\n"
          "float sum(float const* pArray, int const ELEMENTS) { ...the same again... }" },
        { "Function templates",
          "template <typename T> in front of a function makes T a placeholder for a type. Use T wherever the type would go. "
          "T is just a name - DataType or anything else that isn't already a type works too.",
          "template <typename T>\n"
          "T sum(T const* pArray, int const ELEMENTS) {\n"
          "    T sum = 0;\n"
          "    for (int index = 0; index < ELEMENTS; index++) {\n"
          "        sum += pArray[index];\n"
          "    }\n"
          "    return sum;\n"
          "}\n"
          "\n"
          "cout << sum(intArray, 10);     // T is int\n"
          "cout << sum(floatArray, 10);   // T is float" },
        { "How the compiler uses templates",
          "For each type you use, the compiler copies the template and replaces T with the real type - it does the drudgery of making the "
          "copies for you. It works out T from the arguments you pass.\n\n"
          "The type of a template function is never determined by its return value, only by its parameters. "
          "You can also name the type yourself: sum<double>(values, 3).",
          NULL },
        { "Class templates",
          "A whole class can be generic too. Put template <typename T> before the class and use T inside. "
          "When you create an instance you must give the type in angle brackets after the class name.",
          "template <typename T>\n"
          "class Adder {\n"
          "private:\n"
          "    T m_sum;\n"
          "public:\n"
          "    Adder() { m_sum = 0; }\n"
          "    void add(T number) { m_sum += number; }\n"
          "    T sum() { return m_sum; }\n"
          "};\n"
          "\n"
          "Adder<int> intAdder;\n"
          "Adder<float> floatAdder;\n"
          "intAdder.add(5);" },
        { "Methods outside the class",
          "You can write a class template's methods outside the class. Each one still needs template <typename T> in front, "
          "and the class name is written with <T> after it. Templates normally live entirely in the header file, because the compiler "
          "needs the full code to make each copy.",
          "// Adder.h\n"
          "template <typename T>\n"
          "class Adder {\n"
          "    T m_sum;\n"
          "public:\n"
          "    void add(T number);\n"
          "    T sum();\n"
          "};\n"
          "\n"
          "template <typename T> void Adder<T>::add(T number) { m_sum += number; }\n"
          "template <typename T> T Adder<T>::sum() { return m_sum; }" },
        { "Several type parameters",
          "A template can have any number of type parameters, separated by commas. Give them descriptive names, since each has a different job. "
          "Here Sumtype is the type being added up and Averagetype the type returned.",
          "template <typename Sumtype, typename Averagetype>\n"
          "Averagetype average(Sumtype const* pArray, Averagetype const ELEMENTS) {\n"
          "    Sumtype sum = 0;\n"
          "    for (int i = 0; i < ELEMENTS; i++) sum += pArray[i];\n"
          "    return (Averagetype)sum / ELEMENTS;\n"
          "}\n"
          "\n"
          "average(intArray, 10);     // <int, int>:   integer average\n"
          "average(intArray, 10.0f);  // <int, float>: keeps decimals" },
        { "Extra: What T must be able to do",
          "A template only works for types that support everything the code does with T. sum needs T sum = 0 and +=, which int and float have - "
          "but a std::string can't be set to 0. Writing T() instead of 0 gives the type's default value (0 for numbers, \"\" for strings), "
          "which makes the template work for more types.\n\n"
          "If T can't do what the template needs, the error appears where you use the template, and can be long. Read the first line first.",
          "template <typename T>\n"
          "T sumAll(const T* items, int n) {\n"
          "    T total = T();    // 0, 0.0, or \"\" for strings\n"
          "    for (int i = 0; i < n; i++) total += items[i];\n"
          "    return total;\n"
          "}" },
    },
    .quizCount = 4,
    .quiz = {
        { "What is the main advantage of templates?", NULL, 4,
          { "They make code run faster", "One version of a function or class works for many types", "They replace pointers", "They avoid compile errors" }, 1,
          "Templates remove the need for a separate copy of the code for each type." },
        { "Can a template function's type be worked out from its return value?", NULL, 4,
          { "Yes, always", "Only for classes", "No - only from the parameters", "Only for int" }, 2,
          "The compiler deduces the template types from the arguments only." },
        { "How do you create an Adder that adds doubles?", NULL, 4,
          { "Adder intAdder(double);", "Adder<double> adder;", "template Adder<double>;", "Adder adder<double>;" }, 1,
          "Give the type in angle brackets after the class name." },
        { "What does T() give when T is int?", NULL, 4,
          { "Garbage", "0", "1", "A compile error" }, 1,
          "T() is the default value: 0 for numbers and an empty string for std::string." },
    },
    .projectTitle = "Template Toolkit",
    .projectBrief =
        "Finish two templates:\n\n"
        "template <typename T> T biggest(const T items[], int n) returns the largest item (using >).\n"
        "template <typename T> class Adder, like the lecture's, with add(T) and sum(). Start m_sum at T() so it also works for strings, where + joins them.\n\n"
        "main is written. It reads 3 ints, 3 doubles and 3 words and uses your templates on each. For  4 9 2  1.5 0.25 3  pear apple fig:\n\n"
        "Biggest int: 9\n"
        "Biggest double: 3\n"
        "Biggest word: pear\n"
        "Sum of ints: 15\n"
        "Sum of doubles: 4.75\n"
        "Joined words: pearapplefig",
    .starterCode =
        "#include <iostream>\n"
        "#include <string>\n"
        "using namespace std;\n"
        "\n"
        "template <typename T>\n"
        "T biggest(const T items[], int n)\n"
        "{\n"
        "    return items[0];   // return the largest item instead\n"
        "}\n"
        "\n"
        "template <typename T>\n"
        "class Adder {\n"
        "private:\n"
        "    T m_sum;\n"
        "public:\n"
        "    Adder() { }               // start m_sum at T()\n"
        "    void add(T number) { }    // add number to m_sum\n"
        "    T sum() { return T(); }   // return the total\n"
        "};\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int ints[3];\n"
        "    double doubles[3];\n"
        "    string words[3];\n"
        "    for (int i = 0; i < 3; i++) cin >> ints[i];\n"
        "    for (int i = 0; i < 3; i++) cin >> doubles[i];\n"
        "    for (int i = 0; i < 3; i++) cin >> words[i];\n"
        "\n"
        "    cout << \"Biggest int: \" << biggest(ints, 3) << \"\\n\";\n"
        "    cout << \"Biggest double: \" << biggest(doubles, 3) << \"\\n\";\n"
        "    cout << \"Biggest word: \" << biggest(words, 3) << \"\\n\";\n"
        "\n"
        "    Adder<int> intAdder;\n"
        "    Adder<double> doubleAdder;\n"
        "    Adder<string> wordAdder;\n"
        "    for (int i = 0; i < 3; i++) {\n"
        "        intAdder.add(ints[i]);\n"
        "        doubleAdder.add(doubles[i]);\n"
        "        wordAdder.add(words[i]);\n"
        "    }\n"
        "    cout << \"Sum of ints: \" << intAdder.sum() << \"\\n\";\n"
        "    cout << \"Sum of doubles: \" << doubleAdder.sum() << \"\\n\";\n"
        "    cout << \"Joined words: \" << wordAdder.sum() << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .hint = "template <typename T> T biggest(const T items[], int n) { T best = items[0]; for (int i = 1; i < n; i++) if (items[i] > best) best = items[i]; return best; } "
            "For Adder: template <typename T> class Adder { T m_sum; public: Adder() { m_sum = T(); } void add(T number) { m_sum += number; } T sum() { return m_sum; } };",
    .solution =
        "#include <iostream>\n"
        "#include <string>\n"
        "using namespace std;\n"
        "\n"
        "template <typename T>\n"
        "T biggest(const T items[], int n)\n"
        "{\n"
        "    T best = items[0];\n"
        "    for (int i = 1; i < n; i++) {\n"
        "        if (items[i] > best) {\n"
        "            best = items[i];\n"
        "        }\n"
        "    }\n"
        "    return best;\n"
        "}\n"
        "\n"
        "template <typename T>\n"
        "class Adder {\n"
        "private:\n"
        "    T m_sum;\n"
        "public:\n"
        "    Adder() { m_sum = T(); }\n"
        "    void add(T number) { m_sum += number; }\n"
        "    T sum() { return m_sum; }\n"
        "};\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int ints[3];\n"
        "    double doubles[3];\n"
        "    string words[3];\n"
        "    for (int i = 0; i < 3; i++) cin >> ints[i];\n"
        "    for (int i = 0; i < 3; i++) cin >> doubles[i];\n"
        "    for (int i = 0; i < 3; i++) cin >> words[i];\n"
        "\n"
        "    cout << \"Biggest int: \" << biggest(ints, 3) << \"\\n\";\n"
        "    cout << \"Biggest double: \" << biggest(doubles, 3) << \"\\n\";\n"
        "    cout << \"Biggest word: \" << biggest(words, 3) << \"\\n\";\n"
        "\n"
        "    Adder<int> intAdder;\n"
        "    Adder<double> doubleAdder;\n"
        "    Adder<string> wordAdder;\n"
        "    for (int i = 0; i < 3; i++) {\n"
        "        intAdder.add(ints[i]);\n"
        "        doubleAdder.add(doubles[i]);\n"
        "        wordAdder.add(words[i]);\n"
        "    }\n"
        "    cout << \"Sum of ints: \" << intAdder.sum() << \"\\n\";\n"
        "    cout << \"Sum of doubles: \" << doubleAdder.sum() << \"\\n\";\n"
        "    cout << \"Joined words: \" << wordAdder.sum() << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .testCount = 3,
    .tests = {
        { "4 9 2\n1.5 0.25 3\npear apple fig\n",
          "Biggest int: 9\nBiggest double: 3\nBiggest word: pear\nSum of ints: 15\nSum of doubles: 4.75\nJoined words: pearapplefig\n" },
        { "-1 -5 -3\n0.1 0.2 0.3\ncat cab can\n",
          "Biggest int: -1\nBiggest double: 0.3\nBiggest word: cat\nSum of ints: -9\nSum of doubles: 0.6\nJoined words: catcabcan\n" },
        { "7 7 7\n2 2 2\na b c\n",
          "Biggest int: 7\nBiggest double: 2\nBiggest word: c\nSum of ints: 21\nSum of doubles: 6\nJoined words: abc\n" },
    },
},

// ===================================================================== 10
{
    .id = "cpp-strings-vectors", .lang = LANG_CPP,
    .title   = "Strings, Vectors and Iterators",
    .summary = "The STL, std::string, vector functions, size and capacity, iterators, insert and erase.",
    .lecture = "Templates and STL",
    .pageCount = 8,
    .pages = {
        { "The Standard Template Library",
          "The STL is a big library of templates for everyday jobs like storing, searching, sorting and copying data. At its core are three ideas:\n\n"
          "Containers hold other objects: vector, list, deque (sequence containers), map (associative) and stack and queue (container adapters).\n"
          "Algorithms such as sort, min and max work on containers.\n"
          "Iterators are generalised pointers that move through a container.",
          NULL },
        { "std::string",
          "std::string is technically not part of the STL, but it's a container of characters and works with the STL algorithms. "
          "It grows by itself, compares with ==, joins with +, and has member functions such as size(), find() and substr().",
          "#include <string>\n"
          "\n"
          "string name = \"Ada\";\n"
          "string greeting = \"Hi \" + name;     // + joins strings\n"
          "cout << greeting.size() << endl;     // 6\n"
          "if (name == \"Ada\") cout << \"yes\";  // == compares the text\n"
          "getline(cin, name);                  // a whole line, spaces included" },
        { "Vector basics",
          "A vector is a dynamic array that grows and shrinks as needed - use one wherever you'd have used an ordinary array. "
          "Unlike arrays, vectors know their own size, so you never pass the count separately. at(i) is like [i] but checks the index.",
          "#include <vector>\n"
          "\n"
          "vector<int> v;\n"
          "v.push_back(10);     // add to the end - it resizes itself\n"
          "v.push_back(20);\n"
          "v.push_back(30);\n"
          "v.at(0) = v.at(2);   // copy 30 into the first slot\n"
          "v[0] = v[2];         // the same, without the check\n"
          "v.pop_back();        // erase the last item\n"
          "int size = v.size(); // 2\n"
          "v.resize(32);        // now 32 items, the new ones are 0" },
        { "Size and capacity",
          "size() is how many elements are in the vector. capacity() is how many fit in the memory it has already allocated. "
          "When they're equal, the next push_back must allocate a bigger block (often double), copy everything across and free the old one - which costs time.\n\n"
          "reserve(n) sets the capacity up front to avoid that. shrink_to_fit() gives spare capacity back.",
          "vector<int> v(10, 0);   // size 10, all zeros\n"
          "cout << v.capacity();   // 10\n"
          "v.push_back(20);        // reallocates: size 11, capacity grows\n"
          "\n"
          "vector<int> big;\n"
          "big.reserve(100000);    // one allocation, ready for lots of items" },
        { "Iterators",
          "An iterator works like a pointer into a container: * gets the item and ++ and -- move along. begin() points to the first item, "
          "and end() points one past the last - so it's not a valid item, but it's perfect for stopping a loop.",
          "vector<int> v = {30, 10, 5};\n"
          "vector<int>::iterator iter = v.begin();\n"
          "int i = *iter;     // 30\n"
          "++iter;            // move to 10\n"
          "--iter;            // back to 30\n"
          "*iter = 35;        // v is now 35, 10, 5\n"
          "\n"
          "for (iter = v.begin(); iter != v.end(); iter++) {\n"
          "    cout << *iter << endl;\n"
          "}" },
        { "insert and erase",
          "insert and erase take iterators that say where. Inserting or erasing in the middle of a vector moves every later element along by one, "
          "so if you do it a lot, a std::list is a better choice.",
          "// v is 35, 10, 5\n"
          "v.insert(v.begin(), 3, 100);   // 100 100 100 35 10 5\n"
          "\n"
          "vector<int>::iterator iter = v.begin();\n"
          "iter++;\n"
          "v.erase(iter);                 // remove one: 100 100 35 10 5\n"
          "v.erase(v.begin(), v.begin() + 2);  // remove a range: 35 10 5" },
        { "Iterator invalidation",
          "Inserting into or erasing from a vector can invalidate iterators, pointers and references into it - they may point at memory that has moved. "
          "push_back can reallocate, so any iterator from before it is no longer safe. erase invalidates iterators at and after the erased element.",
          "vector<int> v;\n"
          "v.push_back(30);\n"
          "vector<int>::iterator iter = v.begin();\n"
          "v.push_back(10);        // may reallocate: iter is now invalid\n"
          "cout << *iter << endl;  // runtime error / undefined!\n"
          "\n"
          "iter = v.begin();       // fix: get a fresh iterator" },
        { "Extra: Range-based for and auto",
          "Modern C++ has shortcuts for loops over containers. for (int x : v) visits every element - add & to change them in place. "
          "auto lets the compiler work out a long type such as vector<int>::iterator for you.",
          "for (int x : v) {           // read each element\n"
          "    cout << x << \" \";\n"
          "}\n"
          "for (int &x : v) {          // change each element\n"
          "    x *= 2;\n"
          "}\n"
          "auto iter = v.begin();      // same as vector<int>::iterator" },
    },
    .quizCount = 4,
    .quiz = {
        { "How do you add 5 to the end of vector<int> v?", NULL, 4,
          { "v.add(5)", "v.push_back(5)", "v[end] = 5", "v + 5" }, 1,
          "push_back adds to the end and grows the vector if needed." },
        { "What does v.end() point to?", NULL, 4,
          { "The last element", "The first element", "One past the last element", "nullptr" }, 2,
          "end() is one past the last element, so it can't be dereferenced - but it's perfect for stopping loops." },
        { "What is the difference between size() and capacity()?", NULL, 4,
          { "They are the same", "size is elements stored, capacity is how many fit before reallocating", "capacity is always 2 x size", "size is in bytes" }, 1,
          "When size reaches capacity, the next insertion makes the vector reallocate." },
        { "Why is this code wrong?", "auto iter = v.begin();\nv.push_back(10);\ncout << *iter;", 4,
          { "It isn't", "push_back may have invalidated iter", "begin() needs an argument", "auto isn't allowed" }, 1,
          "push_back can move the vector's memory, leaving iter pointing at freed memory." },
    },
    .projectTitle = "Shopping List",
    .projectBrief =
        "Read words until there are no more (use while (cin >> item)), storing each one in a vector<string>.\n\n"
        "Then print how many items there are, each item numbered in the order given, and the longest item "
        "(if there is a tie, the one that came first).\n\n"
        "For the input milk bread cheese the output should be:\n\n"
        "Items: 3\n"
        "1. milk\n"
        "2. bread\n"
        "3. cheese\n"
        "Longest: cheese",
    .starterCode =
        "#include <iostream>\n"
        "#include <string>\n"
        "#include <vector>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    vector<string> list;\n"
        "    string item;\n"
        "\n"
        "    // Read every word into list\n"
        "\n"
        "    // Print the count, the numbered items and the longest one\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "while (cin >> item) { list.push_back(item); } reads everything. Print with for (size_t i = 0; i < list.size(); i++) using i + 1 as the number. Track the longest by comparing .size() with > so ties keep the first.",
    .solution =
        "#include <iostream>\n"
        "#include <string>\n"
        "#include <vector>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    vector<string> list;\n"
        "    string item;\n"
        "\n"
        "    while (cin >> item) {\n"
        "        list.push_back(item);\n"
        "    }\n"
        "\n"
        "    cout << \"Items: \" << list.size() << \"\\n\";\n"
        "    string longest = list[0];\n"
        "    for (size_t i = 0; i < list.size(); i++) {\n"
        "        cout << i + 1 << \". \" << list[i] << \"\\n\";\n"
        "        if (list[i].size() > longest.size()) {\n"
        "            longest = list[i];\n"
        "        }\n"
        "    }\n"
        "    cout << \"Longest: \" << longest << \"\\n\";\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "milk bread cheese\n",           "Items: 3\n1. milk\n2. bread\n3. cheese\nLongest: cheese\n" },
        { "eggs\n",                        "Items: 1\n1. eggs\nLongest: eggs\n" },
        { "tea jam pie\n",                 "Items: 3\n1. tea\n2. jam\n3. pie\nLongest: tea\n" },
        { "apples bananas kiwi cherries\n","Items: 4\n1. apples\n2. bananas\n3. kiwi\n4. cherries\nLongest: cherries\n" },
    },
},

// ===================================================================== 11
{
    .id = "cpp-algorithms", .lang = LANG_CPP,
    .title   = "STL Algorithms",
    .summary = "sort with comparators, the erase-remove idiom, remove_if with predicates and range functions.",
    .lecture = "Templates and STL",
    .pageCount = 8,
    .pages = {
        { "Sorting",
          "<algorithm> has more than 70 generic algorithms that work on containers through iterators. sort takes two iterators and sorts everything "
          "between them - not including the item the end iterator points to, so begin() and end() sort the whole container.",
          "#include <algorithm>\n"
          "\n"
          "vector<int> v = {30, 10, 5, 20};\n"
          "sort(v.begin(), v.end());        // 5 10 20 30\n"
          "\n"
          "for (auto iter = v.begin(); iter != v.end(); iter++) {\n"
          "    cout << *iter << endl;\n"
          "}" },
        { "Sorting with a comparator",
          "To sort objects, or to sort in a different order, pass a comparison function as the third argument. "
          "It takes two items and returns true if the first should come before the second.",
          "class MyClass {\n"
          "public:\n"
          "    int num;\n"
          "};\n"
          "\n"
          "bool Compare(const MyClass &a, const MyClass &b) {\n"
          "    return a.num < b.num;\n"
          "}\n"
          "\n"
          "vector<MyClass> v{ {6}, {1}, {2}, {4} };\n"
          "sort(v.begin(), v.end(), Compare);   // 1 2 4 6" },
        { "remove doesn't remove!",
          "std::remove(start, finish, value) gets rid of every element equal to value - but it can't change the vector's size. "
          "It moves the elements you're keeping to the front and returns an iterator to the new end. Everything after that is junk, "
          "and the vector still has its old size.",
          "// v is 3 0 1 2 3 4 5 6 7 8 9   (11 elements)\n"
          "vector<int>::iterator endIter = remove(v.begin(), v.end(), 3);\n"
          "// v is 0 1 2 4 5 6 7 8 9 8 9   (still 11!)\n"
          "//                         ^ endIter points here" },
        { "The erase-remove idiom",
          "To really delete the elements, erase the junk from remove's new end to the real end. Doing it in one line is so common it has a name: "
          "the erase-remove idiom. It's the best way to remove all objects with a specific value from a vector, deque or string.",
          "vector<int>::iterator endIter = remove(v.begin(), v.end(), 3);\n"
          "v.erase(endIter, v.end());       // now 0 1 2 4 5 6 7 8 9\n"
          "\n"
          "// in one line:\n"
          "v.erase(remove(v.begin(), v.end(), 3), v.end());" },
        { "Predicates and remove_if",
          "A predicate is a function that returns true or false. remove_if removes every element for which the predicate is true - "
          "for example particles whose time to live has run out. Combine it with erase just like remove.",
          "class Particle {\n"
          "    int m_timetolive;\n"
          "public:\n"
          "    Particle(int ttl) : m_timetolive(ttl) {}\n"
          "    int timetolive() const { return m_timetolive; }\n"
          "};\n"
          "\n"
          "bool isExpired(const Particle &p) { return p.timetolive() == 0; }\n"
          "\n"
          "v.erase(remove_if(v.begin(), v.end(), isExpired), v.end());" },
        { "Range member functions",
          "Many container functions take a range of two iterators (or pointers) instead of one item. Copying an array into the front of a vector "
          "with one range insert is much faster than inserting one element at a time: one call, each existing element moved only once, "
          "and at most one memory reallocation.",
          "int data[] = {32, 71, 12, 45, 26, 80, 53, 33};\n"
          "int numValues = 8;\n"
          "\n"
          "// slow: numValues separate inserts\n"
          "// fast: one range insert\n"
          "v.insert(v.begin(), data, data + numValues);" },
        { "Containers hold copies",
          "STL containers copy objects in and copy them out, so changing the element inside the vector doesn't change your original object. "
          "If copying is expensive you can store pointers instead - but then the container will NOT delete the objects when it is destroyed. "
          "Deleting that memory is your job.",
          "Landmark carlow(0, 0);\n"
          "vector<Landmark> places;\n"
          "places.push_back(carlow);   // a copy goes in\n"
          "places[0].m_x = 10;         // carlow.m_x is still 0\n"
          "\n"
          "vector<SomeClass*> vec;\n"
          "vec.push_back(new SomeClass());   // leaks unless you delete it" },
        { "Extra: Find, count and lambdas",
          "Some other handy algorithms: find returns an iterator to the first match (or end()), count counts matches, and "
          "min_element / max_element return iterators to the smallest and largest. The _if versions take a predicate.\n\n"
          "A lambda writes a small predicate right where you need it: [](int x) { return x < 0; }",
          "auto it = find(v.begin(), v.end(), 42);\n"
          "if (it != v.end()) cout << \"found\";\n"
          "\n"
          "int sevens = count(v.begin(), v.end(), 7);\n"
          "int biggest = *max_element(v.begin(), v.end());\n"
          "int negatives = count_if(v.begin(), v.end(), [](int x) { return x < 0; });" },
    },
    .quizCount = 4,
    .quiz = {
        { "After v.erase(remove(v.begin(), v.end(), 3), v.end()); what has happened?", NULL, 4,
          { "Only the first 3 is gone", "Every 3 is gone and the size has shrunk", "The vector is sorted", "Nothing until you call shrink_to_fit" }, 1,
          "remove packs the kept elements to the front, and erase cuts off the leftovers." },
        { "What does a predicate return?", NULL, 4,
          { "An iterator", "true or false", "A copy of the element", "The container's size" }, 1,
          "Predicates are yes/no functions, used by algorithms like remove_if and count_if." },
        { "What should a sort comparator Compare(a, b) return?", NULL, 4,
          { "true if a should come before b", "a - b", "true if a == b", "The larger value" }, 0,
          "sort puts a before b whenever the comparator returns true." },
        { "A vector<SomeClass*> goes out of scope. What happens to the objects?", NULL, 4,
          { "They are deleted automatically", "They are copied", "Nothing - they leak unless you delete them", "The program crashes" }, 2,
          "The vector only frees its own memory - the pointers, not the objects they point to." },
    },
    .projectTitle = "Score Cleaner",
    .projectBrief =
        "Read how many scores there are, then the scores into a vector<int>. Negative scores are mistakes.\n\n"
        "1. Remove every negative score with the erase-remove_if idiom and a predicate function isNegative.\n"
        "2. Sort the rest from highest to lowest with a comparator function descending.\n"
        "3. Print what you removed, the sorted scores, the highest and lowest, and how many are 50 or more.\n\n"
        "For  6  then  80 -1 95 42 -7 80:\n"
        "Removed: 2\n"
        "Sorted: 95 80 80 42\n"
        "Highest: 95\n"
        "Lowest: 42\n"
        "Passed: 3\n\n"
        "If no scores are left, print  Removed: N  and then  No valid scores.",
    .starterCode =
        "#include <iostream>\n"
        "#include <vector>\n"
        "#include <algorithm>\n"
        "using namespace std;\n"
        "\n"
        "bool isNegative(int score)\n"
        "{\n"
        "    return false;\n"
        "}\n"
        "\n"
        "bool descending(int a, int b)\n"
        "{\n"
        "    return false;\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int n;\n"
        "    cin >> n;\n"
        "    vector<int> scores;\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        int s;\n"
        "        cin >> s;\n"
        "        scores.push_back(s);\n"
        "    }\n"
        "\n"
        "    // Remove the negatives, sort, and print the results\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "size_t before = scores.size(); scores.erase(remove_if(scores.begin(), scores.end(), isNegative), scores.end()); "
            "Removed is before - scores.size(). sort(scores.begin(), scores.end(), descending); with descending returning a > b. "
            "After sorting, the highest is scores.front() and the lowest scores.back().",
    .solution =
        "#include <iostream>\n"
        "#include <vector>\n"
        "#include <algorithm>\n"
        "using namespace std;\n"
        "\n"
        "bool isNegative(int score)\n"
        "{\n"
        "    return score < 0;\n"
        "}\n"
        "\n"
        "bool descending(int a, int b)\n"
        "{\n"
        "    return a > b;\n"
        "}\n"
        "\n"
        "bool passed(int score)\n"
        "{\n"
        "    return score >= 50;\n"
        "}\n"
        "\n"
        "int main()\n"
        "{\n"
        "    int n;\n"
        "    cin >> n;\n"
        "    vector<int> scores;\n"
        "    for (int i = 0; i < n; i++) {\n"
        "        int s;\n"
        "        cin >> s;\n"
        "        scores.push_back(s);\n"
        "    }\n"
        "\n"
        "    size_t before = scores.size();\n"
        "    scores.erase(remove_if(scores.begin(), scores.end(), isNegative), scores.end());\n"
        "    cout << \"Removed: \" << before - scores.size() << \"\\n\";\n"
        "\n"
        "    if (scores.empty()) {\n"
        "        cout << \"No valid scores\\n\";\n"
        "        return 0;\n"
        "    }\n"
        "\n"
        "    sort(scores.begin(), scores.end(), descending);\n"
        "    cout << \"Sorted:\";\n"
        "    for (vector<int>::iterator it = scores.begin(); it != scores.end(); it++) {\n"
        "        cout << \" \" << *it;\n"
        "    }\n"
        "    cout << \"\\n\";\n"
        "\n"
        "    cout << \"Highest: \" << scores.front() << \"\\n\";\n"
        "    cout << \"Lowest: \" << scores.back() << \"\\n\";\n"
        "    cout << \"Passed: \" << count_if(scores.begin(), scores.end(), passed) << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "6\n80 -1 95 42 -7 80\n", "Removed: 2\nSorted: 95 80 80 42\nHighest: 95\nLowest: 42\nPassed: 3\n" },
        { "3\n10 20 30\n",          "Removed: 0\nSorted: 30 20 10\nHighest: 30\nLowest: 10\nPassed: 0\n" },
        { "2\n-5 -6\n",             "Removed: 2\nNo valid scores\n" },
        { "5\n50 49 100 0 -3\n",    "Removed: 1\nSorted: 100 50 49 0\nHighest: 100\nLowest: 0\nPassed: 2\n" },
    },
},

// ===================================================================== 12
{
    .id = "cpp-lists", .lang = LANG_CPP,
    .title   = "std::list, Stacks and Queues",
    .summary = "The STL doubly linked list, splice, empty() vs size(), and the stack and queue adapters.",
    .lecture = "STL Lists and Maps",
    .pageCount = 7,
    .pages = {
        { "std::list",
          "The STL list class is a doubly linked list: each node points to both the next and the previous node, so you can move through it in either direction. "
          "It is a sequence, but not random access - there's no list[3]. You reach items with iterators, or front() and back().",
          "#include <list>\n"
          "\n"
          "list<int> listObject;\n"
          "listObject.push_back(50);\n"
          "listObject.push_back(40);\n"
          "listObject.push_front(10);\n"
          "listObject.push_front(20);    // 20 10 50 40\n"
          "cout << listObject.front();   // 20" },
        { "Sorting and iterating",
          "The general sort algorithm needs random access, so list has its own sort() member function. "
          "Loop over a list with iterators exactly like a vector - and change items through the iterator.",
          "listObject.sort();                // 10 20 40 50\n"
          "\n"
          "list<int>::iterator iterEnd = listObject.end();\n"
          "for (list<int>::iterator it = listObject.begin(); it != iterEnd; ++it) {\n"
          "    (*it) *= 2;                    // 20 40 80 100\n"
          "}" },
        { "Splicing",
          "To splice means to join. Only std::list can move elements from one place to another without copying any data - it just re-links nodes. "
          "splice(position, otherList) moves everything from otherList in before position, and otherList ends up empty. "
          "Other versions move a single element or a range.",
          "list<int> mylist1 = {1, 2, 3, 4};\n"
          "list<int> mylist2 = {10, 20, 30};\n"
          "\n"
          "list<int>::iterator it = mylist1.begin();\n"
          "++it;                           // points to 2\n"
          "mylist1.splice(it, mylist2);    // mylist1: 1 10 20 30 2 3 4\n"
          "                                // mylist2: (empty)\n"
          "mylist2.splice(mylist2.begin(), mylist1, it);   // moves just the 2" },
        { "empty() or size() == 0?",
          "There are two ways to check for an empty container: size() == 0 and empty(). Prefer empty(). It is guaranteed to be fast for every "
          "standard container, while list::size() is slow on some implementations because of the way splice works.",
          "if (listObject.empty()) {         // preferred\n"
          "    cout << \"nothing here\";\n"
          "}\n"
          "if (listObject.size() == 0) {     // works, but can be slower\n"
          "}" },
        { "list or vector?",
          "Use a vector wherever you might use an array: it is compact and has fast random access.\n\n"
          "Use a list when your program frequently inserts and removes elements from the middle of the sequence. "
          "A vector has to copy every later element back one position to fill the gap; a list just re-links two pointers.",
          NULL },
        { "Extra: std::stack",
          "stack is a container adapter: it wraps another container (a deque by default) and only lets you work at one end - Last In, First Out (LIFO), "
          "like a stack of plates. push adds to the top, top() looks at it and pop() removes it. Undo buttons and checking brackets use stacks.",
          "#include <stack>\n"
          "\n"
          "stack<int> s;\n"
          "s.push(1);\n"
          "s.push(2);\n"
          "s.push(3);\n"
          "cout << s.top();    // 3 - the last one in\n"
          "s.pop();            // removes 3 (pop returns nothing)\n"
          "cout << s.top();    // 2" },
        { "Extra: std::queue",
          "queue is First In, First Out (FIFO), like a queue at a shop: push adds at the back, front() is the next one out and pop() removes it. "
          "Turn-based games, print jobs and messages waiting to be handled all use queues.",
          "#include <queue>\n"
          "\n"
          "queue<string> q;\n"
          "q.push(\"Ann\");\n"
          "q.push(\"Ben\");\n"
          "cout << q.front();   // Ann - the first one in\n"
          "q.pop();\n"
          "cout << q.front();   // Ben\n"
          "cout << q.size();    // 1" },
    },
    .quizCount = 4,
    .quiz = {
        { "Can you write myList[2] for a std::list?", NULL, 4,
          { "Yes", "No - lists aren't random access", "Only for list<int>", "Only after sort()" }, 1,
          "Lists are reached with iterators, front() and back()." },
        { "What is in mylist2 after mylist1.splice(it, mylist2)?", NULL, 4,
          { "The same as before", "Nothing - its elements moved into mylist1", "A copy of mylist1", "Only its first element" }, 1,
          "splice moves the nodes without copying, so the source list is left empty." },
        { "A stack has 1, 2, 3 pushed in that order. What is top()?", NULL, 4,
          { "1", "2", "3", "6" }, 2,
          "A stack is Last In, First Out." },
        { "Which container fits frequent inserting in the middle best?", NULL, 4,
          { "vector", "list", "array", "stack" }, 1,
          "A list just re-links pointers; a vector must shift every later element." },
    },
    .projectTitle = "Ticket Queue",
    .projectBrief =
        "Keep the queue at a ticket desk in a list<string>. Read commands until the input runs out:\n\n"
        "join NAME - add NAME at the back\n"
        "vip NAME - add NAME at the front\n"
        "serve - print  Serving NAME  for the person at the front and remove them, or  Nobody waiting  if the list is empty\n"
        "leave NAME - remove NAME wherever they are (list has a remove(value) member function)\n\n"
        "At the end print everyone still waiting, front first:  Still waiting: Ben Cara  - or  Still waiting: nobody",
    .starterCode =
        "#include <iostream>\n"
        "#include <list>\n"
        "#include <string>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    list<string> line;\n"
        "    string command;\n"
        "\n"
        "    while (cin >> command) {\n"
        "        // Handle join, vip, serve and leave\n"
        "    }\n"
        "\n"
        "    // Print who is still waiting\n"
        "    return 0;\n"
        "}\n",
    .hint = "For join and vip, read the name with cin >> name; then push_back or push_front. For serve, check line.empty() first, "
            "then print line.front() and call line.pop_front();. For leave, line.remove(name);. "
            "Finish with an iterator loop over the list.",
    .solution =
        "#include <iostream>\n"
        "#include <list>\n"
        "#include <string>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    list<string> line;\n"
        "    string command;\n"
        "\n"
        "    while (cin >> command) {\n"
        "        string name;\n"
        "        if (command == \"join\") {\n"
        "            cin >> name;\n"
        "            line.push_back(name);\n"
        "        } else if (command == \"vip\") {\n"
        "            cin >> name;\n"
        "            line.push_front(name);\n"
        "        } else if (command == \"serve\") {\n"
        "            if (line.empty()) {\n"
        "                cout << \"Nobody waiting\\n\";\n"
        "            } else {\n"
        "                cout << \"Serving \" << line.front() << \"\\n\";\n"
        "                line.pop_front();\n"
        "            }\n"
        "        } else if (command == \"leave\") {\n"
        "            cin >> name;\n"
        "            line.remove(name);\n"
        "        }\n"
        "    }\n"
        "\n"
        "    cout << \"Still waiting:\";\n"
        "    if (line.empty()) {\n"
        "        cout << \" nobody\";\n"
        "    }\n"
        "    for (list<string>::iterator it = line.begin(); it != line.end(); ++it) {\n"
        "        cout << \" \" << *it;\n"
        "    }\n"
        "    cout << \"\\n\";\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "join Ann\njoin Ben\nvip Zed\nserve\njoin Cara\n", "Serving Zed\nStill waiting: Ann Ben Cara\n" },
        { "serve\n",                                          "Nobody waiting\nStill waiting: nobody\n" },
        { "join A\njoin B\njoin C\nleave B\nserve\n",          "Serving A\nStill waiting: C\n" },
        { "vip X\nvip Y\nserve\nserve\nserve\n",               "Serving Y\nServing X\nNobody waiting\nStill waiting: nobody\n" },
    },
},

// ===================================================================== 13
{
    .id = "cpp-maps", .lang = LANG_CPP,
    .title   = "std::map",
    .summary = "Key/value maps: [], insert and make_pair, find and count, iterating, erase and unordered_map.",
    .lecture = "STL Lists and Maps",
    .pageCount = 7,
    .pages = {
        { "What is a map?",
          "A map (or table) maps keys to values, so you can find information quickly by its key. The keys are unique - each key maps to at most one value - "
          "but two keys can map to the same value. A DNS server mapping host names to IP addresses is a real-world example.\n\n"
          "std::map keeps its keys sorted (it's built on a binary tree), so keys must be comparable with <.",
          "#include <map>\n"
          "\n"
          "map<string, int> population;      // key type, value type\n"
          "population[\"carlow\"] = 15000;\n"
          "cout << population[\"carlow\"];   // 15000" },
        { "Storing and changing values",
          "The index operator [] does both jobs. Used on the left of =, it stores a value. Used to read, it fetches the value for that key. "
          "Each key is stored only once: assigning to a key that's already there replaces the old value, which is lost.",
          "map<string, int> freq;\n"
          "freq[\"One\"] = 1;          // insert\n"
          "int value = freq[\"One\"];  // retrieve: 1\n"
          "freq[\"One\"] = 134;        // replace: now 134\n"
          "freq.erase(\"One\");        // remove the key and its value" },
        { "Looking things up safely",
          "find(key) returns an iterator to the key/value pair, or end() if the key isn't there. Each pair has first (the key) and second (the value). "
          "count(key) returns 1 if the key exists and 0 if not - an easy way to ask \"is it in the map?\".",
          "map<string, int>::iterator iter = freq.find(\"One\");\n"
          "if (iter != freq.end()) {\n"
          "    cout << iter->first << \" = \" << iter->second;\n"
          "}\n"
          "\n"
          "if (freq.count(\"Two\") > 0) {\n"
          "    cout << \"Two is in the map\";\n"
          "}" },
        { "insert and make_pair",
          "The preferred way to add an entry is insert with a pair. Beware: insert will not replace a key that already exists. "
          "It returns a pair of its own - an iterator to the entry, and a bool that's false if the key was already there - so you can tell what happened.",
          "freq.insert(make_pair(\"one\", 3));\n"
          "\n"
          "pair<map<string, int>::iterator, bool> result =\n"
          "    freq.insert(make_pair(\"one\", 1));\n"
          "if (result.second == false) {\n"
          "    // the key existed: result.first points at the old entry\n"
          "    result.first->second++;\n"
          "}" },
        { "Looping over a map",
          "Iterate from begin() to end() like any container. Because std::map keeps keys sorted, the loop visits them in order - alphabetical for strings. "
          "The lecture's URL shortener prints all its mappings this way.",
          "map<string, string> urlMapping;\n"
          "urlMapping[\"abc123\"] = \"https://www.example.com/some/long/url\";\n"
          "\n"
          "map<string, string>::iterator it;\n"
          "for (it = urlMapping.begin(); it != urlMapping.end(); it++) {\n"
          "    cout << it->first << ':' << it->second << endl;\n"
          "}" },
        { "unordered_map",
          "C++11 adds std::unordered_map, built on a hash table instead of a tree. It's used in the same way and is usually faster, "
          "but its keys are not sorted, so a loop visits them in no particular order. Pick map when you need the keys in order.\n\n"
          "Don't use pointers as map keys: sorting pointers is not the same as sorting what they point at.",
          "#include <unordered_map>\n"
          "\n"
          "unordered_map<string, int> ages;\n"
          "ages[\"Ann\"] = 30;\n"
          "ages[\"Ben\"] = 25;   // same functions as map, no ordering" },
        { "Extra: The [] trap",
          "Reading a missing key with [] doesn't fail - it quietly inserts that key with a default value (0 for numbers) and returns it. "
          "That makes counting easy, because freq[word]++ works even for a new word. But it also means if (freq[\"x\"] == 0) adds \"x\" to your map! "
          "When you only want to check, use count or find.",
          "map<string, int> freq;\n"
          "freq[\"cat\"]++;               // cat = 1: a new key starts at 0\n"
          "freq[\"cat\"]++;               // cat = 2\n"
          "\n"
          "if (freq[\"dog\"] == 0) { }     // oops: dog is now in the map\n"
          "cout << freq.size();         // 2\n"
          "if (freq.count(\"emu\") == 0) { }   // safe: nothing added" },
    },
    .quizCount = 4,
    .quiz = {
        { "In what order does a loop over a std::map<string, int> visit the keys?", NULL, 4,
          { "The order they were added", "Sorted (alphabetical)", "Random", "Reverse order" }, 1,
          "std::map keeps its keys sorted. unordered_map is the one with no order." },
        { "\"One\" is already in freq. What does freq.insert(make_pair(\"One\", 5)) do?", NULL, 4,
          { "Replaces the value with 5", "Adds a second \"One\"", "Nothing - result.second is false", "Throws an error" }, 2,
          "insert never overwrites an existing key. freq[\"One\"] = 5; would." },
        { "What does find return when the key isn't in the map?", NULL, 4,
          { "nullptr", "0", "The map's end() iterator", "An empty string" }, 2,
          "Compare the result with map.end() to see whether the key was found." },
        { "What does reading freq[\"dog\"] do when \"dog\" isn't in the map?", NULL, 4,
          { "Crashes", "Returns 0 and adds \"dog\" to the map", "Returns -1", "Does nothing" }, 1,
          "[] inserts missing keys with a default value. Use count or find to check without adding." },
    },
    .projectTitle = "Digit Counter",
    .projectBrief =
        "This is the lecture's map exercise. Read a whole number (0 or more, up to 18 digits) and count how many times each digit appears, "
        "using a map<string, int> keyed by the digit's name: zero, one, two, ... nine.\n\n"
        "Peel the digits off with % 10 and / 10, like the lecture does. Then loop over the map and print each entry as  name = count. "
        "Because the map is sorted, the names come out in alphabetical order. For 12345033:\n\n"
        "five = 1\n"
        "four = 1\n"
        "one = 1\n"
        "three = 3\n"
        "two = 1\n"
        "zero = 1",
    .starterCode =
        "#include <iostream>\n"
        "#include <map>\n"
        "#include <string>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    string numbers[] = { \"zero\", \"one\", \"two\", \"three\", \"four\",\n"
        "                         \"five\", \"six\", \"seven\", \"eight\", \"nine\" };\n"
        "    map<string, int> freq;\n"
        "    long long number;\n"
        "    cin >> number;\n"
        "\n"
        "    // Count each digit, then print the map\n"
        "\n"
        "    return 0;\n"
        "}\n",
    .hint = "Use a do { ... } while (number > 0); loop so that 0 still counts one zero. Inside: int j = number % 10; freq[numbers[j]]++; number = number / 10;. "
            "Print with for (map<string, int>::iterator it = freq.begin(); it != freq.end(); ++it) cout << it->first << \" = \" << it->second << endl;",
    .solution =
        "#include <iostream>\n"
        "#include <map>\n"
        "#include <string>\n"
        "using namespace std;\n"
        "\n"
        "int main()\n"
        "{\n"
        "    string numbers[] = { \"zero\", \"one\", \"two\", \"three\", \"four\",\n"
        "                         \"five\", \"six\", \"seven\", \"eight\", \"nine\" };\n"
        "    map<string, int> freq;\n"
        "    long long number;\n"
        "    cin >> number;\n"
        "\n"
        "    do {\n"
        "        int j = number % 10;\n"
        "        map<string, int>::iterator iter = freq.find(numbers[j]);\n"
        "        int frequency;\n"
        "        if (iter == freq.end()) {\n"
        "            frequency = 0;\n"
        "        } else {\n"
        "            frequency = iter->second;\n"
        "        }\n"
        "        freq[numbers[j]] = ++frequency;\n"
        "        number = number / 10;\n"
        "    } while (number > 0);\n"
        "\n"
        "    for (map<string, int>::iterator it = freq.begin(); it != freq.end(); ++it) {\n"
        "        cout << it->first << \" = \" << it->second << endl;\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
    .testCount = 4,
    .tests = {
        { "12345033\n",   "five = 1\nfour = 1\none = 1\nthree = 3\ntwo = 1\nzero = 1\n" },
        { "0\n",          "zero = 1\n" },
        { "9999\n",       "nine = 4\n" },
        { "1234567890\n", "eight = 1\nfive = 1\nfour = 1\nnine = 1\none = 1\nseven = 1\nsix = 1\nthree = 1\ntwo = 1\nzero = 1\n" },
    },
},

};

const int CPP_TUTORIAL_COUNT = (int)(sizeof(CPP_TUTORIALS) / sizeof(CPP_TUTORIALS[0]));
