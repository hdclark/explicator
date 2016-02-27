// Explicator_Module_DICOM_Hash.cc - DICOMautomaton, 2012.
//

#include <stddef.h>
#include <bitset>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <utility>

#include "Misc.h"

typedef std::bitset<70> feature_space_vec;

static std::list<std::pair<std::string, feature_space_vec>> hashed_lexicon; // clean string,  hash(dirty string).

// This file provides a hash function which is geared toward matching similar strings.
// specifically for DICOM-ish (medical) tags and strings. Emphasis is placed on
// differentiating between tags like 'left ...' and 'right ...', '6_...' and '7_...', and
// DICOM-standard tags.
//
// Ultimately, this approach simply maps strings to an N-dimensional 'feature space.' The
// features are shown below. They will probably not perform well for general text.
// The goal of the mapping is to minimize the differences due to:
// - Spelling
// - Capitalization
// - Length
// - Spaces or non-[a-zA-Z] characters
//
//  -------Good Ideas-----------------
//  - Check if ~'L' or 'R' are first relevant characters.
//  - Count # of words.
//  - Find special modifiers like '+3mm' or '_plus_margin' to help differentiate modified labels.
//
//  -------Untested/Unknown Ideas-----
//  - Enumerating chars present.              <----((Should I disregard vowels?))
//
//  --------Bad Ideas----------------- (tested)
//  - Number of vowels or consonants.
//  - Length of string.
//  - Differentiating between lower/uppercase chars.
//
// To more appropriately rate the importance of these things, we will assign a weight to
// each bit when computing the 'bitwise-distance.'

feature_space_vec DICOM_Hash(const std::string &in) {
    feature_space_vec out(0);
    size_t found;

    //-----------------------------------------------Let the hashing
    // begin-------------------------------------------------
    //----BITS [0,25]: Denote characters present.
    for(unsigned int i = 0; i < 26; ++i) { // A (65) - Z (90)  ((ascii codes)).
        found = in.find(static_cast<unsigned char>(i + 65));
        if(found != std::string::npos) {
            out.set(i);
        }
    }
    for(unsigned int i = 0; i < 26; ++i) { // a (97) - z (122)  ((ascii codes)).
        found = in.find(static_cast<unsigned char>(i + 97));
        if(found != std::string::npos) {
            out.set(i);
        }
    }

    //----BITS [26,51]: Denote ASCII Characters present at beginning of words.
    {
        std::istringstream inss(in.c_str());
        std::string temp;
        bool wasgood;
        while(inss.good()) {
            wasgood = inss.good();
            inss >> temp;
            if(wasgood && (temp.size() > 0)) {
                const char *char_out = temp.c_str();
                if(char_out[0] == '\0') {
                    break;
                }

                unsigned int ascii_code = static_cast<unsigned int>(char_out[0]);
                unsigned int index;
                if(isininc(65, ascii_code, 90)) { // This is what we want. Lowercase.
                    index = ascii_code - 65;
                } else if(isininc(97, ascii_code, 122)) { // Uppercase
                    index = ascii_code - 65 - 32;
                } else {
                    break;
                }
                // std::cout << "First letter was '" << static_cast<unsigned char>(ascii_code) << "', converted to index
                // (" << index << ")" << std::endl;
                out.set(index + 26);
            }
        }
    }

    //----BIT [52]: If the first non-space char is 'L' or 'l'.
    found = in.find_first_not_of(' ');
    if((found != std::string::npos) && ((in[found] == 'l') || (in[found] == 'L'))) {
        out.set(52);
    }

    //----BIT [53]: If the first non-space char is 'r' or 'R'.
    found = in.find_first_not_of(' ');
    if((found != std::string::npos) && ((in[found] == 'r') || (in[found] == 'R'))) {
        out.set(53);
    }

    //----BIT [54]: If the string is one word or multiple words.
    //    (Here we make use of how strngstreams iterators break up streams into strings (separated by any amount of
    //     whitespace.))
    {
        std::istringstream inss(in.c_str());
        std::istream_iterator<std::string> it(inss);
        std::istream_iterator<std::string> end; // Default constructor is 'End of Stream.'
        size_t wrd_cnt = 0;
        while((it++) != end) {
            ++wrd_cnt; // NOTE: Won't work with ++it!
        }
        if(wrd_cnt > 1) {
            out.set(54);
        }
    }

    return out;
}

long int DICOM_Hash_dist(const feature_space_vec &A, const feature_space_vec &B) {
    // This function is responsible for determining the 'distance' between two separate
    // hashes. There is NO score adjustment in this case - look at DICOM_Hash_score(...)
    // for this functionality.
    //
    // NOTE: That this metric differs significantly than the DICOM_Hash_score(...) metric.
    // For instance: for the score, higher is better. For the distance, lower is better!
    if(A.size() != B.size())
        FUNCERR("Attempting to compare hashes of different lengths");
    long int res = 0;
    for(unsigned int i = 0; i < A.size(); ++i) {
        if(A[i] != B[i]) {
            ++res;
        }
    }
    return res;
}

