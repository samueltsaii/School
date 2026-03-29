# include <stdlib.h>
# include <stdio.h>
# include <stdint.h>
# include <stdbool.h>

typedef struct {

    uint8_t  Boot_Jump[3];
    uint8_t  OEM_name[8];
    uint16_t Bytes_per_Sector;  
    uint8_t  Sectors_per_Cluster;
    uint16_t Reserved_Sector_Count;
    uint8_t  Fat_Count;            
    uint16_t Root_Entry_Count;     
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



bool Read_Boot_Sector(FILE* disk, FAT12BootSector* boot);

bool Read_Sectors(FILE* disk, uint32_t lba, FAT12BootSector* boot, uint32_t count, void* buffer);

bool Read_FAT(FILE* disk, FAT12BootSector* boot, uint8_t** g_FAT);

FAT12Directory* Read_Root_Directory(FILE* disk, FAT12BootSector* boot); 

FAT12Directory* Find_File(const char* name, FAT12Directory* Dir, FAT12BootSector* boot);

bool Read_File(FAT12Directory* file_entry, FILE* disk, FAT12BootSector* boot, uint8_t* fat, uint8_t* buffer); 