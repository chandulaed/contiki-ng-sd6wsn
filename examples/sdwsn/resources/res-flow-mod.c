/*
 * Copyright (c) 2013, Institute for Pervasive Computing, ETH Zurich
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *      Change flow and request flow
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 * \author
 *      Cristiano De Alti <cristiano_dealti@hotmail.com>
 * 
 *      Marcio Miguel <marcio.miguel@gmail.com> based on examples written
 *      by Matthias Kovatsch <kovatsch@inf.ethz.ch>
 *      modified by Chandula Ekanayake<chandulaed@gmail.com>
 */

#include "contiki.h"
#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include "net/ipv6/uip-debug.h"
#include "net/ipv6/res-flow-mod.h"
#include "net/ipv6/uip-ds6.h"
#include "lib/list.h"
#include "lib/memb.h"
#define FLOW_TABLE_SIZE 30
#define NO_FLOW_ENTRIES 20
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

flow_s flow_table[FLOW_TABLE_SIZE];
uint8_t table_entries = 0;
uip_ipaddr_t tmp_addr;
flow_entry_s linked_table;
uint8_t table_pos;
uint8_t table_index;
uint8_t noflow_packet_count = 0;
uint8_t current_packet_count;
uint8_t noflow_packet_srcaddr[NO_FLOW_ENTRIES];
uint8_t noflow_packet_dstaddr[NO_FLOW_ENTRIES];
uint16_t noflow_packet_srcport[NO_FLOW_ENTRIES];
uint16_t noflow_packet_dstport[NO_FLOW_ENTRIES];




static void packet_in_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_packet_in_handler(void);


PERIODIC_RESOURCE(res_packet_in, "title=\"Packet-in\";rt=\"Text\";obs",
		packet_in_handler, //get
		NULL,//post
		NULL,//put
		NULL,//delete
		60*CLOCK_SECOND,
		res_periodic_packet_in_handler);

/*get next hop from flow table*/
uip_ipaddr_t * get_next_hop_by_flow(uip_ip6addr_t *srcaddress,uip_ip6addr_t *dstaddress,
		uint16_t *srcport,uint16_t *dstport,uint8_t *proto){
	
 	 table_pos = 0;
  
  	if(*dstport == 5683 || *srcport == 5683 ) {
		return NULL;
	}

	while(table_pos<=table_entries){
		printf("loop ");
		LOG_INFO_6ADDR(&flow_table[table_pos].ipv6dst);
		printf("\n");
		if(uip_ipaddr_cmp(dstaddress,&flow_table[table_pos].ipv6dst)) {
			if(uip_ipaddr_cmp(srcaddress,&flow_table[table_pos].ipv6src)){
							printf("flow found !\n");
							printf("get_next_hop_by_flow ipv6dst: ");
							LOG_INFO_6ADDR(&flow_table[table_pos].ipv6dst);
							printf("\n");
							break;
			}
		}
		table_pos++;
	}
/*
	printf("number of table_entries: %d\n",table_entries);
	while(table_pos<=table_entries){
		if(uip_ipaddr_cmp(dstaddress,&flow_table[table_pos].ipv6dst)) {
			if(uip_ipaddr_cmp(srcaddress,&flow_table[table_pos].ipv6src)){
				if(*srcport == flow_table[table_pos].srcport
						|| &flow_table[table_pos].srcport == NULL ){
					if(*dstport == flow_table[table_pos].dstport
							|| &flow_table[table_pos].dstport == NULL){
						if(*proto == flow_table[table_pos].ipproto
								|| &flow_table[table_pos].ipproto == NULL){
							printf("flow found !\n");

							break;
						}
					}
				}
			}
		}
		table_pos++;
	}
*/
	
	

  if(table_pos>table_entries) {
		printf("no record found\n");
		noflow_packet_srcaddr[noflow_packet_count] = srcaddress->u8[15];
		noflow_packet_dstaddr[noflow_packet_count] = dstaddress->u8[15];
		noflow_packet_srcport[noflow_packet_count] = (uint16_t)*srcport;
		noflow_packet_dstport[noflow_packet_count] = (uint16_t)*dstport;
		noflow_packet_count++;
    return NULL;
    	}else {
		if(flow_table[table_pos].action == 0 ) { // action = forward
			printf("next hop returned: ");
			LOG_INFO_6ADDR(&flow_table[table_pos].nhipaddr);
			LOG_INFO_6ADDR(&flow_table[table_pos].ipv6dst);
			printf("\n");

			return (&flow_table[table_pos].nhipaddr);
		} else{
				return NULL; // action = drop
			}
		}
	}


static void
flow_mod_handler(coap_message_t  *request, coap_message_t  *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset);

RESOURCE(res_flow_mod, "title=\"Flow-mod\";rt=\"Text\"",
		NULL, //get
		flow_mod_handler,//post
		NULL,//put
		NULL);//delete

