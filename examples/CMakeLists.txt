

add_executable(explicator_lexicon_dogfooder
    Lexicon_Dogfooder.cc
)
target_link_libraries(explicator_lexicon_dogfooder
    LINK_PUBLIC explicator
    m
    Threads::Threads
)


add_executable(explicator_cross_verify
    Cross_Verify.cc
)
target_link_libraries(explicator_cross_verify
    LINK_PUBLIC explicator
    m
    Threads::Threads
)


add_executable(explicator_translate_string
    Translate_String.cc
)
target_link_libraries(explicator_translate_string
    LINK_PUBLIC explicator
    m
    Threads::Threads
)


add_executable(explicator_translate_string_levenshtein
    Translate_String_Levenshtein.cc
)
target_link_libraries(explicator_translate_string_levenshtein
    LINK_PUBLIC explicator
    m
    Threads::Threads
)


add_executable(explicator_translate_string_jarowinkler
    Translate_String_JaroWinkler.cc
)
target_link_libraries(explicator_translate_string_jarowinkler
    LINK_PUBLIC explicator
    m
    Threads::Threads
)


add_executable(explicator_translate_string_all_general
    Translate_String_All_General.cc
)
target_link_libraries(explicator_translate_string_all_general
    LINK_PUBLIC explicator
    m
    Threads::Threads
)


add_executable(explicator_print_weights_thresholds
    Print_Weights_Thresholds.cc
)
target_link_libraries(explicator_print_weights_thresholds
    LINK_PUBLIC explicator
    m
    Threads::Threads
)


INSTALL(TARGETS 
    explicator_lexicon_dogfooder
    explicator_cross_verify
    explicator_translate_string
    explicator_translate_string_levenshtein
    explicator_translate_string_jarowinkler
    explicator_translate_string_all_general
    explicator_print_weights_thresholds
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

file(GLOB example_sources "*cc")

INSTALL(FILES 
    ${example_sources}
        DESTINATION share/explicator/example_sources/
)

