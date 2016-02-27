// Explicator_Module_Emplacement.h - DICOMautomaton, 2012.
//

#include <string>
#include <map>
#include <vector>
#include <utility>
#include <memory>

void Explicator_Module_Emplacement_Init(const std::map<std::string, std::string> &, float threshold);

std::unique_ptr<std::map<std::string, float> >
Explicator_Module_Emplacement_Query(const std::map<std::string, std::string> &, const std::string &, float threshold);

void Explicator_Module_Emplacement_Deinit(void);
