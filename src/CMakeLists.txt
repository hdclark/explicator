
file(GLOB explicator_modules "Explicator_Module*cc")

add_library(explicator
    Explicator.cc
    ${explicator_modules}
    Files.cc
    String.cc
)

target_include_directories(explicator 
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

install(TARGETS 
    explicator
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
)

install(FILES 
    Explicator.h
        DESTINATION include
)

