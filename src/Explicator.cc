// Explicator.cc - DICOMautomaton, 2012.
//

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <iterator> //Needed for std::advance. (Is it needed?)
#include <limits>
#include <list>
#include <map>
#include <memory>
#include <random> //Needed in Cross_Check member function.
#include <regex>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "Explicator.h"
#include "Files.h"  //Needed for Does_File_Exist_And_Can_Be_Read(...)
#include "Misc.h"   //Needed for FUNCINFO(), FUNCERR(), FUNCWARN() macros.
#include "String.h" //Needed for Canonicalization().

const std::regex
    regex_first_non_whitespace_is_pound(R"***(^[[:blank:]]*#)***", std::regex::ECMAScript | std::regex::icase);
const std::regex regex_contains_a_colon(R"***(:)***", std::regex::basic | std::regex::icase);

#include "Explicator_Module_DICOM_Hash.h"
#include "Explicator_Module_DS_Head_and_Neck.h"
#include "Explicator_Module_Double_Metaphone.h"
#include "Explicator_Module_Emplacement.h"
#include "Explicator_Module_JaroWinkler.h"
#include "Explicator_Module_Levenshtein.h"
#include "Explicator_Module_MRA.h"
#include "Explicator_Module_NGrams.h"
#include "Explicator_Module_Soundex.h"
#include "Explicator_Module_Subsequence.h"
#include "Explicator_Module_Substrings.h"

// Constructors.
Explicator::Explicator(const std::string &file_name) : filename(file_name) {
    // Check if the file can be opened/read.
    if(!Does_File_Exist_And_Can_Be_Read(this->filename))
        FUNCERR("Input lexicon '" << this->filename << "' could not be read");
    this->ResetDefaults();
    this->ReReadFile();
    this->ReInitModules();
}

Explicator::Explicator(const std::string &file_name, uint64_t modulemask) : filename(file_name) {
    // Check if the file can be opened/read.
    if(!Does_File_Exist_And_Can_Be_Read(this->filename))
        FUNCERR("Input lexicon '" << this->filename << "' could not be read");
    this->ResetDefaults();
    this->ReReadFile();
    this->modmask = modulemask;
    this->ReInitModules();
}

Explicator::Explicator(const std::map<std::string, std::string> &new_lexicon) {
    // Canoninicalize the data. Chomp off preceeding/trailing whitespace. Shorten ' '{2,} to single space.
    this->lexicon.clear();
    for(auto it = new_lexicon.begin(); it != new_lexicon.end(); ++it) {
        const auto dirty     = Canonicalize_String2(it->first, CANONICALIZE::TRIM | CANONICALIZE::TO_UPPER);
        const auto clean     = Canonicalize_String2(it->second, CANONICALIZE::TRIM);
        this->lexicon[dirty] = clean;
    }
    this->ResetDefaults();
    this->ReInitModules();
}

Explicator::Explicator(const std::map<std::string, std::string> &new_lexicon,
                       uint64_t modulemask,
                       std::map<uint64_t, float> mod_wghts,
                       std::map<uint64_t, float> mod_tholds) {
    // Canoninicalize the data. Chomp off preceeding/trailing whitespace. Shorten ' '{2,} to single space.
    this->lexicon.clear();
    for(auto it = new_lexicon.begin(); it != new_lexicon.end(); ++it) {
        const auto dirty     = Canonicalize_String2(it->first, CANONICALIZE::TRIM | CANONICALIZE::TO_UPPER);
        const auto clean     = Canonicalize_String2(it->second, CANONICALIZE::TRIM);
        this->lexicon[dirty] = clean;
    }
    this->ResetDefaults();
    this->modmask = modulemask;
    this->ReInitModules(mod_wghts, mod_tholds);
}

// Destructors.
Explicator::~Explicator() {
    // De-initialize all the modules we've got.
    for(auto it = modules.begin(); it != modules.end(); ++it) { (std::get<2>(*it))(); }
}

