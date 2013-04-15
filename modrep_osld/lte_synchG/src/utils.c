/*
 * utils.c
 *
 *  Created on: Apr 11, 2012
 *      Author: odissey09
 */
//#include <fftw3.h>
#include <math.h>
#include <complex.h>
#include <fftw3.h>
#include "utils.h"


/**
* void write(buffer* buffer, _Complex float value, int length)
* writes value into the buffer
* @param buffer* buffer
*   pointer to buffer to be used
* @param _Complex float value
*   value to be written in buffer
*/

int writeCbuff(buffctrl* buffer, _Complex float *buffdata, _Complex float *in, int length){
	int i;

	if(buffer->writeIndex >= buffer->readIndex){
		buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
	}
	else{
		buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
	}

	if(buffer->buffsize - buffer->occuplevel >= length){
		for(i=0; i<length; i++){
			buffdata[buffer->writeIndex]=*(in+i);
			buffer->writeIndex++;
			if(buffer->writeIndex==buffer->buffsize){
				buffer->writeIndex=0;
			}
		}
		if(buffer->writeIndex >= buffer->readIndex){
			buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
		}
		else{
			buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
		}

		return 0;
	}else{
		printf("Error: Not enough space in buffer\n");
		return -1;
	}
}
/**
* void readn(buffer* buffer, int Xn)
* reads specified value from buffer
* @param buffer* buffer
*   pointer to buffer to be read from
* @param int Xn
*   specifies the value to be read from buffer counting backwards from the most recently written value
*   i.e. the most recently writen value can be read with readn(buffer, 0), the value written before that with readn(buffer, 1)
*/
int readCbuff(buffctrl* buffer,_Complex float *buffdata, _Complex float *out, int length){
	int i;

	if(buffer->writeIndex >= buffer->readIndex){
		buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
	}
	else{
		buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
	}
	if(buffer->occuplevel >= length){
		for(i=0; i<length; i++){
			*(out+i) = buffdata[buffer->readIndex];
			buffer->readIndex++;
			if(buffer->readIndex==buffer->buffsize){
				buffer->readIndex=0;
			}
		}
		if(buffer->writeIndex >= buffer->readIndex){
			buffer->occuplevel = buffer->writeIndex - buffer->readIndex;
		}
		else{
			buffer->occuplevel=buffer->buffsize-(buffer->readIndex-buffer->writeIndex);
		}

		return 0;
	}else{
/*		printf("Error: Not enough data in buffer\n");
*/		return -1;
	}
}


