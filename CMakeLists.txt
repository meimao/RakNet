cmake_minimum_required(VERSION 3.1)

project(RakNet)

set(CMAKE_CXX_STANDARD 11)

if( NOT APPLE )
    # check 64 bit
    if( CMAKE_SIZEOF_VOID_P MATCHES "4" )
      set( HAVE_64_BIT 0 )
    else(  CMAKE_SIZEOF_VOID_P MATCHES "4")
      set( HAVE_64_BIT 1 )
    endif( CMAKE_SIZEOF_VOID_P MATCHES "4")
endif( NOT APPLE )

IF (WIN32 AND NOT UNIX)
    set (PROGRAMFILESX86 $ENV{PROGRAMFILES})
    string(REPLACE "\\" "/" PROGRAMFILESX86 ${PROGRAMFILESX86})
ENDIF(WIN32 AND NOT UNIX)

IF (WIN32 AND NOT UNIX)
    set(RAKNET_LIBRARY_LIBS ws2_32.lib)
ELSE(WIN32 AND NOT UNIX)
    set(RAKNET_LIBRARY_LIBS pthread)
ENDIF(WIN32 AND NOT UNIX)


# Options
option(RAKNET_ENABLE_SAMPLES "Generate RakNet sample projects." FALSE)
option(RAKNET_ENABLE_DLL "Generate the DLL project." TRUE)
option(RAKNET_ENABLE_STATIC "Generate the static library project." TRUE)

option(RAKNET_ENABLE_LIBCAT_SECURITY "Enable secure connection support." FALSE)

include(CmakeIncludes/CmakeMacros.cmake)

if(MSVC)
    FIXLINKOPTIONS()
    FIXCOMPILEOPTIONS()
endif(MSVC)


include_directories(SYSTEM include/raknet DependentExtensions)

add_subdirectory(lib)

set(RAKNET_COMMON_LIBS RakNetLibStatic)

if(RAKNET_GENERATE_SAMPLES)
    add_subdirectory(Samples)
endif()

file(COPY include DESTINATION .)