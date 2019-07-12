

set(base_SRCS etcd-def.h etcd-packet.cpp etcd-packet.h)
add_library(etcdcli STATIC ${base_SRCS} )
target_link_libraries(etcdcli evpp_static event glog util)


add_executable(testetcdcli ${base_SRCS} main.cpp )
target_link_libraries(testetcdcli evpp_static event glog util)