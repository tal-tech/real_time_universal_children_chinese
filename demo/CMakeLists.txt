add_executable(vadec
  vadec.cc
)

target_link_libraries(vadec
  libvadec
  tlvkaldi
  staticjson
)

add_executable(vadec_s
  vadec_s.cc
)

target_link_libraries(vadec_s
  libvadec
  tlvkaldi
  staticjson
)

if(DEFINED PROFILE AND PROFILE STREQUAL "1")
target_link_libraries(vadec_s
  libvadec
  tlvkaldi
  staticjson
  profiler
)
else()
target_link_libraries(vadec_s
  libvadec
  tlvkaldi
  staticjson
)
endif()

add_executable(vadec_mt
  vadec_mt.cc
)

target_link_libraries(vadec_mt
  libvadec
  tlvkaldi
)

add_executable(run_script
  run_script.cc
)

target_link_libraries(run_script
  pthread
  lm_extend
)
