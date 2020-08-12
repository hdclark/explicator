
file(GLOB explicator_modules "Explicator_Module*cc")

add_library(explicator
    Explicator.cc
    ${explicator_modules}
    Files.cc
    String.cc
)
target_link_libraries(explicator
    stdc++fs # Workaround for gcc v8 -- can be removed when newer compilers are expected.
)

target_include_directories(explicator 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS explicator
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES Explicator.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

