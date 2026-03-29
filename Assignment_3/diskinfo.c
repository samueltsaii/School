# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>
# include <inttypes.h>
# include "FAT12.h"

FAT12BootSector g_boot;

void find_label(FAT12Directory* rootDir, FAT12BootSector* boot){
    bool found = false;
    for(uint32_t i = 0; i < boot -> Root_Entry_Count; i++){
        if(rootDir[i].Attributes == 0x08){
            printf("Label of the disk: %8.8s\n", rootDir[i].File_Name);
            found = true;
            return; 
        }
    }
    printf("Cannot find label...\n"); 
}

uint32_t count_free(uint8_t* fat, FAT12BootSector* boot) {
    if (fat == NULL) return 0;

    uint32_t free_clusters = 0;

    uint32_t root_dir_size = sizeof(FAT12Directory) * boot->Root_Entry_Count;
    uint32_t root_dir_sectors = (root_dir_size + boot->Bytes_per_Sector - 1) / boot->Bytes_per_Sector;
    
    uint32_t non_data_sectors = boot->Reserved_Sector_Count + 
                                (boot->Fat_Count * boot->Sectors_Per_Fat) + 
                                root_dir_sectors;

    uint32_t total_sectors = boot->Total_Sectors;
    uint32_t data_sectors = total_sectors - non_data_sectors;
    uint32_t total_data_clusters = data_sectors / boot->Sectors_per_Cluster;

    for (uint32_t i = 2; i < (total_data_clusters + 2); i++) {
        uint32_t fat_offset = i + (i / 2);
        uint16_t val = *(uint16_t*)(fat + fat_offset);
        
        if (i & 1){
         val >>= 4;
              }      // Odd entry
        else{
            val &= 0x0FFF;  // Even entry
        }

        if (val == 0x000) {
            free_clusters++;
        }
    }
    return free_clusters * boot->Sectors_per_Cluster * boot->Bytes_per_Sector;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <disk image>\n", argv[0]);
        return -1; 
    }

    FILE* fptr = fopen(argv[1], "rb");
    if (fptr == NULL) {
        perror("Error opening image"); 
        return -1; 
    }

    if(!Read_Boot_Sector(fptr, &g_boot)){
        fprintf(stderr, "Error reading boot Sector\n");
        fclose(fptr);
        return -1; 
    }

    FAT12Directory* rootDir = Read_Root_Directory(fptr, &g_boot);

    uint32_t total_disk_size = (uint32_t)g_boot.Bytes_per_Sector * g_boot.Total_Sectors; 
    uint8_t* fatTable = NULL;

     if (rootDir != NULL) {
        find_label(rootDir, &g_boot);
    } 
    else {
        fprintf(stderr, "Failed to read Root Directory\n");
    }
    uint32_t free_size = 0;

    if(!Read_FAT(fptr, &g_boot, &fatTable)){
        fprintf(stderr, "Cannot read FAT\n"); 
    }

    if(fatTable != NULL){
        free_size = count_free(fatTable,&g_boot); 
    }
    
    printf("OS Name: %8.8s\n", g_boot.OEM_name);
    printf("Total disk size: %u bytes\n", total_disk_size);
    printf("Free size of disk: %"PRIu32" bytes\n", free_size); 
    printf("============\n");
    printf("The number of files in disk (including all files in root directory and files in subdirectories)");
    printf("\n");
    printf("\n");
    printf("============\n");
    printf("Number of FATs: %d\n", g_boot.Fat_Count);
    printf("Sectors per FAT: %d\n", g_boot.Sectors_Per_Fat); 

    if(rootDir){
        free(rootDir); 
    }
    if(fatTable){
        free(fatTable); 
    }
    fclose(fptr);
    return 0;
}