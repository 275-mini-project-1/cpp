# C/C++ - 3code lab

For many this might be the most complex of the three sub-projects as it 
introduces you to C/C++ and CMake.

## You will need

These packages will be required:

  * A C/C++ compiler (gcc, clang). While xcode (MAC OSX) is needed 
    to bootstrap building, please try to use gcc or clang.
  * CMake. A build tool similar to imake, and maven.
  * An editor (IDE for C++, Atom, vim, nano, etc)

Approaches: Using an installer (e.g., dnf, apt-get, brew) or build from 
source using autoconf.

## Where to go from here

Well, there are a lot of topics to jump from this introduction that will
be discussed in class. Working on the C/C++ will help you be prepared 
when we start. Some suggestions on what to work on:

   * Adding libraries like Boost headers and/or libraries
   * Building a library and linking it into another project (code)
   * Tools for writting better code
   * Exploring the newest language features (20, 17, 14, 11). Some
     suggestions: 
       * Move semantics
       * Lambda support
       * Mutexes
       * Co-routines
       * Templates, namespaces


## Write Better Code

Suggestions to help you figure out what your code is doing. Oh, 
and write better code!

### Using a debugger

IDE's contain builtin graphical debuggers. For instance, installing 
Eclipse's C/C++ and VS Code IDEs. There are command line tools like 
gdb or ddd. These can be installed using your installer (e.g., brew).

### Linters

Lint tools using best practices to check you code are available 
across multiple languages. 

Clang's linter, clang-tidy, can be ran from the cmake file provided.

### Sanitizers

Compiler checking with sanitize (ASAN) is ran to check for address, memory, or
thread errors. **Note gcc/g++ does not support sanitize on apple/arm.**

    ```shell
    $ clang-install-dir/bin/clang++ -Wall -Wextra -fsanitize=address -fsanitize-address-use-after-scope parser.cpp -g -o parser
    $ ./parser
    ```
 
 ### Code Analysis

 Clang's static analyzer is a tool to find common bugs in C/C++ code. 
 Note `-o` is the directory where the analyzer's report (*.html) files 
 are found.
 
    ```shell
    $ clang-install-dir/bin/clang++ --analyze -Xclang -analyzer-output=html -g parser.cpp -o out 
    $ cd out 
    $ open report*.html
    ```

 Or to view the report in scan-view:

    ```shell
    $ clang-install-dir/bin/scan-build clang-install-dir/bin/clang++ -g parser.cpp -o out 
      output will show the command to run 'scan-view <path-to-report>'
    ```

You will need to either have python in your `$PATH` or run like:

    ```shell
    $ python3 clang-install-dir/bin/scan-view <path-to-report>
    ```

### Other Tools

Valgrind (https://valgrind.org) is a suite of tools for debugging and performance. 


## Reading

  * https://www.cplusplus.com/doc/tutorial/
  * https://www.geeksforgeeks.org/c-plus-plus/?ref=ghm
  * https://github.com/AnthonyCalandra/modern-cpp-features
  * https://gcc.gnu.org/onlinedocs/gcc/Developer-Options.html 
  * https://www.youtube.com/watch?app=desktop&v=lkgszkPnV8g&feature=youtu.be
  * https://horstmann.com/ccj2/ccjapp3.html

### Debugging

  * https://clang-analyzer.llvm.org 
  * https://llvm.org/devmtg/2020-09/slides/Using_the_clang_static_ananalyzer_to_find_bugs.pdf
  * https://clang.llvm.org/docs/AddressSanitizer.html
  * https://godbolt.org

