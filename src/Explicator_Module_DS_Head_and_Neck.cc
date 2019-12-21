// Explicator_Module_DS_Head_and_Neck.cc - DICOMautomaton, 2012.
//
// Contains domain-specific knowledge about head and neck structures.
// In a sense, this is a way to 'cheat' a small/incomplete lexicon. On the other
// hand, we cannot anticipate *every* incoming string, so it is probably best to
// utilize as much data as we can.

#include <stddef.h>
#include <map>
#include <memory>
#include <string>
#include <utility>

#include "String.h"

// Initializor function.
void Explicator_Module_DS_Head_and_Neck_Init(const std::map<std::string, std::string> &lexicon, float threshold) {
    return;
}

static std::string Last(const std::string &in, size_t N) { // Gets the last N characters from a string, if possible.
    if(in.size() <= N) {
        return in;
    }
    return in.substr(in.size() - N, N); // Need the -1 because size() is 1-based, but operator[] is 0-based.
}

static std::string First(const std::string &in, size_t N) { // Gets the first N characters from a string, if possible.
    if(in.size() <= N) {
        return in;
    }
    return in.substr(0, N);
}

static std::string Position(const std::string &in, size_t N) { // Gets the character at position N, if possible.
    if(in.size() <= N) {
        return in;
    }
    std::string temp;
    temp += in[N];
    return temp;
}
static bool Contains(const std::string &in, const std::string &pattern) {
    auto it = in.find(pattern);
    return (it != std::string::npos);
}

