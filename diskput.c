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
struct datetime_t{
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
};
int k;
char* path[100] = {0};
char sub_filename[1000];
int sub_file_status;
int file_info;  
int fatblockcount, fatstartblock, blocksize, blockcount, rootdir_startblock, rootdir_block, file_info, cpfile;
int sub_dir_starting_block, sub_dir_number_of_block;

int file_start_block;

int find_next(char* file, int now_block) {
    int fat_value;
    memcpy(&fat_value, file + fatstartblock * blocksize + 4 * now_block, 4);
    fat_value = htonl(fat_value);
    if (fat_value != 0) {
        return find_next(file, now_block + 1);
    }
    return now_block;
}
void *try_malloc(unsigned long int size){
    void *p = malloc(size); 
    if(p == NULL){
        perror("Error allocating memory");
        exit(1);
    }
    return p;
}
void getCurrentTime(struct datetime_t *timeb){
    struct tm *UTCtime;
    time_t now = time(0);
    UTCtime = gmtime(&now);
    timeb->sec = (UTCtime->tm_sec);
    timeb->min = (UTCtime->tm_min);
    timeb->hour = ((UTCtime->tm_hour+16)%24);
    timeb->day = (UTCtime->tm_mday-1);
    timeb->month = (UTCtime->tm_mon+1);
    timeb->year = ((UTCtime->tm_year+1900));
}
void find_in_subdir(char* file, int now_block, int number_of_block, int cur, int _file_blocks, int _file_size) {
    int i;
    // printf("%d %d %s\n", now_block, number_of_block, path[cur]);
    if (cur == k) {
        file_start_block = now_block;
        return;
    }
    int pre_block = now_block;
    for(i = 0; i < number_of_block * (blocksize / 64); i++){
        memcpy(&sub_file_status,file+(now_block*blocksize)+(i%8)*64,1);
        memcpy(&sub_filename,file+(now_block*blocksize)+(i%8)*64+27,31);
        memcpy(&sub_dir_starting_block, file+(now_block*blocksize)+(i%8)*64+1, 4);
        memcpy(&sub_dir_number_of_block, file+(now_block*blocksize)+(i%8)*64+5, 4);

        // printf("%d %d %d %d %s\n", i, now_block, htonl(sub_dir_starting_block), sub_file_status, sub_filename);
        if(sub_file_status == 5 && strcmp(sub_filename, path[cur]) == 0) {
            find_in_subdir(file, htonl(sub_dir_starting_block), htonl(sub_dir_number_of_block), cur + 1, _file_blocks, _file_size);
            return;
        }
        if (cur == k - 1 && sub_file_status == 3 && strcmp(sub_filename, path[cur]) == 0) {
            printf("file {%s} exsited", path[cur]);
            exit(1);
        }

        if(i + 1 < number_of_block * (blocksize / 64) && (i + 1) % 8 == 0){
            memcpy(&now_block, file + (fatstartblock * blocksize) + 4 * now_block, 4);
            now_block = htonl(now_block);
            // printf("%d\n", now_block);
        }
    }

    now_block = pre_block;
    int next_block = find_next(file, now_block);
    int temp = 0xFFFFFFFF;
    memcpy(file + fatstartblock * blocksize + 4 * next_block, &temp, 4);
    char status = (cur == k - 1 ? 3 : 5);
    int start_block = htonl(next_block);
    int number_block = htonl((cur == k - 1 ? _file_blocks : 1));
    int file_size = htonl((cur == k - 1 ? _file_size : blocksize));
    
    struct datetime_t *timeb = try_malloc(sizeof(struct datetime_t));
    getCurrentTime(timeb);
    short year = htons(timeb->year);

    for(i = 0; i < number_of_block * (blocksize / 64); i++){
        memcpy(&sub_file_status,file+(now_block*blocksize)+(i%8)*64,1);

        //current location is available
        if(sub_file_status == 0){
            memcpy(file + (now_block*blocksize) + (i%8)*64 , &status, 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 1, &start_block, 4);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 5, &number_block, 4);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 9, &file_size, 4);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 13, &year, 2);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 15, &(timeb->month), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 16, &(timeb->day), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 17, &(timeb->hour), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 18, &(timeb->min), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 19, &(timeb->sec), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 20, &year, 2);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 22, &(timeb->month), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 23, &(timeb->day), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 24, &(timeb->hour), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 25, &(timeb->min), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 26, &(timeb->sec), 1);
            memcpy(file + (now_block*blocksize) + (i%8)*64 + 27, path[cur], strlen(path[cur])+1);            

            find_in_subdir(file, htonl(start_block), htonl(number_block), cur + 1, _file_blocks, _file_size);
            return;
        }

        if(i + 1 < number_of_block * (blocksize / 64) && (i + 1) % 8 == 0){
            memcpy(&now_block, file + (fatstartblock * blocksize) + 4 * now_block, 4);
            now_block = htonl(now_block);
        }
    }
    //edit the file info
    next_block = find_next(file, next_block);
    temp = htonl(next_block);
    memcpy(file + fatstartblock * blocksize + 4 * now_block, &temp, 4);
    temp = 0xFFFFFFFF;
    memcpy(file + fatstartblock * blocksize + 4 * next_block, &temp, 4);
    temp = htonl(number_of_block + 1);
    memcpy(file + pre_block * blocksize + 5, &temp, 4);
    memcpy(&temp, file + pre_block * blocksize + 9, 4);
    temp = htonl(htonl(temp) + blocksize);
    memcpy(file + pre_block * blocksize + 9, &temp, 4);
    now_block = next_block;

    memcpy(file + (now_block*blocksize) , &status, 1);
    memcpy(file + (now_block*blocksize) + 1, &start_block, 4);
    memcpy(file + (now_block*blocksize) + 5, &number_block, 4);
    memcpy(file + (now_block*blocksize) + 9, &file_size, 4);
    memcpy(file + (now_block*blocksize) + 13, &year, 2);
    memcpy(file + (now_block*blocksize) + 15, &(timeb->month), 1);
    memcpy(file + (now_block*blocksize) + 16, &(timeb->day), 1);
    memcpy(file + (now_block*blocksize) + 17, &(timeb->hour), 1);
    memcpy(file + (now_block*blocksize) + 18, &(timeb->min), 1);
    memcpy(file + (now_block*blocksize) + 19, &(timeb->sec), 1);
    memcpy(file + (now_block*blocksize) + 20, &year, 2);
    memcpy(file + (now_block*blocksize) + 22, &(timeb->month), 1);
    memcpy(file + (now_block*blocksize) + 23, &(timeb->day), 1);
    memcpy(file + (now_block*blocksize) + 24, &(timeb->hour), 1);
    memcpy(file + (now_block*blocksize) + 25, &(timeb->min), 1);
    memcpy(file + (now_block*blocksize) + 26, &(timeb->sec), 1);
    memcpy(file + (now_block*blocksize) + 27, path[cur], strlen(path[cur])+1);            

    find_in_subdir(file, htonl(start_block), htonl(number_block), cur + 1, _file_blocks, _file_size);
}

