#include "tutorials.h"
#include <stddef.h>

// ---------------------------------------------------------------------------
// C++ section. Projects here are compiled with g++ (-std=c++17).
// ---------------------------------------------------------------------------

const Tutorial CPP_TUTORIALS[] = {

// ===================================================================== 1
{
    .id = "cpp-hello", .lang = LANG_CPP,
    .title   = "Hello, C++",
    .summary = "From C to C++: printing with cout and reading with cin.",
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
    .id = "cpp-strings-vectors", .lang = LANG_CPP,
    .title   = "Strings and Vectors",
    .summary = "std::string for text and std::vector for lists that grow by themselves.",
    .pageCount = 4,
    .pages = {
        { "std::string",
          "#include <string> gives you a real string type. No more char arrays, strcpy or strcmp: you can assign with =, "
          "join with +, compare with == and ask for the length with .size().",
          "string name = \"Ada\";\n"
          "string greeting = \"Hello, \" + name;\n"
          "if (name == \"Ada\") {\n"
          "    cout << greeting << \"\\n\";   // Hello, Ada\n"
          "}\n"
          "cout << name.size() << \"\\n\";    // 3" },
        { "Reading text",
          "cin >> word reads one word, stopping at a space. getline(cin, line) reads a whole line, spaces and all.",
          "string word, line;\n"
          "cin >> word;\n"
          "getline(cin, line);" },
        { "std::vector",
          "#include <vector> gives you a list that grows as needed - like an array that resizes itself. "
          "push_back adds to the end, .size() says how many there are, and [i] reads an element just like an array.",
          "vector<int> nums;\n"
          "nums.push_back(4);\n"
          "nums.push_back(9);\n"
          "cout << nums.size() << \"\\n\";   // 2\n"
          "cout << nums[0] << \"\\n\";       // 4" },
        { "Looping over a vector",
          "A range-based for loop visits every element without needing an index. "
          "When you do need the index, a normal for loop up to .size() works too.",
          "for (int x : nums) {\n"
          "    cout << x << \"\\n\";\n"
          "}\n"
          "\n"
          "for (size_t i = 0; i < nums.size(); i++) {\n"
          "    cout << i << \": \" << nums[i] << \"\\n\";\n"
          "}" },
    },
    .quizCount = 3,
    .quiz = {
        { "How do you add 5 to the end of vector<int> v?", NULL, 4,
          { "v.add(5)", "v.push_back(5)", "v[end] = 5", "v + 5" }, 1,
          "push_back adds an element to the end and grows the vector." },
        { "What does this print?", "string a = \"ab\";\nstring b = a + \"cd\";\ncout << b.size();", 4,
          { "2", "abcd", "4", "6" }, 2,
          "b is \"abcd\", which has 4 characters." },
        { "Which reads a whole line, including spaces?", NULL, 4,
          { "cin >> s;", "scanf(\"%s\", s);", "cout << s;", "getline(cin, s);" }, 3,
          "cin >> stops at the first space. getline reads up to the end of the line." },
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

// ===================================================================== 3
{
    .id = "cpp-classes", .lang = LANG_CPP,
    .title   = "Classes",
    .summary = "Bundling data with the functions that work on it: classes, objects and constructors.",
    .pageCount = 4,
    .pages = {
        { "What is a class?",
          "A class is like a struct that also has functions built in. Those functions are called methods. "
          "A class bundles data together with the actions that work on that data, so the two always travel together.",
          NULL },
        { "Writing a class",
          "Members under public: can be used from anywhere. Members under private: can only be used by the class's own methods. "
          "Keeping the data private stops other code from putting it into a broken state.",
          "class Counter {\n"
          "public:\n"
          "    void increment() { count++; }\n"
          "    int get() { return count; }\n"
          "\n"
          "private:\n"
          "    int count = 0;\n"
          "};" },
        { "Objects",
          "A variable whose type is a class is called an object. Each object has its own copy of the data. "
          "Call its methods with a dot, just like reaching a struct's fields.",
          "Counter a, b;\n"
          "a.increment();\n"
          "a.increment();\n"
          "b.increment();\n"
          "cout << a.get() << \" \" << b.get() << \"\\n\";   // 2 1" },
        { "Constructors",
          "A constructor is a method with the same name as the class and no return type. It runs automatically when an "
          "object is created, so the object starts life in a sensible state.",
          "class Timer {\n"
          "public:\n"
          "    Timer(int start) { seconds = start; }\n"
          "    int left() { return seconds; }\n"
          "\n"
          "private:\n"
          "    int seconds;\n"
          "};\n"
          "\n"
          "Timer t(60);   // the constructor sets seconds to 60" },
    },
    .quizCount = 3,
    .quiz = {
        { "Where can private members be used?", NULL, 4,
          { "Anywhere", "Only in main", "Only inside the class's own methods", "Only in other classes" }, 2,
          "Private data can only be touched by the class itself, which keeps it safe." },
        { "When does a constructor run?", NULL, 4,
          { "When the object is created", "When the program ends", "Every time a method is called", "Only if you call it by name" }, 0,
          "Creating the object - Timer t(60); - runs the constructor automatically." },
        { "What is an object?", NULL, 4,
          { "A kind of loop", "A variable whose type is a class", "A header file", "A pointer to main" }, 1,
          "The class is the blueprint; each object is one thing built from it." },
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

};

const int CPP_TUTORIAL_COUNT = (int)(sizeof(CPP_TUTORIALS) / sizeof(CPP_TUTORIALS[0]));
