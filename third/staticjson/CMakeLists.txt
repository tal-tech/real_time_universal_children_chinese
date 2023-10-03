include_directories(include)

add_library(staticjson
            STATIC
            src/staticjson.cpp)
			
if (WIN32)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zi")

	foreach(CompilerFlag ${CompilerFlags})
        string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	endforeach()
else()
endif()