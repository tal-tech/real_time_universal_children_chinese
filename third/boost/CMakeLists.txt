include_directories(..)

if (WIN32)
  set(
    BOOST_THREAD_SOURCES
    src/thread/win32/thread.cpp
    src/thread/win32/tss_dll.cpp
    src/thread/win32/tss_pe.cpp
  )
else (WIN32)
  set(
    BOOST_THREAD_SOURCES
    src/thread/pthread/thread.cpp
    src/thread/pthread/once_atomic.cpp
    src/thread/pthread/once.cpp
  )
endif (WIN32)

message(${BOOST_THREAD_SOURCES})

add_library(
  boost
  STATIC
  src/filesystem/utf8_codecvt_facet.cpp
  src/filesystem/unique_path.cpp
  src/filesystem/portability.cpp
  src/filesystem/path_traits.cpp
  src/filesystem/path.cpp
  src/filesystem/operations.cpp
  src/filesystem/codecvt_error_category.cpp
  src/filesystem/windows_file_codecvt.cpp
  src/system/error_code.cpp
  src/thread/tss_null.cpp
  src/thread/future.cpp
  ${BOOST_THREAD_SOURCES}
)
