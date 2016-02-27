// Lexicon_Dogfooder.cc.

// This example loads a user-specified lexicon and uses it to translate itself.
// It can be used to detect modules which do not correctly map from A-->A (all
// modules *should* ideally do so).

#include <stdexcept>
//#include <iostream>
#include <string>
//#include <memory>
//#include <iomanip>
//#include <set>
//#include <tuple>

#include "Explicator.h"

int main(int argc, char **argv) {
    if(argc != 2) {
        throw std::runtime_error("Please provide a lexicon filename.");
    }
    const std::string filename(argv[1]);

    Explicator X(filename);

    X.Dump_Translated_Lexicon();

    return 0;
}