// Query function.
std::unique_ptr<std::map<std::string, float>>
Explicator_Module_DS_Head_and_Neck_Query(const std::map<std::string, std::string> &lexicon,
                                         const std::string &in,
                                         float threshold) {
    // Remember: The lexicon looks like: < dirty : clean >
    std::unique_ptr<std::map<std::string, float>> output(new std::map<std::string, float>());
    const std::string X
        = Canonicalize_String2(in, CANONICALIZE::TRIM_ALL | CANONICALIZE::TO_UPPER); // Remove all spaces.
    // FUNCINFO("X is now " << X); //Show that this string has correctly been canonicalized. Do not be confused by the
    // Cross_Verify output!

    //--------------------------------------------------------------------------------------------
    // JUNK - stuff which can easily be picked out.
    if(Last(X, 4) == "OPTI") {
        (*output)["JUNK"] = 1.0;

    } else if(Contains(X, "+") && (Contains(X, "MM") || Contains(X, "CM") || Contains(X, "MARGIN"))) {
        (*output)["JUNK"] = 1.0;

    } else if(Contains(X, "LOBES")) {
        (*output)["JUNK"] = 1.0;

    } else if(Contains(X, "EYES")) {
        (*output)["JUNK"] = 1.0;

    } else if(Contains(X, "PAROTIDS")) {
        (*output)["JUNK"] = 1.0;

        // Body. Should not be many surprises here.
    } else if(First(X, 4) == "BODY") {
        (*output)["Body"] = 1.0;

        // Brainstem.
    } else if(Last(X, 4) == "STEM") {
        (*output)["Brainstem"] = 1.0;
    } else if(First(X, 5) == "BSTEM") {
        (*output)["Brainstem"] = 1.0;

        // Chiasm.
    } else if(Contains(X, "IASM")) {
        (*output)["Chiasm"] = 1.0;

        // Cord.
    } else if(Last(X, 4) == "CORD") {
        (*output)["Cord"] = 1.0;
    } else if(Contains(X, "SPINAL")) {
        (*output)["Cord"] = 1.0;

        // CTV.
    } else if(First(X, 3) == "CTV") {
        (*output)["CTV"] = 1.0;
    } else if(Contains(X, "CLINICAL") && (Contains(X, "TARGET") || Contains(X, "VOL"))) {
        (*output)["CTV"] = 1.0;

        // GTV.
    } else if(First(X, 3) == "GTV") {
        (*output)["GTV"] = 1.0;
    } else if(Contains(X, "GROSS") && (Contains(X, "TARGET") || Contains(X, "VOL"))) {
        (*output)["GTV"] = 1.0;

        // Larynx.
    } else if(Contains(X, "LARYNX")) {
        (*output)["Larynx"] = 1.0;
    } else if(Contains(X, "VOICE")) {
        (*output)["Larynx"] = 1.0;

        // Left Eye.
    } else if((First(X, 1) == "L") && (Last(X, 3) == "EYE")) {
        (*output)["Left Eye"] = 1.0;

        // Left Optic Nerve.
    } else if(Contains(X, "L") && Contains(X, "NERVE")) {
        (*output)["Left Optic Nerve"] = 1.0;
    } else if(Contains(X, "L") && Contains(X, "NRV")) {
        (*output)["Left Optic Nerve"] = 1.0;
    } else if(Contains(X, "L") && Contains(X, "OPTIC")) {
        (*output)["Left Optic Nerve"] = 1.0;

        // Left Parotid.
    } else if(Contains(X, "L") && Contains(X, "PAR")) {
        (*output)["Left Parotid"] = 1.0;
    } else if(X.substr(0, 4) == "LPAR") {
        (*output)["Left Parotid"] = 1.0;
    } else if(X.substr(0, 4) == "LTPA") {
        (*output)["Left Parotid"] = 1.0;
    } else if(X.substr(0, 7) == "LEFTPAR") {
        (*output)["Left Parotid"] = 1.0;

        // Right Submandibular.
    } else if(((First(X, 1) == "R") || (Position(X, 3) == "R")) && Contains(X, "SUB")) {
        (*output)["Right Submand"] = 1.0;
    } else if(((First(X, 1) == "R") || (Position(X, 3) == "R")) && Contains(X, "SMGLAND")) {
        (*output)["Right Submand"] = 1.0;
    } else if(Contains(X, "RSUB")) {
        (*output)["Right Submand"] = 1.0;

        // Left Submandibular.
    } else if(((First(X, 1) == "L") || (Position(X, 3) == "L")) && Contains(X, "SUB")) {
        (*output)["Left Submand"] = 1.0;
    } else if(((First(X, 1) == "L") || (Position(X, 3) == "L")) && Contains(X, "SMGLAND")) {
        (*output)["Left Submand"] = 1.0;
    } else if(Contains(X, "LSUB")) {
        (*output)["Left Submand"] = 1.0;

        // Right Temp Lobe.
    } else if(((First(X, 1) == "R") || (Position(X, 3) == "R")) && Contains(X, "TEMP")) {
        (*output)["Right Temp Lobe"] = 1.0;
    } else if(Contains(X, "RTEMPORAL")) {
        (*output)["Right Temp Lobe"] = 1.0;
    } else if(Contains(X, "RTEMP")) {
        (*output)["Right Temp Lobe"] = 1.0;

        // Left Temp Lobe.
    } else if(((First(X, 1) == "L") || (Position(X, 3) == "L")) && Contains(X, "TEMP")) {
        (*output)["Left Temp Lobe"] = 1.0;
    } else if(Contains(X, "LTEMPORAL")) {
        (*output)["Left Temp Lobe"] = 1.0;
    } else if(Contains(X, "LTEMP")) {
        (*output)["Left Temp Lobe"] = 1.0;

        // Right Eye.
    } else if((First(X, 1) == "R") && (Last(X, 3) == "EYE")) {
        (*output)["Right Eye"] = 1.0;

        // Right Optic Nerve.
    } else if(Contains(X, "R") && Contains(X, "NERVE")) {
        (*output)["Right Optic Nerve"] = 1.0;
    } else if(Contains(X, "R") && Contains(X, "NRV")) {
        (*output)["Right Optic Nerve"] = 1.0;
    } else if(Contains(X, "R") && Contains(X, "OPTIC")) {
        (*output)["Right Optic Nerve"] = 1.0;

        // Right Parotid.
    } else if(Contains(X, "R") && (Contains(X, "PAR"))) {
        (*output)["Right Parotid"] = 1.0;
    } else if(X.substr(0, 4) == "RPAR") {
        (*output)["Right Parotid"] = 1.0;
    } else if(X.substr(0, 4) == "RTPA") {
        (*output)["Right Parotid"] = 1.0;
    } else if(X.substr(0, 8) == "RIGHTPAR") {
        (*output)["Right Parotid"] = 1.0;
    }

    //--------------------------------------------------------------------------------------------
    //---- Is it OK to call the rest junk? Is it smart to? (I think probably not! - probably creates lots of extra false
    // positives...)
    if(output->empty()) {
        (*output)["JUNK"] = 1.0;
    }

    return output;
}

// De-initializor function. Ensure this function can be called both after AND before the init function.
void Explicator_Module_DS_Head_and_Neck_Deinit(void) {
    return;
}
