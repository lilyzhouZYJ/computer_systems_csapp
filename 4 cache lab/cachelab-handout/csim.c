#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <getopt.h>         // for getopt
#include <math.h>           // for pow

#include "cachelab.h"

#define HIT (0)
#define MISS (1)
#define MISS_EVICT (2)

// cache line
typedef struct {
    int validBit;
    int tag;
    int timeStamp;
}* CacheLine;

typedef CacheLine * CacheSet;   // cache set
typedef CacheSet * Cache;       // cache

// global variables
int verbose = 0;        // verbose mode
int s = 0;              // number of set index bits
int setCount;           // number of sets
int E = 0;              // associativity
int b = 0;              // number of block offset bits
FILE * tFile = NULL;    // trace file

Cache myCache = NULL;

int hits = 0;           // number of hits
int misses = 0;         // number of misses
int evicts = 0;         // number of evictions

// increment time stamp of all valid cache lines
void incrementTimeStamp(){
    for(int i = 0; i < setCount; i++){
        for(int j = 0; j < E; j++){
            CacheLine currLine = myCache[i][j];
            if(currLine != 0 && currLine->validBit)
                currLine->timeStamp += 1;
        }
    }
}

// find Least Recently Used line in currSet,
// return that line
CacheLine findLRU(CacheSet currSet){

    int maxTime = -1;
    CacheLine lru = 0;

    for(int i = 0; i < E; i++){
        if(currSet[i]->timeStamp > maxTime){
            maxTime = currSet[i]->timeStamp;
            lru = currSet[i];
        }
    }

    return lru;
}

// print hit, miss, or miss eviction
void printRes(int result){
    switch (result){
        case HIT:
            printf(" %s", "hit");
            break;
        case MISS:
            printf(" %s", "miss");
            break;
        case MISS_EVICT:
            printf(" %s", "miss eviction");
    }
}

// Access cache based on the given address.
// Returns HIT (1), MISS (2), or MISS_EVICT (3)
int access(int address){
    
    // get set index
    unsigned int setIndex;
    unsigned int mask = 0;
    for(int i = 0; i < s; i++)
       mask |= 1 << i;
    setIndex = (address >> b) & mask;

    // get tag
    int tag = address >> (b + s);

    // look for tag in the given set
    CacheSet currSet = myCache[setIndex];
    for(int i = 0; i < E; i++){
        CacheLine currLine = currSet[i];
        if(currLine != 0 && currLine->validBit && currLine->tag == tag){
            // a hit!
            hits++;
            incrementTimeStamp();       // update time stamp of all lines
            currLine->timeStamp = 0;    // set currLine to most recently used
            return HIT;
        }
    }

    // miss
    misses++;

    // find empty line in currSet
    for(int i = 0; i < E; i++){
        if(currSet[i] == 0){
            // found empty space!
            // add the new data block
            currSet[i] = malloc(sizeof(CacheLine));
            currSet[i]->tag = tag;
            currSet[i]->validBit = 1;
            incrementTimeStamp();       // update time stamp of all lines
            currSet[i]->timeStamp = 0;  // set curr line to most recently used
            return MISS;
        }
    }

    // no empty line in currSet
    // eviction is necessary
    evicts++;

    // find LRU line to evict
    CacheLine lru = findLRU(currSet);

    // change LRU line to new line
    lru->tag = tag;
    lru->validBit = 1;
    incrementTimeStamp();       // update time stamp of all lines
    lru->timeStamp = 0;         // set curr line to most recently used

    return MISS_EVICT;
}

// read each line of trace file
void readTrace(){

    char identifier;
    int address;
    int size;

    while(fscanf(tFile, " %c %x, %d", &identifier, &address, &size) > 0){
        // [verbose] print trace, if not instruction load
        if(verbose && identifier != 'I')
            printf("%c %x, %d", identifier, address, size);
        
        // process trace
        int result;
        switch(identifier){
            case 'L':       // data load
                result = access(address);
                if(verbose) printRes(result);
                break;
            case 'M':       // data modify, need 2 accesses (load + store)
                result = access(address);
                if(verbose) printRes(result);
            case 'S':       // data store
                result = access(address);
                if(verbose) printRes(result);
                break;   
        }
        // add line break
        if(verbose) printf("\n");
    }
}

// frees cache
void freeCache(){
    for(int i = 0; i < setCount; i++){
        for(int j = 0; j < E; j++){
            free(myCache[i][j]);
        }
        free(myCache[i]);
    }
    free(myCache);
}

// initialize cache
void initCache(){
    setCount = pow(2, s);
    myCache = malloc(sizeof(CacheSet) * setCount);
    for(int i = 0; i < setCount; i++){
        myCache[i] = calloc(E, sizeof(CacheLine));
    }
}

// prints usage message
void printUsage(){
    printf("Usage: ./csim [-hv] -s <num> -E <num> -b <num> -t <file>\n"
                        "Options:\n"
                        "  -h         Print this help message.\n"
                        "  -v         Optional verbose flag.\n"
                        "  -s <num>   Number of set index bits.\n"
                        "  -E <num>   Number of lines per set.\n"
                        "  -b <num>   Number of block offset bits.\n"
                        "  -t <file>  Trace file.\n"
                        "\n"
                        "Examples:\n"
                        "  linux>  ./csim -s 4 -E 1 -b 4 -t traces/yi.trace\n"
                        "  linux>  ./csim -v -s 8 -E 2 -b 4 -t traces/yi.trace\n");
}

// get command-line arguments
void getArgs(int argc, char ** argv){
    int opt;
    while((opt = getopt(argc, argv, "hvs:E:b:t:")) != -1){
        switch(opt){
            case 'h':                   // prints help message
                printUsage();
                exit(0);
            case 'v':                   // verbose mode
                verbose = 1;
                break;
            case 's':                   // number of set index bits
                s = atoi(optarg);
                break;
            case 'E':                   // associativity
                E = atoi(optarg);
                break;
            case 'b':                   // number of block offset bits
                b = atoi(optarg);
                break;
            case 't':                   // trace file
                tFile = fopen(optarg, "r");
                break;
            default:
                printUsage();
                exit(0);
        }
    }
    
    // check that all required args are valid
    if(s && E && b && tFile != NULL) return;
    else {    // print error message
        fprintf(stderr, "./csim: Missing required command line argument\n");
        printUsage();
        exit(0);
    }
}

int main(int argc, char ** argv)
{
    // get command-line arguments
    getArgs(argc, argv);

    // initialize cache based on arguments
    initCache();

    // read in each line of trace file
    readTrace();

    // close file
    fclose(tFile);

    // free memory spaces
    freeCache();

    // print summary
    printSummary(hits, misses, evicts);
    return 0;
}
