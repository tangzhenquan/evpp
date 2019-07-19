//
// Created by tom on 19-7-15.
//

#include <sstream>
#include <assert.h>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include "etcd_def.h"
#include "etcd_packet.h"
#include "etcd_cluster.h"
namespace etcdcli {
    /**
          * @note APIs just like this
          * @see https://coreos.com/etcd/docs/latest/dev-guide/api_reference_v3.html
          * @see https://coreos.com/etcd/docs/latest/dev-guide/apispec/swagger/rpc.swagger.json
          * @note KeyValue: { "key": "KEY", "create_revision": "number", "mod_revision": "number", "version": "number", "value": "", "lease": "number" }
          *   Get data => curl http://localhost:2379/v3beta/kv/range -X POST -d '{"key": "KEY", "range_end": ""}'
          *       # Response {"kvs": [{...}], "more": "bool", "count": "COUNT"}
          *   Set data => curl http://localhost:2379/v3beta/kv/put -X POST -d '{"key": "KEY", "value": "", "lease": "number", "prev_kv": "bool"}'
          *   Renew data => curl http://localhost:2379/v3beta/kv/put -X POST -d '{"key": "KEY", "value": "", "prev_kv": "bool", "ignore_lease": true}'
          *       # Response {"header":{...}, "prev_kv": {...}}
          *   Delete data => curl http://localhost:2379/v3beta/kv/deleterange -X POST -d '{"key": "KEY", "range_end": "", "prev_kv": "bool"}'
          *       # Response {"header":{...}, "deleted": "number", "prev_kvs": [{...}]}
          *
          *   Watch => curl http://localhost:2379/v3beta/watch -XPOST -d '{"create_request":  {"key": "WATCH KEY", "range_end": "", "prev_kv": true} }'
          *       # Response {"header":{...},"watch_id":"ID","created":"bool", "canceled": "bool", "compact_revision": "REVISION", "events": [{"type":
          *                  "PUT=0|DELETE=1", "kv": {...}, prev_kv": {...}"}]}
          *
          *   Allocate Lease => curl http://localhost:2379/v3beta/lease/grant -XPOST -d '{"TTL": 5, "ID": 0}'
          *       # Response {"header":{...},"ID":"ID","TTL":"5"}
          *   Keepalive Lease => curl http://localhost:2379/v3beta/lease/keepalive -XPOST -d '{"ID": 0}'
          *       # Response {"header":{...},"ID":"ID","TTL":"5"}
          *   Revoke Lease => curl http://localhost:2379/v3beta/kv/lease/revoke -XPOST -d '{"ID": 0}'
          *       # Response {"header":{...}}
          *
          *   List members => curl http://localhost:2379/v3beta/cluster/member/list -XPOST -d '{}'
          *       # Response {"header":{...},"members":[{"ID":"ID","name":"NAME","peerURLs":["peer url"],"clientURLs":["client url"]}]}
          *
          *   Authorization Header => curl -H "Authorization: TOKEN"
          *   Authorization => curl http://localhost:2379/v3beta/auth/authenticate -XPOST -d '{"name": "username", "password": "pass"}'
          *       # Response {"header":{...}, "token": "TOKEN"}
          *       # Return 401 if auth token invalid
          *       # Return 400 with {"error": "etcdserver: user name is empty", "code": 3} if need TOKEN
          *       # Return 400 with {"error": "etcdserver: authentication failed, ...", "code": 3} if username of password invalid
          *   Authorization Enable:
          *       curl -L http://127.0.0.1:2379/v3beta/auth/user/add -XPOST -d '{"name": "root", "password": "3d91123233ffd36825bf2aca17808bfe"}'
          *       curl -L http://127.0.0.1:2379/v3beta/auth/role/add -XPOST -d '{"name": "root"}'
          *       curl -L http://127.0.0.1:2379/v3beta/auth/user/grant -XPOST -d '{"user": "root", "role": "root"}'
          *       curl -L http://127.0.0.1:2379/v3beta/auth/enable -XPOST -d '{}'
          */

#define ETCD_API_V3_ERROR_HTTP_CODE_AUTH 401
#define ETCD_API_V3_ERROR_HTTP_INVALID_PARAM 400
#define ETCD_API_V3_ERROR_HTTP_PRECONDITION 412
// @see https://godoc.org/google.golang.org/grpc/codes
#define ETCD_API_V3_ERROR_GRPC_CODE_UNAUTHENTICATED 16

#define ETCD_API_V3_MEMBER_LIST "/v3beta/cluster/member/list"
#define ETCD_API_V3_AUTH_AUTHENTICATE "/v3beta/auth/authenticate"

#define ETCD_API_V3_KV_GET "/v3beta/kv/range"
#define ETCD_API_V3_KV_SET "/v3beta/kv/put"
#define ETCD_API_V3_KV_DELETE "/v3beta/kv/deleterange"

#define ETCD_API_V3_WATCH "/v3beta/watch"

#define ETCD_API_V3_LEASE_GRANT "/v3beta/lease/grant"
#define ETCD_API_V3_LEASE_KEEPALIVE "/v3beta/lease/keepalive"
#define ETCD_API_V3_LEASE_REVOKE "/v3beta/kv/lease/revoke"

namespace details{
    const std::string &get_default_user_agent() {
        static std::string ret;
        if (!ret.empty()) {
            return ret;
        }

        char        buffer[256] = {0};
        const char *prefix      = "Mozilla/5.0";
        const char *suffix      = "Etcdcli/1.0";
#if defined(_WIN32) || defined(__WIN32__)
        #if (defined(__MINGW32__) && __MINGW32__)
                const char *sys_env = "Win32; MinGW32";
#elif (defined(__MINGW64__) || __MINGW64__)
                const char *sys_env = "Win64; x64; MinGW64";
#elif defined(__CYGWIN__) || defined(__MSYS__)
#if defined(_WIN64) || defined(__amd64) || defined(__x86_64)
                const char *sys_env = "Win64; x64; POSIX";
#else
                const char *sys_env = "Win32; POSIX";
#endif
#elif defined(_WIN64) || defined(__amd64) || defined(__x86_64)
                const char *sys_env = "Win64; x64";
#else
                const char *sys_env = "Win32";
#endif
#elif defined(__linux__) || defined(__linux)
        const char *sys_env = "Linux";
#elif defined(__APPLE__)
        const char *sys_env = "Darwin";
#elif defined(__DragonFly__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__OpenBSD__) || defined(__NetBSD__)
                const char *sys_env = "BSD";
#elif defined(__unix__) || defined(__unix)
                const char *sys_env = "Unix";
#else
                const char *sys_env = "Unknown";
#endif

        std::snprintf(buffer, sizeof(buffer) - 1, "%s (%s) %s", prefix, sys_env, suffix);
        ret = &buffer[0];

        return ret;
    }
}


