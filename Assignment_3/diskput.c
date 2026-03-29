#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>
#include <limits.h>
#include <errno.h>
#include "FAT12.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

FAT12BootSector g_boot; 


void format_to_fat(const char* input, char* output) {
    memset(output, ' ', 11);
    output[11] = '\0'; 
    const char* dot = strchr(input, '.');
    int name_len;
    if(dot != NULL){
         name_len = (int)(dot - input);
    }
    else{
        name_len = (int)strlen(input);
    }

    for(int i = 0; i < name_len && i < 8; i++) 
        output[i] = toupper(input[i]);

    if(dot != NULL) {
        for (int i = 0; i < 3 && dot[i+1] != '\0'; i++){
             output[8 + i] = toupper(dot[i+1]);
        }
           
    }
}

char* copy_linux_file(char* path){ 
    FILE* fptr = fopen(path, "rb");
    if (fptr == NULL) {
        perror("File not found");
        return NULL;
    }

    fseek(fptr, 0, SEEK_END);
    long fsize = ftell(fptr);
    if (fsize == -1) {
        perror("Error getting file size");
        fclose(fptr);
        return NULL;
    }
    rewind(fptr);

    char *buffer = malloc(fsize + 1);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(fptr);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1, fsize, fptr);
    if (bytes_read != fsize) {
        perror("Error reading file");
        free(buffer);
        fclose(fptr);
        return NULL;
    }

    buffer[fsize] = '\0';

    fclose(fptr);
    return buffer;
}


bool directory_exists(const char* path){
    struct stat info;
    if(stat(path, &info) != 0){
        if(errno == ENOENT || errno == ENOTDIR){
            return false;
        }
        else{
            perror("stat error");
            return false; 
        }
    }
    return S_ISDIR(info.st_mode); 
}


int disk_put(uint8_t* disk_base, FAT12BootSector* boot, char* destination_path, char* source_buffer, size_t file_size) {
    uint32_t root_dir_size = sizeof(FAT12Directory) * boot->Root_Entry_Count;
    uint32_t root_dir_sectors = (root_dir_size + boot->Bytes_per_Sector - 1) / boot->Bytes_per_Sector;
    uint32_t first_data_sector = boot->Reserved_Sector_Count + (boot->Fat_Count * boot->Sectors_Per_Fat) + root_dir_sectors;
    uint32_t total_data_clusters = (boot->Total_Sectors - first_data_sector) / boot->Sectors_per_Cluster;

    uint32_t root_offset = (boot->Reserved_Sector_Count + (boot->Fat_Count * boot->Sectors_Per_Fat)) * boot->Bytes_per_Sector;
    FAT12Directory* current_dir = (FAT12Directory*)(disk_base + root_offset);
    uint32_t current_dir_max_entries = boot->Root_Entry_Count;
    uint8_t* fatTable = disk_base + (boot->Reserved_Sector_Count * boot->Bytes_per_Sector);

    char* path_copy = strdup(destination_path);
    char* token = strtok(path_copy, "/");
    char* next_token = strtok(NULL, "/");

    while(next_token != NULL){
        char search_name[12];
        format_to_fat(token, search_name);
        bool found_subdir = false;
        
        for(uint32_t i = 0; i < current_dir_max_entries; i++) {
            if (memcmp(current_dir[i].File_Name, search_name, 11) == 0){
                if(!(current_dir[i].Attributes & 0x10)){
                    free(path_copy); return 0;
                }
                uint32_t cluster = current_dir[i].Low_Bits;
                uint32_t cluster_offset = (first_data_sector + (cluster - 2) * boot->Sectors_per_Cluster) * boot->Bytes_per_Sector;
                current_dir = (FAT12Directory*)(disk_base + cluster_offset);
                current_dir_max_entries = (boot->Bytes_per_Sector * boot->Sectors_per_Cluster) / sizeof(FAT12Directory);
                found_subdir = true;
                break;
            }
        }
        if (!found_subdir){ 
            fprintf(stderr, "Dir %s not found\n", token); 
            free(path_copy); return 0; 
        }
        token = next_token;
        next_token = strtok(NULL, "/");
    }

    char final_fat_name[12];
    format_to_fat(token, final_fat_name);

    for (uint32_t i = 2; i < (total_data_clusters + 2); i++){
        uint32_t f_offset = (i * 3) / 2;
        uint16_t val;
        if (i % 2 == 0) val = ((fatTable[f_offset + 1] & 0x0F) << 8) | fatTable[f_offset];
        else val = (fatTable[f_offset + 1] << 4) | (fatTable[f_offset] >> 4);

        if(val == 0x000){
            uint32_t d_offset = (first_data_sector + (i - 2) * boot->Sectors_per_Cluster) * boot->Bytes_per_Sector;
            memcpy(disk_base + d_offset, source_buffer, file_size);

            if(i % 2 == 0){
                fatTable[f_offset] = 0xFF;
                fatTable[f_offset + 1] = (fatTable[f_offset + 1] & 0xF0) | 0x0F;
            } 
            else{
                fatTable[f_offset] = (fatTable[f_offset] & 0x0F) | 0xF0;
                fatTable[f_offset + 1] = 0xFF;
            }

            for(int j = 0; j < current_dir_max_entries; j++) {
                if(current_dir[j].File_Name[0] == 0x00 || (uint8_t)current_dir[j].File_Name[0] == 0xE5) {
                    printf("Target: %s | Cluster: %u | Offset: %u | Size: %zu\n", final_fat_name, i, d_offset, file_size);
                    memcpy(current_dir[j].File_Name, final_fat_name, 11);
                    current_dir[j].Low_Bits = (uint16_t)i;
                    current_dir[j].File_Size = (uint32_t)file_size;
                    current_dir[j].Attributes = 0x00;
                    free(path_copy);
                    return 1; 
                }
            }
        }
    }
    free(path_copy);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <disk image> <path on disk>\n", argv[0]);
        return -1;
    }

    char* path_on_disk = argv[2];
    char* local_filename = strrchr(path_on_disk, '/');

    if(local_filename != NULL){
        local_filename++; 
    } 
    else{
        local_filename = path_on_disk; 
    }

    struct stat linux_st;
    if(stat(local_filename, &linux_st) != 0) {
        fprintf(stderr, "Error: Local file '%s' not found.\n", local_filename);
        return -1;
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0){ 
        perror("open disk");
        return -1; 
    }

    struct stat disk_st;
    fstat(fd, &disk_st);

    uint8_t* disk_base = mmap(NULL, disk_st.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (disk_base == MAP_FAILED){
        perror("mmap"); return -1; 
    } 

    FAT12BootSector* boot = (FAT12BootSector*)disk_base;
    
    char* file_content = copy_linux_file(local_filename); 
    if(!file_content){
        return -1;
    } 
    
    if (disk_put(disk_base, boot, path_on_disk, file_content, (size_t)linux_st.st_size)) {
        printf("File copied successfully to %s\n", path_on_disk);
    } 
    else {
        printf("Failed to copy file.\n"); 
    }

    munmap(disk_base, disk_st.st_size);
    close(fd);
    free(file_content);
    return 0;
}