/**
 * @file
 * @author Erwin Meza Vega <emezav@unicauca.edu.co>
 * @brief Listar particiones de discos duros MBR/GPT
 * @copyright MIT License
 */

#include <fcntl.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>

#include "mbr.h"
#include "gpt.h"

/**
 * @brief Sector size defined in 512 bytes
 *
 */
#define SECTOR_SIZE 512
unsigned char buffer[SECTOR_SIZE];

/**
 * @brief Hex dumps a buffer
 * @param buf Pointer to buffer
 * @param size Buffer size
 */
void hex_dump(char *buf, size_t size);

/**
 * @brief ASCII dumps a buffer
 * @param buf Pointer to buffer
 * @param size Buffer size
 */
void ascii_dump(char *buf, size_t size);

/**
 * @brief Read a sector from a disk
 *
 * @param disk Disk filename
 * @param lba Sector to read (from0 to amount of LBA sectors on disk)
 * @param buf Buffer to read the sector into
 * @return int 1 to success, 0 on failure
 */
int read_lba_sector(char *disk, unsigned long long lba, char buf[512]);

/**
 * @brief Print usage information
 *
 */
void usage();

/**
 * @brief Print MBR partition table
 * 
 * @param boot_record 
 */
void print_mbr_partitions(mbr *boot_record);

/**
 * @brief Print GPT partition table
 * 
 * @param gpt_hdr 
 * @param disk 
 */
void print_gpt_partitions(gpt_header *gpt_hdr, char *disk);

int main(int argc, char *argv[])
{
	int i;
	char *disk;

	// 1. Validar argumentos de línea de comandos
	if (argc < 2)
	{
		printf("Invalid number of arguments\n");
		usage();
		exit(EXIT_FAILURE);
	}

	// 2. Leer el primer sector del disco
	mbr boot_record;
	for (i = 1; i < argc; i++)
	{
		printf("Processing disk: %s\n", argv[i]);
		disk = argv[i];

		// 2.1. Si la lectura falla, terminar.
		printf("Reading first sector disk: %s\n", disk);
		if (read_lba_sector(disk, 0, (char *)&boot_record) == 0)
		{
			fprintf(stderr, "Unable to open the device%s\n", disk);
			exit(EXIT_FAILURE);
		}

		if (is_mbr(&boot_record)) {
			if (is_protective_mbr(&boot_record)) {
				printf("Disk initialized as GPT\n");
				print_mbr_partitions(&boot_record);
				gpt_header gpt_hdr;

				printf("Reading GPT header sector\n");
				if (read_lba_sector(disk, 1, (char *)buffer)) {
					printf("GPT header sector read successfully\n");

					printf("GPT Header Info: \n");
					memcpy(&gpt_hdr, buffer, sizeof(gpt_header));
					printf("  Signature: %.8s\n", gpt_hdr.signature);
					printf("  Header Size: %u\n", gpt_hdr.header_size);
					printf("  Revision: 0x%x\n", gpt_hdr.revision);
					printf("  First usable LBA: %llu\n", gpt_hdr.first_usable_lba);
					printf("  Last usable LBA: %llu\n", gpt_hdr.last_usable_lba);
					printf("  Disk GUID: %s\n", guid_to_str(&gpt_hdr.disk_guid));
					printf("  Partition Entry LBA: %llu\n", gpt_hdr.partition_entry_lba);
					printf("  Number of Partition Entries: %u\n", gpt_hdr.num_partition_entries);
					printf("  Size of Partition Entry: %u\n", gpt_hdr.size_of_partition_entry);
					printf("  Total of partition table entries sectors: %u\n", gpt_hdr.num_partition_entries * gpt_hdr.size_of_partition_entry / SECTOR_SIZE);
					printf("  Size of a partition descriptor: %lu\n", sizeof(gpt_partition_descriptor));

					if (is_valid_gpt_header(&gpt_hdr)) {
						printf("GPT detected\n");
						print_gpt_partitions(&gpt_hdr, disk);
					} else {
						printf("Invalid GPT header\n");
					}
				} else {
					printf("Error reading GPT header sector\n");
				}
			} else {
				// 4.Listar las particiones
				printf("Disk initialized as MBR\n");
				print_mbr_partitions(&boot_record);
			}
		} else {
			printf("Unknown partition table\n");
		}
	}

	return 0;
}

