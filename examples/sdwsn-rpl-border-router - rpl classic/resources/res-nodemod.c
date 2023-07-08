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
 *     	Node-modeification-details
 * \author
 *      Matthias Kovatsch <kovatsch@inf.ethz.ch>
 * \author
 *      Cristiano De Alti <cristiano_dealti@hotmail.com>
 *       Modified by Marcio Miguel <marcio.miguel@gmail.com>
 *      Modified by Chandula Ekanayake <chandulaed@gmail.com>
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "coap-engine.h"
#include "net/ipv6/uip.h"
#include "net/routing/rpl-classic/rpl.h"
#include "net/routing/rpl-classic/rpl-private.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "contiki.h"
#include "contiki-net.h"
#include "net/ipv6/uip-ds6-nbr.h"
#include "net/ipv6/uip-ds6-route.h"
#include "net/routing/routing.h"
#include "net/ipv6/uip.h"
#define DEBUG DEBUG_NONE

int count;

uint16_t ipaddr_last_chunk(const uip_ipaddr_t *addr, uint8_t *buffer);
static void node_mod_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer,
		uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);

PERIODIC_RESOURCE(res_node_mod, "title=\"Nodes List\";rt=\"Text\";obs",
		node_mod_handler, //get
		NULL,//post
		NULL,//put
		NULL,//delete
		120 * CLOCK_SECOND,
		res_periodic_handler);

uint16_t ipaddr_last_chunk(const uip_ipaddr_t *addr, uint8_t *buffer) {
	uint16_t a, n;
	n = 0;
	a = (addr->u8[14] << 8) + addr->u8[15]; //only the end of address
	n += sprintf((char *)&buffer[n], "n%d", a);
	return n;
}

static void
node_mod_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /*
   * For minimal complexity, request query and options should be ignored for GET on observable resources.
   * Otherwise the requests must be stored with the observer list and passed by coap_notify_observers().
   * This would be a TODO in the corresponding files in contiki/apps/erbium/!
   */

  	uip_ds6_route_t *r;
	volatile uint8_t i;
	uint16_t n = 0;
	const uip_ipaddr_t *myaddr;
    myaddr = &uip_ds6_if.addr_list[1].ipaddr;
	/* count the number of routes and return the total */
	count = uip_ds6_route_num_routes();

	
	printf("number of node : %d\n",count);

	i = 1;

	
	if(count > 0) {
		n += sprintf((char *)&(buffer[n]), "{\"node\":\"");
		coap_set_status_code(response, CHANGED_2_04);
		for (r = uip_ds6_route_head(); r != NULL;
				r = uip_ds6_route_next(r), i++) {	
			n += ipaddr_last_chunk(&r->ipaddr,&(buffer[n]));
			n += sprintf((char *)&(buffer[n]), ",");
		}
		n += ipaddr_last_chunk(myaddr,&(buffer[n])); // add self mote id
		n += sprintf(((char *)&buffer[n]), "\"}"); 
	} else {
		n += sprintf((char *)&(buffer[n]), "{\"noden\":\"");
		n += sprintf(((char *)&buffer[n]), "\"}");
	} 
  coap_set_header_content_format(response,APPLICATION_JSON);
  coap_set_header_max_age(response,res_node_mod.periodic->period / CLOCK_SECOND);
  printf("send responce");
  printf("%s",(char *)buffer);
  coap_set_payload(response, buffer , strlen((char *)buffer));

  /* The coap_subscription_handler() will be called for observable resources by the coap_framework. */
}

/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the coap_manager process with the defined period.
 */
static void
res_periodic_handler()
{
 if(1) {
		if(count != uip_ds6_route_num_routes()) {
			printf("nofity");
			/* Notify the registered observers which will trigger the res_get_handler to create the response. */
			coap_notify_observers(&res_node_mod);//REST.notify_subscribers(&res_node_mod);
		}
	}
}