void put_file(char* file_name, char* loc, char* file) {
    int fd = open(file_name, O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);
    char* p = mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int file_size = buffer.st_size;
    int file_blocks = file_size / blocksize + (file_size % blocksize == 0 ? 0 : 1);

    char input[1000];
    
    if(loc!=NULL){
        strcpy(input, loc);
        char* token = strtok(input, "/");
        while(token != NULL) {
            path[k++] = token;
            token = strtok(NULL, "/");
        }
    }
    find_in_subdir(file, rootdir_startblock, rootdir_block, 0, file_blocks, file_size);

    // 文件复制
    int i;
    int file_now_block = file_start_block;
    for (i = 0; i < file_size / blocksize; i++) {
        memcpy(file + file_now_block * blocksize, p + i * blocksize, blocksize);
        if (file_size % blocksize == 0 && i + 1 == file_size / blocksize) {
            int temp = 0xFFFFFFFF;
            memcpy(file + fatstartblock * blocksize + 4 * file_now_block, &temp, 4);
        } else {
            int file_next_block = htonl(find_next(file, file_now_block));
            memcpy(file + fatstartblock * blocksize + 4 * file_now_block, &file_next_block, 4);
            file_now_block = htonl(file_next_block);
        }   
    }
    if (file_size % blocksize > 0) {
        memcpy(file + file_now_block * blocksize, p + (file_size / blocksize) * blocksize, file_size % blocksize);
        int temp = 0xFFFFFFFF;
        memcpy(file + fatstartblock * blocksize + 4 * file_now_block, &temp, 4);
    }
}

void main(int argc, char* argv[]) {
    char filename[1000];
    char file_status;
    
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
	
    put_file(argv[2], argv[3], file);

    close(fd);
    return;
}

 