int read_lba_sector(char *disk, unsigned long long lba, char buf[SECTOR_SIZE])
{

	FILE *fd;

	// Abrir el disco en modo lectura
	fd = fopen(disk, "r");

	// Valida que el disco se haya abierto correctamente
	if (fd == NULL)
	{
		perror("Error opening disk");
		return 0;
	}

	// Move forward to the reading prompt
	if (fseek(fd, lba * SECTOR_SIZE, SEEK_SET) != 0)
	{
		perror("Error seeking disk");
		fclose(fd);
		return 0;
	};

	// Read disk sector
	if (fread(buf, 1, SECTOR_SIZE, fd) != SECTOR_SIZE)
	{
		perror("Error reading disk");
		return 0;
	}

	fclose(fd);

	return 1;
	// return 0;
}

void ascii_dump(char *buf, size_t size)
{
	for (size_t i = 0; i < size; i++)
	{
		if (buf[i] >= 0x20 && buf[i] < 0x7F)
		{
			printf("%c", buf[i]);
		}
		else
		{
			printf(".");
		}
	}
}

void hex_dump(char *buf, size_t size)
{
	int cols;
	cols = 0;
	for (size_t i = 0; i < size; i++)
	{
		printf("%02x ", buf[i] & 0xff);
		if (++cols % 16 == 0)
		{
			ascii_dump(&buf[cols - 16], 16);
			printf("\n");
		}
	}
}

void usage()
{
	printf("Usage: \n");
	printf("listpart disk : List disk partitions\n");
}

void print_mbr_partitions(mbr *boot_record){
	char type_name[TYPE_NAME_LEN];
	printf("MBR Partition Table\n");
	printf("Start LBA    End LBA      Type\n");
	printf("------------ ------------ -------------------------------------\n");
	for (int i = 0; i < 4; i++)
	{
		if (boot_record->partition_table[i].type != 0)
		{
			mbr_partition_type(boot_record->partition_table[i].type, type_name);
			printf("%12u %12u %20s\n",
				boot_record->partition_table[i].start_lba,
				boot_record->partition_table[i].start_lba + boot_record->partition_table[i].size_in_lba - 1,
				type_name);
		}
	}
	printf("------------ ------------ -------------------------------------\n");
}

void print_gpt_partitions(gpt_header *gpt_hdr, char *disk) {
	unsigned char sector_buffer[SECTOR_SIZE];
	gpt_partition_descriptor descriptor;
	unsigned int descriptors_per_sector = SECTOR_SIZE / gpt_hdr->size_of_partition_entry;
	unsigned long long total_desc = gpt_hdr->num_partition_entries;
	unsigned long long lba = gpt_hdr->partition_entry_lba;

	printf("Start LBA    End LBA      Size         Partition Type GUID                  Partition Name\n");
	printf("------------ ------------ ------------ ------------------------------------ ------------------------------\n");

	for (unsigned long long i = 0; i < total_desc; i++)
	{
		if (i % descriptors_per_sector == 0)
		{
			if (!read_lba_sector(disk, lba, (char *)sector_buffer))
			{
				fprintf(stderr, "Error reading sector %llu\n", lba);
				break;
			}
			lba++;
		}

		memcpy(&descriptor, &sector_buffer[(i % descriptors_per_sector) * gpt_hdr->size_of_partition_entry], gpt_hdr->size_of_partition_entry);

		if (!is_null_descriptor(&descriptor))
		{
			char *guid_str = guid_to_str(&descriptor.partition_type_guid);
			char *partition_name = gpt_decode_partition_name((char *)descriptor.partition_name);

			printf("%12llu %12llu %12llu %-37s %s\n",
					descriptor.starting_lba,
					descriptor.ending_lba,
					descriptor.ending_lba - descriptor.starting_lba + 1,
					guid_str,
					partition_name ? partition_name : "(Unnamed)");

			free(guid_str);
		}
	}
	printf("------------ ------------ ------------ ------------------------------------ ------------------------------\n");
}