// This score function returns low values for bad matches and high values for good matches.
long int DICOM_Hash_score(const feature_space_vec &A, const feature_space_vec &B) {
    if(A.size() != B.size())
        FUNCERR("Attempting to compare hashes of different lengths");
    long int res = 0;

    // To increase the flexibility of weighting, each basic token of 'score' is 6000.
    // This allows us to multiply it, divide it, and break it into reasonable units.
    const long int unit = 6000;
    for(long int i = 0; i < static_cast<long int>(A.size()); ++i) {
        //----BITS [0,25]: Denote characters present.
        // We penalize a small amount for being off. We increase unit score for matching bits.
        if(isininc(0, i, 25)) {
            if(A[i] == B[i]) {
                res += unit;
            } else {
                res -= unit / 3;
            }
        }

        //----BITS [26,51]: Denote Characters present at beginning of words.
        // We doubly-value first-of-word letters. They rarely seem to get dropped upon shortening.
        if(isininc(26, i, 51)) {
            if(A[i] == B[i]) {
                res += 2 * unit;
            }
        }

        //----BIT [52]: If the first non-space char is 'L' or 'l'.
        // We highly value preceeding 'l' or 'L's They are unlikely to EVER be lost upon shortening.
        if(isininc(52, i, 52)) {
            if(A[i] == B[i]) {
                res += 4 * unit;
            }
        }

        //----BIT [53]: If the first non-space char is 'R' or 'r'.
        // We highly value preceeding 'r' or 'R's They are unlikely to EVER be lost upon shortening.
        if(isininc(53, i, 53)) {
            if(A[i] == B[i]) {
                res += 4 * unit;
            }
        }

        //----BIT [54]: If the string is one word or multiple words.
        // I am contemplating even leaving this one in here...
        if(isininc(54, i, 54)) {
            if(A[i] == B[i]) {
                res += unit / 2;
            }
        }
    }
    return res;
}

// Initializor function.
void Explicator_Module_DICOM_Hash_Init(const std::map<std::string, std::string> &lexicon,
                                       float threshold) { // The lexicon looks like: < dirty : clean >.
    // We run through the data and compute a hash of each (dirty) string. Upon a query, we compute the hash and compare
    // hashes.
    hashed_lexicon.clear();
    for(auto it = lexicon.begin(); it != lexicon.end(); ++it) {
        hashed_lexicon.push_back(std::pair<std::string, feature_space_vec>(it->second, DICOM_Hash(it->first)));
    }
}

// Query function.
std::unique_ptr<std::map<std::string, float>>
Explicator_Module_DICOM_Hash_Query(const std::map<std::string, std::string> &lexicon,
                                   const std::string &in,
                                   float threshold) {
    // Remember: The lexicon looks like: < dirty : clean >
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());
    const feature_space_vec in_hashed      = DICOM_Hash(in);
    const feature_space_vec inverse_hashed = feature_space_vec(in_hashed).flip();

    const float theobest = static_cast<float>(
        DICOM_Hash_score(in_hashed, in_hashed)); // Perfect match. This score will be the highest possible.
    const float theoworst = static_cast<float>(
        DICOM_Hash_score(in_hashed, inverse_hashed)); // Worst match. This score will be lowest possible.

    // It turns out that the lowest possible score is very low. It is infrequently encountered in "normal" data. This
    // module may require
    // a higher threshold than normal. Scores tend to cluster around 0.75-0.85.
    //
    // Typical best/worst scores are:   theobest 519000 and theoworst -52000.

    if(theobest <= theoworst) {
        FUNCWARN("The theoretical maximum score is <= theoretical minimum - unable to compute anything meaningful");
        return std::move(output);
    }

    for(auto it = hashed_lexicon.begin(); it != hashed_lexicon.end(); ++it) {
        const float score  = static_cast<float>(DICOM_Hash_score(it->second, in_hashed));
        const float scaled = (score - theoworst) / (theobest - theoworst);

        if((scaled > threshold) && (!(output->find(it->first) != output->end())
                                    || ((*output)[it->first] < scaled))) { // If this score is higher.
            (*output)[it->first] = scaled;
            // Do not break on an exact match. This is not a very exact module and this is detrimental to mixing with
            // other modules.
            // if(score == theobest) break; //This is an exact match - no need to look further.
        }
    }
    return std::move(output);
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_DICOM_Hash_Deinit(void) {
    hashed_lexicon.clear();
}