// Member functions.
void Explicator::ResetDefaults(void) { // Resets everything except filename and lexicon.
    this->modmask          = Ex_Mods::Sane_Defaults;
    this->last_best_score  = -1.0;
    this->last_best_module = Ex_Mods::None;
    this->group_threshold = 0.45;
    this->last_results.reset(new std::map<std::string, float>()); // Allocate space for the last_results.

    // Ensure the 'no reasonable match' string does not collide with any of the cleans in the lexicon.
    this->suspected_mistranslation = "";
    bool nocollision = false;
    while(!nocollision) {
        nocollision = true;
        for(auto it = this->lexicon.begin(); it != this->lexicon.end(); ++it) {
            if(it->second == this->suspected_mistranslation) {
                FUNCWARN(
                    "The lexicon contains a 'clean' string which collides with the string used to signal a suspected "
                    "mistranslation: '"
                    << suspected_mistranslation << "'");
                this->suspected_mistranslation += "_";
                FUNCWARN(" Altering the signalling string to '" << suspected_mistranslation
                                                                << "' to avoid the collision. Beware of the change!");
                nocollision = false;
                break;
            }
        }
    }

    // NOTE: We cannot reset the module thresholds because the module_init function may depend on the threshold value.
    // Do not attempt a work-around, because module initialization depends on this behaviour.
    return;
}

void Explicator::ReReadFile(void) {
    // File syntax is: " clean string(s) : dirty string(s) "
    // The purpose of this function is to read a '.lexicon' or '.lex' file to fill the this->lexicon map.

    // Check if the file can be opened/read.
    if(!Does_File_Exist_And_Can_Be_Read(filename))
        FUNCERR("Input lexicon '" << this->filename << "' could not be read during reloading");
    std::fstream FI(this->filename.c_str(), std::ifstream::in);

    // Clear the current lexicon and populate the new one.
    this->lexicon.clear();
    std::string rawline, dirty, clean; // Each line and Parts of the line (A...):(B...)
    while(!((getline(FI, rawline)).eof())) {
        if(std::regex_search(rawline, regex_first_non_whitespace_is_pound)) {
            continue; // If the first non-whitespace character is #, then ignore.
        }
        if(!std::regex_search(rawline, regex_contains_a_colon)) {
            continue; // If the line does not contain :, then ignore.
        }

        // Iterate over the two pieces A... and B... from A... : B...
        clean.clear();
        dirty.clear();
        const std::sregex_token_iterator end;
        std::sregex_token_iterator iter(rawline.begin(), rawline.end(), regex_contains_a_colon,
                                        -1); //-1 implies we tokenize the parts that do not match.

        // It wouldn't be good to try iterate over the two parts if they do not exist, so we iterate carefully.
        if(iter != end) {
            clean = (*iter);
            if(++iter != end) {
                dirty = (*iter);
            }
        }

        // Chomp off extra whitespace (all from front, all from back, shorten whitespace within to a single space.)
        dirty = Canonicalize_String2(dirty, CANONICALIZE::TRIM | CANONICALIZE::TO_UPPER);
        clean = Canonicalize_String2(clean, CANONICALIZE::TRIM); // Keys can have anything, so only trim excess space.

        // Push back the strings if they are valid (and sane...)
        if(!clean.empty() && !dirty.empty()) {
            lexicon[dirty] = clean;
        }
    }
    return;
}

