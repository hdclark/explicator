// Explicator_Module_JaroWinkler.cc - DICOMautomaton, 2013.
//

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Misc.h"

#define NOTNUM(c) (((c) > 57) || ((c) < 48))
#define INRANGE(c) (((c) > 0) && ((c) < 91))

#ifndef NaN
#define NaN (0.0 / 0.0)
#endif

// This function computes the well known (normalized) Jaro-Winkler distance between
// two strings.
//
// The code is based off a reference implementation at
// https://github.com/sunlightlabs/jellyfish/blob/master/jaro.c
// which was accessed on March 20th, 2013. This code itself is "borrowed heavily
// from strcmp95.c - http://www.census.gov/geo/msb/stand/strcmp.c" which is the
// original implementation from Jaro (and Winkler?). I believe substantial enough
// modifications have been made to deem this a distinct implementation from the
// jellyfish code.
//

// double _jaro_winkler(const char *A, long int A_length, const char *B, long int B_length, bool long_tolerance, bool
// winklerize){
// double _jaro_winkler(const std::string &A, const std::string &B, bool long_tolerance, bool winklerize){
double JaroWinkler(const std::string &A, const std::string &B) {
    const bool long_tolerance = false; // Better for long strings.
    const bool winklerize     = true;  // Places extra weighting on the first few characters.

    if(A.empty() && B.empty()) {
        return 1.0;
    }
    if(A.empty() || B.empty()) {
        return 0.0;
    }

    const auto min_len      = static_cast<long int>(EXPLICATORMAX(A.size(), B.size()));
    const auto search_range = EXPLICATORMAX(0, (min_len / 2 - 1)); //      ((min_len/2 - 1) < 0) ? 0 : (min_len/2 - 1);

    std::vector<bool> A_flag(A.size() + 1, false);
    std::vector<bool> B_flag(B.size() + 1, false);

    const auto Asize = static_cast<long int>(A.size());
    const auto Bsize = static_cast<long int>(B.size());

    long int common_chars(0); // Common chars within the search range.
    for(long int i = 0; i < Asize; ++i) {
        const long int lowlim = EXPLICATORMAX(0, i - search_range); //(i >= search_range) ? (i - search_range) : 0;
        const long int hilim
            = EXPLICATORMIN(i + search_range,
                      Bsize - 1); //      ((i + search_range) <= (Bsize-1)) ? (i + search_range) : (Bsize - 1);
        for(long int j = lowlim; j <= hilim; ++j) {
            if(!B_flag[j] && (B[j] == A[i])) {
                B_flag[j] = true;
                A_flag[i] = true;
                ++common_chars;
                break;
            }
        }
    }
    if(common_chars == 0) {
        return 0.0; // No matching chars => similarity = 0.
    }

    long int k(0), trans_count(0); // Number of transpositions.
    for(long int i = 0; i < Asize; ++i) {
        if(A_flag[i]) {
            long int j = k;
            for(; j < Bsize; ++j) {
                if(B_flag[j]) {
                    k = j + 1;
                    break;
                }
            }
            if((j < Bsize) && (A[i] != B[j])) {
                ++trans_count;
            }
        }
    }
    trans_count /= 2;

    // This is the score. It can be modified (see below).
    auto out = (static_cast<double>(common_chars) / static_cast<double>(Asize)
                + static_cast<double>(common_chars) / static_cast<double>(Bsize)
                + static_cast<double>(common_chars - trans_count) / static_cast<double>(common_chars))
               / 3.0;

    if(winklerize && (out > 0.7)) { // If they appear fairly similar, perform more precise corrections.
        // Adjust for having up to the first 4 characters in common.
        const long int j = EXPLICATORMIN(4, min_len); //    (min_len >= 4) ? 4 : min_len;
        long int cnt(0);
        for(long int i = 0; ((i < j) && (A[i] == B[i]) && NOTNUM(A[i])); ++i) { ++cnt; }
        if(cnt != 0) {
            out += static_cast<double>(cnt) * 0.1 * (1.0 - out);
        }

        // Try to account for long strings. We require at least two more agreeing chars.
        // Furthermore, the total number of agreeing chars must be more than 50% of chars.
        if(long_tolerance && (min_len > 4) && (common_chars > (cnt + 1)) && ((2 * common_chars) >= (min_len + cnt))
           && NOTNUM(A[0])) {
            const auto numer  = static_cast<double>(common_chars - cnt - 1);
            const auto denom1 = static_cast<double>(Asize + Bsize);
            const auto denom2 = 2.0 * (1.0 - static_cast<double>(cnt));
            out += (1.0 - out) * numer / (denom1 + denom2);
        }
    }

    return out;
}

void Explicator_Module_JaroWinkler_Init(const std::map<std::string, std::string> &lexicon, float threshold) {
    return;
}

std::unique_ptr<std::map<std::string, float>>
Explicator_Module_JaroWinkler_Query(const std::map<std::string, std::string> &lexicon,
                                    const std::string &in,
                                    float threshold) {
    // Remember: The lexicon looks like: < dirty : clean >
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());

    for(auto it = lexicon.begin(); it != lexicon.end(); ++it) {
        const float score = static_cast<float>(JaroWinkler(it->first, in));

        if(score > threshold) { // If not present, insert it. If present, keep highest score.
            if(!(output->find(it->second) != output->end()) || ((*output)[it->second] < score)) {
                (*output)[it->second] = score;
            }
        }
    }
    return output;
}

void Explicator_Module_JaroWinkler_Deinit(void) {
    return;
}
