// KeyExtraction_All.cpp : Defines the entry point for the console application.
//

#define _CRT_SECURE_NO_DEPRECATE

#include "stdafx.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <string.h>

//#define MAX 100

#define MAX 500
#define ROW 1997
#define COL 500

// Transmitter signals (100 samples)
/*int tx[MAX] = {-80,-81,-79,-77,-71,-71,-73,-82,-80,-72,-72,-72,-72,-80,-84,-71,-70,-74,-74,-75,-74,-76,-71,-72,-72,-76,-76,-76,-78,-74,-77,-77,
-78,-83,-73,-74,-72,-74,-75,-81,-74,-79,-77,-78,-84,-86,-84,-81,-80,-77,-79,-81,-73,-76,-75,-74,-74,-78,-79,-87,-73,-73,-73,-73,-78,-74,-76,-78,-79,-86,
-83,-83,-83,-85,-76,-72,-72,-76,-79,-88,-75,-73,-70,-71,-72,-75,-74,-72,-70,-70,-72,-77,-84,-73,-70,-70,-71,-74,-79,-76};

//Receiver signals (100 samples)
int rx[MAX] = {-84,-83,-82,-82,-74,-74,-75,-85,-84,-75,-74,-75,-75,-83,-86,-74,-72,-76,-77,-78,-76,-78,-74,-75,-75,-80,-79,-79,-82,-77,-81,-80,-82,-88,-77,-77,-75,
-78,-79,-85,-77,-81,-79,-81,-85,-87,-86,-82,-82,-82,-81,-84,-76,-80,-80,-78,-79,-82,-81,-92,-77,-76,-76,-77,-81,-78,-79,-82,-82,-90,-88,-87,-88,-89,-79,-76,-76,-79,
-81,-90,-79,-76,-73,-74,-76,-79,-78,-75,-73,-73,-74,-80,-89,-77,-72,-72,-73,-78,-82,-80};
*/

int Tx[ROW][COL];
int Rx[ROW][COL];

int tx[MAX];
int rx[MAX];

int Mathur(int *x, int *n_sd, int *n_sdpos) {

   int sum=0; double var=0;
   int i = 0, j=0;
   double upper = 0, lower = 0;
   int n = MAX;
   int len = 0;
   double avg = 0; double sd = 0;

   //compute standard deviation
   for(i=0; i<n; i++) 
        sum += x[i];
    
   avg =  (double) sum / (double) n;

   //printf("\n Avg: %f", avg);
    		
    for(i=0; i<n; i++) 
       var += pow(((double) x[i] - avg),2);
    
    sd = (double) sqrt(var)/(double) n;

  // printf("\n Var : %f", var);
  // printf("\n Std Dev : %f", sd);

   //compute upper and lower threshold
   upper = avg + (0.05*(sd));
   lower = avg -(0.1*(sd));

  // printf("\n Lower Threshold : %f", lower);

  // printf("\n Upper Threshold : %f", upper);

   //Extract bits as per upper and lower threshold and the positions of signal array where bit transformation occurred
    for(i=0; i<n; i++) {

        if(x[i] < lower){
        	n_sd[j] = 0; 
			n_sdpos[j] = i;
        	j++;
        }

        if (x[i] >  upper){
		    n_sd[j] = 1; 
			n_sdpos[j] = i;
			j++;
        }
    }
 
	//Determine length of bit extracted
    len = j;

	//Print the positions and bits
  /*  printf("\n SD Positions: ");
	 for (j=0; j < len; j++) 
        printf("%d ", n_sdpos[j]);

	  printf("\n SD_Bits ( ");
    printf("%d",len);
    printf(") : ");

  for (j=0; j < len; j++) 
        printf("%d ", n_sd[j]);
    
   printf(" ##\n");*/

	return len;
 }