void Explicator::ReInitModules(std::map<uint64_t, float> mod_wghts, std::map<uint64_t, float> mod_tholds) {
    // De-init modules which are currently loaded (even statically) Purge them after de-init.
    for(auto it = modules.begin(); it != modules.end(); ++it) { (std::get<2>(*it))(); }
    modules.clear();

    const float auto_thold = -1.0; // Used to indicate that we should use the default (automatically handle it).
    const float auto_wght  = -1.0; // Alternatively, we can override with a totally-fixed value by hardcoding it.

    // Push back the compiled-in module functions. Set the module's threshold to -1.0 to follow group adjustment.
    // Otherwise it will be fixed to whatever you specify.

    // NOTE: [No modules] is valid. It means the user wants a strictly exact-match setup!
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::Levenshtein)) {
        modules.push_back(std::make_tuple(Explicator_Module_Levenshtein_Init, Explicator_Module_Levenshtein_Query,
                                          Explicator_Module_Levenshtein_Deinit, auto_thold, Ex_Mods::Levenshtein,
                                          auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::DICOM_Hash)) {
        modules.push_back(std::make_tuple(Explicator_Module_DICOM_Hash_Init, Explicator_Module_DICOM_Hash_Query,
                                          Explicator_Module_DICOM_Hash_Deinit, auto_thold, Ex_Mods::DICOM_Hash,
                                          auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::Emplacement)) {
        modules.push_back(std::make_tuple(Explicator_Module_Emplacement_Init, Explicator_Module_Emplacement_Query,
                                          Explicator_Module_Emplacement_Deinit, auto_thold, Ex_Mods::Emplacement,
                                          auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::NGrams)) {
        modules.push_back(std::make_tuple(Explicator_Module_NGrams_Init, Explicator_Module_NGrams_Query,
                                          Explicator_Module_NGrams_Deinit, auto_thold, Ex_Mods::NGrams, auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::Soundex)) {
        modules.push_back(std::make_tuple(Explicator_Module_Soundex_Init, Explicator_Module_Soundex_Query,
                                          Explicator_Module_Soundex_Deinit, auto_thold, Ex_Mods::Soundex, auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::MRA)) {
        modules.push_back(std::make_tuple(Explicator_Module_MRA_Init, Explicator_Module_MRA_Query,
                                          Explicator_Module_MRA_Deinit, auto_thold, Ex_Mods::MRA, auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::Dbl_Metaphone)) {
        modules.push_back(
            std::make_tuple(Explicator_Module_Double_Metaphone_Init, Explicator_Module_Double_Metaphone_Query,
                            Explicator_Module_Double_Metaphone_Deinit, auto_thold, Ex_Mods::Dbl_Metaphone, auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::DS_Head_Neck)) {
        modules.push_back(
            std::make_tuple(Explicator_Module_DS_Head_and_Neck_Init, Explicator_Module_DS_Head_and_Neck_Query,
                            Explicator_Module_DS_Head_and_Neck_Deinit, auto_thold, Ex_Mods::DS_Head_Neck, auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::Subsequence)) {
        modules.push_back(std::make_tuple(Explicator_Module_Subsequence_Init, Explicator_Module_Subsequence_Query,
                                          Explicator_Module_Subsequence_Deinit, auto_thold, Ex_Mods::Subsequence,
                                          auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::JaroWinkler)) {
        modules.push_back(std::make_tuple(Explicator_Module_JaroWinkler_Init, Explicator_Module_JaroWinkler_Query,
                                          Explicator_Module_JaroWinkler_Deinit, auto_thold, Ex_Mods::JaroWinkler,
                                          auto_wght));
    }
    if(BITMASK_BITS_ARE_SET(this->modmask, Ex_Mods::Substrings)) {
        modules.push_back(std::make_tuple(Explicator_Module_Substrings_Init, Explicator_Module_Substrings_Query,
                                          Explicator_Module_Substrings_Deinit, auto_thold, Ex_Mods::Substrings,
                                          auto_wght));
    }

    // Load dynamic modules here. (None at the moment.)

    // Set any custom module thresholds which have been passed in.
    for(auto it = modules.begin(); it != modules.end(); ++it) {
        const auto mod_id = std::get<4>(*it);
        const bool ispres = (mod_tholds.find(mod_id) != mod_tholds.end());
        if(ispres) {
            std::get<3>(*it) = mod_tholds[mod_id];
        }
    }

    // Set the default per-module threshold. This threshold is used to limit the amount of data sent back for
    // combination. Do not set it too low - it will interfere with the post-combination threshold. Use 0.3 when you know
    // ~15% of the total lexicon. Use 0.5 when you know ~50% of the total lexicon. Use 0.7 when you know ~80% of the
    // total lexicon. Setting too low produces a lot of noise. Setting too high will cause valid matches to be ignored.
    // Err on the side of caution and use the lowest threshold you can handle.
    const float def_thold = 0.3;
    for(auto it = modules.begin(); it != modules.end(); ++it) {
        if(std::get<3>(*it) == auto_thold) {
            std::get<3>(*it) = def_thold;
        }
    }

    // Set any custom module weights which have been passed in.
    for(auto it = modules.begin(); it != modules.end(); ++it) {
        const auto mod_id = std::get<4>(*it);
        const bool ispres = (mod_wghts.find(mod_id) != mod_wghts.end());
        if(ispres) {
            std::get<5>(*it) = mod_wghts[mod_id];
        }
    }

    // Set the default weighting factor. No need to normalize because it is done at each lookup.
    const float def_wght = 1.0;
    for(auto it = modules.begin(); it != modules.end(); ++it) {
        if(std::get<5>(*it) == auto_wght) {
            std::get<5>(*it) = def_wght;
        }
    }

    // Init all modules. A threadpool was originally used to speed this, but was more hassle than it was worth.
    for(auto it = modules.begin(); it != modules.end(); ++it) { (std::get<0>(*it))(lexicon, std::get<3>(*it)); }

    return;
}

std::string Explicator::operator()(const std::string &dirty) {
    if(this->lexicon.empty())
        FUNCERR("Attempted to perform matching with an empty lexicon!");
    if(this->last_results == nullptr)
        FUNCERR("this->last_results was a nullptr. Unable to continue");

    this->last_results->clear();
    this->last_best_score  = -1.0;
    this->last_best_module = Ex_Mods::None;

    std::string clean(this->suspected_mistranslation); // This is the string we will return.
    const std::string dirty_chomped = Canonicalize_String2(dirty, CANONICALIZE::TRIM | CANONICALIZE::TO_UPPER);

    // Check if there is an exact match. If there is, we can skip evaluating any modules.
    {
        auto it = this->lexicon.find(dirty_chomped);
        if(it != this->lexicon.end()) {
            (*(this->last_results))[it->second] = 1.0;
            this->last_best_score  = 1.0;
            this->last_best_module = Ex_Mods::Exact;
            return it->second;
        }
    }
    if(this->modules.empty()) {
        return this->suspected_mistranslation;
    }

    // Cycle through all the modules. Push the results back into the vector.
    float tot_wght(0.0);
    std::vector<std::pair<std::unique_ptr<std::map<std::string, float>>, float>> result_vector;

    for(auto it = this->modules.begin(); it != this->modules.end(); ++it) {
        const auto thethold = std::get<3>(*it);
        const auto thewght  = std::get<5>(*it);
        const auto f_query  = std::get<1>(*it);

        auto themap = f_query(this->lexicon, dirty_chomped, thethold);
        result_vector.push_back(std::make_pair(std::move(themap), thewght));
        tot_wght += thewght;
    }

    // Normalize the weighting in the output vector.
    if(tot_wght <= 0.0) {
        // FUNCWARN("No plausible output. Consider increasing the threshold");
        return this->suspected_mistranslation;
    }
    for(auto v_it = result_vector.begin(); v_it != result_vector.end(); ++v_it) { v_it->second /= tot_wght; }

    // I've tried a variety of methods to combine the results, including (1) picking out the highest-scoring result; (2)
    // multiplying all results for a given clean string (i.e., excessively penalizing non-matches); (3) arithmetical
    // averaging of the (clamped) result scores; (4) normalizing the results per-occurence. They did not fare well.

    // Cycle through the results and sum all the weighted result scores. No need to scale by # of modules.
    for(auto v_it = result_vector.begin(); v_it != result_vector.end(); ++v_it) {
        const auto wght = v_it->second;
        for(auto s_it = v_it->first->begin(); s_it != v_it->first->end(); ++s_it) {
            const auto score = wght * s_it->second;
            (*(this->last_results))[s_it->first] += score;
        }
    }

    for(auto it = this->last_results->begin(); it != this->last_results->end(); ++it) {
        const auto final_score = it->second;
        if(this->last_best_score < final_score) {
            this->last_best_score = final_score;
        }
    }

    // Verify that there is at least one plausible output. It is not an error to have none, but it may indicate that the
    // user has set unreasonable thresholds.
    if(this->last_results->empty()) {
        // FUNCWARN("No plausible output. Consider increasing the threshold");
        return this->suspected_mistranslation;
    }

    // Find the highest score and associated suspected clean string.           ----ISN'T THIS WHAT 'last_best_score' is?
    float max_score = -std::numeric_limits<float>::infinity();
    for(auto it = this->last_results->begin(); it != this->last_results->end(); ++it) {
        if(it->second > max_score) {
            max_score = it->second;
            clean     = it->first;
        }
    }

    // If the highest-scoring score was not above the threshold, we indicate that we have no prediction.
    // NOTE: We do not use '<=' in case the user genuinely wants no threshold.
    if(max_score < this->group_threshold) {
        return this->suspected_mistranslation;
    }
    return clean;
}

std::unique_ptr<std::map<std::string, float>> Explicator::Get_Last_Results(void) {
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());
    last_results.swap(output); // Transfer last_results ownership, leaving an empty pointer in-place.
    return output;
}

float Explicator::Get_Last_Best_Score(void) const {
    return this->last_best_score;
}

uint64_t Explicator::Get_Last_Best_Module(void) const {
    return this->last_best_module;
}

// Breaks the input file into (random) chunks, handles one chunk at a time, and attempts to translate the other chunks.
// Instead of returning the fraction of successful translations, we return:
//
//  - the frac of translations we correctly judged to be valid.
//
//  - the theo max frac of translations we could possibly judge correctly (after inf measurements).
//
//  - the frac of translations incorrectly judged to be invalid (when they were valid - false negatives)
//
//  - the frac of translations incorrectly judged to be valid (when they were invalid - false positives)
//
// "chunks" is a [0-1]-clamped percentage of the lexicon to keep.
//
// "runs" is a factor for the number of back-to-back tests to run. Increasing "runs" above 1 should only result in
// better statistics and a longer run time.
//
// "mod_tholds" is an (optional) set of thresholds for specific modules. This is mostly used for optimization.
std::tuple<float, float, float, float> Explicator::Cross_Verify(float chunks,
                                                                long int runs,
                                                                bool verbose_dump,
                                                                std::map<uint64_t, float> mod_wghts,
                                                                std::map<uint64_t, float> mod_tholds) const {
    if(!isininc(0.001, chunks, 1.0) || (runs <= 0)) {
        FUNCWARN("Invalid input. chunks = " << chunks << " and runs = " << runs << ". Bailing");
        return std::make_tuple(-1.0, -1.0, -1.0, -1.0);
        // NOTE: This function takes a [0-1]-clamped float as the ratio of elements (per total) to use in
        // a chunk and a numb-of-times-to-loop factor.
    }
    const long int per_chunk        = static_cast<const long int>(chunks * static_cast<float>(this->lexicon.size()));
    const long int number_of_chunks = runs * static_cast<const long int>(1.0 / chunks);
    if(per_chunk < 1) {
        FUNCWARN("Invalid input. Req frac " << chunks << " dirties per fold produces " << per_chunk
                                            << " elements in the new lexicons! Bailing");
        return std::make_tuple(-1.0, -1.0, -1.0, -1.0);
    }

    //------
    // Determine the theoretical maximum (fraction of correct translations) possible. This is due to the possibility of
    // producing a lexicon which does not contain any of a given clean. In other words, if the trial lexicon does not
    // contain 'Left Parotid' clean, we have a zero chance of  translating any dirties to 'Left Parotid'. The
    // theoretical maximum tries to account for these loses in the asymptotic case of infinite measurements. In other
    // words, unless we cheat, a similarity measure/module should not be able to beat this value!

    // NOTE: This theoretical limit assumes that we only need 1 dirty to properly match all of a clean families' dirties
    // to the clean family. This is, in general, not true. This means that given a single JUNK dirty, we *should* be
    // able to properly guess that ALL other JUNK dirties correspond to the JUNK clean. Clearly this is not the case!
    // Take this upper limit with a  grain of salt.
    float theo_best(0.0);
    {
        std::map<std::string, float> Nofdirties; // Number of dirties for each clean.
        for(auto it = this->lexicon.begin(); it != this->lexicon.end(); ++it) {
            const std::string clean(it->second);
            Nofdirties[clean] += 1.0;
        }
        const auto Ntotal                  = static_cast<float>(this->lexicon.size());
        const auto new_lex_size            = static_cast<float>(per_chunk);
        const auto Prob_of_missing_clean_i = [=](float N, float Ndirt_i, float n) -> float {
            if(!isininc(0, n, (N - Ndirt_i))) {
                return 0.0;
            }
            return std::exp(std::lgamma(N - Ndirt_i + 1.0) - std::lgamma(N - Ndirt_i + 1.0 - n)
                            + std::lgamma(N + 1.0 - n) - std::lgamma(N + 1.0));
        };
        for(auto it = Nofdirties.begin(); it != Nofdirties.end(); ++it) {
            theo_best += (it->second / Ntotal) * Prob_of_missing_clean_i(Ntotal, it->second, new_lex_size);
        }
        theo_best = 1.0 - theo_best;
    }
    //------

    std::random_device rd;
    std::mt19937 gen(rd());

    long int number_correct = 0, number_false_neg = 0, number_false_pos = 0;
    auto ltcomp = [](const std::pair<std::string, float> &A, const std::pair<std::string, float> &B) -> bool {
        return A.second < B.second;
    };

    long int TOT(0);
    for(long int i = 0; i < number_of_chunks; ++i) {
        // Duplicate the lexicon. Remove elements at random until the selected amount remains.
        decltype(this->lexicon) sub_lexicon(this->lexicon);
        while(static_cast<long int>(sub_lexicon.size()) != per_chunk) {
            // This is constructed here because the easier-to-reason alternative of using a modulus will be biased to
            // the lower numbers!
            std::uniform_int_distribution<> dis(0, sub_lexicon.size()
                                                       - 1); // Inclusive endpoints. Now generate with "dis(gen)".
            auto iter = sub_lexicon.begin();
            std::advance(iter, dis(gen));
            sub_lexicon.erase(iter);
        }

        // Now create a new Explicator instance with this data. Duplicate everything except the lexicon. The module
        // thresholds will not be duplicated - pass on the thresholds passed in.
        Explicator scant(sub_lexicon, this->modmask, mod_wghts, mod_tholds);
        scant.group_threshold = this->group_threshold;

        // Now cycle through every element in the (complete) lexicon. Ask the spawned explicator to translate the entry.
        // Compare whether or not it is correct.
        for(auto it = this->lexicon.begin(); it != this->lexicon.end(); ++it) {
            const std::string dirty(it->first);
            const std::string clean(it->second);
            const std::string output(scant(dirty));

            ++TOT;

            if(output == clean) {
                if(verbose_dump)
                    FUNCINFO("Correctly translated (dirty) '" << dirty << "' to (clean) '" << clean << "'");
                ++number_correct;
                continue;

                // If the translation was not successful, we try figure out which type of error it was.
            } else {
                // We have four possible cases here.

                // 1) False negatives (type II errors). We can deal with these, but they are still annoying. We do not
                // produce an answer because it is below-threshold and yet our top-scoring string was actually correct.
                // We have missed out doing a legitimate prediction because of our ~statistical uncertainty of the
                // result.

                // 2) False positives (type I errors). These are very dangerous and hard to recognize, rectify. We
                // produce an answer which is not correct, but we believe it is correct. This case doesn't care about
                // what the top-scoring guess was. These are bad, and we should try to minimize them.

                // 3) Legitimately absent. These occur when input which is not logically a part of the lexicon is tested
                // and no suitable translation was found. This is considered a successful translation! It is not
                // possible with the given setup. In fact, without splitting the lexicon into three parts, I'm not sure
                // how to test this. Just randomly generating input strings simply won't work. We would likely have to
                // produce a separate lexicon specifically for this purpose. Either way, it doesn't affect my use case
                // too much. Further, I think we can estimate this case using false pos/negs and the number of corrects.

                // 4) Successful block of poor choice. This occurs when we are able to correctly detect that the best
                // guess is not correct. It does not count as a correct guess because it doesn't produce the correct
                // output. However, it does not count as an error because we have successfully indicated to the human
                // that there was a mistranslation. The careful human should catch this.
                if(verbose_dump)
                    FUNCINFO("Incorrectly translated (dirty) '" << dirty << "' to (clean) '" << output
                                                                << "'. Is actually '" << clean << "'");

                const auto results = scant.Get_Last_Results();

                if(results->empty()) { // This is type II error.
                    ++number_false_neg;
                    continue;
                }
                const auto bestguess_pair = *(std::max_element(results->begin(), results->end(), ltcomp));
                const auto bestguess_str  = bestguess_pair.first;

                if((bestguess_str == clean) && (output == this->suspected_mistranslation)) {
                    // The best guess was correct, but it was below threshold or otherwise blocked. Case 1 - Type II.
                    ++number_false_neg;
                    continue;
                }
                if(output != this->suspected_mistranslation) {
                    // We have provided a guess we are confident about, but it is not correct. Case 2 - Type I.
                    ++number_false_pos;
                    continue;
                }

                if((bestguess_str != clean) && (output == this->suspected_mistranslation)) {
                    // We have sucessfully caught a bad answer and have indicated it to the human. Case 4. This is
                    // certainly not an error, but is closer to a correct guess than anything...
                    continue;
                }

                FUNCINFO("dirty='" << dirty << "' clean='" << clean << "' suspected_mistranslation='"
                                   << suspected_mistranslation << "' output='" << output << "' bestguess_str='"
                                   << bestguess_str << "'");
                FUNCERR("Should not have been possible to get here. A programming error has occured");
            }
        }
    }
    const float frac_correct   = static_cast<float>(number_correct) / static_cast<float>(TOT);
    const float frac_false_neg = static_cast<float>(number_false_neg) / static_cast<float>(TOT);
    const float frac_false_pos = static_cast<float>(number_false_pos) / static_cast<float>(TOT);
    return std::make_tuple(frac_correct, theo_best, frac_false_neg, frac_false_pos);
}

std::map<uint64_t, float> Explicator::Get_Module_Thresholds(void) const {
    std::map<uint64_t, float> out;
    for(auto it = this->modules.begin(); it != this->modules.end(); ++it) {
        const auto mod_id    = std::get<4>(*it);
        const auto mod_thold = std::get<3>(*it);
        out[mod_id]          = mod_thold;
    }
    return out;
}

std::map<uint64_t, float> Explicator::Get_Module_Weights(void) const {
    std::map<uint64_t, float> out;
    for(auto it = this->modules.begin(); it != this->modules.end(); ++it) {
        const auto mod_id   = std::get<4>(*it);
        const auto mod_wght = std::get<5>(*it);
        out[mod_id]         = mod_wght;
    }
    return out;
}

// Run through lexicon to see if it will properly translate itself. Useful for module checking.
void Explicator::Dump_Translated_Lexicon(void) {
    for(auto it = this->lexicon.begin(); it != this->lexicon.end(); ++it) {
        const auto dirty  = it->first;
        const auto clean  = it->second;
        const auto suspc  = (*this)(dirty);
        const auto worked = (suspc == clean);

        std::cout << "Dirty: '" << std::setw(30) << dirty << "' ---> Clean: '" << std::setw(30) << suspc
                  << "'. Actual: '" << std::setw(30) << clean << "'. Success: " << !!worked << std::endl;
    }
    return;
}
