// Explicator_Module_Exact.cc - DICOMautomaton, 2012.
//
// This file contains a simple module for the Explicator class: Given a string and the lexicon,
// it iterates through the lexicon looking for an exact match.
//
/////////////////////////////////////////////////////////////////////////////////////////////////////////
// NOTE: This module has been integrated directly into the Explicator class and should no longer be called
// as a module. This code is therefore strictly for illustrative/example purposes.
/////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <memory>
#include <string>
#include <utility>

// Initializor function.
void Explicator_Module_Exact_Init(const std::map<std::string, std::string> &lexicon, float threshold) {
    // Do nothing.
}

// Query function.
std::unique_ptr<std::map<std::string, float> >
Explicator_Module_Exact_Query(const std::map<std::string, std::string> &lexicon,
                              const std::string &in,
                              float threshold) {
    std::unique_ptr<std::map<std::string, float> > output(new std::map<std::string, float>());

    // We simply cycle through and compare the string to each (dirty) element in the lexicon.
    // The lexicon looks like: < dirty : clean >
    auto it = lexicon.find(in);
    if(it != lexicon.end()) {
        (*output)[it->second] = 1.0;
    }
    return output;
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_Exact_Deinit(void) {
    // Do nothing.
}
