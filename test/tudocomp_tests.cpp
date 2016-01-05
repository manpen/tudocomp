#include <iostream>
#include <sstream>
#include <utility>
#include <algorithm>
#include <stdexcept>

#include <boost/filesystem.hpp>

#include "test_util.h"
#include "gtest/gtest.h"
#include "tudocomp.h"

using namespace tudocomp;

TEST(Test, test_file) {
    ASSERT_EQ(test_file_path("test.txt"), "test_files/test.txt");
    write_test_file("test.txt", "foobar");
    ASSERT_EQ(read_test_file("test.txt"), "foobar");

    std::string err;
    try {
        read_test_file("not_test.txt");
    } catch (std::runtime_error e) {
        err = e.what();
    }
    ASSERT_EQ("Could not open test file \"test_files/not_test.txt\"", err);
}

TEST(Util, bitsFor) {
    ASSERT_EQ(bitsFor(0b0), 1u);
    ASSERT_EQ(bitsFor(0b1), 1u);
    ASSERT_EQ(bitsFor(0b10), 2u);
    ASSERT_EQ(bitsFor(0b11), 2u);
    ASSERT_EQ(bitsFor(0b100), 3u);
    ASSERT_EQ(bitsFor(0b111), 3u);
    ASSERT_EQ(bitsFor(0b1000), 4u);
    ASSERT_EQ(bitsFor(0b1111), 4u);
    ASSERT_EQ(bitsFor(0b10000), 5u);
    ASSERT_EQ(bitsFor(0b11111), 5u);
}

TEST(Util, bytesFor) {
    ASSERT_EQ(bytesFor(0), 0u);
    ASSERT_EQ(bytesFor(1), 1u);
    ASSERT_EQ(bytesFor(8), 1u);
    ASSERT_EQ(bytesFor(9), 2u);
    ASSERT_EQ(bytesFor(16), 2u);
    ASSERT_EQ(bytesFor(17), 3u);
    ASSERT_EQ(bytesFor(24), 3u);
    ASSERT_EQ(bytesFor(25), 4u);
    ASSERT_EQ(bytesFor(32), 4u);
}

TEST(Input, vector) {
    using namespace input;
    using Inp = input::Input;

    std::vector<uint8_t> v { 97, 98, 99 };
    Inp inp = Inp::from_memory(v);

    {
        auto guard = inp.as_view();
        auto ref = *guard;

        ASSERT_EQ(ref, "abc");
    }

    {
        auto guard = inp.as_stream();
        auto& stream = *guard;

        std::string s;

        stream >> s;

        ASSERT_EQ(s, "abc");
    }
}

std::string fn(std::string suffix) {
    return "tudocomp_tests_" + suffix;
}

TEST(Input, file) {
    using namespace input;
    using Inp = input::Input;

    write_test_file(fn("short.txt"), "abc");
    Inp inp = Inp::from_path(test_file_path(fn("short.txt")));

    {
        auto guard = inp.as_view();
        auto ref = *guard;

        ASSERT_EQ(ref, "abc");
    }

    {
        auto guard = inp.as_stream();
        auto& stream = *guard;

        std::string s;

        stream >> s;

        ASSERT_EQ(s, "abc");
    }
}

TEST(Input, stream_view) {
    using namespace input;
    using Inp = input::Input;

    ViewStream stream { (char*) "abc", 3 };

    Inp inp = Inp::from_stream(stream.stream());

    {
        auto guard = inp.as_stream();
        auto& stream = *guard;

        std::string s;

        stream >> s;

        ASSERT_EQ(s, "abc");
    }
}

TEST(Input, stream_stream) {
    using namespace input;
    using Inp = input::Input;

    ViewStream stream { (char*) "abc", 3 };

    Inp inp = Inp::from_stream(stream.stream());

    {
        auto guard = inp.as_stream();
        auto& stream = *guard;

        std::string s;

        stream >> s;

        ASSERT_EQ(s, "abc");
    }
}

TEST(Output, memory) {
    using namespace output;
    using Out = output::Output;

    std::vector<uint8_t> vec;

    Out out = Out::from_memory(vec);

    {
        auto guard = out.as_stream();
        auto& stream = *guard;

        stream << "abc";
    }

    ASSERT_EQ(vec, (std::vector<uint8_t> { 97, 98, 99 }));
}

TEST(Output, file) {
    using namespace output;
    using Out = output::Output;

    Out out = Out::from_path(test_file_path(fn("short_out.txt")));

    {
        auto guard = out.as_stream();
        auto& stream = *guard;

        stream << "abc";
    }

    ASSERT_EQ(read_test_file(fn("short_out.txt")), "abc");
}

TEST(Output, stream) {
    using namespace output;
    using Out = output::Output;

    std::stringstream ss;
    Out out = Out::from_stream(ss);

    {
        auto guard = out.as_stream();
        auto& stream = *guard;

        stream << "abc";
    }

    ASSERT_EQ(ss.str(), "abc");
}
