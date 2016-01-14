#include <vector>
#include <cmath>
#include <climits>
#include <iostream>
#include <sstream>
#include <algorithm>

#include "glog/logging.h"
#include "sdsl/int_vector.hpp"

#include "lzw.h"
#include "rule.h"
#include "bit_iostream.h"
#include "tudocomp_util.h"
#include "lz78_trie.h"

namespace lz_compressor {

using namespace tudocomp;

LzwEntries LZWCompressor::compress(const Input& input) {
    LzwEntries entries;

    boost::string_ref input_ref((const char*)(&input[0]), input.size());
    Trie trie;

    for (uint32_t i = 0; i <= 0xff; i++) {
        trie.insert(i);
    }

    for (size_t i = 0; i < input.size(); i++) {
        auto s = input_ref.substr(i);

        Result phrase_and_size = trie.find_or_insert(s);

        DLOG(INFO) << "looking at " << input_ref.substr(0, i)
            << "|" << s << " -> " << phrase_and_size.size;

        if (phrase_and_size.size > 1) {
            i += phrase_and_size.size - 2;
        }

        entries.push_back(phrase_and_size.entry.index - 1);
    }

    trie.root.print(0);

    return entries;
}

void LZWDebugCode::code(LzwEntries entries, Input input, std::ostream& out) {
    for (LzwEntry entry : entries) {
        if (entry >= 32 && entry <= 127) {
            out << "'" << char(entry) << "',";
        } else {
            out << size_t(entry) << ",";
        }
    }
}

template<class F>
void decode_(F ReadIndex, std::ostream& out) {
    LzwEntries entries;
    std::vector<std::tuple<LzwEntry, uint8_t>> dict;
    for (size_t i = 0; i <= 0xff; i++) {
        dict.push_back(std::tuple<LzwEntry, uint8_t> { 0, i });
    }

    auto AddDict = [&](LzwEntry C, uint8_t x) -> LzwEntry {
        dict.push_back(std::tuple<LzwEntry, uint8_t> { C, x });
        return dict.size() - 1;
    };

    auto GetString = [&](LzwEntry C) -> std::string {
        std::stringstream s;
        while (true) {
            s << std::get<1>(dict[C]);
            C = std::get<0>(dict[C]);
            if (C == 0) {
                break;
            }
        }
        std::string r = s.str();
        return std::string(r.rbegin(), r.rend());
    };

    auto IsIndexInDict = [&](LzwEntry C) -> bool {
        return C < dict.size();
    };

    LzwEntry C = ReadIndex();
    std::string W = GetString(C);
    out << W;
    //std::cout << W << "\n";
    auto f = [&](uint C,
                 uint C_,
                 std::string W,
                 bool indict,
                 uint adddict) {
        std::cout
            << " |\t" << byte_to_nice_ascii_char(C)
            << " |\t" << byte_to_nice_ascii_char(C_)
            << " |\t" << W
            << " |\t" << indict
            << " |\t" << byte_to_nice_ascii_char(adddict)
                << "(" << byte_to_nice_ascii_char(C) << ","
                << W[0] << ")"
            << " |\t" << W << "\n";
    };

    //f(C, 0, W, false, 0);

    int counter = 10;

    while (C != LzwEntry(-1) && counter > 0) {
        counter--;
        LzwEntry C_ = ReadIndex();

        if (C_ == LzwEntry(-1)) {
            break;
        }

        LzwEntry new_dict;
        bool isindict;

        if ((isindict = IsIndexInDict(C_))) {
            W = GetString(C_);
            new_dict = AddDict(C, W[0]);
        } else {
            new_dict = C_ = AddDict(C, W[0]);
            W = GetString(C_);
        }
        out << W;
        //f(C, C_, W, isindict, new_dict);

        C = C_;
    }
}

void LZWDebugCode::decode(std::istream& inp, std::ostream& out) {
    bool more = true;
    char c;
    decode_([&]() -> LzwEntry {
        if (!more) {
            return LzwEntry(-1);
        }

        LzwEntry v;
        more &= parse_number_until_other(inp, c, v);

        if (c == '\'') {
            more &= bool(inp.get(c));
            v = uint8_t(c);
            more &= bool(inp.get(c));
            CHECK(c == '\'');
            more &= bool(inp.get(c));
        }

        CHECK(c == ',');

        if (!more) {
            return LzwEntry(-1);
        }
        //std::cout << byte_to_nice_ascii_char(v) << "\n";
        return v;

    }, out);
}

void LZWBitCode::code(LzwEntries entries, Input input, std::ostream& out_) {

}

void LZWBitCode::decode(std::istream& inp_, std::ostream& out) {

}

}