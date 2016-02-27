// Explicator.h - DICOMautomaton, 2012.

#pragma once

// This file is a rewrite of the experimental Hashfilter.h file written in 2011. It removes the use of the string
// wrappers (because they presented an unnecessary overhead in development) and introduces a more modular approach. The
// scope of this file has been reduced somewhat - all 'tough' processing has been offloaded into external modules. This
// file simply parses a lexicon file and provides a base for the modules.

#include <stdint.h>
#include <list>
#include <map>
#include <memory>
#include <string>
#include <tuple>

// These are the functions (signatures) each module must contain. The initialization function, which is called when the
// module is dynamically loaded OR upon creation of a explicator instance.

typedef void (*explicator_module_func_init)(const std::map<std::string, std::string> &, float);

// The query routine, which is called to act on a string (to translate it.) We get a list of the best matches and their
// ([0:1] clamped) score.

typedef std::unique_ptr<std::map<std::string, float>> (*explicator_module_func_query)(
    const std::map<std::string, std::string> &, const std::string &, float);

// The de-initialization routine. Used for typical destructor tasks.
typedef void (*explicator_module_func_deinit)(void);

namespace Ex_Mods {
    // Custom signals.
    const uint64_t None  = 1 << 1; // Signals an error or indicates a problem.
    const uint64_t Exact = 1 << 2;

    // Edit measures.
    const uint64_t Levenshtein = 1 << 3;
    const uint64_t Cosine      = 1 << 4; // Removed!
    const uint64_t JaroWinkler = 1 << 5;

    // Phonetic measures.
    const uint64_t Soundex       = 1 << 6;
    const uint64_t Dbl_Metaphone = 1 << 7;
    const uint64_t MRA           = 1 << 8;

    // Statistical measures.
    const uint64_t NGrams       = 1 << 9;
    const uint64_t Bag_of_Chars = 1 << 10; // Removed!
    const uint64_t Subsequence  = 1 << 11;
    const uint64_t Emplacement  = 1 << 12;

    // Custom/domain-specific measures.
    const uint64_t DICOM_Hash   = 1 << 13;
    const uint64_t DS_Head_Neck = 1 << 14;

    // Experimental.
    const uint64_t ANN        = 1 << 15; // Removed!
    const uint64_t Substrings = 1 << 16;

    //----------------------

    // Lower bound for any dynamically-loaded modules.
    const uint64_t Dyn_Load_Min = 1 << 17;

    // Default to a good general set.
    const uint64_t Sane_Defaults = Substrings | JaroWinkler | Levenshtein;
}

class Explicator {
  public:
    // The lexicon filename which was used as the dictionary.
    std::string filename;

    // A collection of the raw entries in the lexicon: <dirty:clean>.
    std::map<std::string, std::string> lexicon;
    std::list<std::tuple<explicator_module_func_init,   // Init function. Allocate memory and precompute, if req'd.
                         explicator_module_func_query,  // Query function. Perform translation of given string.
                         explicator_module_func_deinit, // Deinit function. Deallocate memory and clear storage.
                         float,           // Specific module's threshold. Higher -> computation speed/mem drops.
                         uint64_t,        // Module ID. Useful for keeping track of module thresholds.
                         float>> modules; // Importance weighting. Normalized internally. Useful for optimization.

    // Bitwise OR with Ex_Mods::... to specify which modules should be used. Default is a subset.
    uint64_t modmask;

    // The total accumulated scores from the most recent query. Passed out to user on request.
    std::unique_ptr<std::map<std::string, float>> last_results;
    float last_best_score;
    uint64_t last_best_module;

    // The string output to the user to signify a suspected mistranslation (due to score below threshold, etc..) It is
    // not fixed and may be automatically regenerated so as to not match any lexicon entries.
    std::string suspected_mistranslation;

    // Global threshold. It does not apply to individual modules, but rather the total (final) output score. This
    // threshold should be somewhat higher than the individual module thresholds on average.
    float group_threshold;

    //------- Constructors/Destructor --------
    Explicator(const std::string &file_name);
    Explicator(const std::string &file_name, uint64_t modulemask);
    Explicator(const std::map<std::string, std::string> &new_lexicon);
    Explicator(const std::map<std::string, std::string> &new_lexicon,
               uint64_t modulemask,
               std::map<uint64_t, float> mod_wghts  = {},
               std::map<uint64_t, float> mod_tholds = {});
    ~Explicator();

    //------- Member functions --------
    void ResetDefaults(void); // Resets non-module things to default values.
    void ReReadFile(void);    // Discards the lexicon and reloads from original filename.
    void ReInitModules(std::map<uint64_t, float> mod_wghts = {},
                       std::map<uint64_t, float> mod_tholds
                       = {}); // Deinits existing modules, inits new modules according to the modmask.

    // This is the most important function for the user. Perform translation of given string.
    std::string operator()(const std::string &);

    // Retrieval of info from most recent translation.
    std::unique_ptr<std::map<std::string, float>> Get_Last_Results(void); // Can only be called once per query!
    float Get_Last_Best_Score(void) const;
    uint64_t Get_Last_Best_Module(void) const;

    //------- Measurement routines --------
    // Perform folding cross-validation. Returns frac of correct translations, maximum theoretical frac of correct, frac
    // of false negs, frac of false pos. Also used internally for optimization.
    std::tuple<float, float, float, float> Cross_Verify(float chunks,
                                                        long int runs,
                                                        bool verbose_dump,
                                                        std::map<uint64_t, float> mod_wghts  = {},
                                                        std::map<uint64_t, float> mod_tholds = {}) const;

    std::map<uint64_t, float> Get_Module_Thresholds(void) const;
    std::map<uint64_t, float> Get_Module_Weights(void) const;

    void Dump_Translated_Lexicon(
        void); // Run through lexicon to see if it will properly translate itself. Useful for module checking.
};
