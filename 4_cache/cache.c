#include <stdint.h>
#include <stdio.h>
#include "cache.h"

block** cache;
int total_reads;
int total_writes;
int write_backs;
int read_hits;
int write_hits;
int read_misses;
int write_misses;
int set;
int words;
int way;
int blocksize;

uint32_t PC = 0;

void read(uint32_t addr) {
    total_reads++;

    char found = 0;
    int i;

    uint32_t set_index = ((addr/blocksize) & (set - 1));
    block* line = cache[set_index];
    block blk;
    int victim_index = 0;

    uint32_t tag = addr/set/blocksize;
    uint32_t start_addr = (addr / blocksize) * blocksize;

    for (i=0; i<way; i++) {
        blk = line[i];
        if (blk.valid && (blk.tag == tag)){ // Hit
            found = 1;
            read_hits++;
            line[i].used_at = PC;
        }
    }

    if (!found) { // Miss
        read_misses++;
        for (i=0; i<way; i++) {
            // Choose victim
            if (!line[i].valid) { // Invalid Block or
                victim_index = i;
                break;
            }

            else if (line[i].used_at < line[victim_index].used_at) { // LRU
                victim_index = i;
            }
        }
        if (line[victim_index].dirty) { // If Dirty Block
            write_backs++;
        }
        line[victim_index].valid = 1;
        line[victim_index].dirty = 0;
        line[victim_index].tag = tag;
        line[victim_index].address = start_addr;
        line[victim_index].used_at = PC;
    }
}

void write(uint32_t addr) {
    total_writes++;

    char found = 0;
    int i;

    uint32_t set_index = ((addr/blocksize) & (set - 1));
    block* line = cache[set_index];
    block blk;
    int victim_index = 0;

    uint32_t tag = addr/set/blocksize;
    uint32_t start_addr = (addr / blocksize) * blocksize;

    for (i=0; i<way; i++) {
        blk = line[i];
        if (blk.valid && (blk.tag == tag)){ // Hit
            found = 1;
            write_hits++;
            line[i].used_at = PC;
            line[i].dirty = 1;
        }
    }

    if (!found) { // Miss
        write_misses++;
        for (i=0; i<way; i++) {
            // Choose victim
            if (!line[i].valid) { // Invalid Block or
                victim_index = i;
                break;
            }

            else if (line[i].used_at < line[victim_index].used_at) { // LRU
                victim_index = i;
            }
        }
        if (line[victim_index].dirty) { // If Dirty Block
            write_backs++;
        }
        line[victim_index].valid = 1;
        line[victim_index].dirty = 1;
        line[victim_index].tag = tag;
        line[victim_index].address = start_addr;
        line[victim_index].used_at = PC;
    }
}

void access(char x, uint32_t addr) {
    if (x=='R') {
        read(addr);
    }
    else {
        write(addr);
    }
    PC++;
}
