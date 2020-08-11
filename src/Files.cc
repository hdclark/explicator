// YgorFilesDirs.cc - Routines for interacting with files and directories.
//

#include <fstream>  //Needed for fstream (for file checking.)
#include <string>
#include <system_error>
#include <filesystem>

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
   std::error_code ec;
    const auto p = std::filesystem::path(dir);
    const auto perms = std::filesystem::status(p, ec).permissions();
    return std::filesystem::is_directory(p)
        && ((perms & std::filesystem::perms::owner_read)  != std::filesystem::perms::none)
        && ((perms & std::filesystem::perms::group_read)  != std::filesystem::perms::none)
        && ((perms & std::filesystem::perms::others_read) != std::filesystem::perms::none);
}

} //namespace explicator_internals

