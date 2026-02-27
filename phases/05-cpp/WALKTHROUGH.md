# Phase 05 Walkthrough: C++ — Classes, RAII & Modern Idioms

> Audience: web developer with JavaScript/TypeScript background, learning C++.

---

## 1. Introduction: C++ is C with superpowers — but every superpower has a cost

You already know C: manual memory, raw pointers, no guardrails. C++ keeps everything from C and layers a set of powerful abstractions on top:

- **Classes** — bundle data and behavior (like JS objects, but with guaranteed construction and destruction).
- **Constructors / Destructors** — functions that run *automatically* when an object is created or goes out of scope.
- **RAII** — the single most important idea in C++: tie a resource (memory, file, socket, mutex) to an object's lifetime so the compiler cleans it up for you.
- **Smart pointers** (`std::unique_ptr`) — heap ownership made safe.
- **Templates** — generic code that the compiler stamps out for each type (think TypeScript generics, but resolved at compile time with zero overhead).

The cost is real: longer compile times, subtler rules around copies and moves, and a steeper mental model than JS. But once RAII clicks, safe C++ feels natural.

---

## 2. Bridging the mental model: RAII vs. patterns you already know

In JavaScript you write:

```js
const fd = fs.openSync('file.txt', 'r');
try {
  // use fd
} finally {
  fs.closeSync(fd);  // you must remember this
}
```

In C# you write:

```csharp
using (var sr = new StreamReader("file.txt"))
{
    // sr.Close() is called automatically at end of using block
}
```

In C++ with RAII, there is no special syntax — the destructor *is* the cleanup:

```cpp
{
    FileHandle fh("file.txt");   // constructor opens the file
    // use fh
}   // destructor runs here — file closes automatically, even on exception
```

No `try/finally`. No `using`. The object going out of scope *is* the guarantee.
The compiler enforces it. It cannot be forgotten, cannot be skipped by an early `return`, and is exception-safe by default.

---

## 3. Exercises

### ex01 — `Stack<T>`: class with a fixed array, push/pop/size

**What you are building:** a generic stack that works for any type, backed by a fixed-size C array. This introduces classes, templates, and the constructor/destructor lifecycle.

**Step 1 — create the file**

```
phases/05-cpp/ex01/stack.cpp
```

**Step 2 — write the code**

```cpp
#include <iostream>
#include <stdexcept>

// Template class: T is a placeholder for any type.
// The compiler generates a separate Stack<int>, Stack<float>, etc. on demand.
template <typename T>
class Stack {
public:
    static const int CAPACITY = 64;

    // Constructor: runs automatically when a Stack object is created.
    // Initializes top_ to -1 (empty stack).
    Stack() : top_(-1) {}

    // Destructor: runs automatically when the object goes out of scope.
    // No dynamic memory here, so nothing to free — but the pattern is established.
    ~Stack() {}

    void push(const T& value) {
        if (top_ >= CAPACITY - 1)
            throw std::overflow_error("Stack is full");
        data_[++top_] = value;
    }

    T pop() {
        if (top_ < 0)
            throw std::underflow_error("Stack is empty");
        return data_[top_--];
    }

    int size() const { return top_ + 1; }

    bool empty() const { return top_ < 0; }

private:
    T   data_[CAPACITY];
    int top_;
};

int main() {
    Stack<int> s;

    s.push(10);
    s.push(20);
    s.push(30);

    std::cout << "size: " << s.size() << "\n";   // 3
    std::cout << "pop: "  << s.pop()  << "\n";   // 30
    std::cout << "pop: "  << s.pop()  << "\n";   // 20
    std::cout << "size: " << s.size() << "\n";   // 1

    // Template works with any type
    Stack<std::string> words;
    words.push("hello");
    words.push("world");
    std::cout << words.pop() << "\n";  // world

    return 0;
}
```

**Step 3 — compile and run**

```bash
g++ -std=c++17 -Wall -o ex01 ex01/stack.cpp
./ex01
```

