

set(base_SRCS etcd_def.h etcd_packet.cpp etcd_packet.h etcd_cluster.cpp etcd_cluster.h etcd_client.cpp etcd_client.h)
add_library(etcdcli STATIC ${base_SRCS} )
target_link_libraries(etcdcli evpp_static event glog util)


add_executable(testetcdcli ${base_SRCS} main.cpp )
target_link_libraries(testetcdcli evpp_static event glog util)