aux_source_directory(src TLV_KALDI_VADEC_SRC)

add_library(libvadec
  STATIC
  ${TLV_KALDI_VADEC_SRC}
)

target_link_libraries(libvadec
  boost
  libwebrtc
  staticjson
  easylogging
)