**Expected output**

```
size: 3
pop: 30
pop: 20
size: 1
world
```

**Key concepts**

| C++ | JS/TS equivalent |
|-----|-----------------|
| `template <typename T>` | `class Stack<T>` in TypeScript |
| `Stack() : top_(-1) {}` | `constructor() { this.top = -1; }` |
| `const` member function | read-only method (no mutation) |
| `private:` members | private fields (`#top` in modern JS) |

---

### ex02 — RAII: `FileHandle` — open in constructor, close in destructor

**What you are building:** a class that wraps a `FILE*`. The file opens in the constructor and closes in the destructor. This is the canonical RAII pattern.

**Step 1 — create the file**

```
phases/05-cpp/ex02/filehandle.cpp
```

**Step 2 — write the code**

```cpp
#include <cstdio>
#include <stdexcept>
#include <string>
#include <iostream>

class FileHandle {
public:
    // Constructor: acquire the resource.
    // If fopen fails, throw immediately — no half-open object exists.
    explicit FileHandle(const std::string& path, const char* mode = "r") {
        fp_ = std::fopen(path.c_str(), mode);
        if (!fp_)
            throw std::runtime_error("Cannot open file: " + path);
        std::cout << "[FileHandle] opened " << path << "\n";
    }

    // Destructor: release the resource.
    // Called automatically when the object goes out of scope — no exceptions allowed here.
    ~FileHandle() {
        if (fp_) {
            std::fclose(fp_);
            std::cout << "[FileHandle] closed\n";
        }
    }

    // Provide access to the underlying FILE* for reading/writing.
    FILE* get() { return fp_; }

    // Disable copying — owning a resource should be unique (see ex03 for unique_ptr).
    FileHandle(const FileHandle&)            = delete;
    FileHandle& operator=(const FileHandle&) = delete;

private:
    FILE* fp_;
};

int main() {
    // Create a small test file first
    {
        FileHandle out("test.txt", "w");
        std::fputs("hello from RAII\n", out.get());
    }  // <-- destructor runs here, file is flushed and closed

    // Now read it back
    {
        FileHandle in("test.txt", "r");
        char buf[128];
        while (std::fgets(buf, sizeof(buf), in.get()))
            std::cout << buf;
    }  // <-- destructor runs here again

    std::cout << "main returns — no leaks, no manual fclose\n";
    return 0;
}
```

**Step 3 — compile and run**

```bash
g++ -std=c++17 -Wall -o ex02 ex02/filehandle.cpp
./ex02
```

**Expected output**

```
[FileHandle] opened test.txt
[FileHandle] closed
[FileHandle] opened test.txt
hello from RAII
[FileHandle] closed
main returns — no leaks, no manual fclose
```

**Notice:** the `{` braces create a nested scope. The destructors fire at the closing `}` — before `main` returns, before any later code. This is the entire point of RAII: scope = lifetime = resource lifetime.

---

### ex03 — `std::unique_ptr`: replace manual `new`/`delete`

**What you are building:** an example that starts with manual heap allocation and refactors it to `std::unique_ptr`. This is the rule you will follow for the rest of your C++ career: **always use `unique_ptr` for heap ownership**.

**Step 1 — create the file**

```
phases/05-cpp/ex03/smartptr.cpp
```

**Step 2 — write the code**

