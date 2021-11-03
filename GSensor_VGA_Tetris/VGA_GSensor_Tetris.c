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
#include "VGA_GSensor_Tetris.h"
#include "hwlib.h"
#include <time.h>

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
 * Draw the Score based on the number from the Score Counter
****************************************************************************************/
void VGA_Draw_Score(int score, void *virtual_base)
{ 
	char tempArray[MAXSCORELENGTH];
	sprintf(tempArray, "%d", score); 
	VGA_text (SCORELEFTOFFSET, SCORETOPOFFSET, tempArray, virtual_base);
}

/****************************************************************************************
 * Draw the Lines based on the number from the Line Counter
****************************************************************************************/
void VGA_Draw_Line(int lines, void *virtual_base)
{ 
	char tempArray[MAXSCORELENGTH];
	sprintf(tempArray, "%d", lines);
	VGA_text (LINELEFTOFFSET, LINETOPOFFSET, tempArray, virtual_base);
}

/****************************************************************************************
 * Draw the Level based on the number from the Level Counter
****************************************************************************************/
void VGA_Draw_Level(int level, void *virtual_base)
{ 
	char tempArray[MAXSCORELENGTH];
	sprintf(tempArray, "%d", level);
	VGA_text (LEVELLEFTOFFSET, LEVELTOPOFFSET, tempArray, virtual_base);
}

/****************************************************************************************
 * Draw Tetronimo Square
****************************************************************************************/
void VGA_Draw_Tetronimo_Square(int xgrid, int ygrid, short color, void *virtual_base)
{
	VGA_box(xgrid, ygrid, xgrid + SQUAREWIDTH -1, ygrid + SQUAREHEIGHT -1, WHITE, virtual_base);
	VGA_box(xgrid + 2, ygrid + 2, xgrid + SQUAREWIDTH - 3, ygrid + SQUAREHEIGHT -5, color, virtual_base);


}

/****************************************************************************************
 * Draw Default Tetronimoes
****************************************************************************************/
void VGA_Draw_Tetronimo(int choice, void *virtual_base)
{ 	
	switch(choice)
	{
		case 1:
			// Draw I Tetronimo
			
			break;
		case 2:
			// Draw J Tetronimo
			break;
		case 3:
			// Draw L Tetronimo
			break;
		case 4:
			// Draw O Tetronimo
			break;
		case 5:
			// Draw S Tetronimo
			break;
		case 6:
			// Draw T Tetronimo
			break;
		case 7:
			// Draw Z Tetronimo
			break;
		default:
			break;
	}
}

/****************************************************************************************
 * Initial Setup
****************************************************************************************/
void VGA_Tetris_Setup(void *virtual_base)
{ 
	VGA_Clear(virtual_base);	

	// Draw Play Area Border
	//VGA_SquareTetronimoBorderDraw(virtual_base); 
	
	// Draw Score Area
	char score_text[10] = "Score: \0";
	char lines_text[10] = "Lines:  \0";
	char level_text[10] = "Level: \0";
	char next_text[10] = "Next: \0";
	VGA_text (SCORETEXTOFFSET, SCORETOPOFFSET, score_text, virtual_base);
	VGA_text (SCORETEXTOFFSET, LINETOPOFFSET, lines_text, virtual_base);
	VGA_text (SCORETEXTOFFSET, LEVELTOPOFFSET, level_text, virtual_base);
	VGA_text (SCORETEXTOFFSET, NEXTPIECETOPOFFSET, next_text, virtual_base);
}

/****************************************************************************************
 * Initial Array
****************************************************************************************/
/*void Init_Array(short ** gridArray, void *virtual_base)
{ 

	int i ,j;
	gridArray = malloc(sizeof(int *) * ROWS);
	
	for (i = 0; i < ROWS; i++) 
	{
		// malloc space for row i's M column elements
		gridArray[i] = malloc(sizeof(int) * COLUMNS);
	}
	
	for (i = 0; i < ROWS; i++) {
		for (j = 0; j < COLUMNS; j++) 
		{
			gridArray[i][j] = 0x0000;
		}
    }
}
*/

