// String.h
#pragma once

#include <map>
#include <set>
#include <string>

namespace explicator_internals {

//-------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------ Self-contained N-gram routines
//-----------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------
namespace NGRAMS {
    const unsigned char CHARS = 0x1;
    const unsigned char WORDS = 0x2;
}

std::map<std::string, float> NGrams_With_Occurence(const std::string &thestring,
                                                   long int numb_of_ngrams,
                                                   long int length_of_ngrams,
                                                   const unsigned char &type);
std::set<std::string>
NGrams(const std::string &thestring, long int numb_of_ngrams, long int length_of_ngrams, const unsigned char &type);
std::set<std::string> NGram_Matches(const std::set<std::string> &A, const std::set<std::string> &B);
long int NGram_Match_Count(const std::set<std::string> &A, const std::set<std::string> &B);

//-------------------------------------------------------------------------------------------------------------------------------
//--------------------------------------------- Substring and Subsequence routines
//----------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------
std::string ALongestCommonSubstring(const std::string &A, const std::string &B);

//-------------------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------- Common text transformations
//------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------------------
namespace CANONICALIZE {                 // Canonicalization masks. Use several by bit-ORing ('|') them together.
    const unsigned char TO_UPPER  = 1;   // Uppercase the characters, if possible.
    const unsigned char TRIM_ENDS = 4;   // Trim whitespace to the left of first, to the right of last char.
    const unsigned char TRIM      = 8;   // Trim the edges and shrink long whitespace to a single space.
    const unsigned char TRIM_ALL  = 16;  // Remove ALL whitespace.
    const unsigned char TO_AZ     = 32;  // Remove all non [ A-Za-z] characters.
    const unsigned char TO_NUM    = 64;  // Remove all non [ 0-9.-] characters.
    const unsigned char TO_NUMAZ  = 128; // Remove all non [ A-Za-z0-9.-] characters.
}

std::string Canonicalize_String2(const std::string &in, const unsigned char &mask); //<--- prefer this version

} //namespace explicator_internals

