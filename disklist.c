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

int k;
char* path[100] = {0};
char sub_filename[1000];
int sub_file_status;
int file_info;  
int fatblockcount, fatstartblock, blocksize, blockcount, rootdir_startblock, rootdir_block;
int sub_dir_starting_block, sub_dir_number_of_block;

void search_in_subdir(char* file, int now_block, int number_of_block, int cur) {
    int counter = 0;
    if (cur == k) {
        for(int i = 0; i < number_of_block*(blocksize/64); i++){
            counter++;
            memcpy(&sub_file_status,file+(now_block*blocksize)+(i%8)*64, 1);
            memcpy(&sub_filename,file+(now_block*blocksize)+(i%8)*64+27, 31);
            memcpy(&sub_dir_starting_block, file+(now_block*blocksize)+(i%8)*64+1, 4);
            memcpy(&sub_dir_number_of_block, file+(now_block*blocksize)+(i%8)*64+5, 4);
            if(sub_file_status!=0){
            
            if(sub_file_status==3){
                printf("F");
            }else if(sub_file_status==5){
                printf("D");
            }
            

            memcpy(&file_info, file+(now_block*blocksize)+(i%8)*64+9, 4);
			file_info = ntohl(file_info);
			
            printf("%10d ", file_info);
            
            printf("%30s",sub_filename);
            
            memcpy(&file_info, file+(now_block*blocksize)+(i%8)*64+20, 2);
			file_info = ntohs(file_info);
			printf("\t%d/", file_info);
            char file_info; 
			memcpy(&file_info, file+(now_block*blocksize)+(i%8)*64+22, 1);
			printf("%.2d/", file_info);
			memcpy(&file_info, file+(now_block*blocksize)+(i%8)*64+23, 1);
			printf("%.2d ", file_info);
			memcpy(&file_info, file+(now_block*blocksize)+(i%8)*64+24, 1);
			printf("%.2d:", file_info);
			memcpy(&file_info, file+(now_block*blocksize)+(i%8)*64+25, 1);
			printf("%.2d:", file_info);
			memcpy(&file_info, file+(now_block*blocksize)+(i%8)*64+26, 1);
			printf("%.2d", file_info);
			printf("\n");  
            }
           

            if(counter%8==0){
                memcpy(&now_block,file+(fatstartblock*blocksize)+4*now_block,4);
                now_block=htonl(now_block);
            }
        }

        return;
    }
    for(int i = 0; i < number_of_block*(blocksize/64); i++){
        counter++;
        memcpy(&sub_file_status,file+(now_block*blocksize)+(i%8)*64,1);
        memcpy(&sub_filename,file+(now_block*blocksize)+(i%8)*64+27,31);
        memcpy(&sub_dir_starting_block, file+(now_block*blocksize)+(i%8)*64+1, 4);
        memcpy(&sub_dir_number_of_block, file+(now_block*blocksize)+(i%8)*64+5, 4);

        if(sub_file_status==5 && strcmp(sub_filename,path[cur])==0){
            search_in_subdir(file, htonl(sub_dir_starting_block), htonl(sub_dir_number_of_block), cur + 1);
        
            return;
        }

        if(counter%8==0){
            memcpy(&now_block,file+(fatstartblock*blocksize)+4*now_block,4);
            now_block=htonl(now_block);
        }
    }
            printf("Directory Not Found\n");
            return;
}

void main(int argc, char* argv[]) {
    
    char input[1000];
    
    if(argv[2]!=NULL){
    strcpy(input, argv[2]);
    char* token = strtok(input, "/");
        while(token != NULL) {
            path[k++] = token;
            token = strtok(NULL, "/");
        }
    }
    

    char filename[1000];
    char file_status;
    
    

    int jump =0;
    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);
    void* Address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    struct superblock_t* SuperBlock;
    SuperBlock=(struct superblock_t*)Address;
    
    fatblockcount =  htonl(SuperBlock->fat_block_count);
    fatstartblock = htonl(SuperBlock->fat_start_block);
    blocksize = htons(SuperBlock->block_size);
    blockcount = htonl(SuperBlock->file_system_block_count);
    rootdir_startblock=htonl(SuperBlock->root_dir_start_block);
    rootdir_block=htonl(SuperBlock->root_dir_block_count);
    
    int start_byte = (rootdir_startblock)*blocksize;

    char* file = mmap(NULL, blockcount*blocksize, PROT_READ | PROT_WRITE, MAP_SHARED, fd,0);
	
    search_in_subdir(file, rootdir_startblock, rootdir_block, 0);

    close(fd);
    return;

   

}	

 
