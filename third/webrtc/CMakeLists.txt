include_directories(.)

file(GLOB WEBRTC_SRCS common_audio/*/*.c rtc_base/*.c*)

add_library(libwebrtc
            STATIC
            ${WEBRTC_SRCS})

if (WIN32)
else()
	target_link_libraries(libwebrtc
	                      pthread)
endif()

if (WIN32)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")

	foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()
else()
endif()