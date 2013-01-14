/** @file simple_client.c
 *
 * @brief This simple client demonstrates the most basic features of JACK
 * as they would be used by many applications.
 */

#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#include "set.h"
#include "str.h"
#include "cfg_parser.h"

jack_port_t *input_port;
jack_port_t *output_port;
jack_client_t *client;

const double PI = 3.14;

typedef jack_default_audio_sample_t sample_t;

int semid, semidx;


/*one cycle of our sound*/
float *output, *input;
/*samples in cycle*/
jack_nframes_t samincy;
/*the current offset*/
long offset = 0;

/*frequency of our sound*/
int tone = 492;

void (*syncFunction)(void);

/**
 * The process callback for this JACK application is called in a
 * special realtime thread once for each audio cycle.
 *
 * This client does nothing more than copy data from its input
 * port to its output port. It will exit when stopped by
 * the user (e.g. using Ctrl-C on a unix-ish operating system)
 */
int process(jack_nframes_t nframes, void *arg) {
	/*grab our output buffer*/
	sample_t *out = (sample_t *) jack_port_get_buffer(output_port, nframes);
	sample_t *in = (sample_t *) jack_port_get_buffer(input_port, nframes);

	memcpy(out, output, sizeof(float) * nframes);
	memcpy(input, in, sizeof(float) * nframes);

	syncFunction();
	return 0;

}

int bufferchg(jack_nframes_t nframes, void *arg) {
	printf("Buffer size is %lu\n", nframes);
	return 0;
}

int srate(jack_nframes_t nframes, void *arg) {
	printf("the sample rate is now %lu/sec\n", nframes);
	return 0;
}

void error(const char *desc) {
	fprintf(stderr, "JACK error: %s\n", desc);
}

/**
 * JACK calls this shutdown_callback if the server ever shuts down or
 * decides to disconnect the client.
 */
void jack_shutdown(void *arg) {
	printf("CAUTINO: Jack server shut down!!\n");
}

char filepath[200];

int SamplingFrequency;
int NofSamples;

int sndcard_readcfg(char *name, int *NsamplesIn, int *NsamplesOut) {
	int i, j, offset;
	char *buffer;
	cfg_o cfg;
	sect_o sect;
	key_o freq,nsamples;

	sprintf(filepath, "/usr/local/etc/%s", name);

	hwapi_mem_silent(1);
	offset = hwapi_res_parseall_(filepath, &buffer,1);
	if (offset < 0) {
		printf ("Error reading file %s\n", filepath);
		return 0;
	}

	cfg = cfg_new(buffer, offset);
	if (!cfg) {
		printf ("Error parsing file %s\n", filepath);
		free(buffer);
		return 0;
	}
	
	sect = Set_get(cfg_sections(cfg),0);
	if (!sect) {
		printf("Error can't find any section\n");
		free(buffer);
		return 0;
	}	
	
	if (strcmp(sect_title(sect), "main")) {
		printf ("Error unknown section %s\n", sect_title (sect));
		free(buffer);
		return 0;
	}
	
	freq=Set_find(sect_keys(sect),"sampling_freq",key_findname);
	nsamples=Set_find(sect_keys(sect),"nof_samples",key_findname);
	if (!freq) {
		printf("Error can't find sampling_freq field\n");
		free(buffer);
		return 0;
	}
	if (!nsamples) {
		printf("Error can't find nof_samples field\n");
		free(buffer);
		return 0;
	}
	key_value(freq,0,PARAM_INT,&SamplingFrequency);	
	key_value(nsamples,0,PARAM_INT,&NofSamples);	
	
	*NsamplesIn=NofSamples;
	*NsamplesOut=NofSamples;

	free(buffer);

	cfg_delete(&cfg);

	return 1;
	
}

int sndcard_init(int *ts_len, float *rxBuffer, float *txBuffer, void (*sync)(void)) {
	const char **ports;
	const char *client_name = "simple";
	const char *server_name = NULL;
	jack_options_t options = JackNullOption;
	jack_status_t status;

	syncFunction=sync;

	/* open a client connection to the JACK server */

	client = jack_client_open(client_name, options, &status, server_name);
	if (client == NULL) {
		fprintf(stderr, "jack_client_open() failed, "
			"status = 0x%2.0x\n", status);
		if (status & JackServerFailed) {
			fprintf(stderr, "Unable to connect to JACK server\n");
		}
		return -1;
	}
	if (status & JackServerStarted) {
		fprintf(stderr, "JACK server started\n");
	}
	if (status & JackNameNotUnique) {
		client_name = jack_get_client_name(client);
		fprintf(stderr, "unique name `%s' assigned\n", client_name);
	}

	/* tell the JACK server to call `process()' whenever
	 there is work to be done.
	 */

	jack_set_process_callback(client, process, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	 it ever shuts down, either entirely, or if it
	 just decides to stop calling us.
	 */

	jack_on_shutdown(client, jack_shutdown, 0);

	jack_set_error_function(error);

	/* display the current sample rate.
	 */

	printf("engine sample rate: %" PRIu32 "\n", jack_get_sample_rate(client));

	SamplingFrequency = (int) jack_get_sample_rate(client);
	*ts_len=1000000*NofSamples/SamplingFrequency;
	
	jack_set_sample_rate_callback(client, srate, 0);

	jack_set_buffer_size_callback(client, bufferchg, 0);

	if (jack_set_buffer_size(client,NofSamples)) {
		 printf("error setting buffer size to %d\n",NofSamples);
		 return -1;
	 }


	/* create two ports */

	input_port = jack_port_register(client, "input", JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsInput, 0);
	output_port = jack_port_register(client, "output", JACK_DEFAULT_AUDIO_TYPE,
			JackPortIsOutput, 0);

	if ((input_port == NULL) || (output_port == NULL)) {
		fprintf(stderr, "no more JACK ports available\n");
		return -1;
	}

	input = rxBuffer;
	output = txBuffer;

	/* Tell the JACK server that we are ready to roll.  Our
	 * process() callback will start running now. */

	if (jack_activate(client)) {
		fprintf(stderr, "cannot activate client");
		return -1;
	}

	/* Connect the ports.  You can't do this before the client is
	 * activated, because we can't make connections to clients
	 * that aren't running.  Note the confusing (but necessary)
	 * orientation of the driver backend ports: playback ports are
	 * "input" to the backend, and capture ports are "output" from
	 * it.
	 */

	ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical
			| JackPortIsOutput);
	if (ports == NULL) {
		fprintf(stderr, "no physical capture ports\n");
		return -1;
	}

	if (jack_connect(client, ports[0], jack_port_name(input_port))) {
		fprintf(stderr, "cannot connect input ports\n");
	}

	free(ports);

	ports = jack_get_ports(client, NULL, NULL, JackPortIsPhysical
			| JackPortIsInput);
	if (ports == NULL) {
		fprintf(stderr, "no physical playback ports\n");
		return -1;
	}

	if (jack_connect(client, jack_port_name(output_port), ports[0])) {
		fprintf(stderr, "cannot connect output ports\n");
	}

	free(ports);

	return 1;
}

void sndcard_close() {
	jack_client_close(client);

}