int Aono(int *x,int *y,int *n_med, int *n_medpos) {
    
  int temp, i=0, j=0, n = MAX;
  double med;
  int minRSS=0; int maxRSS=0;
  int len =0;

  // Sort the signals array

  for(i=0; i<n-1; i++) {

	 for(j=i+1; j<n; j++) {
            
           if(x[j] > x[i]) {
                // swap elements
                temp = x[i];
                x[i] = x[j];
                x[j] = temp;
            }
        }
    }
	
  //Determine minRSS and maxRSS
       minRSS = x[MAX-6]; maxRSS = x[5];

    //   printf("\n minRSS <-> maxRSS: %d  %d", minRSS,maxRSS);			 
 
	   // Determine median
    if(n%2==0) {
        // if there is an even number of elements, return mean of the two elements in the middle
	med = ((double) (x[n/2] + x[n/2 - 1]) / (double) 2);
    //    printf("\n Median : %f", med);             
    }

	 else {
        // else return the element in the middle
        med = (double) x[n/2];
      //  printf("\n Median : %f",med);  
     }   

	 //Determine upper and lower threshold
    double lt = 0, rt = 0;

  
	lt =  (med - (med - minRSS)/2);
	rt =  (med + (maxRSS - med)/2);

    printf("\n LT: %f",lt);
	printf("\n RT: %f",rt);
        
	j=0;

	//Extract bits as per upper and lower threshold and the positions of signal array where bit transformation occurred
   for(i=0; i<n; i++) {

		if(y[i] < lt){ 
		   n_med[j] = 0; 
           n_medpos[j] = i;
		   j++;
	 }
		
	 if (y[i] > rt){	
		 n_med[j] = 1; 
		 n_medpos[j] = i;
		 j++;
	}
 }
    
   //Compute length of bit extracted
  //  printf("\n j is: %d",j);  
    len = j;
	
	//print the positions and bits
	printf("\n Positions:");
	for (j=0; j < len; j++) 
       	 printf("%d ", n_medpos[j]);

	printf("\n MedBits (");
    printf("%d",len);
    printf(") :\n ");
   
    for (j=0; j < len; j++) 
        printf("%d ", n_med[j]);

    printf("##\n");

return len;
}

int Tope(int *x, int *n_diff, int *n_diffpos){

 int i, j;
 int sum=0;
 int diff[MAX];
 double var=0;
 double avg;
 double upper=0,lower=0;
 int len; 
 double sd;

 //Determine differential values of the RSS,x
 for(i=0;i<(MAX-1);i++)
   diff[i] = (x[i]-x[i+1]);

 diff[MAX-1]= (x[MAX-1] - x[0]);

 //Determine the mean of RSS
  for(i=0; i<(MAX-1); i++) 
        sum += diff[i];
    
    avg =  ((double) sum / (double) (MAX-1));

  //  printf("\n Avg: %f", avg);
    		
    for(i=0; i<(MAX-1); i++) 
       var += pow(((double) diff[i] - avg),2);
   

//Determine standard deviation
	sd = ((double) sqrt(var)/ (double) (MAX));

 // Determine thresholds

   lower =  (avg +(0.4*(sd)));

   upper =  (avg + ((sd)));

  // printf("\n Lower Threshold : %f", lower);

  // printf("\n Upper Threshold : %f", upper);

 //Extract bits as per upper and lower threshold and the positions of signal array where bit transformation occurred
     j=0;
    for(i=0; i<MAX; i++) {

        if(diff[i] < lower){
        	n_diff[j] = 0; 
		    n_diffpos[j] = i;
        	j++ ;
        }

       if (diff[i] > upper){
		n_diff[j] = 1; 
		n_diffpos[j] = i;
		j++ ;
        }
    }
 
	//Determine length of bits extracted
// printf("\n j is: %d",j);  
    len = j;
	
	//print the bits and their positions
   /* printf("\n Differential Bits (");
    printf("%d",len);
    printf(") : ");
   
    for (j=0; j < len; j++) 
        printf("%d ", n_diff[j]);

    printf("##\n");

	 printf("\n Positions:\n");
	 for (j=0; j < len; j++) 
       	 printf("%d ", n_diffpos[j]); */

return len;
}

// BIT MISMATCH RATE IS THE NUMBER OF INDICES MISSING IN EITHER Tx OR Rx AND THE NUMBER OF BITS MISMATCHED IN SAME INDICES OF Tx AND Rx

