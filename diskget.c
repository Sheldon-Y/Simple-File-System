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
int fatblockcount, fatstartblock, blocksize, blockcount, rootdir_startblock, rootdir_block, file_info, cpfile;
int sub_dir_starting_block, sub_dir_starting_block_tmp, sub_dir_number_of_block;

void search_in_subdir(int cpfile,char* file, int now_block, int number_of_block, int cur) {
    int counter = 0;
    if (cur == k-1) {
        for(int i = 0; i < number_of_block*(blocksize/64); i++){
            counter++;
            memcpy(&sub_file_status,file+(now_block*blocksize)+(i%8)*64, 1);
            memcpy(&sub_filename,file+(now_block*blocksize)+(i%8)*64+27, 31);
            memcpy(&sub_dir_starting_block_tmp, file+(now_block*blocksize)+(i%8)*64+1, 4);
            memcpy(&sub_dir_number_of_block, file+(now_block*blocksize)+(i%8)*64+5, 4);
            memcpy(&file_info, file+(now_block*blocksize)+(i%8)*64+9, 4);
            printf("%d %d %s %s\n", htonl(file_info), htonl(sub_dir_starting_block), sub_filename, path[cur]);
            int sub_dir_starting_block=htonl(sub_dir_starting_block_tmp);
            int file_size=htonl(file_info);
            if(strcmp(sub_filename,path[cur])==0){
                char buffer;
                printf("%d\n", file_size);
                while(sub_dir_starting_block!= 0xFFFFFFFF){
                    printf("%d\n", sub_dir_starting_block);
                    if(file_size>blocksize){
                        for(int i = 0; i < blocksize; i++){
							memcpy(&buffer,file+(blocksize* sub_dir_starting_block)+i, 1);
							write(cpfile,&buffer, 1);
						}	
                        file_size = file_size - blocksize;
                    }else{
						for(int i = 0; i < file_size; i++){
							memcpy(&buffer,file+(blocksize*sub_dir_starting_block)+i, 1);
							write(cpfile,&buffer, 1);
						}
                        return;	
                    }
                    memcpy(&sub_dir_starting_block,file+(fatstartblock*blocksize)+4*sub_dir_starting_block,4);
                    sub_dir_starting_block=htonl(sub_dir_starting_block);
                }
                return;
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
            search_in_subdir(cpfile,file, htonl(sub_dir_starting_block), htonl(sub_dir_number_of_block), cur + 1);
        
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

  
    int cpfile = open(argv[3], O_TRUNC | O_CREAT | O_WRONLY | O_RDONLY | O_APPEND, 777);






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
	
    search_in_subdir(cpfile,file, rootdir_startblock, rootdir_block, 0);

    close(fd);
    return;
}

 
