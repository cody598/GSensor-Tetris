#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdbool.h>
#include <linux/i2c-dev.h>
#include "ADXL345.h"
#include "hwlib.h"

#define HW_REGS_BASE ( 0xFC000000 )
#define HW_OCRAM_BASE ( 0xC8000000 )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )
#define FPGA_CHAR_BASE      0xC9000000

// Define Colors
#define CYAN 0x07FF // I
#define YELLOW 0xFF40 // O
#define PURPLE 0x8813 // T
#define GREEN 0x07E3 // S
#define RED 0xF800 // Z
#define BLUE 0x02B5 // J
#define ORANGE 0xFC80 // L
#define BLACK 0x0000

#define PHYSMEM_32(addr) (*((unsigned int *)(virtual_base + (addr & HW_REGS_MASK)))) // Char
#define PHYSMEM_16(addr) (*((unsigned short *)(virtual_base + (addr & HW_REGS_MASK)))) // Pixel

/****************************************************************************************
 * GSensor Functions ADXL345
****************************************************************************************/
bool ADXL345_REG_WRITE(int file, uint8_t address, uint8_t value){
	bool bSuccess = false;
	uint8_t szValue[2];
	
	// write to define register
	szValue[0] = address;
	szValue[1] = value;
	if (write(file, &szValue, sizeof(szValue)) == sizeof(szValue)){
			bSuccess = true;
	}
		
	
	return bSuccess;		
}

bool ADXL345_REG_READ(int file, uint8_t address,uint8_t *value){
	bool bSuccess = false;
	uint8_t Value;
	
	// write to define register
	if (write(file, &address, sizeof(address)) == sizeof(address)){
	
		// read back value
		if (read(file, &Value, sizeof(Value)) == sizeof(Value)){
			*value = Value;
			bSuccess = true;
		}
	}
		
	
	return bSuccess;	
}

bool ADXL345_REG_MULTI_READ(int file, uint8_t readaddr,uint8_t readdata[], uint8_t len){
	bool bSuccess = false;

	// write to define register
	if (write(file, &readaddr, sizeof(readaddr)) == sizeof(readaddr)){
		// read back value
		if (read(file, readdata, len) == len){
			bSuccess = true;
		}
	}
	
		
	return bSuccess;
}


/****************************************************************************************
 * Draw a filled rectangle on the VGA monitor 
****************************************************************************************/
void VGA_box(int x1, int y1, int x2, int y2, short pixel_color, void *virtual_base)
{ 
	unsigned int pixel_ptr, row, col;

	/* assume that the box coordinates are valid */
	for (row = y1; row <= y2; row++)
		for (col = x1; col <= x2; ++col)
		{
			pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
			PHYSMEM_16(pixel_ptr) = pixel_color;		// set pixel color
		}
}

/****************************************************************************************
 * Draw text to the VGA monitor 
****************************************************************************************/
void VGA_text(int x, int y, char * text_ptr, void *virtual_base)
{
	int offset;
  	unsigned int  char_ptr;

	/* assume that the text string fits on one line */
	offset = (y << 7) + x;
	while ( *(text_ptr) )
	{
		char_ptr =  FPGA_CHAR_BASE + offset;
		
		PHYSMEM_32(char_ptr) = *(text_ptr);
		
		++text_ptr;
		++offset;
	}
}

/****************************************************************************************
 * Clear anything on the VGA monitor 
****************************************************************************************/
void VGA_Clear(void * virtual_base)
{
	int offset;
	unsigned int pixel_ptr, row, col, char_ptr;
	for(row = 0; row <479; row++)
	{
		for(col = 0; col<639; col++)
		{
			pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
			PHYSMEM_16(pixel_ptr) = 0x0000;	
		}
	}
	
	row = 0;
	col = 0;
	offset = (row << 7) + col;
	for(row = 0; row <59; row++)
	{
		for(col = 0; col<79; col++)
		{
			offset = (row << 7) + col;
			char_ptr =  FPGA_CHAR_BASE + offset;	
			PHYSMEM_32(char_ptr) = *("\0");
			++offset;
		}
	}		
}

