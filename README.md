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
```
#include "glsl_include.h"

using namespace mkr;

int main() {
    string base = "#include <incl0.frag>\n" // Each `#include` MUST be on a new line on its own.
                  "#include <incl1.frag>\n"
                  "void main() {}";

    string incl0 = "#pragma once\n" // Each `#pragma once` MUST be on a new line on its own.
                   "void foo() {}\n"
                   "#include <incl2.frag>";

    string incl1 = "#pragma once\n"
                   "void bar() {}\n"
                   "#include <incl2.frag>";

    string incl2 = "void baz() {} // Has no #pragma once, can be included twice.";

    glsl_include include;
    include.add("base.frag", base); 
    include.add("incl0.frag", incl0); // The first argument _name, must match the name between the arrow brackets <>.
    include.add("incl1.frag", incl1);
    include.add("incl2.frag", incl2);

    string merged = shaders.merge();
    cout << merged << endl;

    return 0;
}
```

Console Output:
```
void foo() {}
void baz() {} // Has no #pragma once, can be included twice.
void bar() {}
void baz() {} // Has no #pragma once, can be included twice.
void main() {}
```