    etcd_cluster::etcd_cluster() : flags_(0), loop_(NULL) {
        conf_.http_cmd_timeout              = std::chrono::seconds(10);
        conf_.etcd_members_next_update_time = std::chrono::system_clock::from_time_t(0);
        conf_.etcd_members_update_interval  = std::chrono::minutes(5);
        conf_.etcd_members_retry_interval   = std::chrono::minutes(1);

        conf_.lease                      = 0;
        conf_.keepalive_next_update_time = std::chrono::system_clock::from_time_t(0);
        conf_.keepalive_timeout          = std::chrono::seconds(16);
        conf_.keepalive_interval         = std::chrono::seconds(5);
        conf_.path_node = "http://127.0.0.1:2379";

    }

    void etcd_cluster::init(evpp::EventLoop *loop) {
        loop_ = loop;
   }

    etcd_cluster::~etcd_cluster() {
        reset();
    }

    void etcd_cluster::reset() {
        flags_ = 0;

        conf_.http_cmd_timeout = std::chrono::seconds(10);
        conf_.authorization_header.clear();
        conf_.path_node.clear();
        conf_.etcd_members_next_update_time = std::chrono::system_clock::from_time_t(0);
        conf_.etcd_members_update_interval  = std::chrono::minutes(5);
        conf_.etcd_members_retry_interval   = std::chrono::minutes(1);

        conf_.lease                      = 0;
        conf_.keepalive_next_update_time = std::chrono::system_clock::from_time_t(0);
        conf_.keepalive_timeout          = std::chrono::seconds(16);
        conf_.keepalive_interval         = std::chrono::seconds(5);
}

     bool etcd_cluster::is_available() const {
         if (check_flag(flag_t::CLOSING)) {
             return false;
         }

         // empty other actions will be delayed
         if (conf_.path_node.empty()) {
             return false;
         }
         return true;
}

