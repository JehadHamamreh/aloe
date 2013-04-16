/* 
 * Copyright (c) 2012, Ismael Gomez-Miguelez <ismael.gomez@tsc.upc.edu>.
 * This file is part of ALOE++ (http://flexnets.upc.edu/)
 * 
 * ALOE++ is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * ALOE++ is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with ALOE++.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <oesr.h>
#include <rtdal.h>
#include <rtdal_datafile.h>
#include <params.h>
#include <skeleton.h>
#include <complex.h>
#include "udp_sink.h"

static int fd;
struct sockaddr_in servaddr;

pmid_t blen_id;

int create_client_upd(char *address, int port, struct sockaddr_in *server) {
	int sockfd,n;

	modinfo_msg("Opening socket to %s:%d\n",address,port);

	sockfd=socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if (sockfd == -1) {
		perror("socket");
		return -1;
	}
	bzero(server,sizeof(struct sockaddr_in));
	server->sin_family = AF_INET;
	server->sin_addr.s_addr=inet_addr(address);
	server->sin_port=htons(port);

	return sockfd;

}


/**
 * @ingroup udp_sink
 *
 * \param address IP address to connect
 * \param port	Port to connect
 */
int initialize() {
	char address[64];
	var_t pm;
	int i;
	int port;

	blen_id = param_id("nof_pkts");

	if (param_get_int_name("port",&port)) {
		moderror("Port undefined\n");
		return -1;
	}

#ifdef _COMPILE_ALOE
	pm = oesr_var_param_get(ctx, "address");
	if (!pm) {
		moderror("Parameter file_name undefined\n");
		return -1;
	}

	if (oesr_var_param_get_value(ctx, pm, address, 64) == -1) {
		moderror("Error getting file_name value\n");
		return -1;
	}
#endif

	fd = create_client_upd(address,port,&servaddr);
	if (fd == -1) {
		moderror("Creating socket\n");
		return -1;
	}
	modinfo_msg("sock=%d\n",fd);

	return 0;
}

/**
 * @ingroup udp_sink
 *
 *  Writes the received samples to the dac output buffer
 *
 */
int work(void **inp, void **out) {
	int rcv_samples;
	int n;
	char tmp[16];
	int nof_pkts,pkt_len,i;
	input_t *input = inp[0];

	rcv_samples = get_input_samples(0);
	if (!rcv_samples) {
		return 0;
	}

	nof_pkts=1;
	param_get_int(blen_id,&nof_pkts);

	if (rcv_samples % nof_pkts) {
		modinfo_msg("Warning received packet len %d not multiple of %d packets\n",
				rcv_samples,nof_pkts);
	}
	pkt_len = rcv_samples/nof_pkts;

	modinfo_msg("Sending %d packets of %d bytes\n",nof_pkts,pkt_len);

	for (i=0;i<nof_pkts;i++) {
		n = sendto(fd,&input[i*pkt_len],pkt_len,0,(struct sockaddr *) &servaddr, sizeof (servaddr));
		if (n == -1) {
			perror("sendto");
			return -1;
		}
	}

	return 0;
}

/**  Deallocates resources created during initialize().
 * @return 0 on success -1 on error
 */
int stop() {
	close(fd);
	return 0;
}

