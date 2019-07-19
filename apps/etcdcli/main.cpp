//
// Created by tom on 19-7-12.
//

#include "etcd_def.h"
#include "rapidjson/document.h"
#include "etcd_packet.h"
#include "etcd_cluster.h"
#include "evpp/httpc/request.h"
#include "evpp/httpc/response.h"

int main(int argc, char* argv[]) {
    /*etcdcli::etcd_response_header header;
    printf("sdasdasd:%ld \n", header.revision);
    rapidjson::Document doc;
    etcdcli::etcd_packet::parse_object(doc, "{\"a\":1}");
    printf("a:%d \n", doc["a"].GetInt());*/
    google::InitGoogleLogging(argv[0]);
    FLAGS_stderrthreshold = 0;
    FLAGS_minloglevel=0;
    etcdcli::etcd_cluster cluster;
    evpp::EventLoop t;
    cluster.init(&t);



    t.RunAfter(evpp::Duration(2.0), [&cluster]()
    {
        auto req = cluster.watch("123");
        //req->AddHeader("connection", "close");
        req->Execute([](const std::shared_ptr<evpp::httpc::Response>& response){
            LOG_INFO << "http_code=" << response->http_code() << " [" << response->body().ToString() << "]";
            const char*  header = response->FindHeader("Connection");
            LOG_INFO << "HTTP HEADER Connection=" << header;
            LOG_INFO << "HTTP host=" <<  response->request()->host();
            delete response->request();

        });
    } );
    t.Run();
    LOG_INFO << "EventLoopThread stopped.";
    return 0;

}