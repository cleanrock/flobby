#include "UserId.h"

#include <boost/functional/hash.hpp>

#include <ifaddrs.h>
#include <netpacket/packet.h>
#include <net/if_arp.h>


static std::size_t hash_ = 0;

uint32_t UserId::get()
{
    if (hash_ == 0)
    {
        struct ifaddrs *ifaddrs;

        if (::getifaddrs(&ifaddrs) == 0)
        {
            struct ifaddrs *ifa;

            for (ifa = ifaddrs; ifa != NULL; ifa = ifa->ifa_next) {
                 if (ifa->ifa_addr == NULL)
                     continue;

                 if (ifa->ifa_name != 0 && ifa->ifa_addr->sa_family == AF_PACKET)
                 {
                     struct sockaddr_ll *s = (struct sockaddr_ll*)ifa->ifa_addr;
                     if (s->sll_halen != 6 || s->sll_hatype != ARPHRD_ETHER)
                         continue;

                     boost::hash_range(hash_, s->sll_addr, s->sll_addr+6);

//                     char buf[100];
//                     int len = 0;
//                     int chksum = 0;
//                     for (int i=0; i<6; ++i)
//                     {
//                         len += sprintf(buf+len,"%02X%s", s->sll_addr[i], i < 5 ? ":" : "");
//                         chksum += s->sll_addr[i];
//                     }
//                     LOG(ERROR)<< "HERE " << ifa->ifa_name << ", " << static_cast<int>(s->sll_hatype) << ", " << buf << ", " << chksum;
                 }
             }

             freeifaddrs(ifaddrs);
        }
    }

    return hash_;
}
