//
// Created by tom on 19-7-12.
//

#include "etcd-def.h"
#include "rapidjson/document.h"
#include "etcd-packet.h"

int main() {
    etcdcli::etcd_response_header header;
    printf("sdasdasd:%ld \n", header.revision);
    rapidjson::Document doc;
    etcdcli::etcd_packet::parse_object(doc, "{\"a\":1}");
    printf("a:%d \n", doc["a"].GetInt());
}