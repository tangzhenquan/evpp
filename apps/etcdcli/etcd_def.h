//
// Created by tom on 19-7-12.
//

#ifndef APP_ETCDCLI_ETCD_DEF_H
#define APP_ETCDCLI_ETCD_DEF_H

#include <string>
#include <stdint.h>


namespace etcdcli{

    struct etcd_response_header {
        uint64_t cluster_id;
        uint64_t member_id;
        int64_t revision;
        uint64_t raft_term;
    };

    struct etcd_key_value {
        std::string key;
        int64_t create_revision;
        int64_t mod_revision;
        int64_t version;
        std::string value;
        int64_t lease;
    };

    struct etcd_watch_event {
        enum type {
            EN_WEVT_PUT = 0,   // put
            EN_WEVT_DELETE = 1 // delete
        };
    };
}

#endif //APP_ETCDCLI_ETCD_DEF_H
