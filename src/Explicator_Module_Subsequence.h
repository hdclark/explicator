// Explicator_Module_Subsequence.h - DICOMautomaton, 2012.

#include <string>
#include <map>
#include <memory>

void Explicator_Module_Subsequence_Init(const std::map<std::string, std::string> &, float threshold);

std::unique_ptr<std::map<std::string, float>>
Explicator_Module_Subsequence_Query(const std::map<std::string, std::string> &, const std::string &, float threshold);

void Explicator_Module_Subsequence_Deinit(void);
