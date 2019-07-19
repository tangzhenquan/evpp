//
// Created by tom on 19-7-15.
//

#ifndef APP_ETCDCLI_ETCD_CLUSTER_H
#define APP_ETCDCLI_ETCD_CLUSTER_H


#include <ctime>
#include <list>
#include <string>
#include <vector>
#include <chrono>

#include "evpp/event_loop.h"
#include "evpp/httpc/request.h"


namespace etcdcli {
    class etcd_cluster {
    public:
        struct flag_t {
            enum type {
                CLOSING      = 0x0001, // closeing
                ENABLE_LEASE = 0x0100, // enable auto get lease
            };
        };
        struct conf_t {
            std::vector<std::string>            conf_hosts;
            std::vector<std::string>            hosts;
            std::string                         authorization;
            std::chrono::system_clock::duration http_cmd_timeout;

            // generated data for cluster members
            std::string                           authorization_header;
            std::string                           path_node;
            std::chrono::system_clock::time_point etcd_members_next_update_time;
            std::chrono::system_clock::duration   etcd_members_update_interval;
            std::chrono::system_clock::duration   etcd_members_retry_interval;

            // generated data for lease
            int64_t                               lease;
            std::chrono::system_clock::time_point keepalive_next_update_time;
            std::chrono::system_clock::duration   keepalive_timeout;
            std::chrono::system_clock::duration   keepalive_interval;
        };
    public:
        etcd_cluster();
        ~etcd_cluster();
        void init(evpp::EventLoop *loop);
        void                               reset();

        bool                               is_available() const;

        inline bool check_flag(uint32_t f) const { return 0 != (flags_ & f); };
        void        set_flag(flag_t::type f, bool v);

        // ====================== apis for configure ==================
        inline const std::vector<std::string> &get_available_hosts() const { return conf_.hosts; }
        inline const std::string &             get_selected_host() const { return conf_.path_node; }



        inline void                            set_conf_hosts(const std::vector<std::string> &hosts) { conf_.conf_hosts = hosts; }
        inline const std::vector<std::string> &get_conf_hosts() const { return conf_.conf_hosts; }



        inline void                                       set_conf_http_timeout(std::chrono::system_clock::duration v) { conf_.http_cmd_timeout = v; }
        inline void                                       set_conf_http_timeout_sec(time_t v) { set_conf_http_timeout(std::chrono::seconds(v)); }
        inline const std::chrono::system_clock::duration &get_conf_http_timeout() const { return conf_.http_cmd_timeout; }
        time_t                                            get_http_timeout_ms() const;



        inline void set_conf_etcd_members_update_interval(std::chrono::system_clock::duration v) { conf_.etcd_members_update_interval = v; }
        inline void set_conf_etcd_members_update_interval_min(time_t v) { set_conf_etcd_members_update_interval(std::chrono::minutes(v)); }
        inline const std::chrono::system_clock::duration &get_conf_etcd_members_update_interval() const { return conf_.etcd_members_update_interval; }

        inline void set_conf_etcd_members_retry_interval(std::chrono::system_clock::duration v) { conf_.etcd_members_retry_interval = v; }
        inline void set_conf_etcd_members_retry_interval_min(time_t v) { set_conf_etcd_members_retry_interval(std::chrono::minutes(v)); }
        inline const std::chrono::system_clock::duration &get_conf_etcd_members_retry_interval() const { return conf_.etcd_members_retry_interval; }

        inline void                                       set_conf_keepalive_timeout(std::chrono::system_clock::duration v) { conf_.keepalive_timeout = v; }
        inline void                                       set_conf_keepalive_timeout_sec(time_t v) { set_conf_keepalive_timeout(std::chrono::seconds(v)); }
        inline const std::chrono::system_clock::duration &get_conf_keepalive_timeout() const { return conf_.keepalive_timeout; }

        inline void set_conf_keepalive_interval(std::chrono::system_clock::duration v) { conf_.keepalive_interval = v; }
        inline void set_conf_keepalive_interval_sec(time_t v) { set_conf_keepalive_interval(std::chrono::seconds(v)); }
        inline const std::chrono::system_clock::duration &get_conf_keepalive_interval() const { return conf_.keepalive_interval; }

    public:

        evpp::httpc::RequestPtr kv_get(const std::string &key, const std::string &range_end = "", int64_t limit = 0,
                                                                 int64_t revision = 0);

        evpp::httpc::Request* kv_set(const std::string &key, const std::string &value, bool assign_lease = false,
                                                                 bool prev_kv = false, bool ignore_value = false, bool ignore_lease = false);

        evpp::httpc::Request* watch(const std::string &key, const std::string &range_end = "", int64_t start_revision = 0,
                             bool prev_kv = false, bool progress_notify = true);
    private:
        uint32_t                                       flags_;
        conf_t                                         conf_;
        evpp::EventLoop*                               loop_;

    };
}

#endif //APP_ETCDCLI_ETCD_CLUSTER_H