/****************************************************************************************
 * Draw a line on the VGA monitor 
****************************************************************************************/
void VGA_line(int x1, int y1, int x2, int y2, short pixel_color, void *virtual_base)
{ 
	unsigned int pixel_ptr, row, col;
	float slope, offset;
	
	if(y2-y1 == 0) // Zero Slope/Horizontal
	{
		if(x2-x1 > 0){
		/* assume that the box coordinates are valid */
			for (col = x1; col <= x2; col++)
			{
				row = y2;
				pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
				PHYSMEM_16(pixel_ptr) = pixel_color;		// set pixel color
			}
		}
		else{
			for (col = x2; col <= x1; col++)
			{
				row = y2;
				pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
				PHYSMEM_16(pixel_ptr) = pixel_color;		// set pixel color
			}
		}
	}
	else if((x2-x1 == 0)) // Undefined Slope/Infinity/Vertical
	{
		if(y2-y1 > 0){
			
		/* assume that the box coordinates are valid */
			for (row = y1; row <= y2; row++)
			{
				col = x2;
				pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
				PHYSMEM_16(pixel_ptr) = pixel_color;		// set pixel color
			}
		}
		else{
			for (row = y2; row <= y1; row++)
			{
				col = x2;
				pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
				PHYSMEM_16(pixel_ptr) = pixel_color;		// set pixel color
			}
		}
	}
	else
	{
		slope = (float)(y2-y1)/(float)(x2-x1);
		printf("Slope = %f\n", slope);
		if(slope > 0 && (x2-x1 > 0) && (y2-y1 > 0)){ // CORRECT
			
		/* assume that the box coordinates are valid */
			for (col = x1; col <= x2; col++) // Regular Linear Line
			{
				offset = (float)(y2 - slope*x2);
				row = (int)(slope*col+offset);
				pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
				PHYSMEM_16(pixel_ptr) = pixel_color;		// set pixel color
			}
		}
		else if((slope > 0) && (x2-x1 < 0) && (y2-y1 < 0)){ // INCORRECT
			for (col = x2; col <= x1; col++) // Regular Linear Line
			{
				offset = (float)(y2 - slope*x2);
				row = (int)(slope*col+offset);
				pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
				PHYSMEM_16(pixel_ptr) = pixel_color;		// set pixel color
			}
		}
		else if((slope < 0) && (x2-x1 > 0) && (y2-y1 < 0)){ // CORRECT
			for (col = x1; col <= x2; col++) // Regular Linear Line
			{
				offset = (float)(y2 - slope*x2);
				row = (int)(slope*col+offset);
				pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
				PHYSMEM_16(pixel_ptr) = pixel_color;		// set pixel color
			}			
		}
		else{ // INCORRECT
			for (col = x2; col <= x1; col++) // Regular Linear Line
			{
				offset = (float)(y2 - slope*x2);
				row = (int)(slope*col+offset);
				pixel_ptr = HW_OCRAM_BASE + (row << 10) + (col << 1);
				PHYSMEM_16(pixel_ptr) = pixel_color;		// set pixel color
			}
		}
	}
}

// Test program for use with the DE1-SoC University Computer
// 

