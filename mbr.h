/**
* @file
* @brief Definiciones para discos inicializados con esquema BR
*/

#ifndef MBR_H
#define MBR_H

/** @brief Boot sector signature */
#define MBR_SIGNATURE 0xAA55

/** @brief GPT Partition type - MBR */
#define MBR_TYPE_GPT 0xEE

/** @brief Unused partition table - MBR*/
#define MBR_TYPE_UNUSED 0x00

/** @brief Longitud maxima del texto para el tipo de particion */
#define TYPE_NAME_LEN 256

/** @brief Partition descriptor - MBR */
typedef struct {
	/* TODO definir los atributos para la estructura de un descriptor de particion MBR */
	/* La suma completa de bytes de esta estructura debe ser 16 */
	unsigned char boot_flag;
	unsigned char chs_first[3];
	unsigned char type;
	unsigned char chs_last[3];
	unsigned int start_lba;
	unsigned int size_in_lba; //Number of sectors
}__attribute__((packed)) mbr_partition_descriptor;

/** @brief Master Boot Record. */
typedef struct {
	unsigned char bootsector_code[446];
	mbr_partition_descriptor partition_table[4];
	unsigned short signature;
}__attribute__((packed)) mbr;

/**
* @brief Checks if a bootsector is a MBR.
* @param boot_record Bootsector read in memory]
* @return 1 If the bootsector is a Protective MBR, 0 otherwise.
*/
int is_mbr(mbr * boot_record);

/**
* @brief Text description of a MBR partition type
* @param type Partition type reported in MBR
* @param buf String buffer to store the text description
*/
void mbr_partition_type(unsigned char type, char buf[TYPE_NAME_LEN]);


#endif
