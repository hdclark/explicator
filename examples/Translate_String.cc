// Translate_String.cc.

// This example shows how to translate a user-provided string using a user-provided lexicon. It will be slow to call
// this routine repeatedly compared with using the library interface directly.

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include "Explicator.h"

int main(int argc, char **argv) {
    if(argc != 3) {
        throw std::runtime_error("Please provide a lexicon filename and string to translate.");
    }
    const std::string filename(argv[1]);
    const std::string dirty(argv[2]);

    Explicator X(filename);

    X.suspected_mistranslation = "<no match above the thresholds were found>";

    // Print the suspected translation to stdout.
    std::cout << X(dirty) << std::endl;

    // Print some info to stderr.
    std::cerr << "Best Score = " << X.Get_Last_Best_Score() << std::endl;

    // Print some more info to stderr about all strings that were considered. Note that if an exact match is found, no
    // other module is consulted.
    auto more_results = X.Get_Last_Results();
    for(const auto &aresult : *more_results) {
        std::cerr << "\t Possibility '" << aresult.first << "' scored " << aresult.second << std::endl;
    }

    return 0;
}
