SET(sm_srcs page_hashtable.cpp buffer_mgr.cpp page_handle.cpp storage_mgr.cpp)
SET(MINIDB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/lib)

INCLUDE_DIRECTORIES(
    ${MINIDB_INCLUDE_DIRS}
)

ADD_LIBRARY(sm STATIC ${sm_srcs})
# ADD_LIBRARY(sm SHARED ${sm_srcs})

