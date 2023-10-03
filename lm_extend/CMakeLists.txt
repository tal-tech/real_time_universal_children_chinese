add_library(lm_extend
  STATIC
  lm_extend.cc
)

target_link_libraries(lm_extend
  staticjson
  boost
)
