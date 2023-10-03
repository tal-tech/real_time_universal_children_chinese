include_directories(../..)
include_directories(../../tlv/kaldi)

set(CMAKE_INSTALL_RPATH "/usr/local/lib")

add_executable(mem-leak-test
               mem_leak_test.cc)

target_link_libraries(mem-leak-test
                      cjson
                      tlvkaldi)

add_executable(kld-decoder
               kld-decoder.cc)

target_link_libraries(kld-decoder
                      cjson
                      tlvkaldi)