```cpp
#include <memory>
#include <iostream>
#include <string>

struct Node {
    std::string name;
    int value;

    Node(std::string n, int v) : name(std::move(n)), value(v) {
        std::cout << "[Node] constructed: " << name << "\n";
    }
    ~Node() {
        std::cout << "[Node] destroyed: " << name << "\n";
    }
};

// --- BEFORE: manual new/delete (error-prone) ---
void manual_version() {
    std::cout << "--- manual ---\n";
    Node* p = new Node("alpha", 1);
    std::cout << p->name << " = " << p->value << "\n";
    delete p;  // you must remember this — if you return early, it leaks
}

// --- AFTER: unique_ptr (automatic, safe) ---
void smart_version() {
    std::cout << "--- smart ---\n";

    // make_unique<T>(args...) allocates and constructs in one call.
    // unique_ptr owns the pointer — one owner only.
    auto p = std::make_unique<Node>("beta", 2);

    std::cout << p->name << " = " << p->value << "\n";

    // Ownership transfer: move the pointer to another unique_ptr.
    // After the move, p is null — you cannot use it.
    auto q = std::move(p);
    std::cout << "moved to q: " << q->name << "\n";

    // p is now empty:
    std::cout << "p is null: " << (p == nullptr ? "yes" : "no") << "\n";

    // q goes out of scope here — destructor fires automatically.
}

int main() {
    manual_version();
    smart_version();
    std::cout << "main done\n";
    return 0;
}
```

**Step 3 — compile and run**

```bash
g++ -std=c++17 -Wall -o ex03 ex03/smartptr.cpp
./ex03
```

**Expected output**

```
--- manual ---
[Node] constructed: alpha
alpha = 1
[Node] destroyed: alpha
--- smart ---
[Node] constructed: beta
beta = 2
moved to q: beta
p is null: yes
[Node] destroyed: beta
main done
```

**Key rule:** `unique_ptr` is RAII for heap memory. Treat it the same way you treat a scoped block for `FileHandle` — the object owns the resource, and the resource dies with the object.

---

### ex04 — Template function `max<T>`

**What you are building:** a generic `max` function that the compiler specialises for `int`, `float`, and `char` automatically.

**Step 1 — create the file**

```
phases/05-cpp/ex04/maxtemplate.cpp
```

**Step 2 — write the code**

```cpp
#include <iostream>

// Template function: T is deduced from the arguments at the call site.
// The compiler generates a separate function for each distinct T used.
template <typename T>
T mymax(const T& a, const T& b) {
    return (a > b) ? a : b;
}

// You can also explicitly specialise for a specific type if needed.
// Here the general template already handles all cases.

int main() {
    // int — T deduced as int
    std::cout << "max(3, 7)      = " << mymax(3, 7)       << "\n";

    // float — T deduced as float
    std::cout << "max(1.5f, 0.9f) = " << mymax(1.5f, 0.9f) << "\n";

    // char — T deduced as char (compares by ASCII value)
    std::cout << "max('a', 'z')  = " << mymax('a', 'z')   << "\n";

    // Explicit type argument (rarely needed when deduction works)
    std::cout << "max<int>(10, 5) = " << mymax<int>(10, 5) << "\n";

    return 0;
}
```

**Step 3 — compile and run**

```bash
g++ -std=c++17 -Wall -o ex04 ex04/maxtemplate.cpp
./ex04
```

**Expected output**

```
max(3, 7)      = 7
max(1.5f, 0.9f) = 1.5
max('a', 'z')  = z
max<int>(10, 5) = 10
```

**Comparison with TypeScript generics**

```ts
// TypeScript — resolved at runtime (type erasure)
function mymax<T>(a: T, b: T): T { ... }

// C++ — resolved at compile time (no erasure, full performance)
template <typename T>
T mymax(const T& a, const T& b) { ... }
```

The C++ version produces zero-overhead code: `mymax(3, 7)` compiles to the same instructions as writing `(3 > 7) ? 3 : 7` directly.

---

### ex05 — `std::array` and range-based for loop

**What you are building:** a demonstration of `std::array<T, N>` (a safer, size-aware replacement for C arrays) and the range-based `for` loop (equivalent to JS `for...of`).

**Step 1 — create the file**

```
phases/05-cpp/ex05/arraydemo.cpp
```

**Step 2 — write the code**

