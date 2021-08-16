#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#define BYTES_PER_WORD 4

/***************************************************************/
/*                                                             */
/* Procedure : cdump                                           */
/*                                                             */
/* Purpose   : Dump cache configuration                        */   
/*                                                             */
/***************************************************************/
void cdump(int capacity, int assoc, int blocksize){

	printf("Cache Configuration:\n");
    printf("-------------------------------------\n");
	printf("Capacity: %dB\n", capacity);
	printf("Associativity: %dway\n", assoc);
	printf("Block Size: %dB\n", blocksize);
	printf("\n");
}

/***************************************************************/
/*                                                             */
/* Procedure : sdump                                           */
/*                                                             */
/* Purpose   : Dump cache stat		                       */   
/*                                                             */
/***************************************************************/
void sdump(int total_reads, int total_writes, int write_backs,
	int reads_hits, int write_hits, int reads_misses, int write_misses) {
	printf("Cache Stat:\n");
    printf("-------------------------------------\n");
	printf("Total reads: %d\n", total_reads);
	printf("Total writes: %d\n", total_writes);
	printf("Write-backs: %d\n", write_backs);
	printf("Read hits: %d\n", reads_hits);
	printf("Write hits: %d\n", write_hits);
	printf("Read misses: %d\n", reads_misses);
	printf("Write misses: %d\n", write_misses);
	printf("\n");
}


/***************************************************************/
/*                                                             */
/* Procedure : xdump                                           */
/*                                                             */
/* Purpose   : Dump current cache state                        */ 
/* 							       */
/* Cache Design						       */
/*  							       */
/* 	    cache[set][assoc][word per block]		       */
/*      						       */
/*      						       */
/*       ----------------------------------------	       */
/*       I        I  way0  I  way1  I  way2  I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set0  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*       I        I  word0 I  word0 I  word0 I                 */
/*       I  set1  I  word1 I  word1 I  work1 I                 */
/*       I        I  word2 I  word2 I  word2 I                 */
/*       I        I  word3 I  word3 I  word3 I                 */
/*       ----------------------------------------              */
/*      						       */
/*                                                             */
/***************************************************************/
void xdump(int set, int way, uint32_t** cache){
	int i,j,k = 0;

	printf("Cache Content:\n");
    	printf("-------------------------------------\n");
	for(i = 0; i < way;i++)
	{
		if(i == 0)
		{
			printf("    ");
		}
		printf("      WAY[%d]",i);
	}
	printf("\n");

	for(i = 0 ; i < set;i++)
	{
		printf("SET[%d]:   ",i);
		for(j = 0; j < way;j++)
		{
			if(k != 0 && j == 0)
			{
				printf("          ");
			}
			printf("0x%08x  ", cache[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

int main(int argc, char *argv[]) {
	uint32_t **cache,**db,**ru;//cache, dirty bit, recent use
	int i,j,k;
	char *t=strtok(argv[2],":");
	int capacity=atoi(t);
	int way=atoi(t=strtok(NULL,":"));
	int blocksize=atoi(t=strtok(NULL,":"));
	int set=capacity/way/blocksize;
	int words=blocksize/BYTES_PER_WORD;
	int dump=argc>4;
	cache = (uint32_t**) malloc (sizeof(uint32_t*) * set);
	for(i = 0; i < set; i++)cache[i] = (uint32_t*) malloc(sizeof(uint32_t) * way);
	db = (uint32_t**) malloc (sizeof(uint32_t*) * set);
	for(i = 0; i < set; i++)db[i] = (uint32_t*) malloc(sizeof(uint32_t) * way);
	ru = (uint32_t**) malloc (sizeof(uint32_t*) * set);
	for(i = 0; i < set; i++)ru[i] = (uint32_t*) malloc(sizeof(uint32_t) * way);

	char *file=(char *)malloc(strlen(argv[argc-1])+3);
	strncpy(file,argv[argc-1],strlen(argv[argc-1]));
	if(freopen(file, "r",stdin)==0)exit(1);

	char a[20],c;
	int tr,tw,wb,rh,wh;
	for(i=1,tr=tw=wb=rh=wh=0;scanf("%c %s\n",&c,a)==2;++i){
		uint32_t addr=strtol(a,NULL,16);
		int tag,idx,ent;
		tag=addr/set/blocksize;
		idx=addr/blocksize%set;
		ent=addr/blocksize*blocksize;
		if(c=='R'){
			++tr;
			for(j=0;j<way;++j)if(cache[idx][j]/set/blocksize==tag)break;
			if(j<way)++rh;
			else{
				for(j=k=0;k<way;++k)if(ru[idx][j]>ru[idx][k])j=k;
				cache[idx][j]=ent;
				if(db[idx][j])db[idx][j]=0,++wb;
			}
			ru[idx][j]=i;
		}
		else{
			++tw;
			for(j=0;j<way;++j)if(cache[idx][j]/set/blocksize==tag)break;
			if(j<way)++wh;
			else{
				for(j=k=0;k<way;++k)if(ru[idx][j]>ru[idx][k])j=k;
				cache[idx][j]=ent;
				if(db[idx][j])++wb;
			}
			db[idx][j]=1,ru[idx][j]=i;
		}
	}

	if(dump){
		cdump(capacity, way, blocksize);
    	sdump(tr,tw,wb,rh,wh,tr-rh,tw-wh); 
    	xdump(set, way, cache);
	}

    return 0;
}