int main(int argc,char ** argv) {
	
    void *virtual_base;
    int fd, file;
	const char *filename = "/dev/i2c-0";
	uint8_t id;
	bool bSuccess;
	const int mg_per_digi = 4;
	uint16_t szXYZ[3];
	
	// Dyanmic Box Variables
	int16_t xg, yg, zg;
	uint16_t x1_new, x1_old, x2_new, x2_old, y1_new, y1_old, y2_new, y2_old;
	uint16_t x_width, y_height, delta_move;
	
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
		printf( "ERROR: could not open \"/dev/mem\"...\n" );
		return( 1 );
	}
    
	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_REGS_BASE );
	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
	
	// Set framebuffer addr to beginning of the SRAM
    PHYSMEM_32(0xff203024) = 0xc8000000;
    PHYSMEM_32(0xff203020) = 0xc8000000;
    
    // Unmap registers region, map onchip ram region
    if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}
    virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, HW_OCRAM_BASE );
	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return( 1 );
	}
	
	// open bus
	if ((file = open(filename, O_RDWR)) < 0) {
  	  /* ERROR HANDLING: you can check errno to see what went wrong */
	    perror("Failed to open the i2c bus of gsensor");
  	  exit(1);
	}

	// init	 
	// gsensor i2c address: 101_0011
	int addr = 0b01010011; 
	if (ioctl(file, I2C_SLAVE, addr) < 0) {
  	  printf("Failed to acquire bus access and/or talk to slave.\n");
	    /* ERROR HANDLING; you can check errno to see what went wrong */
  	  exit(1);
	}

    // configure accelerometer as +-2g and start measure
    bSuccess = ADXL345_Init(file);
    if (bSuccess){
        // dump chip id
        bSuccess = ADXL345_IdRead(file, &id);
        if (bSuccess)
            printf("id=%02Xh\r\n", id);
    } 	
	

    
    // Clear the screen
	VGA_Clear(virtual_base);
	//VGA_box(0, 0, 319,479, virtual_base);
	//char text_top_row[40] = "Altera DE1-SoC\0";
	//char text_bottom_row[40] = "Computer\0";
    //VGA_text (34, 29, text_top_row, virtual_base);
	//VGA_text (34, 30, text_bottom_row, virtual_base);
	
	delta_move = 1;
	
	printf("Do you want to read the GSensor to Move a box or Draw a Line?\n ");
	printf("[1] Move a Box\n ");
	printf("[2] Draw a Line\n ");
	char c;
	c =	getchar();
	
	// Draw Box
	if((c == '1'))
	{
		x_width = 20; // Initial Width
		y_height = 40; // Initial Height
		x1_new = 150;
		x1_old = x1_new;
		x2_new = x1_new + x_width;
		x2_old = x2_new;
		y1_new = 150;
		y1_old = y1_new;
		y2_new = y1_new + y_height;
		y2_old = y2_new;
		
		bool changed = false;
		
		while(bSuccess){
			if (ADXL345_IsDataReady(file)){
				bSuccess = ADXL345_XYZ_Read(file, szXYZ);
				if (bSuccess){
					xg = (int16_t) szXYZ[0]*mg_per_digi;
					yg = (int16_t) szXYZ[1]*mg_per_digi;
					zg = (int16_t) szXYZ[2]*mg_per_digi;
					//printf("X=%d mg, Y=%d mg, Z=%d mg\r\n",(int16_t)szXYZ[0]*mg_per_digi, (int16_t)szXYZ[1]*mg_per_digi, (int16_t)szXYZ[2]*mg_per_digi);
					
					// Movement Detection
					if(xg > 100){
						x2_new = x2_new + delta_move;
						x1_new = x1_new + delta_move;
						//printf("X=%d  %d \n", x1_new, x2_new);
						if(x2_new > 319)
						{
							printf("CHANGES  FAR RIGHT X=%d  %d \n", x1_new, x2_new);
							x2_new = 319;
							x1_new = x2_new - x_width;
						}
						changed = true;
					}
					else if(xg < -100){
						x2_new = x2_new - delta_move;
						x1_new = x1_new - delta_move;
						//printf("X=%d  %d \n", x1_new, x2_new);
						if(x1_new < 0 || x1_new > 319)
						{
							printf("CHANGES  FAR LEFT X=%d  %d \n", x1_new, x2_new);
							x1_new = 0;
							x2_new = x1_new + x_width;
						}
						changed = true;
					}
				
					if(yg > 100){
						y2_new = y2_new - 2*delta_move;
						y1_new = y1_new - 2*delta_move;
						printf("Y=%d  %d \n", y1_new, y2_new);
						if((y1_new > 479 || y1_new < 0) && y2_new > 0)
						{
							printf("CHANGES BOTTOM X=%d  %d \n", y1_new, y2_new);
							y1_new = 0;
							y2_new = y1_new + y_height;
						}
						changed = true;
					}
					else if(yg < -100){
						
						y2_new = y2_new + 2*delta_move;
						y1_new = y1_new + 2*delta_move;
						printf("Y=%d  %d \n", y1_new, y2_new);
						if(y1_new < 479 && y2_new > 479)
						{
							printf("CHANGES TOP X=%d  %d \n", y1_new, y2_new);
							y2_new = 479;
							y1_new = y2_new - y_height;
						}
						changed = true;
					}
					if(changed == true)
					{
						//VGA_Clear(virtual_base);
						VGA_box(x1_old,y1_old,x2_old,y2_old, 0x0000, virtual_base); // Erase the Box
						VGA_box(x1_new,y1_new,x2_new,y2_new, 0x2533, virtual_base); // Draw new Box
						x1_old = x1_new;
						x2_old = x2_new;
						y1_old = y1_new;
						y2_old = y2_new;
						changed = false;
					}
					
					usleep(50);
				}
			}
		}
    
	}
	// Draw Line
	else if((c == '2'))
	{
		x1_old = 150;
		x1_new = x1_old;
		y1_old = 150;
		y1_new = y1_old;
		x2_new = 150;
		x2_old = x2_new;
		y2_new = 150;
		y2_old = y2_new;
		
		bool changed = false;
		
		while(bSuccess){
			if (ADXL345_IsDataReady(file)){
				bSuccess = ADXL345_XYZ_Read(file, szXYZ);
				
				if (bSuccess){
					xg = (int16_t) szXYZ[0]*mg_per_digi;
					yg = (int16_t) szXYZ[1]*mg_per_digi;
					zg = (int16_t) szXYZ[2]*mg_per_digi;
					//printf("X=%d mg, Y=%d mg, Z=%d mg\r\n",(int16_t)szXYZ[0]*mg_per_digi, (int16_t)szXYZ[1]*mg_per_digi, (int16_t)szXYZ[2]*mg_per_digi);
					
					// Movement Detection
					if(xg > 100){
						x2_new = x2_new + delta_move;
						printf("X2 = %d\n", (int)x2_new);
						if(x2_new > 319)
						{
							x2_new = 319;
						}
						changed = true;
					}
					else if(xg < -100){
						x2_new = x2_new - delta_move;
						printf("X2 = %d\n", (int)x2_new);
						if(x2_new < 0 || x2_new > 319)
						{
							x2_new = 0;
						}
						changed = true;
					}
				
					if(yg > 100){
						y2_new = y2_new - 2*delta_move;
						printf("Y2 = %d\n", (int)y2_new);
						if(y2_new < 0 || y2_new > 479)
						{
							y2_new = 0;
						}
						changed = true;
					}
					else if(yg < -100){
						y2_new = y2_new + 2*delta_move;
						printf("Y2 = %d\n", (int)y2_new);
						if(y2_new > 479)
						{
							y2_new = 479;
						}
						changed = true;
					}

					if(changed == true)
					{

						//VGA_Clear(virtual_base);
						VGA_line(x1_new,y1_new,x2_old,y2_old, 0x0000, virtual_base); // Erase the Line
						VGA_line(x1_old,y1_old,x2_new,y2_new, 0xABCD, virtual_base); // Draw new Line
						//VGA_line(x1_new,y1_new,x2_new,y2_new, 0x2533, virtual_base); // Draw new Line
						y2_old = y2_new; 
						x2_old = x2_new; 
						changed = false;
					}
			
					usleep(500);
				}
			}
		}
    
	}
	if (!bSuccess)
	printf("Failed to access accelerometer\r\n");

	if (file)
		close(file);
		
	printf("gsensor, bye!\r\n");			
	
	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );

	return( 0 );

}
