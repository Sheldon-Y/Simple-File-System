#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <stdlib.h>
#include <time.h>
#include <netinet/in.h>
#include <unistd.h>

struct __attribute__((__packed__)) superblock_t { 
    uint8_t fs_id [8];
    uint16_t block_size;
    uint32_t file_system_block_count;
    uint32_t fat_start_block;
    uint32_t fat_block_count;
    uint32_t root_dir_start_block;
    uint32_t root_dir_block_count;
};

void main(int argc, char* argv[]) {

    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);
    void* Address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct superblock_t* SuperBlock;
    SuperBlock=(struct superblock_t*)Address;
    int fatblockcount =  htonl(SuperBlock->fat_block_count);
    int fatstartblock = htonl(SuperBlock->fat_start_block);
    int blocksize = htons(SuperBlock->block_size);
    int blockcount = htonl(SuperBlock->file_system_block_count);
    int rootdir_startblock=htonl(SuperBlock->root_dir_start_block);
    int rootdir_block=htonl(SuperBlock->root_dir_block_count);

    
    printf("Super block information:\nBlock size: %d\nBlock count: %d\nFAT starts: %d\nFAT blocks: %d\nRoot directory start: %d\nRoot directory blocks: %d\n",
    blocksize,
    blockcount,
    fatstartblock, 
    fatblockcount,
    rootdir_startblock,
    rootdir_block);

    int start_byte = (fatstartblock)*blocksize;

    void* file = mmap(NULL, blockcount*blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);

    int fssize;
    int counter = 0;
    int reserved = 0;
    int available = 0;
    int allocated = 0;
    
    for(int i = 0; i < fatblockcount*(blocksize/4); i++){
    	memcpy(&fssize, file+start_byte+counter, 4);
    	fssize=ntohl(fssize);
    	if(fssize == 1){
    		reserved++;
    	}else if(fssize == 0){
    		available++;
    	}else{
    		allocated++;
    	}
    	counter = counter + 4;
    }
    
    printf("\nFAT Information:\nFree Blocks: %d\nReserved Blocks: %d\nAllocated Blocks: %d\n",
    available,
    reserved,
    allocated);


    munmap(Address,buffer.st_size);
    close(fd);
}