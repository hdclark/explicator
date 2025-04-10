
# Explicator

[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Language](https://img.shields.io/github/languages/top/hdclark/Explicator.svg)](https://gitlab.com/hdeanclark/Explicator)
[![LOC](https://tokei.rs/b1/gitlab/hdeanclark/Explicator)](https://gitlab.com/hdeanclark/Explicator)

[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/hdclark/explicator.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/hdclark/explicator/context:cpp)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/74729f834e8e4881910bffd3edc26063)](https://www.codacy.com/manual/hdclark/explicator?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=hdclark/explicator&amp;utm_campaign=Badge_Grade)

[![GitLab CI Pipeline Status](https://gitlab.com/hdeanclark/Explicator/badges/master/pipeline.svg)](https://gitlab.com/hdeanclark/Explicator/-/commits/master)

[![Latest Release DOI](https://zenodo.org/badge/52650291.svg)](https://zenodo.org/badge/latestdoi/52650291)

## Introduction

This C++ library is designed to provide a simple method of translating snippets
of text (like names). Here is a handy reference for evaluating whether or not
this library will be of use for you:

Will it help me...

    ...translate strings into a given (small) language?                              -- yes.
    ...search for proteins, base pairs, or other large single-string domains?        -- no.
    ...translate a given document from {English,French,...} to {French,English,...}? -- slowly.
    ...translate some given, badly-transcribed terms into a handful of known values? -- yes.
    ...pick out terms of relevance (and remove non-relevant terms) from a document?  -- yes.

The general idea is that you have a string of data "from the wild." Lets say
that a user gives you the name of a major city. Being an imperfect user, they
type "Amterdan" instead of "Amsterdam". Processing this data in some automated
way might be troublesome as-is, so we use this library to solve the problem.
There are two steps:

1. We provide a list of (properly-spelled) cities. This is the 'lexicon' of the
   library. To give the library some idea of what to expect, we also provide some
   examples of (possible) misspellings. The format of the lexicon file is like:

        # A comment.
        ...
        Amsterdam : amsterdam
        Amsterdam : Amersdtam
        Amsterdam : Amteistam
        Amsterdam : Ams ter dam
        New York  : New Yerk
        New York  : New York
        New York  : Nwe new york
        Vancouver : Vancoover
        ...

2. Then we write the code. We use it like so:

   ```C++
   #include <iostream>
   #include <string>
   #include <Explicator.h>                           // <--- There is a single header.

   int main(int argc, char **argv){
       std::string lexicon("/path/to/your/lexicon");
       Explicator X(lexicon);                        // <--- Loads the file, initializes.

       std::string dirty_str("Amterdan");            // <--- Not in the lexicon.
       std::string clean_str = X(dirty_str);

       std::cout << "The cleaned string is " << clean_str << std::endl;
       return 0;
   }
   ```

Compile and run with:

    g++ --std=c++17 ... -lexplicator -lstdc++fs -lm && ./a.out

The output should be: `The cleaned string is Amsterdam`.

The translation of the text uses a variety of highly configurable string
similarity modules. For more advanced functionality, e.g., retrieving the
matched score, consult the examples or read the only header file: Explicator.h.

## Dependencies

No external binary or run-time dependencies are required. A standard `C++`
toolchain is required to build the `Explicator` library and binaries.

## Installation

This project uses `CMake`. Use the usual commands to compile:
 
     $>  cd /path/to/source/directory
     $>  mkdir build && cd build/
 
Then, iff by-passing your package manager:
 
     $>  cmake ../ -DCMAKE_INSTALL_PREFIX=/usr
     $>  make && sudo make install
 
or, if building for Debian:
 
     $>  cmake ../ -DCMAKE_INSTALL_PREFIX=/usr
     $>  make && make package
     $>  sudo apt install -f ./*.deb
 
or, if building for Arch Linux:
 
     $>  rsync -aC --exclude build ../ ./
     $>  makepkg --syncdeps --noconfirm # Optionally also [--install].
 
A helper script that will auto-detect the system and package or install properly
can be invoked as:
 
     $>  ./compile_and_install.sh

What gets installed?

- libexplicator.so
  - A single library. Contains substring modules of various kinds.

- Explicator.h
  - The single header file, which contains a definition of the single
    `Explicator` class.

- Various examples
  - Binary examples are installed, by default, to `${INSTALL_PREFIX}/bin/`.
    Their sources are installed to
    `${INSTALL_PREFIX}/usr/share/explicator/example_sources/`.

- Sample lexicons
  - A few lexicons are provided. Some are examples, some are real-world lexicons
    used by downstream projects. They are installed to
    `${INSTALL_PREFIX}/usr/share/explicator/lexicons/`.

## Getting started

Consult the example sources installed to
`${INSTALL_PREFIX}/usr/share/explicator/example_sources/`. They illustrate all
aspects of the `Explicator` class. You need to invoke them with a lexicon.
Sometimes a string is also needed. Here is sample usage on the author's machine:


    $> export LEXICON=/usr/share/explicator/lexicons/Misspellings.lexicon
    $> explicator_cross_verify $LEXICON
    # Generated by Cross_Verify(...).
    # Columns: lexicon_threshold(%), frac_of_lexicon(%), frac_correct, ...
    # Each column is a fraction of the total number of translations.
    100 100 1 0 0 1
    100 95 0.945701 0.0443439 0 0.997461
    100 90 0.895928 0.0832579 0 0.990231
    100 85 0.846154 0.129412 0 0.978261
    ...

    $> export LEXICON=/usr/share/explicator/lexicons/Misspellings.lexicon
    $> explicator_print_weights_thresholds $LEXICON
    Default per-module thresholds:
        Module #8 threshold = 0.3
        Module #32 threshold = 0.3
        Module #64 threshold = 0.3
        Module #128 threshold = 0.3
    ...

    $> export LEXICON=/usr/share/explicator/lexicons/Misspellings.lexicon
    $> explicator_translate_string $LEXICON tednecy
    tendency
    Best Score = 0.591575
         Possibility 'Caribbean' scored 0.267535
         Possibility 'Fahrenheit' scored 0.349613
         Possibility 'Neanderthal' scored 0.345116
         Possibility 'Portuguese' scored 0.295754
         Possibility 'accommodate' scored 0.137085
         Possibility 'accommodation' scored 0.10582
    ...

    $> export LEXICON=/usr/share/explicator/lexicons/Misspellings.lexicon
    $> explicator_translate_string_all_general $LEXICON tednecy
    tendency
    Best Score = 0.492674
         Possibility 'Caribbean' scored 0.100326
         Possibility 'Fahrenheit' scored 0.131105
         Possibility 'Neanderthal' scored 0.129418
         Possibility 'Portuguese' scored 0.110908
         Possibility 'accommodate' scored 0.0514069
         Possibility 'accommodation' scored 0.0396825
    ... 

    $> export LEXICON=/usr/share/explicator/lexicons/Misspellings.lexicon
    $> explicator_translate_string_levenshtein $LEXICON tednecy
    tendency
    Best Score = 0.846154
         Possibility 'Caribbean' scored 0.384615
         Possibility 'Fahrenheit' scored 0.461538
         Possibility 'Neanderthal' scored 0.461538
         Possibility 'Portuguese' scored 0.384615
         Possibility 'achieve' scored 0.538462
         Possibility 'across' scored 0.461538
    ...

    $> export LEXICON=/usr/share/explicator/lexicons/Misspellings.lexicon
    $> explicator_translate_string_jarowinkler $LEXICON tednecy
    tendency
    Best Score = 0.928571
         Possibility 'Caribbean' scored 0.417989
         Possibility 'Fahrenheit' scored 0.587302
         Possibility 'Neanderthal' scored 0.57381
         Possibility 'Portuguese' scored 0.502645
         Possibility 'accommodate' scored 0.411255
         Possibility 'accommodation' scored 0.31746
    ...

    $> export LEXICON=/usr/share/explicator/lexicons/Misspellings.lexicon
    $> explicator_lexicon_dogfooder $LEXICON
    Dirty: '    ACCOMMODATE' ---> Clean: '   accommodate'. Actual: '   accommodate'. Success: 1
    Dirty: '  ACCOMMODATION' ---> Clean: ' accommodation'. Actual: ' accommodation'. Success: 1
    Dirty: '     ACCOMODATE' ---> Clean: '   accommodate'. Actual: '   accommodate'. Success: 1
    Dirty: '   ACCOMODATION' ---> Clean: ' accommodation'. Actual: ' accommodation'. Success: 1
    Dirty: '        ACCROSS' ---> Clean: '        across'. Actual: '        across'. Success: 1
    Dirty: '        ACHEIVE' ---> Clean: '       achieve'. Actual: '       achieve'. Success: 1
    Dirty: '        ACHIEVE' ---> Clean: '       achieve'. Actual: '       achieve'. Success: 1
    ...

Details about each example program are available in the source.
 
A short video overview of this project can be seen at
<https://www.youtube.com/watch?v=8K5AvxkZ6Zs>. There is a more technical paper
on it at <http://iopscience.iop.org/article/10.1088/1742-6596/489/1/012088/pdf>.
If you use `Explicator` in a scientific work, please cite as follows:

    @article{1742-6596-489-1-012088,
      author={H Clark and J Wu and V Moiseenko and R Lee and B Gill and C Duzenli and S Thomas},
      title={Semi-automated contour recognition using DICOMautomaton},
      journal={Journal of Physics: Conference Series},
      volume={489},
      number={1},
      pages={012088},
      url={http://stacks.iop.org/1742-6596/489/i=1/a=012088},
      year={2014}
    }

## Listing of string matching routines

1. Common/Standard techniques.
   - Exact matches. (This library is always guaranteed to precisely match exact
     queries.)
   - Levenshtein. (Including deletions,insertions, and permutations.)
   - Character-based N-grams.
   - Word-based N-grams.
   - Jaro-Winkler.

2. Phonetic techniques. (Useful for similar-sounding English strings.)
   - Double-Metaphone.
   - Soundex.
   - Match Rating Approach.

3. Custom/Domain-specific techniques. (All of these are 'home-brew'.)
   - "Emplacements" (Useful for finding/matching/determining abbreviations.)
   - DICOM-tag string embedding. (Useful for medical terms/DICOM user tags.)
   - Subsequences. (Attempts to orthogonalize all subsequences. Useful for
     ordered input.)

## List of removed matching routines

- Artifical neural nets, via `FANN.'
  - The upstream implementation I used was buggy. Patches and pull-requests for
    alternative implementations are welcome.

- Bag-of-characters, via `NCBI similarity.'
  - Tried with `Cosine`, `Minkowski`, and `L2` distance metrics. Seemed to
    underperform compared to others.

- `simstring` library similarities:
  - `Cosine`.
  - `Dice`.
  - `Jaccard`.
  - Overlap.
    - Though all were fast (as advertised!), they produced unreasonable amounts
      of temporary files.

- `Dice` similarity.
  - via Francis Tyers' implementation found at (March 20th, 2013)
    <http://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Dice%27s_coefficient>.
    Removed this implementation for packaging/potential license issues.

- Regex-based similarity.
  - Found to be too time-consuming and brittle to orthogonalize regex for a
    given lexicon.

- Geometrical matching routines.
  - This functionality was moved to a separate, similar library called
    `libdemarcator'.


## FAQs

- What platforms are supported?
  - Currently, only Linux with compilers supporting the C++17 standard. 
  - Porting to BSDs and Windows should be fairly easy. Windows will require
    converting std::string to std::wstring and localizing string literals. BSD
    might work as-is.

- Can I use this library to replace my spell checker?
  - Perhaps, for some problem domains. Likely not in general, though. This
    library is designed to be accurate and easy to use - not as fast as
    possible. Performance concerns were not entirely ignored, but typical
    language dictionaries contain too many entries. Better approaches would
    likely involve B-trees or some branching storage container.

- Which similarity routines should I use?
  - This depends somewhat on the data you will be processing. For medical
    phrases or abbreviations, use the Levenshtein, DICOM hash, and Emplacements.
    For long sentences of English, consider the Cosine, Jaccard, Levenshtein,
    and/or Double-Metaphone. For text that has been transcribed from audio,
    consider Soundex and Double Metaphone. For more precise things (like
    transforming some formal logic expressions or simple substitution) use just
    the exact filter. If you have a very predictable input domain, consider
    writing a custom, domain-specific module. This will provide the best
    all-around performance for your purposes.

- What is the performance like?
  - Not great! See "How is the accuracy?" Some string matching algorithms are
    inherently slow, and usage/accuracy has been emphasized over computational
    speed. This being said, using the entirety of the English dictionaries can
    provide queries in sub-seconds for some algorithms. Typical times for a
    500-entry lexicon on an older laptop are 10's of milliseconds (loading the
    routines, querying the lexicon, printing to screen.) Conversely, if an exact
    match is found there will be little overhead from this library.

- What is the accuracy like?
  - Decent, on average. Great in some domains, poor in others. The accuracy is a
    reflexion of the quality of the lexicon and the computational power
    expended. Choosing the most appropriate matching algorithm has a significant
    impact.

- Can I train the algorithms on a set of data?
  - Some of them. At least, this functionality is at the proof of concept stage
    at the time of writing. Most algorithms include a pre-computation step. Some
    take this opportunity to self-tune and/or train. See the artificial neural
    net and subsequence routines. 

- How can I squeeze performance out of the library?
  - Some general tips: Avoid instantiation of the Explicators which can be
    costly due to processing. Only choose a few algorithms (or one, if
    possible.) Consider reading Wikipedia articles or testing the speed of the
    various algorithms on your specific data. Keep the lexicon as small as
    possible. A slow-running, precise algorithm often beats a dozen fast,
    imprecise algorithms.

- How do I tell the library to warn me if the best match is not very good?
- How do I tell 'how good' a match is?
  - There is no systematic way to indicate how good a match is. There are some
    facilities available for you (the user) to implement what you think is a
    'good match' by passing out the ranking of each term in the lexicon. This is
    the Explicator::Get_Last_Results(void) method. If perfect facilities existed
    to determine the 'goodness' of a match, we would not need these algorithms
    to try predict anything! See the examples for more information.

- The library translates words or phrases that are obviously incorrect. How do I
  fix this?
- The library only knows words that I tell it. How do I fix this?
- My lexicon knows two words: "A" and "B". Why does it translate everything to
  only "A" or "B"?
  - The library only speaks the language you tell it to speak. In some cases,
    this is a problem. Typically when scraping websites or previously-unseen
    documents, one will not have an idea what to expect. If the lexicon does not
    reflect the parts you are interested in, you can be guaranteed to have
    garbage results. Alternatively, there are some benefits of an "everything is
    an 'A' or a 'B' approach." In particular, if the library is emitting
    instructions which will be interpretted by a rigid interpreter, one wants
    each translation to give a valid instruction.
  - One way to deal with this is to populate your lexicon with a representative
    sample of data. If, say, you want to pick out days of the week in a long
    document. You are able to isolate all words which contain the 'day' suffix
    using regex, but now you need to classify the occurence of only fridays.
    Since the author of the document is an idiot, they misspelled almost all
    days. A satisfactory lexicon may look like:

         ...
         Monday       : JUNK
         muns day     : JUNK
         ...etc...
         Tuesday      : JUNK
         Wednesday    : JUNK
         Thursday     : JUNK
         Friday       : Friday
         ...

    Which will pick out all occurences of words that are nearly 'friday'. Note,
    though, that EVERY word translated using this lexicon will either spit out
    'JUNK' or 'Friday'. 

- This library doesn't include X. When can it be added?
  - Probably fairly soon. I'm happy to hear suggestions and even happier to
    implement similarity routines (barring time contraints). I'll also happily
    accept contributions for most code.
          
    
- I want to add a string similarity metric. Where do I start? How do I get
  started hacking on this project?
  - Please take a look at the various 'modules' already implented. There are
    three routines that need to implemented: (1) a pre-query or preparatory
    routine (~constructor), (2) a query function, and (3) a cleanup routine
    (~destructor). Then include the header file in Explicator.cc, add the module
    to the enum of available modules, and recompile. You will then able to load
    your module (see the *_translate_* examples for details on how to do this).

- Why does the lexicon require me to specify 'A : A' in order to do an exact
  match for 'A'?
  - So that, in case you want all occurences of 'A' to be turned into 'B'
    instead of 'A'. If this confuses you, think about a two-step conversion. You
    want to convert all 'A's into 'B's, and all 'B's into 'A's. The first
    conversion has a lexicon like:

         A : B
         B : C

    and the second has a lexicon like:

         C : A

    This allows for maximum flexibility. Due to guaranteed exact matches, this
    library is suitable for use as the backbone of a macro language. The bonus
    of such a thing would be that one could easily, effortlessly, and
    predictably handle user errors/typos.

## Bugs and Errors

- Matches are generally case-insensitive. This can be problematic for exact matches in
  your application logic when combined with case-senstive entities (e.g., std::map).

## License

Informally, GPLv3 for libraries and examples and GFDLv3 for documentation. 
 
For full license info, see [LICENCE](LICENSE). If in doubt or you have
questions, please contact the author.

## Special thanks to

- The British Columbia Cancer Agency for funding and support.
- The University of British Columbia.
- Dr.s Steven Thomas and Jonn Wu.
- Sarah Clark.
- Thomas McElroy.

## Author

Hal Clark. <mailto:gmail.com[at]hdeanclark>. Comments, patches, and pull
requests are all welcome.

