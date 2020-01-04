// Explicator_Module_Levenshtein.cc - DICOMautomaton, 2012.
//
// NOTE: These techniques are STRONGLY dependent on the case of the input. Ensure queries and data
// have the same case! An attempt has been made to ensure everything is capitalized, but check it
// first if something goes wrong.

#include <algorithm> //Needed for min()
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "Misc.h"

static float Longest_String_Length;

// This was originally found online at http://www.merriampark.com/ldcpp.htm on May 27th 2012. The title and author are:
// "Levenshtein Distance Algorithm: C++ Implementation" by Anders Sewerin Johansen. There is no copyright information
// explicitly stated (which I could find). I believe the present usage is allowed as fair use. Some modifications
// have been made and it is not a verbatim copy.
int Levenshtein_Damerau_Dist(const std::string &source, const std::string &target) {
    const int n(source.length()), m(target.length());
    if(n == 0) {
        return m;
    }
    if(m == 0) {
        return n;
    }

    std::vector<std::vector<int>> matrix(n + 1);
    for(int i = 0; i <= n; ++i) { matrix[i].resize(m + 1); }

    for(int i = 0; i <= n; ++i) { matrix[i][0] = i; }
    for(int j = 0; j <= m; ++j) { matrix[0][j] = j; }

    for(int i = 1; i <= n; ++i) {
        const char s_i(source[i - 1]);
        for(int j = 1; j <= m; ++j) {
            const char t_j = target[j - 1];
            const int cost((s_i == t_j) ? 0 : 1);
            const int abov(matrix[i - 1][j]);
            const int left(matrix[i][j - 1]);
            const int diag(matrix[i - 1][j - 1]);
            int cell(std::min(abov + 1, std::min(left + 1, diag + cost)));

            // This part allows for transposition, deletion, insertion, and
            // substitution. Originally from Berghel, Hal & Roach, David's
            // "An Extension of Ukkonen's Enhanced Dynamic Programming ASM
            // Algorithm" http://www.acm.org/~hlb/publications/asm/asm.html
            if(i > 2 && j > 2) {
                int trans(matrix[i - 2][j - 2] + 1);
                if(t_j != source[i - 2]) {
                    ++trans;
                }
                if(s_i != target[j - 2]) {
                    ++trans;
                }
                if(cell > trans) {
                    cell = trans;
                }
            }
            matrix[i][j] = cell;
        }
    }
    return matrix[n][m];
}

// Initializor function.
void Explicator_Module_Levenshtein_Init(const std::map<std::string, std::string> &lexicon, float threshold) {
    // Determine the maximum (dirty) string length. This is used to determine upper bound on score.
    auto string_length_comp
        = [](const std::pair<std::string, std::string> &A, const std::pair<std::string, std::string> &B) -> bool {
        return A.first.size() < B.first.size();
    };
    Longest_String_Length
        = static_cast<float>((std::max_element(lexicon.begin(), lexicon.end(), string_length_comp))->first.size());
}

// Query function.
std::unique_ptr<std::map<std::string, float>>
Explicator_Module_Levenshtein_Query(const std::map<std::string, std::string> &lexicon,
                                    const std::string &in,
                                    float threshold) {
    // Remember: The lexicon looks like: < dirty : clean >
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());

    // I think this is the maximum theoretial distance, but am unsure. If it is not, then one will see negatives in the
    // score output!
    // const float theomax = static_cast<float>(Levenshtein_Damerau_Dist(Explicator_Module_Levenshtein_Max_Dirty_String,
    // ""));
    const float inlength = static_cast<float>(in.size());
    const float theomax
        = (inlength > Longest_String_Length) ? inlength : Longest_String_Length; // Longest of all the string's length.
    const float theomin = 0.0; // The theoretical minimum edit distance.
    auto normalize = [=](float x) -> float {
        return 1.0 - ((x - theomin) / (theomax - theomin));
    }; // Best score = 1.0, worst score = 0.0.

    if(theomax <= theomin) {
        FUNCWARN("The theoretical maximum score is <= theoretical minimum - unable to compute anything meaningful");
        return output;
    }

    // First we push back each string and the Levenshtein distance. We are trying to minimize the distance for each
    // element in the map.
    for(auto it = lexicon.begin(); it != lexicon.end(); ++it) {
        const float levn_dist = static_cast<float>(Levenshtein_Damerau_Dist(it->first, in));
        const float score     = normalize(levn_dist);

        // If the input-dirty string distance is the shortest for this clean string (or it hasn't been injected into the
        // map yet,) replace it.
        if((score > threshold) && (!(output->find(it->second) != output->end()) || ((*output)[it->second] < score))) {
            (*output)[it->second] = score;
            // Levenshtein distance is exact. If we find an exact match, it is best to exit immediately.
            if(levn_dist == 0.0) {
                break;
            }
        }
    }
    return output;
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_Levenshtein_Deinit(void) {
}
