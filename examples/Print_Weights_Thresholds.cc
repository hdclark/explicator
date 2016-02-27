// Print_Weights_Thresholds.cc.

// This example shows how to access the various thresholds and module weights.

#include <stdint.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>

#include "Explicator.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        throw std::runtime_error("Please provide a lexicon filename.");
    }
    const std::string filename(argv[1]);

    Explicator X(filename, Ex_Mods::Exact | Ex_Mods::Levenshtein | Ex_Mods::JaroWinkler | Ex_Mods::Soundex
                               | Ex_Mods::Dbl_Metaphone | Ex_Mods::MRA | Ex_Mods::NGrams | Ex_Mods::Subsequence
                               | Ex_Mods::Emplacement);

    // Print the current (default) module thresholds. Each module maintains a minimum score that a suspected translation
    // must attain before it is considered legitmate. (This is how false positives are controlled.) If a suspected
    // translation attains this threshold score, it is passed on to the final round to compete against suspected
    // translations from other modules.
    std::cout << "Default per-module thresholds:" << std::endl;
    auto mod_thresholds = X.Get_Module_Thresholds();
    for(const auto &apair : mod_thresholds) {
        std::cout << "\t"
                  << "Module #" << apair.first << " threshold = " << apair.second << std::endl;
    }
    std::cout << std::endl;

    // Print the current (default) group threshold. Suspected translations passed up from the modules must have a better
    // score than this number in order to be recognized as a legitimate translation. If this number is not exceeded by
    // any suspected translation, no suspected translation is provided to the user (only the 'unknown' string as defined
    // in Explicator::suspected_mistranslation).
    std::cout << "Default group threshold:" << std::endl;
    std::cout << "\t" << X.group_threshold << std::endl;
    std::cout << std::endl;

    // Print the current (default) module weights. These weights can be set if you have reason to believe one module
    // should be more relevant than the others. There is no way to know, in general, how to weight various modules. An
    // attempt was made to optimize (at runtime) the weights to reduce false positives. But it only worked for entries
    // already within the lexicon -- obviously it had no knowledge of previously-unseen strings. Patches welcome if you
    // can think of a good optimization scheme!
    std::cout << "Default per-module weights:" << std::endl;
    auto mod_weights = X.Get_Module_Weights();
    for(const auto &apair : mod_weights) {
        std::cout << "\t"
                  << "Module #" << apair.first << " weight = " << apair.second << std::endl;
    }
    std::cout << std::endl;

    // Modify the various weights and thresholds.

    // Be very selective. Let only the most certain suspected translations through.
    X.group_threshold = 0.7;
    for(auto &apair : mod_thresholds) { apair.second = 0.6; }
    X.ReInitModules(mod_weights, mod_thresholds);

    // Be very lenient. Let pretty much anything through.
    X.group_threshold = 0.2;
    for(auto &apair : mod_thresholds) { apair.second = 0.15; }
    X.ReInitModules(mod_weights, mod_thresholds);

    // Strongly prefer two specific modules.
    for(auto &apair : mod_weights) {
        if(apair.first == Ex_Mods::Levenshtein) {
            apair.second *= 3.0;
        } else if(apair.first == Ex_Mods::Subsequence) {
            apair.second *= 2.5;
        } else {
            apair.second *= 0.5;
        }
    }
    X.ReInitModules(mod_weights, mod_thresholds);

    // std::string clean = X("blah");

    return 0;
}