double bit_drop_rate(int *t_pos, int *r_pos, int len_t, int len_r) {

int i,j,k=0,len1 =0,len2=0,bitdrops=0;

// merge two arrays
int len = len_t + len_r;

int *arr = (int*)malloc(sizeof(int)*len);
int *freq = (int*)malloc(sizeof(int)*len);

for(i=0;i<len_t;i++)
	arr[k++]=t_pos[i];

for(j=0;j<len_r;j++)
	arr[k++] = r_pos[j];


// find the elements that occur only once by checking the frequency of occurrence

for(i=0;i<len;i++) {
	
	k=0;
	
	for(j=0;j<len;j++) {

		if(arr[i]!=arr[j])
	     k++;		 
		}

	  if(k==(len-1))
		  bitdrops++;
	}

//double avglen = (len_t + len_r)/2;
//double rate =  ((double) bitdrops / avglen);

double rate =  ((double) bitdrops / (double) len);

//printf("\n Bit dropped: %d",bitdrops);

//printf("\n Bit drop rate is %f", rate);

return rate;
}


double bit_mismatch_rate(int *t_pos, int *r_pos, int *t_bits, int *r_bits, int len_t, int len_r) {

int i,j,k,len1 =0,len2=0,bitmismatch=0;

 if(len_t < len_r){
  len1 = len_t;
  len2 = len_r;
}
 else{
  len1 = len_r;
  len2 = len_t;
}

 if(len_t==len_r){
	 len1=len_t; len2=len_r;}

 //double avglen = (len1 + len2)/2;

 int len = 0; 
 if(len1 > len2 )
	 len = len2;
 if(len1 < len2)
	 len = len1;
 if(len1==len2)
	 len=len1;

// compare indices of referred array with the other - one to many. Each time indices match, check the bit in both arrays.

for(i=0;i<len1;i++) {

  for(j=0;j< len2; j++){

    if(t_pos[i]==r_pos[j]){

      if(t_bits[i]!=r_bits[j])
         bitmismatch++;
     }
   }
}

//double rate = ((double) bitmismatch / avglen);

double rate = ((double) bitmismatch / (double)len);

//printf("\n Bit mismatched at same positions: %d",bitmismatch);

//printf("\n Bit mismatch rate is %f", rate);

return rate;
}

