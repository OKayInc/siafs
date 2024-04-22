cmake_minimum_required(VERSION 3.10)

project(siafs
    VERSION 0.0.1
    DESCRIPTION "Access the SIA Storage within a mountpoint"
    LANGUAGES C
)

INCLUDE(CheckIncludeFiles)

IF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	SET(CMAKE_BUILD_TYPE RelWithDebInfo)
ENDIF(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")

add_executable(siafs
    main.c
    memcached_cache.h
    memcached_cache.c
    sia_defaults.h
    siafs.h
    siafs.c
    sia.h
    sia.c
    sia_common.h
    sia_common.c
    sia_bus_accounts.h
    sia_bus_accounts.c
    sia_bus_consensus.h
    sia_bus_consensus.c
    sia_bus_multiparts.h
    sia_bus_multiparts.c
    sia_bus_objects.h
    sia_bus_objects.c
    sia_stats.h
    sia_stats.c
    sia_worker_objects.h
    sia_worker_objects.c
)

option(WITH_XATTR "Enable support for extended attributes" ON)

IF (WITH_XATTR)
	CHECK_INCLUDE_FILES("sys/xattr.h" HAVE_LIBC_XATTR)
	CHECK_INCLUDE_FILES("attr/xattr.h" HAVE_LIBATTR_XATTR)

	IF (HAVE_LIBC_XATTR)
		add_definitions(-DLIBC_XATTR)
	ELSEIF(HAVE_LIBATTR_XATTR)
		add_definitions(-DLIBATTR_XATTR)
	ENDIF()

	IF (NOT HAVE_LIBC_XATTR AND NOT HAVE_LIBATTR_XATTR)
		add_definitions(-DDISABLE_XATTR)
	ENDIF()
ELSE (WITH_XATTR)
	add_definitions(-DDISABLE_XATTR)
ENDIF (WITH_XATTR)

option(WITH_MEMCACHED "Enable support for Memcached" ON)
IF (WITH_MEMCACHED)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBMEMCACHED REQUIRED IMPORTED_TARGET libmemcached)
    if(${LIBMEMCACHED_FOUND})
        add_definitions(-DSIA_MEMCACHED)
    #    MESSAGE(${PKG_CONFIG_EXECUTABLE})
    #    MESSAGE(${LIBMEMCACHED_LIBRARIES})
    #    MESSAGE(${LIBMEMCACHED_LINK_LIBRARIES})
    endif()
    target_link_libraries(siafs PUBLIC ${LIBMEMCACHED_LIBRARIES})
ENDIF (WITH_MEMCACHED)

pkg_check_modules(LIBFUSE3 IMPORTED_TARGET fuse3)
IF(${LIBFUSE3_FOUND})
    execute_process(COMMAND bash "-c" "echo -n ${LIBFUSE3_VERSION}|cut -d '.' -f 1,2 |sed s/'\\.'//g|tr -d '\n'" OUTPUT_VARIABLE LIBFUSE_VERSION2)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUSE_USE_VERSION=${LIBFUSE_VERSION2}")
    target_link_libraries(siafs PUBLIC ${LIBFUSE3_LIBRARIES})
ELSE(${LIBFUSE3_FOUND})
        pkg_check_modules(LIBFUSE2 REQUIRED IMPORTED_TARGET fuse)
        IF(${LIBFUSE2_FOUND})
            execute_process(COMMAND bash "-c" "echo -n ${LIBFUSE2_VERSION}|cut -d '.' -f 1,2 |sed s/'\\.'//g|tr -d '\n'" OUTPUT_VARIABLE LIBFUSE_VERSION2)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUSE_USE_VERSION=${LIBFUSE_VERSION2}")
            target_link_libraries(siafs PUBLIC ${LIBFUSE2_LIBRARIES})
        ENDIF(${LIBFUSE2_FOUND})
ENDIF(${LIBFUSE3_FOUND})

pkg_check_modules(LIBCURL REQUIRED IMPORTED_TARGET libcurl)
target_link_libraries(siafs PUBLIC ${LIBCURL_LIBRARIES})

pkg_check_modules(LIBCJSON REQUIRED IMPORTED_TARGET libcjson)
target_link_libraries(siafs PUBLIC ${LIBCJSON_LIBRARIES})

option(WITH_METACACHE "Enable support for meta cache" ON)
IF (WITH_METACACHE)
	add_definitions(-DSIA_METACACHE)
ENDIF(WITH_METACACHE)


install(FILES mount.siafs DESTINATION sbin)
install(TARGETS siafs RUNTIME DESTINATION bin)