static void
flow_mod_handler(coap_message_t  *request, coap_message_t *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset) {

			uint8_t flowmodpos = 0;
			const char *str = NULL;
			uint8_t len = 0;
			uip_ipaddr_t tmp_src_addr;
			uip_ipaddr_t tmp_dst_addr;
			uip_ipaddr_t nxhopaddr;
			uint8_t existing_flow = 0;
			uint8_t action;

			len = coap_get_header_uri_query(request, &str);
			printf("option query=%s, length=%d\n",str,len);
			snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE - 1, "%.*s", len, str);
			printf("option len and query=%s",(char*)buffer);
			printf("Query-all: %s\n", buffer);

			if ((len = coap_get_post_variable(request, "s", &str))) {
			snprintf((char *) buffer, COAP_MAX_CHUNK_SIZE - 1, "%.*s", len, str);
			printf("src addr = %s\n",(char *)buffer);
			uiplib_ip6addrconv((char *)buffer, &tmp_src_addr);
			}
			if ((len = coap_get_post_variable(request, "d", &str))) {
			snprintf((char *) buffer, COAP_MAX_CHUNK_SIZE - 1, "%.*s", len, str);
			printf("dst addr = %s\n",(char *)buffer);
			uiplib_ip6addrconv((char *)buffer, &tmp_dst_addr);	
			}
			if ((len = coap_get_post_variable(request, "ac", &str))) {
			snprintf((char *) buffer, COAP_MAX_CHUNK_SIZE - 1, "%.*s", len, str);
			action=atoi((char *)buffer);
			LOG_INFO("action = %d\n",action);
			}
			if ((len = coap_get_post_variable(request, "n", &str))) {
			snprintf((char *) buffer, COAP_MAX_CHUNK_SIZE - 1, "%.*s", len, str);
			LOG_INFO("nxhopaddr: %s\n",(char *)buffer);
			uiplib_ip6addrconv((char *)buffer, &nxhopaddr);
			}else{
				LOG_INFO("NO NXHOP ADDR");
			}


				while(flowmodpos<=table_entries){
					printf("loop ");
					LOG_INFO_6ADDR(&flow_table[flowmodpos].ipv6dst);
					printf("\n");
					if(uip_ipaddr_cmp(&tmp_dst_addr,&flow_table[flowmodpos].ipv6dst)) {
						if(uip_ipaddr_cmp(&tmp_src_addr,&flow_table[flowmodpos].ipv6src)){
							printf("flow found !\n");
							printf("get_next_hop_by_flow ipv6dst: ");
							LOG_INFO_6ADDR(&flow_table[flowmodpos].ipv6dst);
							printf("\n");
							break;
			}
		}
		flowmodpos++;
		}
			printf("is flow exist = %d\n",existing_flow);

			len = coap_get_post_variable(request,"o",&str);
			snprintf((char *)buffer, COAP_MAX_CHUNK_SIZE - 1, "%.*s", len, str);
			printf("operation: %s\n",(char *) buffer);

			if ((char)buffer[0] == 'i') {
				if(table_entries<FLOW_TABLE_SIZE){
					LOG_INFO("Insert operation selected\n");
					flow_table[flowmodpos].action=action;
					flow_table[flowmodpos].ipv6dst=tmp_dst_addr;
					flow_table[flowmodpos].ipv6src=tmp_src_addr;
					flow_table[flowmodpos].nhipaddr=nxhopaddr;
					flow_table[flowmodpos].ttl=900;
					table_entries++;
				}else{
					LOG_ERR("FLOW TABLE FULL");
				}
				
			}


printf("%c", buffer[0]);
if ((char) buffer[0] == 'd') {  //operation flow delete
	/*	if ((len = coap_get_post_variable(request, "flowid", &str))) {
			snprintf((char *) buffer, COAP_MAX_CHUNK_SIZE - 1, "%.*s", len, str);
			flowid_temp=atoi((char *)buffer);
		}
		

		printf("operation = delete, flowid = %d",flowid_temp);
		*/
	/*
		while(table_index<=table_entries){  // find the entry
			printf("ti=%d",flow_table[table_index].flowid);
			if(flowid_temp == flow_table[table_index].flowid ) {
				printf("delete:flowid entry found!\n");
				while(table_index <= table_entries){ // shift up the rest of entries
					flow_table[table_index] = flow_table[table_index + 1];
					table_index++;
			    }
				table_entries--;
				break;
			}
		table_index++;
		}
	}
	*/
}
		}

void packet_in_handler(coap_message_t* request, coap_message_t* response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset) {
	
	const uip_ipaddr_t *addr;
	addr = &uip_ds6_if.addr_list[1].ipaddr;

	uint16_t n = 0;
	printf("handler src-dst-address:%d %d\n", noflow_packet_srcaddr[current_packet_count],
			noflow_packet_dstaddr[current_packet_count]);
	n += sprintf((char *)&(buffer[n]), "{\"sn\":\"n%x\",",((addr->u8[14] << 8) + addr->u8[15]));
	n += sprintf((char *)&(buffer[n]), "\"pn\":{");
	n += sprintf((char *)&(buffer[n]),"\"saddr\":\"%x\",\"daddr\":\"%x\"}}",
			noflow_packet_srcaddr[current_packet_count],
			noflow_packet_dstaddr[current_packet_count]);
	coap_set_header_content_format(response, APPLICATION_JSON);
	printf("%s \n ",(char *)buffer);
	printf("noflow packets : %d \n",noflow_packet_count);
	coap_set_header_max_age(response, res_packet_in.periodic->period / CLOCK_SECOND);
	coap_set_payload(response, buffer, n);
}
    
static void
res_periodic_packet_in_handler()
{
	if(1) {
		//PRINTF("packet_in_periodic_handler\n");
		/* Notify the registered observers which will trigger the
		 *  res_get_handler to create the response. */
		//printf("noflow packets 1: %d \n",noflow_packet_count);
		while(noflow_packet_count>0) {
			printf("noflow packets 2: %d \n",noflow_packet_count);
			current_packet_count = noflow_packet_count - 1 ;
			coap_notify_observers(&res_packet_in);
			noflow_packet_count--;
		}
	}
}
