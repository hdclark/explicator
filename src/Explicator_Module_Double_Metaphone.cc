// Explicator_Module_Double_Metaphone.cc - DICOMautomaton, 2012.
//
// This module implements the 'Double Metaphone' string similarity routine.
// It is designed to improve the 'Soundex' algorithm for like-sounding
// strings.
//
// NOTE: This module produces a BINARY match only (it is or it isn't a match.)
// As such, the threshold is completely ignored.
//
// NOTE: This module is best designed for (perfectly) well-formatted and
// structured input. It will not be good for abbreviations or misspellings,
// but may work well for helping to translate converted audio to text,
// dealing with (poorly) translated text, or things like people's names
// (which it was designed for).

#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "String.h" //Needed for Canonicalize_String(...)

static std::vector<std::pair<std::string, std::string>> DM_lexicon;

namespace DOUBLEMETAPHONE {
    const unsigned char VOWEL = 0x1;
    const unsigned char SAME  = 0x2;
    const unsigned char V_SND = 0x4; // Vowel sound.
    const unsigned char F_VOW = 0x8; // Front vowel.
    const unsigned char NOGHF = 0xF;
} // namespace DOUBLEMETAPHONE

static const char alpha[] = {DOUBLEMETAPHONE::VOWEL,
                             DOUBLEMETAPHONE::NOGHF,
                             DOUBLEMETAPHONE::V_SND,
                             DOUBLEMETAPHONE::NOGHF,
                             DOUBLEMETAPHONE::VOWEL | DOUBLEMETAPHONE::F_VOW,
                             DOUBLEMETAPHONE::SAME,
                             DOUBLEMETAPHONE::V_SND,
                             DOUBLEMETAPHONE::NOGHF,
                             DOUBLEMETAPHONE::VOWEL | DOUBLEMETAPHONE::F_VOW,
                             DOUBLEMETAPHONE::SAME,
                             0,
                             DOUBLEMETAPHONE::SAME,
                             DOUBLEMETAPHONE::SAME,
                             DOUBLEMETAPHONE::SAME,
                             DOUBLEMETAPHONE::VOWEL,
                             DOUBLEMETAPHONE::V_SND,
                             0,
                             DOUBLEMETAPHONE::SAME,
                             DOUBLEMETAPHONE::V_SND,
                             DOUBLEMETAPHONE::V_SND,
                             DOUBLEMETAPHONE::VOWEL,
                             0,
                             0,
                             0,
                             DOUBLEMETAPHONE::F_VOW,
                             0};

static char DM_at(const std::string &word, int index) {
    if((index < 0) || (index >= int(word.size()))) {
        return 0;
    }
    return word[index];
}
static bool DM_is(char ch, const unsigned char flag) {
    return alpha[ch - 'A'] & flag;
}