int main(){

int len_sdtx=0, len_sdrx=0, len_mtx=0, len_mrx=0,len_dtx=0, len_drx=0;
double bitsdroprate=0, bitsmismatch=0, sdtotal_bitmismatchrate=0; double mtotal_bitmismatchrate=0; double dtotal_bitmismatchrate=0;

//Enter the array of Tx and Rx signals
int temp_t[MAX];int temp_r[MAX];
int i=0,j=0;

int *t_sd = (int*) malloc(MAX*sizeof(int));
int *r_sd = (int*) malloc(MAX*sizeof(int));
int *t_sdpos = (int*) malloc(MAX*sizeof(int));
int *r_sdpos = (int*) malloc(MAX*sizeof(int));

int *t_med = (int*) malloc(MAX*sizeof(int));
int *r_med = (int*) malloc(MAX*sizeof(int));
int *t_medpos = (int*) malloc(MAX*sizeof(int));
int *r_medpos = (int*) malloc(MAX*sizeof(int));


int *t_diff = (int*) malloc(MAX*sizeof(int));
int *r_diff = (int*) malloc(MAX*sizeof(int));
int *t_diffpos = (int*) malloc(MAX*sizeof(int));
int *r_diffpos = (int*) malloc(MAX*sizeof(int));


FILE *fp1, *fp2, *fp3, *fp4;
int arr[MAX];
char dlm[MAX];
char dlm1[MAX];
int node[1];
char colon[1];
int flag[1];

int set=0;

if((fp1=fopen("Tx3.txt","rb"))== NULL) {
	printf("Error in function: file could not be opened.\n");
	exit(0);
	}

    fseek(fp1,0L,SEEK_END);
	int lsize = ftell(fp1);
	rewind(fp1);

if((fp2=fopen("SequenceLengthMatch.txt","wb"))== NULL) {
	printf("Error in function: file could not be opened.\n");
	exit(0);
	}

if((fp3=fopen("BitMisMatch.txt","wb"))== NULL) {
	printf("Error in function: file could not be opened.\n");
	exit(0);
	}

if((fp4=fopen("Signals.txt","wb"))== NULL) {
	printf("Error in function: file could not be opened.\n");
	exit(0);
	}

// COLLECT ALL TRANSMITTER AND RECEIVER SIGNAL DATA IN 2D ARRAYS Tx AND Rx
int row_t=0,row_r=0;

	while(1){
	 
		fscanf(fp1,"%i %c",&node,colon);
		
		if(feof(fp1))
		break;

	if(node[0]==10){
	  for(i=0;i<COL;i++){
		fscanf(fp1,"%i %c", &Tx[row_t][i], &dlm[i]);
	  }
	row_t++;
	}

	else if(node[0]==20){
	  for(i=0;i<COL;i++){
		fscanf(fp1,"%i %c", &Rx[row_r][i], &dlm[i]);
	  }
	row_r++;
	}


}

fclose(fp1);
fclose(fp4);

// COMPUTE THE PARAMETERS FOR THE SET BY FETCHING 500 SAMPLES OF TRANSMITTER AND RECEIVER EACH IN ONE RUN OF LOOP

fprintf(fp2,"\tTx-Rx SET NO: \t\tMathur\tAono\t\t\tTope\n");

fprintf(fp3,"\tTx-Rx SET NO: \t\tMathur\tAono\t\t\tTope\n");

int loop=0;

for(loop=0;loop<ROW;loop++){

   for(j=0;j<COL;j++){
	tx[j]=Tx[loop][j];
	rx[j]=Rx[loop][j];
 }

fprintf(fp2,"\t\t%d", set);
fprintf(fp3,"\t\t%d", set);

// Compute values for each set and write the results in a separate file

	/*******************************************MATHUR ET AL METHOD OF STANDARD DEVIATION ****************************************************************/

// Compute SD of Tx and determine number of extracted bits
len_sdtx=Mathur(tx,t_sd,t_sdpos);
// Compute SD of Rx and determine number of extracted bits
len_sdrx = Mathur(rx,r_sd,r_sdpos);

//determine bit mismatch rate
bitsdroprate=bit_drop_rate(t_sdpos, r_sdpos,len_sdtx,len_sdrx);

bitsmismatch=bit_mismatch_rate(t_sdpos, r_sdpos, t_sd, r_sd,len_sdtx,len_sdrx);

sdtotal_bitmismatchrate = bitsdroprate + bitsmismatch;

/*******************************************AONO ET AL METHOD OF MEDIAN ****************************************************************/

bitsdroprate=0, bitsmismatch=0;

for(i=0;i<COL;i++)
	temp_t[i]=tx[i];

// Compute SD of Tx and determine number of extracted bits
len_mtx=Aono(tx,temp_t,t_med,t_medpos);

// Compute SD of Rx and determine number of extracted bits
for(i=0;i<COL;i++)
	temp_r[i]=rx[i];

len_mrx = Aono(rx,temp_r,r_med,r_medpos);

bitsdroprate=bit_drop_rate(t_medpos, r_medpos,len_mtx,len_mrx);

bitsmismatch=bit_mismatch_rate(t_medpos, r_medpos, t_med, r_med,len_mtx,len_mrx);

mtotal_bitmismatchrate = bitsdroprate + bitsmismatch;

/*******************************************TOPE ET AL METHOD OF DIFFERENTIAL ****************************************************************/
bitsdroprate=0, bitsmismatch=0;

// Compute SD of Tx and determine number of extracted bits
len_dtx=Tope(tx,t_diff,t_diffpos);

// Compute SD of Rx and determine number of extracted bits
len_drx = Tope(rx,r_diff,r_diffpos);

bitsdroprate=bit_drop_rate(t_diffpos, r_diffpos,len_dtx,len_drx);

bitsmismatch=bit_mismatch_rate(t_diffpos, r_diffpos, t_diff, r_diff,len_dtx,len_drx);

dtotal_bitmismatchrate = bitsdroprate + bitsmismatch;

if(len_sdtx!=len_sdrx)
	fprintf(fp2,"\t\t\tMISMATCH");
else
	fprintf(fp2,"\t\t\tMATCH");


if(len_mtx!=len_mrx)
	fprintf(fp2,"\t\tMISMATCH");

else
	fprintf(fp2,"\t\tMATCH");

if(len_dtx!=len_drx)
	fprintf(fp2,"\t\tMISMATCH\n");

else
	fprintf(fp2,"\t\tMATCH\n");

fprintf(fp3,"\t\t\t%f\t\t%f\t\t%f\n",sdtotal_bitmismatchrate,mtotal_bitmismatchrate,dtotal_bitmismatchrate);

set++;
}
	fclose(fp2);
	fclose(fp3);
	

return 0;
}


