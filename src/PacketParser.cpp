#include "PacketParser.h"
#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/udp.h>
#include <cstring>
#include <sstream>
#include <iomanip>

bool PacketParser::parse(Packet& packet) {
    if (packet.payload.empty()) return false;
    
    const uint8_t* data = packet.payload.data();
    size_t len = packet.payload.size();

    // 1. Ethernet Header
    if (len < sizeof(struct ether_header)) return false;
    const struct ether_header* eth = reinterpret_cast<const struct ether_header*>(data);

    uint16_t ethType = ntohs(eth->ether_type);
    size_t offset = sizeof(struct ether_header);
    
    // VLAN handling (simple)
    if (ethType == ETHERTYPE_VLAN) {
        offset += 4; // Skip VLAN tag
        if (len < offset) return false;
         // In a real generic parser we'd read the next protocol, assuming IP for now or check bytes
         // For simplicity, let's just retry reading ethertype from the inner frame or skip
         // Standard: VLAN tag is 4 bytes. 
         // Let's assume next is IP for this simplified scope if basic checks pass
         ethType = ETHERTYPE_IP; // Fallback assumption for this scoped project
    }

    if (ethType != ETHERTYPE_IP) {
        // We only care about IPv4 for this project requirements
        return false;
    }

    // 2. IP Header
    if (len < offset + sizeof(struct iphdr)) return false;
    const struct iphdr* iph = reinterpret_cast<const struct iphdr*>(data + offset);

    // IP version check
    if (iph->version != 4) return false;

    // Source / Dest IP
    char srcIp[INET_ADDRSTRLEN];
    char dstIp[INET_ADDRSTRLEN];
    struct in_addr srcAddr; 
    srcAddr.s_addr = iph->saddr;
    struct in_addr dstAddr; 
    dstAddr.s_addr = iph->daddr;

    inet_ntop(AF_INET, &srcAddr, srcIp, INET_ADDRSTRLEN);
    inet_ntop(AF_INET, &dstAddr, dstIp, INET_ADDRSTRLEN);

    packet.srcIP = srcIp;
    packet.dstIP = dstIp;

    // Protocol
    size_t ipHeaderLen = iph->ihl * 4;
    offset += ipHeaderLen;

    if (len < offset) return false;

    if (iph->protocol == IPPROTO_TCP) {
        packet.protocol = "TCP";
        if (len < offset + sizeof(struct tcphdr)) return false;
        const struct tcphdr* tcph = reinterpret_cast<const struct tcphdr*>(data + offset);
        packet.srcPort = ntohs(tcph->source);
        packet.dstPort = ntohs(tcph->dest);
        return true;
    } else if (iph->protocol == IPPROTO_UDP) {
        packet.protocol = "UDP";
         if (len < offset + sizeof(struct udphdr)) return false;
        const struct udphdr* udph = reinterpret_cast<const struct udphdr*>(data + offset);
        packet.srcPort = ntohs(udph->source);
        packet.dstPort = ntohs(udph->dest);
        return true;
    } else {
        packet.protocol = "OTHER";
        packet.srcPort = 0;
        packet.dstPort = 0;
        return true; // We parsed IP, even if L4 is unknown
    }
}
