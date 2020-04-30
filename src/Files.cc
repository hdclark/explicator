// YgorFilesDirs.cc - Routines for interacting with files and directories.
//

#include <dirent.h> //Needed for working with directories in C/UNIX.
#include <fstream>  //Needed for fstream (for file checking.)
#include <list>
#include <string>

//#include "External/MD5/md5.h" //Needed for MD5_of_File(...)

//#include "YgorString.h"  //Needed for Xtostring routine.

#include "Files.h"

namespace explicator_internals {

bool Does_File_Exist_And_Can_Be_Read(const std::string &filename) {
    // First check if it a directory (because below will return 'true' if the filename is a directory!)
    if(Does_Dir_Exist_And_Can_Be_Read(filename)) {
        return false;
    }
    std::fstream FI(filename.c_str(), std::ifstream::in);
    FI.close();
    return !FI.fail();
}

bool Does_Dir_Exist_And_Can_Be_Read(const std::string &dir) {
    std::list<std::string> out;
    struct dirent **eps;
    auto one = [](const struct dirent *unused) -> int { return 1; };
    int n    = scandir(dir.c_str(), &eps, one, alphasort);

    return (n >= 0);
}

} //namespace explicator_internals