```cpp
#include <array>
#include <iostream>
#include <numeric>   // std::accumulate
#include <algorithm> // std::sort, std::find

int main() {
    // std::array<T, N>: size is part of the type — checked at compile time.
    // Unlike C arrays: knows its size, can be copied, works with algorithms.
    std::array<int, 5> nums = {5, 3, 1, 4, 2};

    // Range-based for loop — equivalent to JS for...of
    std::cout << "original: ";
    for (const int& n : nums)
        std::cout << n << " ";
    std::cout << "\n";

    // Sort in place
    std::sort(nums.begin(), nums.end());

    std::cout << "sorted:   ";
    for (int n : nums)
        std::cout << n << " ";
    std::cout << "\n";

    // Bounds-checked access with .at() — throws std::out_of_range
    std::cout << "nums.at(2) = " << nums.at(2) << "\n";

    // Sum with standard algorithm
    int sum = std::accumulate(nums.begin(), nums.end(), 0);
    std::cout << "sum = " << sum << "\n";

    // std::array knows its size — no pointer decay
    std::cout << "size = " << nums.size() << "\n";

    // 2D array — size encoded in the type
    std::array<std::array<int, 3>, 2> grid = {{{1, 2, 3}, {4, 5, 6}}};
    for (const auto& row : grid) {
        for (int v : row)
            std::cout << v << " ";
        std::cout << "\n";
    }

    return 0;
}
```

**Step 3 — compile and run**

```bash
g++ -std=c++17 -Wall -o ex05 ex05/arraydemo.cpp
./ex05
```

**Expected output**

```
original: 5 3 1 4 2
sorted:   1 2 3 4 5
nums.at(2) = 3
sum = 15
size = 5
1 2 3
4 5 6
```

**Why `std::array` instead of C array?**

| C array `int a[5]` | `std::array<int, 5>` |
|--------------------|----------------------|
| Decays to pointer when passed to function | Keeps size, passed by reference safely |
| No `.size()` | `.size()` is always correct |
| No bounds checking | `.at(i)` throws on out-of-bounds |
| Cannot be returned from a function by value reliably | Fully copyable and returnable |

---

## 4. RAII deep dive: why destructor-based cleanup is safer than manual free

Consider a function that allocates memory, opens a file, and does work:

```cpp
// C style — fragile
void do_work(const char* path) {
    int* buf = (int*)malloc(256 * sizeof(int));
    FILE* fp = fopen(path, "r");
    if (!fp) {
        free(buf);   // must remember to free before every return
        return;
    }
    if (some_condition()) {
        fclose(fp);
        free(buf);   // must remember again
        return;
    }
    fclose(fp);
    free(buf);       // and again at normal exit
}
```

Every early `return` (and every thrown exception) requires manual cleanup in the right order. Miss one and you have a leak.

```cpp
// C++ RAII style — safe at every exit path
void do_work(const std::string& path) {
    auto buf = std::make_unique<int[]>(256);   // freed when buf goes out of scope
    FileHandle fp(path);                        // closed when fp goes out of scope

    if (some_condition())
        return;  // destructors fire in reverse order of construction: fp, then buf
    // same cleanup on exception, on any return, always
}
```

The destructor call stack is managed by the compiler. The order is deterministic: reverse construction order. There is no code path that can skip it.

**This is why RAII is not a style preference — it is the mechanism that makes C++ memory safety achievable without a garbage collector.**

---

## 5. When to use `unique_ptr` vs raw pointer

### Rule: always use `unique_ptr` for heap ownership

```
Ownership of heap memory
         |
         v
   std::unique_ptr<T>     <- default choice for heap allocation
         |
         | need shared ownership?
         v
   std::shared_ptr<T>     <- multiple owners, ref-counted
         |
         | observing only (no ownership)?
         v
   raw pointer T*          <- fine for non-owning references
   or reference T&
```

In practice:

```cpp
// Owning a dynamically allocated object — unique_ptr
auto node = std::make_unique<Node>("alpha", 1);

// Passing to a function that does NOT take ownership — raw pointer or reference
void print_node(const Node* n);     // fine, n does not own
void print_node(const Node& n);     // preferred when non-null is guaranteed
print_node(node.get());             // .get() returns the raw pointer without transferring ownership

// Transferring ownership — std::move
auto other = std::move(node);       // node is now null, other owns the Node
```

