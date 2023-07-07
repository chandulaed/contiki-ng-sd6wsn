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
 *      Observe ETX value
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
#include "net/routing/rpl-lite/rpl.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

typedef struct etx_s {
	uint16_t nbr_addr;
	uint16_t nbr_etx;
	rpl_parent_t * p;
}etx_s ;

etx_s etx_table[NBR_TABLE_CONF_MAX_NEIGHBORS]; // number of neighbors configured
uint8_t parent_index;

static void res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_periodic_handler(void);


PERIODIC_RESOURCE(res_etx,
		"title=\"etx\";obs",
		res_get_handler,
		NULL,
		NULL,
		NULL,
		90 * CLOCK_SECOND,
		res_periodic_handler);

static void
res_get_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  /*
   * For minimal complexity, request query and options should be ignored for GET on observable resources.
   * Otherwise the requests must be stored with the observer list and passed by coap_notify_observers().
   * This would be a TODO in the corresponding files in contiki/apps/erbium/!
   */

    rpl_dag_t *dag;
    rpl_parent_t *parent;
    int32_t strpos=0;
    const uip_ipaddr_t *addr;
    addr = &uip_ds6_if.addr_list[1].ipaddr;
    dag=rpl_get_any_dag();

    parent_index = 0;
  if (dag!=NULL){
    strpos += sprintf((char *)&(buffer[strpos]),"{\"node\":\"n%d\"",((addr->u8[14] << 8) + addr->u8[15])); // last addr byte of mote
		strpos += sprintf((char *)&(buffer[strpos]),",\"nbr\":{");
		parent = nbr_table_head(rpl_neighbors);  // addr of first neighbor
		while (parent != NULL)
		{
			etx_table[parent_index].nbr_addr = (rpl_parent_get_ipaddr(parent)->u8[14] << 8) + rpl_parent_get_ipaddr(parent)->u8[15];
			etx_table[parent_index].nbr_etx = rpl_neighbor_get_link_metric(parent);
			etx_table[parent_index].p = parent;
			strpos += sprintf((char *)&(buffer[strpos]),"\"n%d\":%u,",etx_table[parent_index].nbr_addr,
					etx_table[parent_index].nbr_etx);
			parent = nbr_table_next(rpl_neighbors, parent);
			parent_index++;
		}
  }

  else
  {
    strpos +=sprintf((char *)&(buffer[strpos]),"{}\n");
  }
  strpos += sprintf((char *)&(buffer[strpos-1]),"}}\n");
  coap_set_header_content_format(response,APPLICATION_JSON);
  coap_set_header_max_age(response,res_etx.periodic->period/CLOCK_SECOND);
  coap_set_payload(response, buffer, strlen((char *)buffer));

  /* The coap_subscription_handler() will be called for observable resources by the coap_framework. */
}

/*
 * Additionally, a handler function named [resource name]_handler must be implemented for each PERIODIC_RESOURCE.
 * It will be called by the coap_manager process with the defined period.
 */
static void
res_periodic_handler()
{
 	uint8_t parent_counter = 0;
	uint16_t etx_temp;
	//uint8_t etx_changed = 0;

	while(parent_counter < parent_index) {
		etx_temp = rpl_neighbor_get_link_metric(etx_table[parent_counter].p);
		LOG_INFO("parent: %d ",etx_table[parent_counter].nbr_addr);
		LOG_INFO("etx_temp:%d\n",etx_temp);
		if(etx_temp > etx_table[parent_counter].nbr_etx * 2
				|| etx_temp < etx_table[parent_counter].nbr_etx / 2 ) {
					etx_table[parent_counter].nbr_etx = etx_temp ;
					//etx_changed = 1;
		}
		parent_counter++;
	}
	
	/* Notify the registered observers which will trigger the res_get_handler to create the response. */
	if (0) {
		LOG_INFO("etx_changed !\n");
		coap_notify_observers(&res_etx);
  }
}


