include_directories(..)
include_directories(kaldi)
include_directories(kaldiext)
include_directories(sphlib)
include_directories(struct)
include_directories(toollib)

file(GLOB_RECURSE TLV_SRCS *.cc *.c)

add_library(tlvkaldi
            SHARED
            ${TLV_SRCS})

target_link_libraries(tlvkaldi
                      boost
                      cjson
                      nosqlite
                      pthread
                      libatlas.so.3
                      libf77blas.so.3
                      libcblas.so.3
                      liblapack_atlas.so.3
                      ${CMAKE_CURRENT_SOURCE_DIR}/../third/openfst/libs/libfst.so)