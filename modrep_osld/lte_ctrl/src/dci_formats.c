#include <stdio.h>

#include "dci_formats.h"

const int ambiguous_sizes[] = {12,14,16,20,24,26,32,40,44,56};
#define NOF_AMBIGUOUS_SIZES 10

void pack_bits(unsigned int value, char **bits, int nof_bits)
{
    int i;

    for(i=0; i<nof_bits; i++) {
        (*bits)[i] = (value >> (nof_bits-i-1)) & 0x1;
    }
    *bits += nof_bits;
}

unsigned int unpack_bits(char **bits, int nof_bits)
{
    int i;
    unsigned int value=0;

    for(i=0; i<nof_bits; i++) {
    	value |= (*bits)[i] << (nof_bits-i-1);
    }
    *bits += nof_bits;
}

int check_ambiguous_size(char **bits, int nof_bits) {
	int i;
	for (i=0;i<NOF_AMBIGUOUS_SIZES;i++) {
		if (nof_bits == ambiguous_sizes[i]) {
			printf("AMBIGUOUS SIZE %d. Correcting...\n",nof_bits);
			pack_bits(0,bits,1);
			return nof_bits+1;
		}
	}
	return nof_bits;
}


int dci_format1_pack(char *packet, struct dci_format1 *data) {
	int i;
	char *buffer = packet;
	int total_len;

	if (data->carrier_indicator_len < 0 || data->carrier_indicator_len > 3) {
		return -1;
	}
	if (data->harq_pnum_len != 3 && data->harq_pnum_len != 4) {
		return -1;
	}

	if (data->carrier_indicator_len) {
		pack_bits(data->carrier_indicator,&buffer,
				data->carrier_indicator_len);
	}
	if (data->ra_type == TYPE0) {
		pack_bits(0,&buffer,1);
	} else if (data->ra_type == TYPE1) {
		pack_bits(1,&buffer,1);
	}
	if (data->ra_type == NA || data->ra_type == TYPE0) {
		for (i=0;i<data->nof_rbg;i++) {
			pack_bits(data->rbg_mask[i],&buffer,1);
		}
	} else {
		printf("TYPE1 RA not implemented\n");
		return -1;
	}
	pack_bits(data->mcs,&buffer,5);

	pack_bits(data->harq_pnum,&buffer,data->harq_pnum_len);
	pack_bits(data->new_data,&buffer,1);
	pack_bits(data->redundancy_version,&buffer,2);
	pack_bits(data->tpc_command,&buffer,2);

	total_len = (int) (buffer-packet);

	/* TODO:
	 * If the number of information bits in format 1 is equal to that for format 0/1A
	 * for scheduling the same serving cell and mapped onto the UE specific search space
	 * given by the C-RNTI as defined in [3], one bit of value zero shall be appended to format 1.
	 */

	total_len = check_ambiguous_size(&buffer,total_len);

	return total_len;
}

int dci_packet_space(char *packet, char *buffer, int packet_len) {
	return ((int) (buffer-packet) - packet_len);
}

int dci_format1_unpack(char *packet, int packet_len, struct dci_format1 *data) {
	int i;
	char *buffer = packet;
	int total_len;


	if (data->carrier_indicator_len < 0 || data->carrier_indicator_len > 3) {
		return -1;
	}
	if (data->harq_pnum_len != 3 && data->harq_pnum_len != 4) {
		return -1;
	}

	if (dci_packet_space(packet,buffer,packet_len) < data->carrier_indicator_len) {
		return -1;
	}

	if (data->carrier_indicator_len) {
		data->carrier_indicator = unpack_bits(&buffer,
				data->carrier_indicator_len);
	}

	if (dci_packet_space(packet,buffer,packet_len) < 1) {
		return -1;
	}

	if (data->ra_type != NA) {
		if (unpack_bits(&buffer,1)) {
			data->ra_type = TYPE1;
		} else {
			data->ra_type = TYPE0;
		}
	}

	if (dci_packet_space(packet,buffer,packet_len) < data->nof_rbg) {
		return -1;
	}

	if (data->ra_type == NA || data->ra_type == TYPE0) {
		for (i=0;i<data->nof_rbg;i++) {
			data->rbg_mask[i] = unpack_bits(&buffer,1);
		}
	} else {
		printf("TYPE1 RA not implemented\n");
		return -1;
	}
	if (dci_packet_space(packet,buffer,packet_len) < 5) {
		return -1;
	}
	data->mcs = unpack_bits(&buffer,5);
	if (dci_packet_space(packet,buffer,packet_len) < data->harq_pnum_len) {
		return -1;
	}
	data->harq_pnum = unpack_bits(&buffer,data->harq_pnum_len);
	if (dci_packet_space(packet,buffer,packet_len) < 5) {
		return -1;
	}
	data->new_data = unpack_bits(&buffer,1);
	data->redundancy_version = unpack_bits(&buffer,2);
	data->tpc_command = unpack_bits(&buffer,2);

	total_len = (int) (buffer-packet);

	/* TODO:
	 * If the number of information bits in format 1 is equal to that for format 0/1A
	 * for scheduling the same serving cell and mapped onto the UE specific search space
	 * given by the C-RNTI as defined in [3], one bit of value zero shall be appended to format 1.
	 */

	total_len = check_ambiguous_size(&buffer,total_len);

	return total_len;
}