**Never use `new` and `delete` directly** in new C++ code. Always go through `make_unique` or `make_shared`. The only exception is when implementing your own data structure that explicitly manages memory (like `Stack<T>` with a fixed internal array in ex01, which avoids heap entirely).

---

## 6. Common mistakes

### 6.1 Object slicing

```cpp
struct Animal {
    virtual std::string sound() { return "..."; }
};
struct Dog : Animal {
    std::string sound() override { return "woof"; }
};

// WRONG: storing by value slices the derived class off
Animal a = Dog();
std::cout << a.sound();  // prints "..." — Dog part is gone

// CORRECT: use a pointer or reference
Animal* p = new Dog();
std::cout << p->sound();  // prints "woof"

// BEST: use unique_ptr
auto p = std::make_unique<Dog>();
std::cout << p->sound();  // prints "woof"
```

Slicing happens because `Animal a = Dog()` copies only the `Animal` subobject. If you use polymorphism, always use pointers or references.

### 6.2 Forgetting `virtual` destructor

```cpp
struct Base {
    ~Base() { std::cout << "Base destroyed\n"; }  // NOT virtual — bug
};
struct Derived : Base {
    int* data = new int[100];
    ~Derived() { delete[] data; std::cout << "Derived destroyed\n"; }
};

Base* p = new Derived();
delete p;  // only ~Base() runs — Derived::data leaks
```

Fix: any class intended as a base class must have `virtual ~Base() {}`.

```cpp
struct Base {
    virtual ~Base() = default;  // always, for any base class
};
```

### 6.3 Moving from a `unique_ptr` and then using it

```cpp
auto p = std::make_unique<Node>("alpha", 1);
auto q = std::move(p);

p->name;  // undefined behaviour — p is null after the move
```

After `std::move(p)`, treat `p` as destroyed. Never read from it again. The compiler will not always warn you.

### 6.4 Returning a `unique_ptr` by reference

```cpp
// WRONG: returning a reference to a local unique_ptr — dangling reference
std::unique_ptr<Node>& get_node() {
    auto p = std::make_unique<Node>("x", 0);
    return p;  // p is destroyed when function returns
}

// CORRECT: return by value — move semantics kick in automatically (no copy)
std::unique_ptr<Node> get_node() {
    return std::make_unique<Node>("x", 0);
}
```

---

## 7. mysh milestone: refactor to C++

After completing the exercises, apply what you have learned to `mysh`. The goal is version 3: wrap the shell loop in a class and use RAII for file descriptors.

### 7.1 Rename `main.c` to `main.cpp`

```bash
git mv mysh/src/main.c mysh/src/main.cpp
```

C++ is largely backward-compatible with C code, so the existing logic compiles as-is. The rename signals to the compiler (and to humans) that this is now C++.

### 7.2 Create `class Shell`

Replace the bare `main` loop with a class that encapsulates shell state:

```cpp
// mysh/src/main.cpp
#include <iostream>
#include <cstdio>
#include "mysh.h"

class Shell {
public:
    Shell() {
        std::printf("mysh v%s — type 'exit' to quit\n", MYSH_VERSION);
    }

    ~Shell() {
        // Future: flush history, close log file, etc.
        // Nothing to do today — but the hook exists.
    }

    void run() {
        char  buf[MAX_INPUT];
        char* argv[MAX_ARGS];

        while (true) {
            std::printf("mysh> ");
            std::fflush(stdout);

            if (mysh_readline(buf, MAX_INPUT) < 0)
                break;  // EOF (Ctrl+D)

            int argc = mysh_tokenize(buf, argv);
            if (argc == 0)
                continue;

            if (mysh_builtin(argc, argv))
                continue;

            // Phase 04: fork + exec goes here
            std::fprintf(stderr, "mysh: %s: command not found\n", argv[0]);
        }
    }
};

int main() {
    Shell shell;
    shell.run();
    return 0;
}
```

