
#link_directories(${PROJECT_BUILD_DIR}/lib)
set(base_SRCS etcd-def.h)
add_library(etcdcli STATIC ${base_SRCS} )
target_link_libraries(etcdcli evpp_static event glog)


add_executable(testetcdcli ${base_SRCS} main.cpp)
target_link_libraries(testetcdcli evpp_static event glog)