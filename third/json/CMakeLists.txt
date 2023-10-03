include_directories()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

add_library(cjson
            STATIC
            cJSON.c)