/****************************************************************************************
 * Random Number Generator
****************************************************************************************/
int Random_Number()
{ 
	int r = rand() % 7;
	return r;
}

/****************************************************************************************
 * Translation Table
****************************************************************************************/
void VGA_Pixel_Translation(int x, int y, int * xPixelOut, int * yPixelOut, int choice, void *virtual_base)
{ 
	if(x < 10 && y < 15 && choice == 0)
	{
		*xPixelOut = PLAYLEFTOFFSET +(x * SQUAREWIDTH);
		*yPixelOut = (y * SQUAREHEIGHT);
	}
	if(x < 4 && y < 4 && choice == 1)
	{
		*xPixelOut = NEXTPIECEPIXELLEFTOFFSET + (x * SQUAREWIDTH);
		*yPixelOut = NEXTPIECEPIXELTOPOFFSET + (y * SQUAREHEIGHT);
	}
}


/****************************************************************************************
 * Draw a Grey Colored Tetromino Square w/ a Black Border around the Grid
****************************************************************************************/
void VGA_SquareTetronimoBorderDraw(short color, void *virtual_base)
{ 
	int row, col;
	for(row = 0; row < ROWS+1; row++)
		for(col = 0; col < COLUMNS+2; col++)
		{
			
			if(col == 0 || col == COLUMNS+1 || row == ROWS)
			{
				//Outer Box
				VGA_box(SQUAREWIDTH*col,
				SQUAREHEIGHT*row,
				SQUAREWIDTH*(col+1),
				SQUAREHEIGHT*(row+1),
				0,
				virtual_base);
					
				//Inner Box
				VGA_box(SQUAREWIDTH*col + 1,
				SQUAREHEIGHT*row + 1,
				SQUAREWIDTH*(col+1) - 1,
				SQUAREHEIGHT*(row+1) - 1,
				color,
				virtual_base);
			}
		}
}

/****************************************************************************************
 * Draw start screen
****************************************************************************************/
void VGA_DrawStartScreen(void *virtual_base)
{ 
	VGA_text (15, SCORETOPOFFSET,"  _______ ______ _______ _____  _____    _____", virtual_base);
    VGA_text (15, SCORETOPOFFSET+1," |__   __|  ____|__   __|  __ \\ |_  _| / ____|", virtual_base);
    VGA_text (15, SCORETOPOFFSET+2,"    | |  | |__     | |  | |__) | | |   | (___ ", virtual_base);
    VGA_text (15, SCORETOPOFFSET+3,"    | |  |  __|    | |  |  _  /  | |    \\___ \\ ", virtual_base);
    VGA_text (15, SCORETOPOFFSET+4,"    | |  | |____   | |  | | \\ \\ _| |_  ____) |", virtual_base);
    VGA_text (15, SCORETOPOFFSET+5,"    |_|  |______|  |_|  |_|  \\_\\_____| _____/ ", virtual_base);
}

