/*
 * ratematching.c
 *
 *  Created on: 27/06/2012
 *      Author: xavier
 */

#include <stdio.h>
#include <string.h>

int float_wrap (float * in0, float * in1, float * in2, float * out, int insize)
{
	int i;
	for (i=0; i<insize; i++)
	{
		*(out++)=*(in0++);
		*(out++)=*(in1++);
		*(out++)=*(in2++);
	}

	return insize*3;
}

int char_unwrap (char * input, char * out0, char * out1, char * out2, int insize)
{
	int i;
	insize /= 3;

	for (i=0; i<insize; i++)
	{
		*(out0++)=*(input++);
		*(out1++)=*(input++);
		*(out2++)=*(input++);
	}

	return insize;
}

/* Return the result of floor(x/y) in integer */
#define intfloor(X, Y) (X/Y)
#define intceil(X, Y) ((X-1)/Y+1)
#define COLS 32
#define ROWS intceil(inlen, COLS)
static int PERM [32] = {0, 16, 8, 24, 4, 20, 12, 28,\
		2, 18, 10, 26, 6, 22, 14, 30, \
		1, 17, 9, 25, 5, 21, 13, 29, \
		3, 19, 11, 27, 7, 23, 15, 31};

#define DUMMYBIT 2

int char_interleave (char *in, char *out, int inlen)
{
	int k,km, blockSize, rows;
	int index;

	blockSize = COLS*ROWS;
	rows = ROWS;

	for (k=inlen; k<blockSize; k++)
		in[k]=DUMMYBIT;

	km=0;
	for (k=0; k<blockSize; k++)
	{
		index = PERM[intfloor(k, rows)]+COLS*(km);
		km++;
		if (km==rows) {
			km=0;
		}
		while(index>=blockSize) {
			index-=blockSize;
		}
		out[k] = in[index];
	}

	return blockSize;
}

int float_deinterleave (float *in, float *out, int blockSize)
{
	int k,km;
	int rows = blockSize/COLS;
	int index;

	for (k=0; k<blockSize; k++)
	{
		km = k;
		while(km>=rows) {
			km-=rows;
		}
		index = PERM[intfloor(k, rows)]+COLS*(km);
		while(index>=blockSize) {
			printf("index=%d blocksize=%d\n",index,blockSize);
			index-=blockSize;
		}
		out[index] = in[k];
	}

	return blockSize;
}

#define MAXCODEDSIZE (6144*3+32)
#define R (interleaversize/32)

int char_ratematching (char * subblock0, char * subblock1, char * subblock2, char * output, int interleaversize, int outlen, int rvidx)
{
	int i, k0, k,km;
	char bit;
	char circularBuffer [MAXCODEDSIZE];
	int buffersize = 3*interleaversize;

	for (i=0; i<interleaversize; i++)
	{
		circularBuffer[i]=subblock0[i];
		circularBuffer[interleaversize+2*i+0]=subblock1[i];
		circularBuffer[interleaversize+2*i+1]=subblock2[i];
	}

 	k0 = R*(2*intceil(buffersize, (8*R))*rvidx+2);
	k=0;
	i=0;
	while(k<outlen)
	{
		km=k0+i;
		while(km>=buffersize) {
			km-=buffersize;
		}
		bit = circularBuffer[km];
		if (bit!=DUMMYBIT){
			output[k]=bit;
			k++;
		}
		i++;
	}

	return outlen;
}

int float_unratematching (float * input, float * subblock0, float * subblock1, float * subblock2, int interleaversize, int inlen, int rvidx, char * dummyMatrix)
{
	int i, k0, k;
	float circularBuffer [MAXCODEDSIZE];
	int buffersize = interleaversize*3;

	memset(circularBuffer, 0, sizeof(float)*MAXCODEDSIZE);

	k0 = R*(2*intceil(buffersize, (8*R))*rvidx+2);
	k=0;
	i=0;
	while(k<inlen)
	{
		if (dummyMatrix[(k0+i)%buffersize]!=DUMMYBIT)
			circularBuffer[(k0+i)%buffersize] = input[k++];
		else
			circularBuffer[(k0+i)%buffersize] = 8.8;
		i++;
	}

	for (i=0; i<interleaversize; i++)
	{
		subblock0[i]=circularBuffer[i];
		subblock1[i]=circularBuffer[2*i+interleaversize+0];
		subblock2[i]=circularBuffer[2*i+interleaversize+1];
	}

	return interleaversize;
}

int getDummyMatrix(char * dummymatrix, int codewordsize)
{
	int matrixsize, i;
	char interleaved0 [MAXCODEDSIZE/3];
	char interleaved1 [MAXCODEDSIZE/3];
	char interleaved2 [MAXCODEDSIZE/3];
	char zeros [MAXCODEDSIZE/3];

	memset(zeros, 0, MAXCODEDSIZE/3);

	matrixsize = char_interleave (zeros, interleaved0, codewordsize);
	matrixsize = char_interleave (zeros, interleaved1, codewordsize);
	matrixsize = char_interleave (zeros, interleaved2, codewordsize);

	for (i=0; i<matrixsize; i++)
	{
		dummymatrix[i]=interleaved0[i];
		dummymatrix[2*i+matrixsize+0]=interleaved1[i];
		dummymatrix[2*i+matrixsize+1]=interleaved2[i];
	}

	return 3*matrixsize;
}

int float_UNRM_block (float * input, float * output, int inlen, int outsize, int rvidx)
{
	int subblocklen, intblocklen;

	float block0 [MAXCODEDSIZE/3];
	float block1 [MAXCODEDSIZE/3];
	float block2 [MAXCODEDSIZE/3];

	float intblock0 [MAXCODEDSIZE/3];
	float intblock1 [MAXCODEDSIZE/3];
	float intblock2 [MAXCODEDSIZE/3];

	subblocklen = intceil(outsize, 3);

	char dummymatrix [MAXCODEDSIZE];
	int interleaversize = getDummyMatrix(dummymatrix, subblocklen)/3;

	float_unratematching(input, intblock0, intblock1, intblock2, interleaversize, inlen, rvidx, dummymatrix);

	intblocklen = float_deinterleave(intblock0, block0, interleaversize);
	intblocklen = float_deinterleave(intblock1, block1, interleaversize);
	intblocklen = float_deinterleave(intblock2, block2, interleaversize);

	float_wrap(block0, block1, block2, output, intblocklen);

	return 0;
}



int char_RM_block (char * input, char * output, int in_len, int out_len, int rvidx)
{
	int insize = in_len;
	int outsize = out_len;

	int subblocklen, interleaversize;

	char block0 [MAXCODEDSIZE];
	char block1 [MAXCODEDSIZE];
	char block2 [MAXCODEDSIZE];

	char intblock0 [MAXCODEDSIZE];
	char intblock1 [MAXCODEDSIZE];
	char intblock2 [MAXCODEDSIZE];

	subblocklen = char_unwrap(input, block0, block1, block2, insize);
	interleaversize = char_interleave(block0, intblock0, subblocklen);
	interleaversize = char_interleave(block1, intblock1, subblocklen);
	interleaversize = char_interleave(block2, intblock2, subblocklen);
	
	char_ratematching(intblock0, intblock1, intblock2, output, interleaversize, outsize, rvidx);

	return 0;
}

