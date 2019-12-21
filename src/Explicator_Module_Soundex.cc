// Explicator_Module_Soundex.cc - DICOMautomaton, 2012.
//
// This module computes the 'Soundex' similarity of two strings. The utility of this
// algorithm is that near-sounding strings can be found.
//
// NOTE: This algorithm lacks fine-grained predicting power that some others have. In
// particular, a match is either perfect or not. There is no well-defined 'distance'.
// One could likely be implemented, but it would be of limited value because Soundex
// is (technically) supposed to address this point itself.
//
// NOTE: No attempt is made to speed up or pre-cache computations because the
// algorithm was found to be somewhat unsuitable for my intended purpose. Feel free
// to optimize if needed.
//

#include <stddef.h>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "String.h"

// Returns a string like 'R123' or 'A120' or '0000' holding the Soundex score.
//
// NOTE: This is NOT a true Soundex because it does not exclude surname prefixes, such
// as 'Van', 'De', 'La', etc.. These require TWO soundexes to be computed, and are
// generally not worth the hassle!
//   Soundex("Robert") should be R163
//   Soundex("Rupert") should be R163
//   Soundex("Rubin") should be R150
//   Soundex("Ashcroft") should be A261
//   Soundex("Ashcraft") should be A261
//   Soundex("Tymczak") should be T522
//   Soundex("Pfister") should be P236
//   Soundex("Gutierrez") should be G362
//   Soundex("Jackson") should be J250
//   Soundex("vanDousen") should be TWO SEPARATE SOUNDEXS! (Not handled here!)
//
std::string Soundex(const std::string &in) {
    std::map<char, char> Soundex_Equivs
        = {// These are grouped into collections of ~similar sounding consonants.
           std::make_pair('B', '1'), std::make_pair('C', '2'), std::make_pair('D', '3'), std::make_pair('F', '1'),
           std::make_pair('G', '2'), std::make_pair('J', '2'), std::make_pair('K', '2'), std::make_pair('L', '4'),
           std::make_pair('M', '5'), std::make_pair('N', '5'), std::make_pair('P', '1'), std::make_pair('Q', '2'),
           std::make_pair('R', '6'), std::make_pair('S', '2'), std::make_pair('T', '3'), std::make_pair('V', '1'),
           std::make_pair('X', '2'), std::make_pair('Z', '2'),

           // These are dummies which are removed after contraction (but not prior!)
           // Note std::make_pair('H','x'), std::make_pair('W','x')
           // are excluded entirely.
           std::make_pair('A', 'x'), std::make_pair('E', 'x'), std::make_pair('I', 'x'), std::make_pair('O', 'x'),
           std::make_pair('U', 'x'), std::make_pair('Y', 'x')};

    // Trim the whitespace and any non-alphanumeric chars.
    std::string out = Canonicalize_String2(in, CANONICALIZE::TO_AZ | CANONICALIZE::TO_UPPER);

    // Handle the special case where the first two chars have the same number.
    while((out.size() > 1) && (Soundex_Equivs[out[0]] != 'x') && (Soundex_Equivs[out[0]] == Soundex_Equivs[out[1]])) {
        out.erase(1, 1); // Remove the latter character.
    }

    // Except for the first character, transform the characters.
    for(size_t i = 1; i < out.size();) {
        if(Soundex_Equivs.find(out[i]) != Soundex_Equivs.end()) {
            out[i] = Soundex_Equivs[out[i]];
            ++i;
        } else {
            out.erase(i, 1);
        }
    }

    // Collapse any sequential duplicates after the 1st char.
    for(size_t j = 2; j < out.size();) {
        const size_t i = j - 1;
        if(out[j] == out[i]) {
            out.erase(j, 1);
        } else {
            ++j;
        }
    }

    // Remove any dummy 'x's.
    for(size_t i = 0; i < out.size();) {
        if(out[i] == 'x') {
            out.erase(i, 1);
        } else {
            ++i;
        }
    }

    // Ensure the string is exactly 4 chars long. Pad with '0' if required.
    while(out.size() < 4) { out.push_back('0'); }
    while(out.size() > 4) { out.erase(4); }
    return out;
}

// Initializor function.
void Explicator_Module_Soundex_Init(const std::map<std::string, std::string> &lexicon, float threshold) {
    return;
}

// Query function.
std::unique_ptr<std::map<std::string, float>>
Explicator_Module_Soundex_Query(const std::map<std::string, std::string> &lexicon,
                                const std::string &in,
                                float threshold) {
    // Reminder: The lexicon looks like: < dirty : clean >
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());
    const auto soundex_in(Soundex(in));

    // Cycle through the lexicon, computing the Soundex of each item. Compare it to that of the input.
    for(auto it = lexicon.begin(); it != lexicon.end(); ++it) {
        if(Soundex(it->first) == soundex_in) {
            (*output)[it->second] = 1.0;
        }
    }
    return output;
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_Soundex_Deinit(void) {
    return;
}
