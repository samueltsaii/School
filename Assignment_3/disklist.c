# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>
# include <inttypes.h>
# include "FAT12.h"

FAT12BootSector g_boot;
/*Takes in a pointer referencing contents to the root directory and a pointer referencing the boot sector. The function iterates through the root directory
and searches for file and directory entries based on the attribute.*/

void disk_list(FAT12Directory* root_dir, FAT12BootSector* boot) {
    for (uint32_t i = 0; i < boot->Root_Entry_Count; i++) {
        /*Continue for recently deleted entries*/
        if(root_dir[i].File_Name[0] == 0xE5){
            continue;
        }
        /*Continue if contains label name*/
        if(root_dir[i].Attributes == 0x08){
            continue; 
        }
        /*Apply bit mask, bit shift operations, and offset for the date.*/
        uint16_t d = root_dir[i].Creation_Date;
        int day = d & 0x1F;           
        int month = (d >> 5) & 0x0F;    
        int year = (d >> 9) + 1980;     
        /*Apply bit mask and bit shift operations for time variables*/
        uint16_t t = root_dir[i].Creation_Time;
        int second = (t & 0x1F) * 2;
        int minute = (t >> 5) & 0x3F;
        int hour = (t >> 11) & 0x1F; 
        /*D: directory.
        F: file*/
        char type;
        if(root_dir[i].Attributes & 0x10){
            type = 'D';
        } 
        else{
            type = 'F';
        }
       
        printf("*%c | Name: %.8s.%.3s | Size: %u Bytes | Date: %02d/%02d/%d | Time: %02d:%02d:%02d\n", type, root_dir[i].File_Name, &root_dir[i].File_Name[8], root_dir[i].File_Size, day, month, year, hour, minute, second);
    }
}

int main(int argc, char* argv[]) {

     /*Handles user input command*/
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <disk image>\n", argv[0]);
        return -1; 
    }
    FILE* fptr = fopen(argv[1], "rb");
    if (!fptr) { 
        perror("Error opening image"); return -1; 
    }
    /*Check boot sector*/
    if (!Read_Boot_Sector(fptr, &g_boot)) {
        fprintf(stderr, "Error reading boot Sector\n");
        fclose(fptr);
        return -1; 
    }
    /*Check pointer to root directory contents*/
    FAT12Directory* rootDir = Read_Root_Directory(fptr, &g_boot);

    if (rootDir != NULL) {
        /*List disk contents*/
        disk_list(rootDir, &g_boot); 
        free(rootDir);
    } else {
        fprintf(stderr, "Failed to read Root Directory\n");
    }

    fclose(fptr);
    return 0;
}