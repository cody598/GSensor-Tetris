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
#include <math.h>
#include <linux/i2c-dev.h>
#include "ADXL345.h"
#include "hwlib.h"

#define HW_REGS_BASE ( 0xFC000000 )
#define HW_OCRAM_BASE ( 0xC8000000 )
#define HW_REGS_SPAN ( 0x04000000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )
#define FPGA_CHAR_BASE      0xC9000000

// Define Grid
#define ROWS 15
#define COLUMNS 10
#define SQUAREHEIGHT 30
#define SQUAREWIDTH 15
// Play Area + Border (Values Are In Regards To Pixels)
#define TOPOFFSET 0
#define BOTTOMOFFSET 480
#define LEFTOFFSET 0 
#define RIGHTOFFSET 180
// Play Area (Values Are In Regards To Pixels)
#define PLAYTOPOFFSET 0
#define PLAYBOTTOMOFFSET 450
#define PLAYLEFTOFFSET 15 
#define PLAYRIGHTOFFSET 165

// Define Scoring 
#define BASESINGLELINEVALUE 40
#define BASEDOUBLELINEVALUE 100
#define BASETRIPLELINEVALUE 300
#define BASEQUADRUPLELINEVALUE 1200
#define MAXSCORELENGTH 6
#define SCORERIGHTOFFSET RIGHTOFFSET + 15
#define SCORETOPOFFSET 60

// Define Colors
#define CYAN 0x07FF // I
#define YELLOW 0xFF40 // O
#define PURPLE 0x8813 // T
#define GREEN 0x07E3 // S
#define RED 0xF800 // Z
#define BLUE 0x02B5 // J
#define ORANGE 0xFC80 // L
#define BLACK 0x0000 
#define GREY 0x9D13

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
 * Draw a Colored Tetromino Square w/ a Black Border
****************************************************************************************/
void Tetromino_Square(int xgrid, int ygrid, short pixel_color, void *virtual_base)
{ 
	unsigned int pixel_ptr, row, col;
	

	VGA_box()
}

/****************************************************************************************
 * Draw a Grey Colored Tetromino Square w/ a Black Border around the Grid
****************************************************************************************/
void VGA_SquareTetronimoBorderDraw(int x1, int y1, int x2, int y2, short pixel_color, void *virtual_base)
{ 
	unsigned int pixel_ptr, row, col;
	

	VGA_box()
}

/****************************************************************************************
 * Draw the Score based on the number from the Score Counter
****************************************************************************************/
void VGA_Draw_Score(int score, void *virtual_base)
{ 
	char tempArray[MAXSCORELENGTH];
	sprintf(tempArray, "%d", score)
	VGA_text (SCORERIGHTOFFSET + 64, SCORETOPOFFSET, tempArray, virtual_base);
}

/****************************************************************************************
 * Draw the Lines based on the number from the Line Counter
****************************************************************************************/
void VGA_Draw_Line(int lines, void *virtual_base)
{ 
	char tempArray[MAXSCORELENGTH];
	sprintf(tempArray, "%d", lines)
	VGA_text (SCORERIGHTOFFSET + 64, SCORETOPOFFSET + 2, tempArray, virtual_base);
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
 * Initial Setup
****************************************************************************************/
void VGA_Tetris_Setup(void *virtual_base)
{ 
	VGA_Clear(virtual_base);	

	// Draw Play Area Border
	VGA_SquareTetronimoBorderDraw(virtual_base); 
	
	// Draw Score Area
	char score_text[10] = "Score: \0";
	char lines_text[10] = "Lines: \0";
	char next_text[10] = "Next: \0";
	VGA_text (SCORERIGHTOFFSET, SCORETOPOFFSET, score_text, virtual_base);
	VGA_text (SCORERIGHTOFFSET, SCORETOPOFFSET + 2, lines_text, virtual_base);
	VGA_text (SCORERIGHTOFFSET, SCORETOPOFFSET + 5, next_text, virtual_base);

}

/****************************************************************************************
 * Draw |____ Tetromino (J)
****************************************************************************************/


/****************************************************************************************
 * Draw ____| Tetromino (L)
****************************************************************************************/

/****************************************************************************************
 * Draw Tetromino (T)
****************************************************************************************/

/****************************************************************************************
 * Draw Tetromino (I)
****************************************************************************************/

/****************************************************************************************
 * Draw Tetromino (Z)
****************************************************************************************/

/****************************************************************************************
 * Draw Tetromino (O)
****************************************************************************************/

/****************************************************************************************
 * Draw Tetromino (S)
****************************************************************************************/


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

int main(int argc,char ** argv) {
	
    void *virtual_base;
    int fd, file;
	const char *filename = "/dev/i2c-0";
	uint8_t id;
	bool bSuccess;
	const int mg_per_digi = 4;
	uint16_t szXYZ[3];
	int delay = 1000000; // 1 second
	
	// Dyanmic Box Variables
	int16_t xg, yg, zg;
	
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
	
	
	printf("Do you want to start playing Tetris?\n ");
	printf("[1] Yes\n ");
	printf("[2] No\n ");
	char c;
	c =	getchar();
	
	// Start Tetris
	if((c == 'Yes'))
	{	
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
						changed = true;
					}
					else if(xg < -100){

						changed = true;
					}
				
					if(yg > 100){
						
						changed = true;
					}
					else if(yg < -100){
						
						changed = true;
					}
					if(changed == true)
					{
						changed = false;
					}
					
					usleep(delay);
				}
			}
		}
    
	}
	// Exit
	else if((c == '2'))
	{
		exit(0);
    
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
