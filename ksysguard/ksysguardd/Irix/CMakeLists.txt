include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/../CContLib ${CMAKE_CURRENT_SOURCE_DIR}/../ )

add_library(libksysguardd STATIC 
            cpu.c
            LoadAvg.c
            Memory.c
            NetDev.c
            ProcessList.c)

