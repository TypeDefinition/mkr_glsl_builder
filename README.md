## GLSL Include
This single header-only C++ helper class adds support for the `#include` and `#pragma once` directives (similar to that in C++), to GLSL.

Natively, GLSL does not support the `#include` and `#pragma once` directives.
However, as the number of shaders in a project grow, there is a high chance that much of the shader code are similar, leading to DRY being broken as multiple shaders having the same code copy-pasted.

While the `ARB_shading_language_include` extension exists, it is also troublesome to set up.

This is my attempt to solve that issue in a simple and elegant way.

## Bug Reporting
If there are any bugs or suggestions, feel free to create a `GitHub Issue`, or even a `Pull Request` if you would like to add anything. 😄

## How to Use
Sample Code:
```C++
#include "glsl_include.h"

using namespace mkr;

// Note that I do not know how to deal with multi-line comments, so avoid putting the directives in one.
int main() {
    string main = "#include <foo.frag>\n" // Each `#include` MUST be on a new line on its own.
                  "#include <boo.frag>\n"
                  "void main() {}";

    string foo = "#pragma once\n" // Each `#pragma once` MUST be on a new line on its own.
                 "void foo() {}\n"
                 "#include <kee.frag>";

    string boo = "#pragma once\n"
                 "void boo() {}\n"
                 "#include <kee.frag>";

    string kee = "void kee() {}"; // Has no #pragma once, can be included twice.

    glsl_include include;
    include.add("main.frag", main); 
    include.add("foo.frag", foo); // The first argument must match the name between the arrow brackets <>.
    include.add("boo.frag", boo);
    include.add("kee.frag", kee);

    string merged = shaders.merge();
    cout << merged << endl;

    return 0;
}
```

Console Output:
```C++
void foo() {}
void kee() {}
void boo() {}
void kee() {}
void main() {}
```
