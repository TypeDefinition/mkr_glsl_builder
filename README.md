## GLSL Include
This single header-only C++ helper class adds support for including files in GLSL.

Natively, GLSL does not support the `#include` directive.
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

// Note that I do not know how to deal with multi-line comments, so avoid putting the #include in one.
int main() {
    string main = "#include <foo.frag>\n" // Each `#include` MUST be on a new line on its own.
                  "#include <boo.frag>\n"
                  "void main() {}";

    string foo = "#include <kee.frag>\n" 
                 "void foo() {}";

    string boo = "#include <kee.frag>\n"
                 "void boo() {}";

    string kee = "void kee() {}";

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
void kee() {}
void foo() {}
void boo() {}
void main() {}
```