/****************************************************************************************
 * Draw Next Tetronimo Tetronimoes
****************************************************************************************/
void VGA_Draw_Next_Tetronimo(int choice, void *virtual_base)
{ 
	int xPixel, yPixel;
	short color = BLACK;
	int i;
	printf("Choice: %d", choice);
	switch(choice)
	{
		case 0:
			// Draw I Tetronimo
			color = CYAN;
			VGA_Pixel_Translation(0, 1, &xPixel, &yPixel, 1, virtual_base);
			for(i = 0; i < 4; i++)
			{
				VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel, color, virtual_base);
			}		
			break;
		case 1:
			// Draw J Tetronimo
			color = BLUE;
			VGA_Pixel_Translation(1, 1, &xPixel, &yPixel, 1, virtual_base);
			VGA_Draw_Tetronimo_Square(xPixel, yPixel, color, virtual_base);
			for(i = 0; i < 3; i++)
			{
				VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel + SQUAREHEIGHT, color, virtual_base);
			}
			break;
		case 2:
			// Draw L Tetronimo
			color = ORANGE;
			VGA_Pixel_Translation(0, 2, &xPixel, &yPixel, 1, virtual_base);
			for(i = 0; i < 3; i++)
			{
				VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel, color, virtual_base);
			}
			VGA_Draw_Tetronimo_Square(xPixel + (2 * SQUAREWIDTH), yPixel - SQUAREHEIGHT, color, virtual_base);
			break;
		case 3:
			// Draw O Tetronimo
			color = YELLOW;
			VGA_Pixel_Translation(1, 1, &xPixel, &yPixel, 1, virtual_base);
			for(i = 0; i < 2; i++)
			{
				VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel, color, virtual_base);
				VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel + SQUAREHEIGHT, color, virtual_base);
			}
			break;
		case 4:
			// Draw S Tetronimo
			color = GREEN;
			VGA_Pixel_Translation(2, 1, &xPixel, &yPixel, 1, virtual_base);
			for(i = 0; i < 2; i++)
			{
				VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel, color, virtual_base);
			}
			VGA_Pixel_Translation(1, 2, &xPixel, &yPixel, 1, virtual_base);
			for(i = 0; i < 2; i++)
			{
				VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel, color, virtual_base);
			}
			
			break;
		case 5:
			// Draw T Tetronimo
			color = PURPLE;
			VGA_Pixel_Translation(1, 2, &xPixel, &yPixel, 1, virtual_base);
			for(i = 0; i < 3; i++)
			{
				VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel, color, virtual_base);
			}
			VGA_Draw_Tetronimo_Square(xPixel + SQUAREWIDTH, yPixel - SQUAREHEIGHT, color, virtual_base);
			break;
		case 6:
			// Draw Z Tetronimo
			color = RED;
			VGA_Pixel_Translation(1, 1, &xPixel, &yPixel, 1, virtual_base);
			for(i = 0; i < 2; i++)
			{
			VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel, color, virtual_base);
			}
			VGA_Pixel_Translation(2, 2, &xPixel, &yPixel, 1, virtual_base);
			for(i = 0; i < 2; i++)
			{
				VGA_Draw_Tetronimo_Square(xPixel + (i * SQUAREWIDTH), yPixel, color, virtual_base);
			}
			break;
		default:
			printf("DEFAULT CASE, we should not be here!\n");
			break;
	}
}

/****************************************************************************************
 * Delete Old Tetronimo and Draw New
****************************************************************************************/
void VGA_Draw_New(int x1_old, int y1_old, int x2_old, int y2_old, int x3_old, int y3_old, int x4_old, int y4_old, int x1_new, int y1_new, int x2_new, int y2_new, int x3_new, int y3_new, int x4_new, int y4_new, short color, short ** gridArray, void *virtual_base)
{ 
	int xPixel, yPixel;
	
	// Delete Old
	VGA_Pixel_Translation(x1_old, y1_old, &xPixel, &yPixel, 0, virtual_base);
	VGA_Draw_Tetronimo_Square(xPixel, yPixel, BLACK, virtual_base);
	gridArray[x1_old][y1_old] = BLACK;
	VGA_Pixel_Translation(x2_old, y2_old, &xPixel, &yPixel, 0, virtual_base);
	VGA_Draw_Tetronimo_Square(xPixel, yPixel, BLACK, virtual_base);
	gridArray[x2_old][y2_old] = BLACK;
	VGA_Pixel_Translation(x3_old, y3_old, &xPixel, &yPixel, 0, virtual_base);
	VGA_Draw_Tetronimo_Square(xPixel, yPixel, BLACK, virtual_base);
	gridArray[x3_old][y3_old] = BLACK;
	VGA_Pixel_Translation(x4_old, y4_old, &xPixel, &yPixel, 0, virtual_base);
	VGA_Draw_Tetronimo_Square(xPixel, yPixel, BLACK, virtual_base);
	gridArray[x4_old][y4_old] = BLACK;
	
	// Draw New
	VGA_Pixel_Translation(x1_new, y1_new, &xPixel, &yPixel, 0, virtual_base);
	VGA_Draw_Tetronimo_Square(xPixel, yPixel, color, virtual_base);
	gridArray[x1_new][y1_new] = color;
	VGA_Pixel_Translation(x2_new, y2_new, &xPixel, &yPixel, 0, virtual_base);
	VGA_Draw_Tetronimo_Square(xPixel, yPixel, color, virtual_base);
	gridArray[x2_new][y2_new] = color;
	VGA_Pixel_Translation(x3_new, y3_new, &xPixel, &yPixel, 0, virtual_base);
	VGA_Draw_Tetronimo_Square(xPixel, yPixel, color, virtual_base);
	gridArray[x3_new][y3_new] = color;
	VGA_Pixel_Translation(x4_new, y4_new, &xPixel, &yPixel, 0, virtual_base);
	VGA_Draw_Tetronimo_Square(xPixel, yPixel, color, virtual_base);
	gridArray[x4_new][y4_new] = color;
}

