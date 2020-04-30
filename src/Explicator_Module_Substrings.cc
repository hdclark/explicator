// Explicator_Module_Substrings.cc - DICOMautomaton, 2012.
//
// A module which uses substrings (NOTE: not subsequences!) to establish
// similarity. The goal is to "soften" this measure for more fuzzy matches
// determining N longest common substrings, and then weighting each
// in a progressively less heavy manner. This will allow for soft matches,
// but will keep the precision up for very long substrings.

#include <string>
#include <map>
#include <memory>
#include <utility>

#include "Misc.h"
#include "String.h"

using namespace explicator_internals;

void Explicator_Module_Substrings_Init(const std::map<std::string, std::string> &lexicon,
                                       float threshold) { // The lexicon looks like: < dirty : clean >.
    return;
}

// Query function.
std::unique_ptr<std::map<std::string, float>>
Explicator_Module_Substrings_Query(const std::map<std::string, std::string> &lexicon,
                                   const std::string &in,
                                   float threshold) {
    // Remember: The lexicon looks like: < dirty : clean >
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());

    for(auto it = lexicon.begin(); it != lexicon.end(); ++it) {
        const auto max_substr_len = static_cast<float>(ALongestCommonSubstring(it->first, in).size());
        const auto max_str_len    = static_cast<float>(EXPLICATORMAX((it->first).size(), in.size()));

        if(max_str_len == 0.0) {
            FUNCEXPLICATORWARN("Comparing two empty strings. Ignoring!");
            continue;
        }

        const auto score = max_substr_len / max_str_len;
        if(score > threshold) { // If not present, insert it. If present, keep highest score.
            if(!(output->find(it->second) != output->end()) || ((*output)[it->second] < score)) {
                (*output)[it->second] = score;
            }
        }
    }
    return output;
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_Substrings_Deinit(void) {
    return;
}
