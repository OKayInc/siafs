cmake_minimum_required(VERSION 3.10)

project(siafs LANGUAGES C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D_FILE_OFFSET_BITS=64")

add_executable(siafs
    main.c
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
    urlcode.c
    urlcode.h
    )

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBMEMCACHED REQUIRED IMPORTED_TARGET libmemcached)
if(${LIBMEMCACHED_FOUND})
#    MESSAGE(${PKG_CONFIG_EXECUTABLE})
#    MESSAGE(${LIBMEMCACHED_LIBRARIES})
#    MESSAGE(${LIBMEMCACHED_LINK_LIBRARIES})
endif()
target_link_libraries(siafs PUBLIC ${LIBMEMCACHED_LIBRARIES})

pkg_check_modules(LIBFUSE REQUIRED IMPORTED_TARGET fuse3)
if(${LIBFUSE_FOUND})
    execute_process(COMMAND bash "-c" "echo -n ${LIBFUSE_VERSION}|cut -d '.' -f 1,2 |sed s/'\\.'//g|tr -d '\n'" OUTPUT_VARIABLE LIBFUSE_VERSION2)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFUSE_USE_VERSION=${LIBFUSE_VERSION2}")
endif()
target_link_libraries(siafs PUBLIC ${LIBFUSE_LIBRARIES})

pkg_check_modules(LIBCURL REQUIRED IMPORTED_TARGET libcurl)
target_link_libraries(siafs PUBLIC ${LIBCURL_LIBRARIES})

pkg_check_modules(LIBCJSON REQUIRED IMPORTED_TARGET libcjson)
target_link_libraries(siafs PUBLIC ${LIBCJSON_LIBRARIES})

#install(FILES mount.memcachedfs DESTINATION sbin)

install(TARGETS siafs RUNTIME DESTINATION bin)
