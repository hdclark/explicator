
file(GLOB sample_lexicons "*lexicon")
file(GLOB frozen_sample_lexicons "Frozen/*lexicon")

install(FILES ${sample_lexicons}
              ${frozen_sample_lexicons}
        DESTINATION "${CMAKE_INSTALL_DATAROOTDIR}/explicator/lexicons/"
)

