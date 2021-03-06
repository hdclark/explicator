// Explicator_Module_DS_Head_and_Neck.h - DICOMautomaton, 2012.
//
// This file contains a simple module for the Explicator class: Given a string and the lexicon,
// it iterates through the lexicon looking for an exact match.

#include <string>
#include <map>
#include <vector>
#include <utility>
#include <memory>

void Explicator_Module_DS_Head_and_Neck_Init(const std::map<std::string, std::string> &, float threshold);

std::unique_ptr<std::map<std::string, float> >
Explicator_Module_DS_Head_and_Neck_Query(const std::map<std::string, std::string> &,
                                         const std::string &,
                                         float threshold);

void Explicator_Module_DS_Head_and_Neck_Deinit(void);
