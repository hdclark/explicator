// Explicator_Module_JaroWinkler.h - DICOMautomaton, 2013.
//

#include <string>
#include <map>
#include <vector>
#include <utility>
#include <memory>

void Explicator_Module_JaroWinkler_Init(const std::map<std::string, std::string> &, float threshold);

std::unique_ptr<std::map<std::string, float>>
Explicator_Module_JaroWinkler_Query(const std::map<std::string, std::string> &, const std::string &, float threshold);

void Explicator_Module_JaroWinkler_Deinit(void);
