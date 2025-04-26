#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt", on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

#define CHECKS 1000
#define MAXBIN 5000

 uint8_t array2[256 * 512];

 void profileCache(uint16_t hits[CHECKS*256], uint16_t misses[CHECKS*256]) {
	 static int optimize;
	 int tries, i, mix_i, junk = 0;
	 register uint64_t time1, time2;
	 volatile uint8_t *addr;
	
	 for (int count = 0; count<2; count++) {
	 	 int clear_cache = 0;
	 	 if (count == 1)
	 	 	clear_cache = 1;
		 for (tries = 0; tries < CHECKS; tries++) {
			 

			 /* Time reads. Mixed-up order to prevent stride prediction */
			 for (i = 0; i < 256; i++) {
				 mix_i = ((i * 167) + 13) & 255;
				 addr = &array2[mix_i * 512];
				 /* Flush array2[256*(0..255)] from cache */
				 if (clear_cache) {
					 for (int j = 0; j < 256; j++)
					 	_mm_clflush(&array2[j * 512]); /* clflush */
			 	 } else {
			 	 	for (int j = 0; j < 256; j++)
			 	 		junk = *addr; /* Time memory access */
			 	 }
				 time1 = __rdtscp(&junk);
				 junk = *addr; /* Time memory access */
				 time2 = __rdtscp(&junk) - time1; /* Compute elapsed time */
				 if (count == 0)
				 	hits[i + tries*256] = time2;
				 else
				 	misses[i + tries*256] = time2;
				 /* use junk to prevent code from being optimized out */
		 		 optimize ^= junk;
			 }
		 }
 	 }
 }

 int main(int argc, const char **argv) {
 	 uint16_t binsH[MAXBIN], binsM[MAXBIN];
	 uint16_t hits[CHECKS*256], misses[CHECKS*256];
	 int repeat = 20;
	 int size = sizeof(hits) / sizeof(hits[0]);
	 
	 for (int i = 0; i < MAXBIN; i++) {
	 	binsH[i] = 0;
	 	binsM[i] = 0;
  	 }
  	 
	 printf("Profiling Cache Hit/Miss:\n");
	 printf("\n\n");
	 for (int count=0; count<repeat; count++)
	 {
		 for (int i = 0; i < size; i++) {
			 	hits[i] = 6;
	  	 }
	  	 
	  	 for (int i = 0; i < size; i++) {
			 	misses[i] = 6;
	  	 }
		 profileCache(hits, misses);
		 for (int i = 0; i < size; i++) {
		 	 if (hits[i] < MAXBIN)
		 	 	binsH[hits[i]]++;
		 	 else
		 	 	binsH[MAXBIN-1]++;
			 //printf("%d,", hits[i]);
	  	 }
	  	 for (int i = 0; i < size; i++) {
	  	 	 if (misses[i] < MAXBIN)
		 	 	binsM[misses[i]]++;
		 	 else
		 	 	binsM[MAXBIN-1]++;
			 //printf("%d,", misses[i]);
	  	 }
	 }
  	 for (int i = 0; i < MAXBIN; i++)
  	 	printf("%d,%d,%d\n",i,binsH[i],binsM[i]);
	 return (0);
 }

