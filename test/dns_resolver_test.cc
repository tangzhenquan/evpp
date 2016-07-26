#include "test_common.h"

#include <evpp/event_loop.h>
#include <evpp/event_loop_thread.h>
#include <evpp/dns_resolver.h>
#include <atomic>

namespace {
static bool resolved = false;
static void OnResolved(const std::vector <struct in_addr>& addrs) {
    resolved = true;
}
}


TEST_UNIT(testDNSResolver) {
    evpp::Duration delay(double(1.0)); // 1s
    std::unique_ptr<evpp::EventLoopThread> t(new evpp::EventLoopThread);
    t->Start(true);
    evpp::DNSResolver dns_resolver(t->event_loop(), "www.so.com", evpp::Duration(1.0), &OnResolved);
    dns_resolver.Start();

    while (!resolved) {
        usleep(1);
    }

    t->Stop(true);
    t.reset();
    H_TEST_ASSERT(evpp::GetActiveEventCount() == 0);
}
