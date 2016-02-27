// Explicator_Module_Soundex.h - DICOMautomaton, 2012.
//

#include <string>
#include <map>
#include <memory>

void Explicator_Module_Soundex_Init(const std::map<std::string, std::string> &, float threshold);

std::unique_ptr<std::map<std::string, float>>
Explicator_Module_Soundex_Query(const std::map<std::string, std::string> &, const std::string &, float threshold);

void Explicator_Module_Soundex_Deinit(void);