/****************************************************************************************
 * Check Each Row and Shift
****************************************************************************************/
int Row_Checker(short ** gridArray, void *virtual_base)
{ 
	int rows[ROWS];
	int currentRow, highestRow, shiftRow, i, j, xPixel, yPixel, rowCount = 0;;
	bool rowBlack = true, rowFilled = true;

	
	for(i = 0; i < ROWS; i++)
	{
		rows[i] = 0;
	}
	for (i = 0; i < ROWS; i++) 
	{
		for (j = 0; j < COLUMNS; j++) 
		{
			if(gridArray[i][j] == BLACK)
			{
				rowFilled = false;
			}
			else
			{
				rowBlack = false;
			}
		}
		if(rowFilled == true)
		{
			rows[i] = 1;
			rowCount++;
		}
		rowFilled = true;
		
		if(rowBlack == false)
		{
			highestRow = i;
			rowBlack = true;
			
		}
    }
	
	for(i = 0; i < highestRow; i++)
	{
		if(rows[i] == 1)
		{
			for(currentRow = i+1; i <= highestRow; currentRow++)
			{
				for(j = 0; j < COLUMNS; j++)
				{
					gridArray[i][j] = gridArray[currentRow][j];
					gridArray[currentRow][j] = BLACK;
					VGA_Pixel_Translation(j, currentRow, &xPixel, &yPixel, 0, virtual_base);
					VGA_Draw_Tetronimo_Square(xPixel, yPixel, BLACK, virtual_base);	
					VGA_Pixel_Translation(j, i, &xPixel, &yPixel, 0, virtual_base);
					VGA_Draw_Tetronimo_Square(xPixel, yPixel, gridArray[i][j], virtual_base);	
				}		
			}
		}
		for(shiftRow = i; shiftRow <= highestRow; shiftRow++)
		{
			rows[shiftRow] = rows[shiftRow + 1];
		}
		highestRow--;
	}
	return rowCount;
}

