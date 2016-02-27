// Explicator_Module_Emplacement.cc - DICOMautomaton, 2012.
//
// Looks for similar placement of characters in order to establish 'nearness' of strings.
//

#include <stddef.h>
#include <algorithm> //Needed for set_intersection(..);
#include <iterator>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "Misc.h"   //Needed for FUNCINFO, FUNCERR, etc..
#include "String.h" //Needed for Canonicalization().

//----------------------------------------------------------------------------------------------------------------
//---------------------------  To use the alternate technique, uncomment this line. ------------------------------
//----------------------------------------------------------------------------------------------------------------
//    -Option A (the default) will probably do better if incoming queries are abbreviations of lexicon elements.
//    -Option B might do better if abbreviations exist in the lexicon.
//----------------------------------------------------------------------------------------------------------------
//#define EXPLICATOR_OPTION_B

static std::vector<std::pair<std::string, std::set<std::string>>> lexicon_emplacements;
static const std::string
    Most_Freq_English("eainorstldumcphgkvbfzywjqx"); // Most frequent first. [e-t] comprises 63% of character frequency.
static const std::string Least_Freq_English("xqjwyzfbvkghpcmudltsroniae"); // Most frequent last.
static std::string Relevant; // This holds the list of characters we will consider.
#ifdef EXPLICATOR_OPTION_B
static float largestsetsize; // Used to compute theoworst.
#endif

std::set<std::string> Emplacement(const std::string &thestring) {
    std::set<std::string> output;

    // Trim ALL whitespace, leaving a single, long sequence of letters.
    const std::string nowhitespace = Canonicalize_String2(thestring, CANONICALIZE::TRIM_ALL);

    // Now cycle through the characters, pushing back those pairs that define the placement of characters.
    for(size_t rhs = 0; rhs < nowhitespace.size(); ++rhs) {
        // Check if rhs_it is in the string of relevant characters.
        if((Relevant.find(nowhitespace[rhs]) != std::string::npos)) {
            for(size_t lhs = 0; lhs < rhs; ++lhs) {
                // Check if lhs_it is in the string of relevant characters.
                if((Relevant.find(nowhitespace[lhs]) != std::string::npos)) {
                    std::string temp;
                    temp.push_back(nowhitespace[lhs]);
                    temp.push_back(nowhitespace[rhs]);
                    output.insert(temp);
                }
            }
        }
    }
    return output;
}

std::set<std::string> Emplacement_Matches(const std::set<std::string> &A, const std::set<std::string> &B) {
    std::set<std::string> output;
    std::set_intersection(A.cbegin(), A.cend(), B.cbegin(), B.cend(), std::inserter(output, output.begin()));
    return output;
}

std::set<std::string> Emplacement_Differences(const std::set<std::string> &A, const std::set<std::string> &B) {
    std::set<std::string> output;
    std::set_symmetric_difference(A.cbegin(), A.cend(), B.cbegin(), B.cend(), std::inserter(output, output.begin()));
    return output;
}

// Initializor function.
void Explicator_Module_Emplacement_Init(const std::map<std::string, std::string> &lexicon, float threshold) {
    // Choose which characters are considered 'relevant.' Choosing overly popular characters waters down the
    // efficiency (large amounts of memory used,) and choosing overtly obscure characters waters down the
    // efficacy (matches all words without 'q' and 'z', for example.)

    // Least_Freq_English.substr(0,18) --> [x-l] --> 37% of all characters in (standard) English.
    // Relevant = Least_Freq_English.substr(0,18);

    // Alternatively, use ALL characters. This will produce more (many more) items, and will thus
    // be very slow (and hard on memory.) Being more selective (including as many characters as possible
    // seems to work better, though!
    Relevant = Canonicalize_String2(Least_Freq_English, CANONICALIZE::TO_UPPER);

    // Cycle through the lexicon and generate emplacements for each 'dirty' string.
    lexicon_emplacements.clear();
    for(auto it = lexicon.begin(); it != lexicon.end(); ++it) {
        const auto theset = Emplacement(it->first);
        lexicon_emplacements.push_back(std::pair<std::string, std::set<std::string>>(it->second, theset));
    }

#ifdef EXPLICATOR_OPTION_B
    // Determine the largest set size for theoretical maximum score.
    largestsetsize = 0.0;
    for(auto it = lexicon_emplacements.begin(); it != lexicon_emplacements.end(); ++it) {
        const float setsize = static_cast<float>(it->second.size());
        if(setsize > largestsetsize)
            largestsetsize = setsize;
    }
#endif
}

