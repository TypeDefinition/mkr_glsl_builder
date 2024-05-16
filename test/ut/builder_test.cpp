#include <gtest/gtest.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include "glsl_builder.h"

using namespace mkr;
using namespace std;

std::string file_to_str(const std::string &_filename) {
    std::ifstream is(_filename);
    std::stringstream ss;
    ss << is.rdbuf();
    return ss.str();
}

// Ensure that each file is only included once.
TEST(ext, case0) {
    glsl_builder builder;
    builder.add("base.frag", file_to_str("case0/base.frag"));
    builder.add("incl0.frag", file_to_str("case0/incl0.frag"));
    builder.add("incl1.frag", file_to_str("case0/incl1.frag"));
    builder.add("incl2.frag", file_to_str("case0/incl2.frag"));
    builder.add("incl3.frag", file_to_str("case0/incl3.frag"));
    EXPECT_TRUE(builder.build() == file_to_str("case0/result.frag"));
}

// Ensure that #includes in comments are ignored.
TEST(ext, case1) {
    glsl_builder builder;
    builder.add("base.frag", file_to_str("case1/base.frag"));
    builder.add("incl0.frag", file_to_str("case1/incl0.frag"));
    EXPECT_TRUE(builder.build() == file_to_str("case1/result.frag"));
}

// Ensure that the number of whitespaces before and between the #include does not matter.
TEST(ext, case2) {
    glsl_builder builder;
    builder.add("base.frag", file_to_str("case2/base.frag"));
    builder.add("incl0.frag", file_to_str("case2/incl0.frag"));
    EXPECT_TRUE(builder.build() == file_to_str("case2/result.frag"));
}

// Ensure that an error is thrown when a missing file is included.
TEST(ext, case3) {
    bool error_thrown = false;
    try {
        glsl_builder builder;
        builder.add("base.frag", file_to_str("case3/base.frag"));
        builder.build();
    } catch (const std::exception& e) {
        EXPECT_TRUE(e.what() == std::string{"glsl_builder - Cannot include missing source incl0.frag."});
        error_thrown = true;
    }
    EXPECT_TRUE(error_thrown);
}

// Ensure that an error is thrown when a missing file is included.
TEST(ext, case4) {
    bool error_thrown = false;
    try {
        glsl_builder builder;
        builder.add("base.frag", file_to_str("case4/base.frag"));
        builder.add("incl0.frag", file_to_str("case4/incl0.frag"));
        builder.add("incl1.frag", file_to_str("case4/incl1.frag"));
        builder.add("incl2.frag", file_to_str("case4/incl2.frag"));
        builder.build();
    } catch (const std::exception& e) {
        EXPECT_TRUE(e.what() == std::string{"glsl_builder - Cyclic dependency detected."});
        error_thrown = true;
    }
    EXPECT_TRUE(error_thrown);
}

// Ensure that there is only 1 "base" file.
TEST(ext, case5) {
    bool error_thrown = false;
    try {
        glsl_builder builder;
        builder.add("base0.frag", file_to_str("case5/base.frag"));
        builder.add("base1.frag", file_to_str("case5/base.frag"));
        builder.add("incl0.frag", file_to_str("case5/incl0.frag"));
        builder.add("incl1.frag", file_to_str("case5/incl1.frag"));
        builder.build();
    } catch (const std::exception& e) {
        EXPECT_TRUE(e.what() == std::string{"glsl_builder - There must be exactly 1 file which is not included by any other file."});
        error_thrown = true;
    }
    EXPECT_TRUE(error_thrown);
}