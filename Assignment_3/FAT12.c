# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>
# include <string.h>

/*Contains the BIOS of the FAT12 file system*/
typedef struct {

    uint8_t  Boot_Jump[3];
    uint8_t  OEM_name[8];
    uint16_t Bytes_per_Sector;  
    uint8_t  Sectors_per_Cluster;
    uint16_t Reserved_Sector_Count;
    uint8_t  Fat_Count;            
    uint16_t Dir_Entry_Count;     
    uint16_t Total_Sectors;     
    uint8_t  Media_Descriptor;
    uint16_t Sectors_Per_Fat;
    uint16_t Sectors_Per_Track;
    uint16_t Head_Count;
    uint32_t Hidden_Sectors;
    uint32_t Total_Sectors_32;

    // Extended Boot Record
    uint8_t  Drive_Number;         
    uint8_t  Reserved;
    uint8_t  Boot_Signature;       
    uint32_t Volume_ID;
    uint8_t  Volume_Label[11];
    uint8_t  File_System_Type[8];  
    
} __attribute__((packed)) FAT12BootSector;

typedef struct{
    uint8_t File_Name[11];
    uint8_t Attributes;
    uint8_t Reserved;
    uint8_t Creation_Time;
    uint16_t Creation_Time_Mult2;
    uint16_t Creation_Date;
    uint16_t Date_Accessed; 
    uint16_t High_Bits;
    uint16_t Last_Mod_Time;
    uint16_t Last_MOD_Date;
    uint16_t Low_Bits;
    uint32_t File_Size; 
}__attribute__((packed)) FAT12Directory; 

/*Helper functions*/
bool Read_Boot_Sector(FILE* disk, FAT12BootSector* boot){
    return fread(boot, sizeof(FAT12BootSector), 1, disk) > 0;
}

bool Read_Sectors(FILE* disk, uint32_t lba, FAT12BootSector* boot, uint32_t count, void* buffer)
{
    bool ok = true;
    ok = ok && (fseek(disk, lba * boot -> Bytes_per_Sector, SEEK_SET) == 0) && (fread(buffer, boot -> Bytes_per_Sector, count, disk) == count);
    return ok;
}

bool Read_FAT(FILE* disk, FAT12BootSector* boot, uint8_t** g_FAT)
{
    *g_FAT = (uint8_t*) malloc(boot->Sectors_Per_Fat * boot->Bytes_per_Sector);
    uint32_t fat_lba = boot->Reserved_Sector_Count;

    return Read_Sectors(disk, fat_lba, boot, boot->Sectors_Per_Fat, *g_FAT);
}

FAT12Directory* Read_Root_Directory(FILE* disk, FAT12BootSector* boot)
{
    uint32_t lba = boot->Reserved_Sector_Count + (boot->Sectors_Per_Fat * boot->Fat_Count);
    uint32_t size = sizeof(FAT12Directory) * boot->Dir_Entry_Count;
    uint32_t sectors = (size + boot->Bytes_per_Sector - 1) / boot->Bytes_per_Sector;
    FAT12Directory* root_buffer = (FAT12Directory*) malloc(sectors * boot->Bytes_per_Sector);
    
    if (root_buffer == NULL) {
        perror("Failed to allocate memory for Root Directory");
        return NULL;
    }

    if (Read_Sectors(disk, lba, boot, sectors, root_buffer)) {
        return root_buffer;
    } else {
        free(root_buffer);
        return NULL;
    }
}

FAT12Directory* Find_File(const char* name, FAT12Directory* Dir, FAT12BootSector* boot) {
    
    for (uint32_t i = 0; i < boot->Dir_Entry_Count; i++) {
        if (memcmp(name, Dir[i].File_Name, 11) == 0) {
            return &Dir[i];
        }
    }
    return NULL;
}

bool Read_File(FAT12Directory* file_entry, FILE* disk, FAT12BootSector* boot, uint8_t* fat, uint8_t* buffer) {
    uint16_t current_cluster = file_entry->Low_Bits;
    
    uint32_t root_dir_size = sizeof(FAT12Directory) * boot->Dir_Entry_Count;
    uint32_t root_dir_sectors = (root_dir_size + boot->Bytes_per_Sector - 1) / boot->Bytes_per_Sector;
    uint32_t first_data_sector = boot->Reserved_Sector_Count + (boot->Fat_Count * boot->Sectors_Per_Fat) + root_dir_sectors;

    while (current_cluster >= 0x002 && current_cluster < 0x0FF8) {
        uint32_t lba = first_data_sector + (current_cluster - 2) * boot->Sectors_per_Cluster;

        Read_Sectors(disk, lba, boot, boot->Sectors_per_Cluster, buffer);
        buffer += (boot->Sectors_per_Cluster * boot->Bytes_per_Sector);

        uint32_t fat_offset = current_cluster + (current_cluster / 2);
        uint16_t next_cluster = *(uint16_t*)(fat + fat_offset);

        if (current_cluster & 1) next_cluster >>= 4;
        else                     next_cluster &= 0x0FFF;

        current_cluster = next_cluster;
    }
    return true;
}