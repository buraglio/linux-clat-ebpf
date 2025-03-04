#include <linux/bpf.h>
#include <linux/if_ether.h>
#include <linux/ip.h>
#include <linux/ipv6.h>
#include <linux/in.h>
#include <bpf/bpf_helpers.h>

#define NAT64_PREFIX 0x0064FF9B // 64:ff9b::/96 well-known prefix per RFC 8215, should probably be configurable eventually

SEC("xdp")
int xdp_clat_ipv4_to_ipv6(struct __sk_buff *skb) {
    void *data_end = (void *)(long)skb->data_end;
    void *data = (void *)(long)skb->data;
    struct ethhdr *eth = data;
    
    if ((void *)(eth + 1) > data_end)
        return XDP_DROP;
    
    if (eth->h_proto != __constant_htons(ETH_P_IP))
        return XDP_PASS; // Only process IPv4 packets

    struct iphdr *ip4 = (struct iphdr *)(eth + 1);
    if ((void *)(ip4 + 1) > data_end)
        return XDP_DROP;
    
    // Ensure enough space for IPv6 translation
    if ((void *)(ip4 + 1) + sizeof(struct ipv6hdr) - sizeof(struct iphdr) > data_end)
        return XDP_DROP;

    struct ipv6hdr *ip6 = (struct ipv6hdr *)ip4;

    // Convert IPv4 to IPv6 header, I think
    ip6->version = 6;
    ip6->priority = 0;
    ip6->flow_lbl[0] = 0;
    ip6->flow_lbl[1] = 0;
    ip6->flow_lbl[2] = 0;
    ip6->payload_len = htons(ntohs(ip4->tot_len) - sizeof(struct iphdr) + sizeof(struct ipv6hdr));
    ip6->nexthdr = ip4->protocol;
    ip6->hop_limit = ip4->ttl;

    // NAT64 IPv6 destination address (64:ff9b::/96 + IPv4 address)
    ip6->daddr.s6_addr32[0] = htonl(NAT64_PREFIX);
    ip6->daddr.s6_addr32[1] = 0;
    ip6->daddr.s6_addr32[2] = 0;
    ip6->daddr.s6_addr32[3] = ip4->daddr;

    // Translate source IPv4 to an IPv6 address
    ip6->saddr.s6_addr32[0] = 0;
    ip6->saddr.s6_addr32[1] = 0;
    ip6->saddr.s6_addr32[2] = htonl(0xFFFF0000);
    ip6->saddr.s6_addr32[3] = ip4->saddr;

    // Update Ethernet type
    eth->h_proto = __constant_htons(ETH_P_IPV6);

    return XDP_TX; // Transmit the modified packet
}

char _license[] SEC("license") = "GPL";
