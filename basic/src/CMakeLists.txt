# src files. For inlined headers, include header as a source
set(lib-srcs
        payload/message.hpp
        payload/basicbuilder.cpp
        socket/sessionhandler.cpp
        socket/client.cpp
        socket/server.cpp
)

# library targets
add_library(basic_socket ${lib-srcs})
TARGET_LINK_LIBRARIES(basic_socket ${Boost_LIBRARIES} ${SOCKET_LIBRARY})


# dependencies
set(BUILD_LIBRARIES basic_socket ${Boost_LIBRARIES} ${SOCKET_LIBRARY} pthread)


# app targets
add_executable(serverApp apps/ServerApp.cpp)
add_executable(clientApp apps/clientApp.cpp)
target_link_libraries(serverApp ${BUILD_LIBRARIES})
target_link_libraries(clientApp ${BUILD_LIBRARIES})

# installs: cmake --target install --config Release
install(TARGETS serverApp clientApp CONFIGURATIONS Release
        DESTINATION release)


# for clang
#set_target_properties(clientApp PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")