     void etcd_cluster::set_flag(etcdcli::etcd_cluster::flag_t::type f, bool v) {
         assert(0 == (f & (f - 1)));
         if (v == check_flag(f)) {
             return;
         }

         if (v) {
             flags_ |= f;
         } else {
             flags_ &= ~f;
         }
         //todo xxxxxx

         /*switch (f) {
             case flag_t::ENABLE_LEASE: {
                 if (v) {
                     create_request_lease_grant();
                 } else if (rpc_keepalive_) {
                     rpc_keepalive_->set_on_complete(NULL);
                     rpc_keepalive_->stop();
                     rpc_keepalive_.reset();
                 }
                 break;
             }
             default: { break; }
         }*/
}

    time_t etcd_cluster::get_http_timeout_ms() const {
        time_t ret = static_cast<time_t>(std::chrono::duration_cast<std::chrono::milliseconds>(get_conf_http_timeout()).count());
        if (ret <= 0) {
            ret = 30000; // 30s
        }

        return ret;
    }




    evpp::httpc::RequestPtr etcd_cluster::kv_get(const std::string &key, const std::string &range_end, int64_t limit,
                                                    int64_t revision) {
        std::stringstream ss;
        ss << conf_.path_node << ETCD_API_V3_KV_GET;

        rapidjson::Document doc;
        rapidjson::Value &  root = doc.SetObject();
        etcd_packet::pack_key_range(root, key, range_end, doc);
        doc.AddMember("limit", limit, doc.GetAllocator());
        doc.AddMember("revision", revision, doc.GetAllocator());
        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        std::string body(buffer.GetString(), buffer.GetSize());

        evpp::httpc::RequestPtr ret = std::make_shared<evpp::httpc::PostRequest>(loop_, ss.str(), body, evpp::Duration(evpp::Duration::kSecond*2));
        ret->AddHeader("User-Agent", details::get_default_user_agent());

        return ret;
}


      evpp::httpc::Request* etcd_cluster::kv_set(const std::string &key, const std::string &value, bool assign_lease,
                                                   bool prev_kv, bool ignore_value, bool ignore_lease) {
          std::stringstream ss;
          ss << conf_.path_node << ETCD_API_V3_KV_SET;

          rapidjson::Document doc;
          rapidjson::Value &  root = doc.SetObject();

          etcd_packet::pack_base64(root, "key", key, doc);
          etcd_packet::pack_base64(root, "value", value, doc);
          doc.AddMember("prev_kv", prev_kv, doc.GetAllocator());
          doc.AddMember("ignore_value", ignore_value, doc.GetAllocator());
          doc.AddMember("ignore_lease", ignore_lease, doc.GetAllocator());

          rapidjson::StringBuffer                    buffer;
          rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
          doc.Accept(writer);

          std::string body(buffer.GetString(), buffer.GetSize());

          evpp::httpc::Request* ret = new evpp::httpc::PostRequest(loop_, ss.str(), body,  evpp::Duration(evpp::Duration::kSecond*5));
          ret->AddHeader("User-Agent", details::get_default_user_agent());

          return ret;
}

      evpp::httpc::Request* etcd_cluster::watch(const std::string &key, const std::string &range_end,
                                                int64_t start_revision, bool prev_kv, bool progress_notify) {
          std::stringstream ss;
          ss << conf_.path_node << ETCD_API_V3_WATCH;

          rapidjson::Document doc;
          rapidjson::Value &  root = doc.SetObject();

          rapidjson::Value create_request(rapidjson::kObjectType);


          etcd_packet::pack_key_range(create_request, key, range_end, doc);
          if (prev_kv) {
              create_request.AddMember("prev_kv", prev_kv, doc.GetAllocator());
          }

          if (progress_notify) {
              create_request.AddMember("progress_notify", progress_notify, doc.GetAllocator());
          }

          if (0 != start_revision) {
              create_request.AddMember("start_revision", start_revision, doc.GetAllocator());
          }

          root.AddMember("create_request", create_request, doc.GetAllocator());

          rapidjson::StringBuffer                    buffer;
          rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
          doc.Accept(writer);

          std::string body(buffer.GetString(), buffer.GetSize());
          evpp::httpc::Request* ret = new evpp::httpc::PostRequest(loop_, ss.str(), body,  evpp::Duration(evpp::Duration::kSecond*5));
          ret->AddHeader("User-Agent", details::get_default_user_agent());
          ret->AddHeader("Connection","keep-alive");

          return ret;
}

}