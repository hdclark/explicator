// String.cc.

#include <stddef.h>
#include <algorithm> //Needed for set_intersection(..), reverse().
// For Canonicalization function.
#include <cctype> //Needed for locale-less ::toupper().
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

#include "Misc.h"   //Needed for error functions (for debugging) and isininc macro.
#include "String.h" //Includes namespace constants, function decl.'s, etc..

namespace explicator_internals {

//-------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------ Self-contained N-gram routines
//-----------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------
// Note: By "N-gram", here we mean that a string like "some string" would be split into the following N-grams:
//                     "some string"   --->   {som, ome, str, tri, rin, ing}
// note that these N-grams do NOT cross whitespace. Other than splitting words, whitespace is ignored.
// To have N-grams which WOULD cross whitespace (but still ignore it,) one will need to move the internal
// while loop outside of the outer loop for these routines (but it shouldn't be too difficult to adjust!)
//
std::map<std::string, float> NGrams_With_Occurence(const std::string &thestring,
                                                   long int numb_of_ngrams,
                                                   long int length_of_ngrams,
                                                   const unsigned char &type) {
    // Pass in numb_of_ngrams = -1 to generate ALL ngrams.
    // Note that the computation is truncated when numb_of_ngrams has been reached - there is *no* selectivity in the
    // output upon truncation.
    std::map<std::string, float> output;
    long int ngrams_generated = 0;
    std::stringstream inss(thestring);
    std::string theword;
    while(inss.good()) {
        inss >> theword;

        if((type & NGRAMS::CHARS) == NGRAMS::CHARS) { // Character N-grams. These are of a (user) specified length.
            while(static_cast<long int>(theword.size()) >= length_of_ngrams) {
                if((ngrams_generated >= numb_of_ngrams) && (numb_of_ngrams != -1)) {
                    return output;
                }

                // Push back the current Ngram. Remove the first character for the next cycle.
                ++ngrams_generated;
                output[theword.substr(0, length_of_ngrams)] += 1.0f;
                theword.erase(theword.begin());
            }

        } else if((type & NGRAMS::WORDS)
                  == NGRAMS::WORDS) { // Word N-grams. These are NOT of a (user) specified length.
            if(ngrams_generated <= numb_of_ngrams) {
                if(theword.size() > 0) {
                    output[theword] += 1.0f;
                    ++ngrams_generated;
                }
            } else {
                return output;
            }
        }
    }
    return output;
}

// TODO(hal):  fix the numb_of_ngrams ignoring in the NGRAMS::WORDS case. (and for the above function.)

std::set<std::string>
NGrams(const std::string &thestring, long int numb_of_ngrams, long int length_of_ngrams, const unsigned char &type) {
    // Pass in numb_of_ngrams = -1 to generate ALL ngrams.
    // Note that the computation is truncated when numb_of_ngrams has been reached - there is *no* selectivity in the
    // output upon truncation.
    std::set<std::string> output;
    long int ngrams_generated = 0;
    std::stringstream inss(thestring);
    std::string theword;
    while(inss.good()) {
        inss >> theword;
        if((type & NGRAMS::CHARS) == NGRAMS::CHARS) { // Character N-grams. These are of a (user) specified length.
            while(static_cast<long int>(theword.size()) >= length_of_ngrams) {
                if((ngrams_generated >= numb_of_ngrams) && (numb_of_ngrams != -1)) {
                    return output;
                }

                // Push back the current Ngram. Remove the first character for the next cycle.
                ++ngrams_generated;
                output.insert(theword.substr(0, length_of_ngrams));
                theword.erase(theword.begin());
            }

        } else if((type & NGRAMS::WORDS)
                  == NGRAMS::WORDS) { // Word N-grams. These are NOT of a (user) specified length.
            if(ngrams_generated <= numb_of_ngrams) {
                if(theword.size() > 0) {
                    output.insert(theword);
                    ++ngrams_generated;
                }
            } else {
                return output;
            }
        }
    }
    return output;
}

std::set<std::string> NGram_Matches(const std::set<std::string> &A, const std::set<std::string> &B) {
    std::set<std::string> output;
    std::set_intersection(A.cbegin(), A.cend(), B.cbegin(), B.cend(), std::inserter(output, output.begin()));
    return output;
}

long int NGram_Match_Count(const std::set<std::string> &A, const std::set<std::string> &B) {
    return static_cast<long int>((NGram_Matches(A, B)).size());
}

//-------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------- Substring and Subsequence routines
//----------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------

// Returns (one of) the longest common sequential substring(s). ie. 'ABCDEF' and 'ACDEF' gives 'CDEF'.
// If there are multiples, it is not specified which will be returned.
std::string ALongestCommonSubstring(const std::string &A, const std::string &B) {
    // Issues with this routine: there could be many ties for the 'longest' common substring. Which do we return?
    //
    // Examples:
    //  "ABCDEF_FEDCBA" and "CDEFED" could return "CDE" or "FED". Both are substrings and have equal length.
    //  "ABCDEFABC" and "ABC" will uniquely return "ABC". But there are two such substrings. This may be relevant.
    //
    // The code below finds one of the longest substrings (I think). If the user doesn't care (often the case) and
    // still chooses to use this routine, then so be it!

    if(A.empty() || B.empty()) {
        return "";
    }
    if(B.size() > A.size()) {
        return ALongestCommonSubstring(B, A); // Less memory usage.
    }

    std::vector<std::string> curr(B.size(), ""), prev(B.size(), ""), forswap;
    std::string out;
    for(size_t i = 0; i < A.size(); ++i) {
        for(size_t j = 0; j < B.size(); ++j) {
            if(A[i] != B[j]) {
                if(out.size() < curr[j].size()) {
                    out = curr[j];
                }
                curr[j].clear();
            } else {
                if(i == 0 || j == 0) {
                    curr[j].clear();
                } else {
                    curr[j] = prev[j - 1];
                }
                curr[j] += A[i];
            }
        }
        forswap = std::move(curr);
        curr    = std::move(prev);
        prev    = std::move(forswap);
    }

    for(size_t i = 0; i < curr.size(); ++i) {
        if(out.size() < curr[i].size()) {
            out = curr[i];
        }
        if(out.size() < prev[i].size()) {
            out = prev[i];
        }
    }
    return out;
}

//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------- Common text transformations
//------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------
static std::string &Canonicalize_String(std::string &in, const unsigned char &opts) {
    // This function can be used as a filter OR as a pass-in-and-act-on-it function.

    // Transform to all upper case.
    if((opts & CANONICALIZE::TO_UPPER) == CANONICALIZE::TO_UPPER) {
        // Using the C (ctype.h or cctype needed) way. No locale involved, I think.
        std::string temp;
        std::transform(in.begin(), in.end(), std::back_inserter(temp), ::toupper);
        in = temp;

        // Using the C++ <locale> way. (Requires a specific locale.)
        // for (size_t i=0; i<in.length(); ++i) in[i] = toupper(in[i],loc);

        // Using Boost. This way is fairly fast.
        // boost::to_upper(in);
    }

    // Whitespace filter. Works for beginning, end, and interim whitespaces.
    if((opts & CANONICALIZE::TRIM) == CANONICALIZE::TRIM) {
        // stringstream >> will spit out an empty string if there is trailing space. This routine *requires* such space!
        std::stringstream ss(in + " ");
        if(!ss.good()) {
            return in;
        }
        in.clear();
        std::string temp;
        ss >> temp;
        do {
            if(!temp.empty()) {
                in += temp;
            }
            ss >> temp;
            if(!temp.empty() && ss.good()) {
                in += " ";
            }
        } while(ss.good());
    }

    // Whitespace filter. Works for beginning, end, and interim whitespaces.
    if((opts & CANONICALIZE::TRIM_ENDS) == CANONICALIZE::TRIM_ENDS) {
        const std::string whitespace(" \t"); //\f\v\n\r"); //Consider vertical tabs and newlines too? Don't assume so...

        // Trailing.
        const auto last_non = in.find_last_not_of(whitespace);
        if(last_non != std::string::npos) {
            in.erase(last_non + 1);
        } else { // All whitespace!
            in.clear();
        }

        // Preceeding.
        const auto first_non = in.find_first_not_of(whitespace);
        if(first_non != std::string::npos) {
            in.erase(0, first_non);
        } else { // All whitespace!
            in.clear();
        }
    }

    // Remove ALL whitespace, leaving a single, long, whitespace-less string (in the order they originally were in.)
    if((opts & CANONICALIZE::TRIM_ALL) == CANONICALIZE::TRIM_ALL) {
        std::stringstream inss(in + " ");
        in.clear();
        std::string theword;
        while(inss.good()) {
            inss >> theword;
            if(!theword.empty() && inss.good()) {
                in += theword;
            }
        }
    }

    // Remove all non-[ A-Za-z] characters.
    if((opts & CANONICALIZE::TO_AZ) == CANONICALIZE::TO_AZ) {
        for(auto it = in.begin(); it != in.end(); ++it) {
            if((*it) != ' ') {
                if(!(isininc('A', *it, 'Z') || isininc('a', *it, 'z'))) { // !( ((*it) >= 'A') && ((*it) <= 'Z') )  &&
                                                                          // !( ((*it) >= 'a') && ((*it) <= 'z') ) ){
                    in.erase(it);
                    --it;
                }
            }
        }
    }

    // Remove all non-[ 0-9-.] characters.
    if((opts & CANONICALIZE::TO_NUM) == CANONICALIZE::TO_NUM) {
        for(auto it = in.begin(); it != in.end(); ++it) {
            if((*it) != ' ') {
                if(!(isininc('1', *it, '9') || (*it == '0') || (*it == '.') || (*it == '-'))) {
                    in.erase(it);
                    --it;
                }
            }
        }
    }

    // Remove all non-[ A-Za-z0-9-.]
    if((opts & CANONICALIZE::TO_NUMAZ) == CANONICALIZE::TO_NUMAZ) {
        for(auto it = in.begin(); it != in.end(); ++it) {
            if((*it) != ' ') {
                if(!(isininc('1', *it, '9') || (*it == '0') || (*it == '.') || (*it == '-') || isininc('A', *it, 'Z')
                     || isininc('a', *it, 'z'))) {
                    in.erase(it);
                    --it;
                }
            }
        }
    }

    return in;
}

std::string Canonicalize_String2(const std::string &in, const unsigned char &mask) {
    std::string temp(in);
    Canonicalize_String(temp, mask);
    return temp;
}

} //namespace explicator_internals

