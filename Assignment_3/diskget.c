# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
#include <string.h>
# include <stdbool.h>
# include <inttypes.h>
#include <ctype.h>
# include "FAT12.h"
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

void format_to_fat(const char* input, char* output) {
    memset(output, ' ', 11);
    output[11] = '\0';
    
    char* dot = strchr(input, '.');
    int name_part_len = dot ? (int)(dot - input) : (int)strlen(input);

    for (int i = 0; i < 8 && i < name_part_len; i++)
        output[i] = toupper(input[i]);

    if (dot) {
        for (int i = 0; i < 3 && dot[i+1] != '\0'; i++)
            output[8 + i] = toupper(dot[i+1]);
    }
}

int disk_get(FILE* fptr, FAT12BootSector* boot, FAT12Directory* root_dir, char* filename_arg) {

    char fat_name[12]; 
    format_to_fat(filename_arg, fat_name);

    FAT12Directory* target_file = Find_File(fat_name, root_dir, boot);
    if (target_file == NULL) {
        fprintf(stderr, "Error: File '%s' not found.\n", filename_arg);
        return 0; 
    }

    uint8_t* fatTable = NULL;
    if (!Read_FAT(fptr, boot, &fatTable)) {
        return 0;
    }

    uint32_t cluster_size = boot->Sectors_per_Cluster * boot->Bytes_per_Sector;
    uint32_t bytes_to_alloc = ((target_file->File_Size + cluster_size - 1) / cluster_size) * cluster_size;
    uint8_t* buffer = malloc(bytes_to_alloc);

    int status = 0;
    if (Read_File(target_file, fptr, boot, fatTable, buffer)) {
        FILE* dest = fopen(filename_arg, "wb");
        if (dest) {
            fwrite(buffer, 1, target_file->File_Size, dest);
            fclose(dest);
            printf("Successfully Copied %s\n", filename_arg);
            status = 1;
        }
    }
    
    free(buffer);
    free(fatTable);
    return status;
}

int main(int argc, char* argv[]) {
    int fd;
	struct stat sb;

	fd = open(argv[1], O_RDWR);
	fstat(fd, &sb);
	printf("Size: %lu\n\n", (uint64_t)sb.st_size);

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <disk image> <file name>\n", argv[0]);
        return -1;
    }

    FILE* fptr = fopen(argv[1], "rb");
    if (!fptr) { perror("Error"); return -1; }

    FAT12BootSector temp_boot;
    Read_Boot_Sector(fptr, &temp_boot);
    FAT12Directory* root_dir = Read_Root_Directory(fptr, &temp_boot);

    if (disk_get(fptr, &temp_boot, root_dir, argv[2])) {
      
    } else {
        fprintf(stderr, "Operation failed.\n");
    }

    // Cleanup everything main is responsible for
    if (root_dir) free(root_dir);
    fclose(fptr);

    return 0;
}