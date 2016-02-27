// Files.h

#pragma once

#include <string>

// Checks if a file exists and is available to be read. Closes the file in either case.
bool Does_File_Exist_And_Can_Be_Read(const std::string &filename);

// Checks if a directory exists and can be read.
bool Does_Dir_Exist_And_Can_Be_Read(const std::string &dir);