std::string Double_Metaphone_To_Condensed_Phonetic(const std::string &input) {
    std::string key;
    std::string word
        = Canonicalize_String2(input, CANONICALIZE::TO_UPPER | CANONICALIZE::TO_AZ | CANONICALIZE::TRIM_ALL);
    if(word.length() <= 1) {
        return word;
    }
    if(((word[0] == 'P' || word[0] == 'K' || word[0] == 'G') && word[1] == 'N') || (word[0] == 'A' && word[0] == 'E')
       || (word[0] == 'W' && word[1] == 'R')) {
        word.erase(0, 1);
    }
    if(word[0] == 'W' && word[1] == 'H') {
        word.erase(0, 1);
        word[0] = 'W';
    }
    if(word[0] == 'X') {
        word[0] = 'S';
    }

    int word_size = int(word.size());
    for(int i = 0; i < word_size; ++i) {
        char ch   = DM_at(word, i);
        char chm1 = DM_at(word, i - 1);

        // Drop duplicates except for CC
        if(chm1 == ch && ch != 'C') {
            continue;
        }

        // Check for FJLMNR or first letter vowel
        if(DM_is(ch, DOUBLEMETAPHONE::SAME) || (i == 0 && DM_is(ch, DOUBLEMETAPHONE::VOWEL))) {
            key.push_back(ch);
        } else {
            char ch1 = DM_at(word, i + 1);
            char ch2 = DM_at(word, i + 2);

            switch(ch) {
                case 'B':
                    // -MB => -M at the end
                    if(chm1 != 'M' || i + 1 < word_size) {
                        key.push_back('B');
                    }
                    break;

                case 'C':
                    // in SCI, SCE, SCY => dropped
                    // in CIA, CH => X
                    // in CI, CE, CY => S
                    // else => K
                    if(chm1 != 'S' || !DM_is(ch1, DOUBLEMETAPHONE::F_VOW)) {
                        if(ch1 == 'I' && ch2 == 'A') {
                            key.push_back('X');
                        } else if(DM_is(ch1, DOUBLEMETAPHONE::F_VOW)) {
                            key.push_back('S');
                        } else if(ch1 == 'H') {
                            key.push_back(((i > 0 || DM_is(ch2, DOUBLEMETAPHONE::VOWEL)) && chm1 != 'S') ? 'X' : 'K');
                        } else {
                            key.push_back('K');
                        }
                    }
                    break;

                case 'D':
                    // in DGE, DGI, DGY => J
                    // else => T
                    key.push_back((ch1 == 'G' && DM_is(ch2, DOUBLEMETAPHONE::F_VOW)) ? 'J' : 'T');
                    break;

                case 'G':
                    // in -GH and not B--GH, D--GH, -H--GH, -H---GH => F
                    // in -GNED, -GN, -DGE-, -DGI-, -DGY- => dropped
                    // in -GE-, -GI-, -GY- and not GG => J
                    // else => K
                    if((ch1 != 'G' || DM_is(ch2, DOUBLEMETAPHONE::VOWEL))
                       && (ch1 != 'N' || (i + 1 < word_size && (ch2 != 'E' || DM_at(word, i + 3) != 'D')))
                       && (chm1 != 'D' || !DM_is(ch1, DOUBLEMETAPHONE::F_VOW))) {
                        key.push_back((DM_is(ch1, DOUBLEMETAPHONE::F_VOW) && ch2 != 'G') ? 'J' : 'K');
                    } else if(ch1 == 'H' && !DM_is(DM_at(word, i - 3), DOUBLEMETAPHONE::NOGHF)
                              && DM_at(word, i - 4) != 'H') {
                        key.push_back('F');
                    }
                    break;

                case 'H':
                    // keep H before a vowel and not after CGPST (DOUBLEMETAPHONE::V_SND group)
                    if(!DM_is(chm1, DOUBLEMETAPHONE::V_SND)
                       && (!DM_is(chm1, DOUBLEMETAPHONE::VOWEL) || DM_is(ch1, DOUBLEMETAPHONE::VOWEL))) {
                        key.push_back('H');
                    }
                    break;

                case 'K':
                    // keep if not after C
                    if(chm1 != 'C') {
                        key.push_back('K');
                    }
                    break;

                case 'P':
                    // PH => F
                    key.push_back(ch1 == 'H' ? 'F' : 'P');
                    break;

                case 'Q':
                    key.push_back('K');
                    break;

                case 'S':
                    // SH, SIO, SIA => X
                    key.push_back((ch1 == 'H' || (ch1 == 'I' && (ch2 == 'O' || ch2 == 'A'))) ? 'X' : 'S');
                    break;

                case 'T':
                    // TIA, TIO => X
                    // TH => 0 (theta)
                    // TCH => dropped
                    if(ch1 == 'I' && (ch2 == 'O' || ch2 == 'A')) {
                        key.push_back('X');
                    } else if(ch1 == 'H') {
                        key.push_back('0');
                    } else if(ch1 != 'C' || ch2 != 'H') {
                        key.push_back('T');
                    }
                    break;

                case 'V':
                    key.push_back('F');
                    break;

                case 'W':
                case 'Y':
                    if(DM_is(ch1, DOUBLEMETAPHONE::VOWEL)) {
                        key.push_back(ch);
                    }
                    break;

                case 'X':
                    key.append(i > 0 ? "KS" : "S");
                    break;

                case 'Z':
                    key.push_back('S');
                    break;
            }
        }
    }
    return key;
}

// Initializor function.
void Explicator_Module_Double_Metaphone_Init(const std::map<std::string, std::string> &lexicon, float threshold) {
    DM_lexicon.clear();

    // Transform each (dirty) string in the lexicon into the double metaphone format.
    for(auto i = lexicon.begin(); i != lexicon.end(); ++i) {
        DM_lexicon.push_back(
            std::pair<std::string, std::string>(Double_Metaphone_To_Condensed_Phonetic(i->first), i->second));
    }
}

// Query function.
std::unique_ptr<std::map<std::string, float>>
Explicator_Module_Double_Metaphone_Query(const std::map<std::string, std::string> &lexicon,
                                         const std::string &in,
                                         float threshold) {
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());

    // If the threshold completely disallows (perfect) matches, then honor it by bailing gracefully.
    if(threshold > 1.0) {
        return output;
    }

    // Compute the double metaphone format of this string. Compare it to those previously computed.
    const std::string condensed(Double_Metaphone_To_Condensed_Phonetic(in));

    // Cycle over the condensed elements in the lexicon, looking for precise matches.
    for(auto it = DM_lexicon.begin(); it != DM_lexicon.end(); ++it) {
        if(it->first == condensed) {
            (*output)[it->second] = 1.0;
        }
    }

    return output;
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_Double_Metaphone_Deinit(void) {
    DM_lexicon.clear();
}
