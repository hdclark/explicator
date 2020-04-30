// Explicator_Module_MRA.cc - DICOMautomaton, 2012.
//
// This module computes the 'Match Rating Approach' (MRA) similarity of two strings.
// It is a phonetic algorithm, in the style of MRA and Metaphone. It is an
// improvement on another algorithm (NYSIIS) developed by Western Airlines in 1977.
//
// NOTE: No attempt is made to speed up or pre-cache computations because the
// algorithm was found to be somewhat unsuitable for my intended purpose. Feel free
// to optimize it!
//

#include <stddef.h>
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "Misc.h"
#include "String.h"

using namespace explicator_internals;

std::string MatchRatingApproach(const std::string &in) {
    std::map<char, char> MRA_Equivs = {
        /*
                //These are grouped into collections of ~similar sounding consonants.
                std::make_pair('B','1'), std::make_pair('C','2'), std::make_pair('D','3'),
                std::make_pair('F','1'), std::make_pair('G','2'), std::make_pair('J','2'),
                std::make_pair('K','2'), std::make_pair('L','4'), std::make_pair('M','5'),
                std::make_pair('N','5'), std::make_pair('P','1'), std::make_pair('Q','2'),
                std::make_pair('R','6'), std::make_pair('S','2'), std::make_pair('T','3'),
                std::make_pair('V','1'), std::make_pair('X','2'), std::make_pair('Z','2'),
        */
        // Vowels which get removed.
        std::make_pair('A', 'x'), std::make_pair('E', 'x'), std::make_pair('I', 'x'), std::make_pair('O', 'x'),
        std::make_pair('U', 'x') /*, std::make_pair('Y','x')*/
    };

    // Trim the whitespace and any non-alphanumeric chars.
    std::string out = Canonicalize_String2(in, CANONICALIZE::TO_AZ | CANONICALIZE::TO_UPPER);

    // Delete any vowels after the first.
    for(size_t i = 1; i < out.size();) {
        if(MRA_Equivs[out[i]] == 'x') {
            out.erase(i, 1);
        } else {
            ++i;
        }
    }

    // Remove any doubled consonants. Do so iteratively (ie. BBBC -> BBC -> BC).
    for(size_t j = 2; j < out.size();) {
        const size_t i = j - 1;
        if((MRA_Equivs[out[j]] != 'x') && (MRA_Equivs[out[j]] == MRA_Equivs[out[i]])) {
            out.erase(j, 1);
        } else {
            ++j;
        }
    }

    // Trim the length to ONLY the first and last three chars. Allow shorts if
    // required (ie. ABCDFGHJ -> ABCFGHJ -> ABCGHJ, ABC -> ABC).
    while(out.size() > 6) { out.erase(3); }
    return out;
}

// Initializor function.
void Explicator_Module_MRA_Init(const std::map<std::string, std::string> &lexicon, float threshold) {
    return;
}

// Query function.
std::unique_ptr<std::map<std::string, float>>
Explicator_Module_MRA_Query(const std::map<std::string, std::string> &lexicon, const std::string &in, float threshold) {
    // Reminder: The lexicon looks like: < dirty : clean >
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());
    const auto mra_in = MatchRatingApproach(in);

    // Cycle through the lexicon, computing the MRA of each item. Compare it to that of the input.
    for(auto it = lexicon.begin(); it != lexicon.end(); ++it) {
        const auto mra_lex = MatchRatingApproach(it->first);

        const auto dlength = EXPLICATORABS(static_cast<long int>(mra_lex.size()) - static_cast<long int>(mra_in.size()));
        const auto length  = static_cast<long int>(mra_lex.size()) + static_cast<long int>(mra_in.size());

        // If the dl >= 3, do not compare (ie. it is not a match).
        if(dlength >= 3) {
            continue;
        }

        // Get the minimum rating.
        long int min;
        if(length <= 4) {
            min = 5;
        } else if(length <= 7) {
            min = 4;
        } else if(length <= 11) {
            min = 3;
        } else {
            min = 2;
        }

        // Copy both strings.
        std::string c_mra_lex(mra_lex), c_mra_in(mra_in);

        // Working L to R, remove common chars.
        for(size_t i = 0; (i < c_mra_lex.size()) && (i < c_mra_in.size());) {
            if(c_mra_lex[i] == c_mra_in[i]) {
                c_mra_lex.erase(i, 1);
                c_mra_in.erase(i, 1);
            } else {
                ++i;
            }
        }

        // Working R to L, remove common chars (padding the shorter to the R).
        std::reverse(c_mra_lex.begin(), c_mra_lex.end());
        std::reverse(c_mra_in.begin(), c_mra_in.end());
        for(size_t i = 0; (i < c_mra_lex.size()) && (i < c_mra_in.size());) {
            if(c_mra_lex[i] == c_mra_in[i]) {
                c_mra_lex.erase(i, 1);
                c_mra_in.erase(i, 1);
            } else {
                ++i;
            }
        }

        const auto longest
            = static_cast<long int>((c_mra_lex.size() > c_mra_in.size()) ? c_mra_lex.size() : c_mra_in.size());
        const auto score = 6 - longest;

        if(score >= min) {
            // The actual MRA does not 'grade' the score. It is a binary match or no match.
            //(*output)[it->second] = 1.0;

            // However, we have some specificity available (thresholds). The best(worst) score is 6(0).
            const float grade = static_cast<float>(score) / 6.0;
            if(grade > threshold) { // If not present, insert it. If present, keep highest score.
                if(!(output->find(it->second) != output->end()) || ((*output)[it->second] < grade)) {
                    (*output)[it->second] = grade;
                }
            }
        }
    }
    return output;
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_MRA_Deinit(void) {
    return;
}