The behaviour is identical to the original. The benefit is that `Shell` now has a single responsibility and a well-defined lifecycle — future state (history buffer, last exit code, open pipes) has a natural home as private members.

### 7.3 Add RAII `class Pipe` for pipe file descriptors

When mysh gains pipe support, raw `int fds[2]` need cleanup on every exit path. Wrap them:

```cpp
// mysh/include/mysh.h (or a new mysh/include/pipe.hpp)
#include <unistd.h>
#include <stdexcept>

class Pipe {
public:
    Pipe() {
        if (::pipe(fds_) != 0)
            throw std::runtime_error("pipe() failed");
    }

    ~Pipe() {
        close_read();
        close_write();
    }

    int read_fd()  const { return fds_[0]; }
    int write_fd() const { return fds_[1]; }

    void close_read()  { if (fds_[0] >= 0) { ::close(fds_[0]); fds_[0] = -1; } }
    void close_write() { if (fds_[1] >= 0) { ::close(fds_[1]); fds_[1] = -1; } }

    // Non-copyable — owning file descriptors must not be duplicated carelessly
    Pipe(const Pipe&)            = delete;
    Pipe& operator=(const Pipe&) = delete;

private:
    int fds_[2] = {-1, -1};
};
```

Usage in the fork/exec path:

```cpp
Pipe p;
pid_t pid = fork();
if (pid == 0) {
    p.close_read();
    dup2(p.write_fd(), STDOUT_FILENO);
    // exec ...
}
p.close_write();
// read from p.read_fd()
// p goes out of scope here — both ends closed automatically
```

Even if `exec` fails and you `return` early, `~Pipe()` fires and both file descriptors are closed. No leaks, no missing `close` calls.

### 7.4 Update the Makefile

```makefile
# mysh/Makefile
CC      = g++
CFLAGS  = -Wall -Wextra -Werror -g -std=c++17 -Iinclude
SAN     = -fsanitize=address,undefined
SRCDIR  = src
BINDIR  = bin
BIN     = $(BINDIR)/mysh

SRCS    = $(wildcard $(SRCDIR)/*.cpp)
OBJS    = $(SRCS:.cpp=.o)

all: $(BINDIR) $(BIN)

$(BINDIR):
	mkdir -p $(BINDIR)

$(BIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

run: all
	./$(BIN)

valgrind: all
	valgrind --leak-check=full --track-origins=yes ./$(BIN)

san: $(BINDIR)
	$(CC) $(CFLAGS) $(SAN) -o $(BINDIR)/mysh-san $(SRCS)
	./$(BINDIR)/mysh-san

clean:
	rm -f $(SRCDIR)/*.o $(BIN) $(BINDIR)/mysh-san

.PHONY: all run valgrind san clean
```

Changes from the C version:
- `CC = g++` (was `gcc`)
- `-std=c++17` (was `-std=c17`)
- `SRCS` pattern changed from `*.c` to `*.cpp`
- `OBJS` suffix changed from `.c=.o` to `.cpp=.o`

---

## 8. Phase 05 checklist

- [ ] **ex01** — `Stack<T>` compiles and produces correct push/pop/size output
- [ ] **ex02** — `FileHandle` opens and closes without manual `fclose`; destructor log appears at scope exit
- [ ] **ex03** — `unique_ptr` example runs; `[Node] destroyed` lines appear before "main done"
- [ ] **ex04** — `mymax` works for `int`, `float`, and `char`
- [ ] **ex05** — `std::array` sorted, summed, and iterated with range-based for
- [ ] **Conceptual** — can explain RAII in one sentence without using the word "RAII"
- [ ] **mysh milestone** — `main.c` renamed to `main.cpp`, `Shell` class compiles with `g++ -std=c++17`, `Pipe` class defined and ready for pipe support
- [ ] **Makefile** — `make run` builds and launches mysh with the new g++ toolchain
