SET(MINIDB_INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/include)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

INCLUDE_DIRECTORIES(
${MINIDB_INCLUDE_DIRS}
)

SET(MDB_SM_LIB ${PROJECT_SOURCE_DIR}/bin/lib/libsm.a)
SET(MINIDB_LIBS ${MDB_SM_LIB})
ADD_EXECUTABLE(page_hashtable_test page_hashtable_test.cpp)
TARGET_LINK_LIBRARIES(page_hashtable_test ${MDB_SM_LIB})

ADD_EXECUTABLE(storage_mgr_test storage_mgr_test.cpp)
TARGET_LINK_LIBRARIES(storage_mgr_test ${MDB_SM_LIB})

ADD_EXECUTABLE(buffer_mgr_test buffer_mgr_test.cpp)
TARGET_LINK_LIBRARIES(buffer_mgr_test ${MDB_SM_LIB})
