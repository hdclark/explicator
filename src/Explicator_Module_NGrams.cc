// Explicator_Module_NGrams.cc - DICOMautomaton, 2012.
//
// Looks for similar N-grams in order to establish 'nearness' of strings. The incoming
// string is split into N-grams. The lexicon is queried.
//    -Any N-grams which are present in the query but not in the lexicon string are penalized.
//    -Any N-grams which are present in the lexicon string but not in the query are ignored.
//         (For a module which takes these into account, see the 'Emplacement' module.)
//
// This module ought to be increasingly precise as the strings get longer, but handling
// abbreviations is (probably) not very good. Longer input is probably more precise, but will
// quickly consume memory.
//
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Misc.h"
#include "String.h" //Needed for NGram functions.

using namespace explicator_internals;

// Choose the size of the N-grams. In this case, we compute N-(character)-grams.
#define NGRAM_N 2

static std::vector<std::pair<std::string, std::set<std::string>>> lexicon_ngrams;

// Initializor function.
void Explicator_Module_NGrams_Init(const std::map<std::string, std::string> &lexicon, float threshold) {
    // We cycle through the lexicon and generate all N-grams of each 'dirty' string.
    lexicon_ngrams.clear();
    for(auto it = lexicon.begin(); it != lexicon.end(); ++it) {
        lexicon_ngrams.push_back(
            std::pair<std::string, std::set<std::string>>(it->second, NGrams(it->first, -1, NGRAM_N, NGRAMS::CHARS)));
    }
}

// Query function.
std::unique_ptr<std::map<std::string, float>>
Explicator_Module_NGrams_Query(const std::map<std::string, std::string> &lexicon,
                               const std::string &in,
                               float threshold) {
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());
    const std::set<std::string> in_ngrams = NGrams(in, -1, NGRAM_N, NGRAMS::CHARS);

    const float theoworst = 0.0;
    const float theobest  = static_cast<float>(in_ngrams.size()); // Maximum number of positive matches.

    if(theobest <= theoworst) {
        FUNCEXPLICATORWARN("The theoretical maximum score is <= theoretical minimum - unable to compute anything meaningful");
        return output;
    }
    auto matchcount_to_score = [=](float x) -> float { return ((x - theoworst) / (theobest - theoworst)); };

    for(auto it = lexicon_ngrams.begin(); it != lexicon_ngrams.end(); ++it) {
        const float matchcount = static_cast<float>(NGram_Match_Count(in_ngrams, it->second));
        const float score      = matchcount_to_score(matchcount);

        if((score > threshold) && (!(output->find(it->first) != output->end()) || ((*output)[it->first] < score))) {
            (*output)[it->first] = score;
            // Do not break on an exact match. This is not a very exact module and this is detrimental to mixing with
            // other modules.
            // if(score == theobest) break; //This is an exact match - no need to look further.
        }
    }
    return output;
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_NGrams_Deinit(void) {
    lexicon_ngrams.clear();
    // Nothing to free!
}
