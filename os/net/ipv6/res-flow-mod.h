
#include "lib/list.h"
typedef struct flow_s {
	uip_ipaddr_t ipv6src;
	uip_ipaddr_t ipv6dst;
	uint8_t action;
	uip_ipaddr_t nhipaddr;
	int ttl;
}flow_s;

typedef struct flow_entry_s{
	struct flow_entry_s *next;
	uip_ipaddr_t ipv6src;
	uip_ipaddr_t ipv6dst;
	uint16_t srcport;
	uint16_t dstport;
	uint8_t action;
	uip_ipaddr_t nhipaddr;
	uint16_t ttl;
} flow_entry_s;

uip_ipaddr_t * get_next_hop_by_flow(uip_ipaddr_t *srcaddress,uip_ipaddr_t *dstaddress,uint16_t *srcport,uint16_t *dstport,uint8_t *proto);