// Query function.
std::unique_ptr<std::map<std::string, float>>
Explicator_Module_Emplacement_Query(const std::map<std::string, std::string> &lexicon,
                                    const std::string &in,
                                    float threshold) {
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());
    const std::set<std::string> in_emplacements = Emplacement(in);

    if(in_emplacements.size() == 0) {
        // This is not really so much of a problem. Often - it just means we have a highly selective criteria or a short
        // string.
        // Since we precompute the emplacements, it would be a pain to switch to a more lax criteria on-the-fly. It is
        // safe
        // to simply return an empty map, so I wonder if this warning is really necessary?
        return std::move(output);
    }

    const float theoperfect = 0.0;

#ifndef EXPLICATOR_OPTION_B
    const float theoworst = static_cast<float>(in_emplacements.size()); // Size of the set produced by incoming string.
#else
    const float theoworst = largestsetsize; // Size of the largest set.
#endif

    if(theoworst <= theoperfect) {
        FUNCWARN("The theoretical maximum score is <= theoretical minimum - unable to compute anything meaningful");
        return std::move(output);
    }

    auto deviations_to_score = [=](float x) -> float { return 1.0 - ((x - theoperfect) / (theoworst - theoperfect)); };

    // std::vector<std::pair<std::string, std::set<std::string>>> lexicon_emplacements;
    for(auto it = lexicon_emplacements.begin(); it != lexicon_emplacements.end(); ++it) {
        const std::string clean(it->first);
        // We want to find the number of explicit deviations in the input from those in the lexicon.
        // This does NOT count simple absenses. It only counts the presence of previously unseen emplacements.
        //    deviationcount =  INTERSECTION( DIFF( SET(lexicon string), SET(input string) ), SET(input string) )'s
        //    size;
        //                   =  INTERSECTION( DIFF( A                  , B                 ), B                 )'s
        //                   size;
        //                   =  INTERSECTION( C                                             , B                 )'s
        //                   size;
        //                   =  D's size;
        const std::set<std::string> A = it->second;
        const std::set<std::string> B = in_emplacements;
        const std::set<std::string> C = Emplacement_Differences(A, B);

#ifndef EXPLICATOR_OPTION_B
        // OPTION A: Gives us the extra pieces from the *query* (good if query is an abbreviation of lexicon string.)
        const std::set<std::string> D = Emplacement_Matches(C, B);
#else
        // OPTION B: Gives us the extra pieces from the *lexicon string* (good if lexicon string is an abbreviation of
        // query string.)
        const std::set<std::string> D = Emplacement_Matches(C, A);
#endif

        const float deviationcount = static_cast<float>(D.size());
        const float score = deviations_to_score(deviationcount); // Score = 1.0 is perfect, score = 0.0 is no match.
        if((score > threshold)
           && (!(output->find(clean) != output->end())
               || (score > (*output)[clean]))) { // We want the number of deviations. Lower is better.
            (*output)[clean] = score;
            // Do not break on an exact match. This is not an exact module and this is detrimental to mixing with other
            // modules.
            // if(score == theobest) break; //This is an exact match - no need to look further.
        }
    }
    return std::move(output);
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_Emplacement_Deinit(void) {
    lexicon_emplacements.clear();
    Relevant.clear();
#ifdef EXPLICATOR_OPTION_B
    largestsetsize = 0.0;
#endif
}
