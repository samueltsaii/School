# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>
# include <inttypes.h>
# include "FAT12.h"

FAT12BootSector g_boot;

void disk_list(FAT12Directory* root_dir, FAT12BootSector* boot) {
    for (uint32_t i = 0; i < boot->Root_Entry_Count; i++) {
       
        if(root_dir[i].File_Name[0] == 0x00){
            break;
        }
        if(root_dir[i].File_Name[0] == 0xE5){
            continue;
        }
        if(root_dir[i].Attributes == 0x08){
            continue; 
        }

        uint16_t d = root_dir[i].Creation_Date;
        int day = d & 0x1F;           
        int month = (d >> 5) & 0x0F;    
        int year = (d >> 9) + 1980;      

        char type;
        if(root_dir[i].Attributes & 0x10){
            type = 'D';
        } 
        else{
            type = 'F';
        }
       
        printf("*%c | Name: %.8s.%.3s | Size: %u Bytes | Date: %02d/%02d/%d\n", 
                type, 
                root_dir[i].File_Name,      
                &root_dir[i].File_Name[8],  
                root_dir[i].File_Size, 
                day, month, year);
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <disk image>\n", argv[0]);
        return -1; 
    }

    FILE* fptr = fopen(argv[1], "rb");
    if (!fptr) { perror("Error opening image"); return -1; }

    if (!Read_Boot_Sector(fptr, &g_boot)) {
        fprintf(stderr, "Error reading boot Sector\n");
        fclose(fptr);
        return -1; 
    }

    FAT12Directory* rootDir = Read_Root_Directory(fptr, &g_boot);

    if (rootDir != NULL) {
        disk_list(rootDir, &g_boot); 
        free(rootDir);
    } else {
        fprintf(stderr, "Failed to read Root Directory\n");
    }

    fclose(fptr);
    return 0;
}