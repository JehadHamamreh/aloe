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

#include "permute.h"
#include "turbocoder.h"

unsigned short table_p[52] = {7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,199,211,223,227,229,233,239,241,251,257};
unsigned char table_v[52] = {3,2,2,3,2,5,2,3,2,6,3,5,2,2,2,2,7,5,3,2,3,5,2,5,2,6,3,3,2,3,2,2,6,5,2,5,2,2,2,19,5,2,3,2,3,2,6,3,7,7,6,3};

unsigned char v;
unsigned short p;
unsigned short s[MAX_COLS],q[MAX_ROWS],r[MAX_ROWS],T[MAX_ROWS];
unsigned short U[MAX_COLS*MAX_ROWS];

Tper PER[MAX_LONG_CB],DESPER[MAX_LONG_CB];

int M_Rows,M_Cols,M_long;
	
int ComputePermutation(struct permute_t *permute, int Long_CodeBlock) {

	//int M_Rows,M_Cols,M_long;
	int i,j;
	int res,prim,aux;
	int kp,k;
	Tper *per,*desper;
	
	permute->PER=PER;
	permute->DESPER=DESPER;

	M_long=Long_CodeBlock;

/*Determinar R*/
	if( (40 <= M_long)&&(M_long <= 159) )
		M_Rows = 5;
	else if( ( (160 <= M_long)&&(M_long <=200) ) || ((481 <= M_long) && (M_long <=530)))
		M_Rows = 10;
	else
		M_Rows = 20;

/*Determinar p i v*/
	if( (481 <= M_long)&&(M_long <= 530) ) {
		p = 53;
		v = 2;
		M_Cols = p;
	}
	else {
		i=0;
		do {
			p = table_p[i];
			v = table_v[i];
			i++;
		} while(M_long > (M_Rows*(p+1)) );
		
	}

/*Determinar C*/
	if( (M_long) <= (M_Rows)*((p)-1) )
		M_Cols = (p) - 1;
	else if(( (M_Rows)*(p-1) < M_long)&&(M_long <= (M_Rows)*(p) ))
		M_Cols = p;
	else if( (M_Rows)*(p) < M_long)
		M_Cols = (p) + 1;


	// calculem q
	q[0]=1;
	prim=6;

	for (i=1;i<M_Rows;i++) {
		do {
			prim++;
			res=mcd(prim,p-1);
		} while(res!=1);
		q[i]=prim;
	}

	// calculem s
	s[0]=1;
	for (i=1;i<p-1;i++) {
		s[i]=(v*s[i-1])%p;
	}
	// calculem T
	if (M_long<=159 && M_long>=40) {
		T[0]=4;T[1]=3;T[2]=2;T[3]=1;T[4]=0;
	}
	else if ((M_long<=200 && M_long>=160) || (M_long<=530 && M_long>=481)) {
		T[0]=9;T[1]=8;T[2]=7;T[3]=6;T[4]=5;T[5]=4;T[6]=3;T[7]=2;T[8]=1;T[9]=0;
	}
	else if ((M_long<=2480 && M_long>=2281) || (M_long<=3210 && M_long>=3161)) {
		T[0]=19;T[1]=9;T[2]=14;T[3]=4;T[4]=0;T[5]=2;T[6]=5;T[7]=7;T[8]=12;T[9]=18;T[10]=16;T[11]=13;T[12]=17;T[13]=15;T[14]=3;T[15]=1;T[16]=6;T[17]=11;T[18]=8;T[19]=10;
	}
	else {
		T[0]=19;T[1]=9;T[2]=14;T[3]=4;T[4]=0;T[5]=2;T[6]=5;T[7]=7;T[8]=12;T[9]=18;T[10]=10;T[11]=8;T[12]=13;T[13]=17;T[14]=3;T[15]=1;T[16]=16;T[17]=6;T[18]=15;T[19]=11;
	}

	// calculem r
	for (i=0;i<M_Rows;i++) {
		r[T[i]]=q[i];
	}
	
	// calculem U
	for (i=0;i<M_Rows;i++) {
		for (j=0;j<p-1;j++) {
			U[i*M_Cols+j]=s[(j*r[i])%(p-1)];
			if (M_Cols==(p-1))
				U[i*M_Cols+j]-=1;
		}
	}
		
	if (M_Cols==p) {
		for (i=0;i<M_Rows;i++)
			U[i*M_Cols+p-1]=0;
	}
	else if (M_Cols==p+1) {
		for (i=0;i<M_Rows;i++) {
			U[i*M_Cols+p-1]=0;
			U[i*M_Cols+p]=p;
		}
		if (M_long==M_Cols*M_Rows) {
			aux=U[(M_Rows-1)*M_Cols+p];
			U[(M_Rows-1)*M_Cols+p]=U[(M_Rows-1)*M_Cols+0];
			U[(M_Rows-1)*M_Cols+0]=aux;
		}
	}
	

	// ----- CALCUL DE LES PERMUTACIONS I DESPERMUTACIONS

	per=&permute->PER[0];
	desper=&permute->DESPER[0];
	
	k=0;
	for (j=0;j<M_Cols;j++) {
		for (i=0;i<M_Rows;i++) {
			kp=T[i]*M_Cols+U[i*M_Cols+j];
			if (kp<M_long) {
				desper[kp]=k;
				per[k]=kp;
				k++;
			}
		}
	}
	
	return (int) per[0];

}
	
int mcd(int x, int y) {
	int r=1;
	
	while(r) {
		r=x%y;
		x=y;
		y=r;
	}
	return x;
}
