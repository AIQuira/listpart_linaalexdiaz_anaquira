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

#include "mbr.h"
#include "gpt.h"

/**
 * @brief Sector size defined in 512 bytes
 * 
 */
#define SECTOR_SIZE 512

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

int main(int argc, char *argv[])
{
	int i;
	char *disk;

	// 1. Validar argumentos de línea de comandos
	if (argc < 2)
	{
		usage();
		exit(EXIT_FAILURE);
	}

	// 2. Leer el primer sector del disco

	mbr boot_record;
	for (i = 1; i < argc; i++)
	{		
		disk = argv[i];

		// 2.1. Si la lectura falla, terminar.
		printf("Reading first sector disk: %s\n", disk);
		if (read_lba_sector(disk, 0, (char*)&boot_record) == 0)
		{
			fprintf(stderr, "Unable to open the device%s\n", disk);
			exit(EXIT_FAILURE);
		}
		
		printf("First sector read successfully. Hex dump:\n");
		hex_dump((char*)&boot_record, sizeof(mbr));

		if (is_mbr(&boot_record)) {
			printf("MBR detected\n");
			//4.Listar las particiones

		} else if (is_protective_mbr(&boot_record)) {
			printf("Protective MBR detected\n");
			gpt_header gpt_hdr;
			if (read_lba_sector(disk, 0, (char*)&gpt_hdr) && is_valid_gpt_header(&gpt_hdr)) {
				printf("GPT detected\n");
				//5. Imprimir la tabla de particiones GPT
				//5.1. El encabezado de la tabla de particiones GPT (se encuentra en el segundo sector en el disco) indica cuántos descritores están definidos.
				//5.2. Leer los descriptores, que se encuentran en los siguientes sectores.
			} else {
				printf("Invalid GPT header\n");
			}
		} else {
			printf("Unknown partition table\n");
		}
	}
	

	//	- La lectura puede fallar por no tener acceso.
	// 3. Verificar si es un MBR o un GPT
	//	- Tanto el MBR como el GTP son MBR. Esto conlleva a que se deba imprimir la tabla de particiones.
	// 4. Listar las particiones
	// 4.1 Si el esquema de particionado es MBR, imprimir la tabla de particiones y terminar el programa.
	// PRE: El sistema de particionado es GPT
	// 5. Imprimir la tabla de particiones GPT.
	// 5.1. El encabezado de la tabla de particiones GPT (se encuentra en el segundo sector en el disco) indica cuántos descritores están definidos.
	// 5.2. Leer los descriptores, que se encuentran en los siguientes sectores.
	// 5.3. Imprimir la información de los descriptores leídos.
	// En cada sector caben 4 descriptores. De acuerdo a los descriptores que dice la tabla, podemos definir la cantidad de sectore que se deben leer.


// - Imprimirlas particiones de MBR
// - Comprobar si es MBR o GPT (Asesoría)
// - Im

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

	//Move forward to the reading prompt
	if(fseek(fd, lba*SECTOR_SIZE, SEEK_SET) != 0)
	{
		perror("Error seeking disk");
		fclose(fd);
		return 0;
	};

	//Read disk sector
	if(fread(buf, 1, SECTOR_SIZE, fd) != SECTOR_SIZE)
	{
		perror("Error reading disk");
		return 0;
	}

	fclose(fd);

	return 1;
	//return 0;
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
}