/****************************************************************************************
 * Rotate Tetronimoes CW
****************************************************************************************/
int VGA_Rotate_Tetronimo(int choice, int currentRotation, void *virtual_base)
{ 
	switch(choice)
	{
		case 1:
			// Rotate I Tetronimo
			switch(currentRotation)
			{
				case 1:
					// Rotate from 1 to 2
					break;
				case 2:
					// Rotate from 2 to 1
					break;
				default:
					break;
			}
			break;
		case 2:
			// Rotate J Tetronimo
			switch(currentRotation)
			{
				case 1:
					// Rotate from 1 to 2
					break;
				case 2:
					// Rotate from 2 to 3
					break;
				case 3:
					// Rotate from 3 to 4
					break;
				case 4:
					// Rotate from 4 to 1
					break;
				default:
					break;
			}
			break;
		case 3:
			// Rotate L Tetronimo
			switch(currentRotation)
			{
				case 1:
					// Rotate from 1 to 2
					break;
				case 2:
					// Rotate from 2 to 3
					break;
				case 3:
					// Rotate from 3 to 4
					break;
				case 4:
					// Rotate from 4 to 1
					break;
				default:
					break;
			}
			break;
		case 4:
			// Rotate O Tetronimo
			switch(currentRotation)
			{
				case 1:
					break;
			}
			break;
		case 5:
			// Rotate S Tetronimo
			switch(currentRotation)
			{
				case 1:
					// Rotate from 1 to 2
					break;
				case 2:
					// Rotate from 2 to 3
					break;
				case 3:
					// Rotate from 3 to 4
					break;
				case 4:
					// Rotate from 4 to 1
					break;
				default:
					break;
			}
			break;
		case 6:
			// Rotate T Tetronimo
			switch(currentRotation)
			{
				case 1:
					// Rotate from 1 to 2
					break;
				case 2:
					// Rotate from 2 to 3
					break;
				case 3:
					// Rotate from 3 to 4
					break;
				case 4:
					// Rotate from 4 to 1
					break;
				default:
					break;
			}
			break;
		case 7:
			// Rotate Z Tetronimo
			switch(currentRotation)
			{
				case 1:
					// Rotate from 1 to 2
					break;
				case 2:
					// Rotate from 2 to 3
					break;
				case 3:
					// Rotate from 3 to 4
					break;
				case 4:
					// Rotate from 4 to 1
					break;
				default:
					break;
			}
			break;
		default:
			break;
	}
	return currentRotation;
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
	int16_t xg, yg;
	
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
	
	VGA_Clear(virtual_base);
	VGA_DrawStartScreen(virtual_base);
	printf("Do you want to start playing Tetris?\n ");
	printf("[1] Yes\n ");
	printf("Choice: ");
	char c;
	c =	getchar();
	
	// Start Tetris
	if((c == '1'))
	{	
		bool changed = false;
		//bool beginPhase = true;
		int randTetronimoChoice = 0;
		short **gridData;			
		int i, j;
		//int xPixel, yPixel;

		int lineCount = 0;
		int level = 0;
		int score = 0;
		
		//int old_x1, old_x2, old_x3, old_x4, new_x1, new_x2, new_x3, new_x4;
		//int old_y1, old_y2, old_y3, old_y4, new_y1, new_y2, new_y3, new_y4;
		
		srand(time(0));
		VGA_Tetris_Setup(virtual_base);
		VGA_Draw_Score(score, virtual_base);
		VGA_Draw_Line(lineCount, virtual_base);
		VGA_Draw_Level(level, virtual_base);
		randTetronimoChoice = Random_Number();
		printf("Tetronimo Num: %d", randTetronimoChoice);
		VGA_Draw_Next_Tetronimo(randTetronimoChoice, virtual_base);
		printf("Tetronimo Num: %d", randTetronimoChoice);
		VGA_SquareTetronimoBorderDraw(WHITE, virtual_base);	//draw border
		
		gridData = malloc(sizeof(int *) * ROWS);
	
		for (i = 0; i < ROWS; i++) 
		{
			// malloc space for row i's M column elements
			gridData[i] = malloc(sizeof(int) * COLUMNS);
		}
	
		for (i = 0; i < ROWS; i++) {
			for (j = 0; j < COLUMNS; j++) 
			{
				gridData[i][j] = 0x0000;
			}
    	}
		//Init_Array(gridData, virtual_base);
		
		usleep(3000);
		//VGA_Pixel_Translation(x, y, &xPixel, &yPixel, 0);
		
		while(bSuccess){

			if (ADXL345_IsDataReady(file)){
				bSuccess = ADXL345_XYZ_Read(file, szXYZ);
				
				if (bSuccess){
					xg = (int16_t) szXYZ[0]*mg_per_digi;
					yg = (int16_t) szXYZ[1]*mg_per_digi;					
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
	else if((c != '1'))
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
