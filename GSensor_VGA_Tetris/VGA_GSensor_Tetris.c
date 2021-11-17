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
 * Clear Score on the VGA monitor 
****************************************************************************************/
void SCORE_Clear(void * virtual_base)
{
	int offset;
	unsigned int row, col, char_ptr;
	row = SCORETOPOFFSET;
	offset = (SCORETOPOFFSET << 7) + SCORELEFTOFFSET;
	for(col = SCORELEFTOFFSET; col < SCORELEFTOFFSET + 7; col++)
	{
		offset = (row << 7) + col;
		char_ptr =  FPGA_CHAR_BASE + offset;		
		PHYSMEM_32(char_ptr) = *("\0");
		++offset;
	}
}

/****************************************************************************************
 * Clear Lines on the VGA monitor 
****************************************************************************************/
void LINE_Clear(void * virtual_base)
{
	int offset;
	unsigned int row, col, char_ptr;
	row = LINETOPOFFSET;
	offset = (LINETOPOFFSET << 7) + LINELEFTOFFSET;
	for(col = LINELEFTOFFSET; col < LINELEFTOFFSET + 7; col++)
	{
		offset = (row << 7) + col;
		char_ptr =  FPGA_CHAR_BASE + offset;		
		PHYSMEM_32(char_ptr) = *("\0");
		++offset;
	}
}

/****************************************************************************************
 * Clear Level on the VGA monitor 
****************************************************************************************/
void LEVEL_Clear(void * virtual_base)
{
	int offset;
	unsigned int row, col, char_ptr;
	row = LEVELTOPOFFSET;
	offset = (LEVELTOPOFFSET << 7) + LEVELLEFTOFFSET;
	for(col = LEVELLEFTOFFSET; col < LEVELLEFTOFFSET + 7; col++)
	{
		offset = (row << 7) + col;
		char_ptr =  FPGA_CHAR_BASE + offset;		
		PHYSMEM_32(char_ptr) = *("\0");
		++offset;
	}
}

/****************************************************************************************
 * Draw the Score based on the number from the Score Counter
****************************************************************************************/
void VGA_Draw_Score(int score, void *virtual_base)
{ 
	char tempArray[MAXSCORELENGTH];
	sprintf(tempArray, "%d", score); 
	SCORE_Clear(virtual_base);
	VGA_text (SCORELEFTOFFSET, SCORETOPOFFSET, tempArray, virtual_base);
}

/****************************************************************************************
 * Draw the Lines based on the number from the Line Counter
****************************************************************************************/
void VGA_Draw_Line(int lines, void *virtual_base)
{ 
	char tempArray[MAXSCORELENGTH];
	sprintf(tempArray, "%d", lines);
	LINE_Clear(virtual_base);
	VGA_text (LINELEFTOFFSET, LINETOPOFFSET, tempArray, virtual_base);
}

/****************************************************************************************
 * Draw the Level based on the number from the Level Counter
****************************************************************************************/
void VGA_Draw_Level(int level, void *virtual_base)
{ 
	char tempArray[MAXSCORELENGTH];
	sprintf(tempArray, "%d", level);
	LEVEL_Clear(virtual_base);
	VGA_text (LEVELLEFTOFFSET, LEVELTOPOFFSET, tempArray, virtual_base);
}

/****************************************************************************************
 * Draw Tetronimo Square with Pixel Translation
****************************************************************************************/
void VGA_Draw_Tetronimo_Square(int xgrid, int ygrid, short color, int choice, void *virtual_base)
// Need GRID ARRAY
{
	int xPixelTopLeftOuter, yPixelTopLeftOuter, xPixelBottomRightOuter, yPixelBottomRightOuter, xPixelTopLeftInner, yPixelTopLeftInner, xPixelBottomRightInner, yPixelBottomRightInner;
	if(xgrid < 10 && ygrid < 15 && choice == 0)
	{
		xPixelTopLeftOuter = PLAYLEFTOFFSET +(xgrid * SQUAREWIDTH);
		yPixelTopLeftOuter = ygrid * SQUAREHEIGHT;
		xPixelTopLeftInner = PLAYLEFTOFFSET +(xgrid * SQUAREWIDTH) + 2;
		yPixelTopLeftInner = (ygrid * SQUAREHEIGHT) + 4;
	}
	if(xgrid < 4 && ygrid < 4 && choice == 1)
	{
		xPixelTopLeftOuter = NEXTPIECEPIXELLEFTOFFSET + (xgrid * SQUAREWIDTH);
		yPixelTopLeftOuter = NEXTPIECEPIXELTOPOFFSET + (ygrid * SQUAREHEIGHT);
		xPixelTopLeftInner = NEXTPIECEPIXELLEFTOFFSET +(xgrid * SQUAREWIDTH) + 2;
		yPixelTopLeftInner = NEXTPIECEPIXELLEFTOFFSET + (ygrid * SQUAREHEIGHT) + 4;
	}
	xPixelBottomRightOuter = xPixelTopLeftOuter + SQUAREWIDTH -1;
	yPixelBottomRightOuter = yPixelTopLeftOuter + SQUAREHEIGHT -1;
	xPixelBottomRightInner = xPixelTopLeftOuter + SQUAREWIDTH -3;
	yPixelBottomRightInner = yPixelTopLeftOuter + SQUAREHEIGHT -5;
	
	VGA_box(xPixelTopLeftOuter, yPixelTopLeftOuter, xPixelBottomRightOuter, yPixelBottomRightOuter, WHITE, virtual_base);
	VGA_box(xPixelTopLeftInner, yPixelTopLeftInner, xPixelBottomRightInner, yPixelBottomRightInner, color, virtual_base);
}

/****************************************************************************************
 * Initial Setup
****************************************************************************************/
void VGA_Tetris_Setup(void *virtual_base)
{ 
	VGA_Clear(virtual_base);	
	
	// Draw Score Area
	char score_text[10] = "Score: \0";
	char lines_text[10] = "Lines:  \0";
	char level_text[10] = "Level: \0";
	char next_text[10] = "Next: \0";
	VGA_text (SCORETEXTOFFSET, SCORETOPOFFSET, score_text, virtual_base);
	VGA_text (SCORETEXTOFFSET, LINETOPOFFSET, lines_text, virtual_base);
	VGA_text (SCORETEXTOFFSET, LEVELTOPOFFSET, level_text, virtual_base);
	VGA_text (SCORETEXTOFFSET, NEXTPIECETOPOFFSET, next_text, virtual_base);
	
	VGA_Draw_Score(0, virtual_base);
	VGA_Draw_Line(0, virtual_base);
	VGA_Draw_Level(0, virtual_base);
	VGA_SquareTetronimoBorderDraw(WHITE, virtual_base);	//draw border
}

/****************************************************************************************
 * Random Number Generator
****************************************************************************************/
int Random_Number()
{ 
	int r = rand() % 7;
	return r;
}

/****************************************************************************************
 * State Machine and Game Logic
****************************************************************************************/
void State_Machine_Check( void *virtual_base)
{

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
    VGA_text (15, SCORETOPOFFSET+1," |__   __|  ____|__   __|  __ \\ |_  _|  / ____|", virtual_base);
    VGA_text (15, SCORETOPOFFSET+2,"    | |  | |__     | |  | |__)|  | |   | (___ ", virtual_base);
    VGA_text (15, SCORETOPOFFSET+3,"    | |  |  __|    | |  |  _  /  | |    \\___ \\ ", virtual_base);
    VGA_text (15, SCORETOPOFFSET+4,"    | |  | |____   | |  | | \\ \\ _| |_  ____) |", virtual_base);
    VGA_text (15, SCORETOPOFFSET+5,"    |_|  |______|  |_|  |_|  \\_\\_____| _____/ ", virtual_base);
	
	VGA_text (18, SCORETOPOFFSET+8,"    PRESS 1 THEN ENTER TO BEGIN PLAYING", virtual_base);
}

/****************************************************************************************
 * Draw Next Tetronimo Tetronimoes
****************************************************************************************/
void VGA_Draw_Next_Tetronimo(int tetronimoChoice, int gridChoice, void *virtual_base)
{ 
	short color = BLACK;
	int i;
	switch(tetronimoChoice)
	{
		// Need to store current tetronemo square locations.
		case 0:
			// Draw I Tetronimo
			color = CYAN;
			if(gridChoice == 0)
			{
				for(i = 0; i < 4; i++)
				{
					VGA_Draw_Tetronimo_Square(i,2, 0xABC0, gridChoice, virtual_base);
					
				}
			}
			else if(gridChoice == 1)
			{

				for(i = 0; i < 4; i++)
				{
					VGA_Draw_Tetronimo_Square(i, 2, color, gridChoice, virtual_base);
				}
			}
	

			break;
		case 1:
			// Draw J Tetronimo
			color = BLUE;
			VGA_Draw_Tetronimo_Square(2, 2, color, gridChoice, virtual_base);
			for(i = 0; i < 3; i++)
			{
				VGA_Draw_Tetronimo_Square(i, 1, color, gridChoice, virtual_base);
			}
			break;
		case 2:
			// Draw L Tetronimo
			color = ORANGE;
			for(i = 0; i < 3; i++)
			{
				VGA_Draw_Tetronimo_Square(i, 1, color, gridChoice, virtual_base);
			}
			VGA_Draw_Tetronimo_Square(0, 2, color, gridChoice, virtual_base);
			break;
		case 3:
			// Draw O Tetronimo
			color = YELLOW;
			for(i = 0; i < 2; i++)
			{
				VGA_Draw_Tetronimo_Square(i, 1, color, gridChoice, virtual_base);
				VGA_Draw_Tetronimo_Square(i, 2, color, gridChoice, virtual_base);
			}
			break;
		case 4:
			// Draw S Tetronimo
			color = GREEN;
			for(i = 1; i < 3; i++)
			{
				VGA_Draw_Tetronimo_Square(i, 1, color, gridChoice, virtual_base);
			}
			for(i = 0; i < 2; i++)
			{
				VGA_Draw_Tetronimo_Square(i, 2, color, gridChoice, virtual_base);
			}
			
			break;
		case 5:
			// Draw T Tetronimo
			color = PURPLE;
			for(i = 0; i < 3; i++)
			{
				VGA_Draw_Tetronimo_Square(i, 2, color, gridChoice, virtual_base);
			}
			VGA_Draw_Tetronimo_Square(2, 1, color, gridChoice, virtual_base);
			break;
		case 6:
			// Draw Z Tetronimo
			color = RED;
			for(i = 0; i < 2; i++)
			{
				VGA_Draw_Tetronimo_Square(i, 1, color, gridChoice, virtual_base);
			}
			for(i = 1; i < 3; i++)
			{
				VGA_Draw_Tetronimo_Square(i, 2, color, gridChoice, virtual_base);
			}
			break;
		default:
			printf("DEFAULT CASE, we should not be here!\n");
			break;
	}
}

/****************************************************************************************
 * Delete Old Tetronimo
****************************************************************************************/
void VGA_Delete_Old(int x1_old, int y1_old, int x2_old, int y2_old, int x3_old, int y3_old, int x4_old, int y4_old, short **gridArray, void *virtual_base)
{ 	
	// Delete Old
	VGA_Draw_Tetronimo_Square(x1_old, y1_old, BLACK, PLAYAREAGRID, virtual_base);
	gridArray[x1_old][y1_old] = BLACK;
	VGA_Draw_Tetronimo_Square(x2_old, y2_old, BLACK, PLAYAREAGRID, virtual_base);
	gridArray[x2_old][y2_old] = BLACK;
	VGA_Draw_Tetronimo_Square(x3_old, y3_old, BLACK, PLAYAREAGRID, virtual_base);
	gridArray[x3_old][y3_old] = BLACK;
	VGA_Draw_Tetronimo_Square(x4_old, y4_old, BLACK, PLAYAREAGRID, virtual_base);
	gridArray[x4_old][y4_old] = BLACK;
}

/****************************************************************************************
 * Draw New Tetronimo
****************************************************************************************/
void VGA_Draw_New(int x1_new, int y1_new, int x2_new, int y2_new, int x3_new, int y3_new, int x4_new, int y4_new, short color, short **gridArray, void *virtual_base)
{ 
	// Draw New
	VGA_Draw_Tetronimo_Square(x1_new, y1_new, color, PLAYAREAGRID, virtual_base);
	gridArray[x1_new][y1_new] = color;
	VGA_Draw_Tetronimo_Square(x2_new, y2_new, color, PLAYAREAGRID, virtual_base);
	gridArray[x2_new][y2_new] = color;
	VGA_Draw_Tetronimo_Square(x3_new, y3_new, color, PLAYAREAGRID, virtual_base);
	gridArray[x3_new][y3_new] = color;
	VGA_Draw_Tetronimo_Square(x4_new, y4_new, color, PLAYAREAGRID, virtual_base);
	gridArray[x4_new][y4_new] = color;
}

/****************************************************************************************
 * Check Each Row and Shift
****************************************************************************************/
void Row_Checker(short ** gridArray, int level, int *score, void *virtual_base)
{ 
	int rows[ROWS];
	int currentRow, highestRow, shiftRow, i, j,  rowCount = 0;
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
					VGA_Draw_Tetronimo_Square(j, currentRow, BLACK, PLAYAREAGRID, virtual_base);	
					VGA_Draw_Tetronimo_Square(j, i, gridArray[i][j], PLAYAREAGRID, virtual_base);	
				}		
			}
		}
		for(shiftRow = i; shiftRow <= highestRow; shiftRow++)
		{
			rows[shiftRow] = rows[shiftRow + 1];
		}
		highestRow--;
	}
	if(rowCount > 0)
	{
		//addRowScore(rowCount, level, &score, &virtual_base);
	}
}

/****************************************************************************************
 * Shift Tetronimo
****************************************************************************************/
void Tetronimo_Shift(short **gridArray, int *xTetronimo, int *yTetronimo, int shiftType, void *virtual_base)
{ 
	int old_x1, old_x2, old_x3, old_x4, old_y1, old_y2, old_y3, old_y4;
	int new_x1, new_x2, new_x3, new_x4, new_y1, new_y2, new_y3, new_y4;
	short color;
	switch(shiftType)
	{
		old_x1 = xTetronimo[0];
		old_x2 = xTetronimo[1];
		old_x3 = xTetronimo[2];
		old_x4 = xTetronimo[3];
		old_y1 = yTetronimo[0];
		old_y2 = yTetronimo[1];
		old_y3 = yTetronimo[2];
		old_y4 = yTetronimo[3];
		
		case 1:
			// Shift Left
				new_x1 = xTetronimo[0] - 1;
				new_y1 = yTetronimo[0];
				new_x2 = xTetronimo[1] - 1;
				new_y2 = yTetronimo[1];
				new_x3 = xTetronimo[2] - 1;
				new_y3 = yTetronimo[2];
				new_x4 = xTetronimo[3] - 1;
				new_y4 = yTetronimo[3];	
			break;
		case 2:
			// Shift Right
				new_x1 = xTetronimo[0] + 1;
				new_y1 = yTetronimo[0];
				new_x2 = xTetronimo[1] + 1;
				new_y2 = yTetronimo[1];
				new_x3 = xTetronimo[2] + 1;
				new_y3 = yTetronimo[2];
				new_x4 = xTetronimo[3] + 1;
				new_y4 = yTetronimo[3];	
			break;
		case 3:
			// Shift Down
				new_x1 = xTetronimo[0];
				new_y1 = yTetronimo[0] + 1;
				new_x2 = xTetronimo[1];
				new_y2 = yTetronimo[1] + 1;
				new_x3 = xTetronimo[2];
				new_y3 = yTetronimo[2] + 1;
				new_x4 = xTetronimo[3];
				new_y4 = yTetronimo[3] + 1;	
			break;
		default:
			break;
	}
	if((new_x1 && new_y1 && new_x2 && new_y2 && new_x3 && new_y3 && new_x4 && new_y4) >= 0 && (new_y1 && new_y2 && new_y3 && new_y4 ) < ROWS && (new_x1 && new_x2 && new_x3 && new_x4 ) < COLUMNS)
	{
		if(gridArray[new_y1][new_x1] == BLACK && gridArray[new_y2][new_x2] == BLACK && gridArray[new_y3][new_x3] == BLACK && gridArray[new_y3][new_x3] == BLACK && gridArray[new_y4][new_x4] == BLACK)
		{
			xTetronimo[0] = new_x1;
			yTetronimo[0] = new_y1;
			xTetronimo[1] = new_x2;
			yTetronimo[1] = new_y2;
			xTetronimo[2] = new_x3;
			yTetronimo[2] = new_y3;
			xTetronimo[3] = new_x4;
			yTetronimo[3] = new_y4;
			color = gridArray[old_y1][old_x1];
			VGA_Delete_Old(old_x1, old_y1, old_x2, old_y2, old_x3, old_y3, old_x4, old_y4, gridArray, virtual_base)
			VGA_Draw_New(new_x1, new_y1, new_x2, new_y2, new_x3, new_y3, new_x4, new_y4, color, gridArray, virtual_base);
		}
	}
}

/****************************************************************************************
 * Add Score to Total Points
****************************************************************************************/
void addRowScore(int rowCount, int level, int *score, void *virtual_base)
{
	int temp = *score;
	switch(rowCount)
	{
		case 1:
			*score = temp + (40 * (level + 1));
			break;
		case 2:
			*score = temp + (100 * (level + 1));
			break;
		case 3:
			*score = temp + (300 * (level + 1));
			break;
		case 4:
			*score = temp + (1200 * (level + 1));
			break;
		default:
			break;	
	}
}

	

/****************************************************************************************
 * Rotate Tetronimoes CW
****************************************************************************************/
void VGA_Rotate_Tetronimo(int choice, int *currentRotation, short **gridArray, int *xTetronimo, int *yTetronimo, void *virtual_base)
{ 

}

/****************************************************************************************
 * Initialize Multiple Arrays
****************************************************************************************/
void initArrays(short **dataArray, int *xArray, int *yArray)
{ 
	free(dataArray);
	free(xArray);
	free(yArray);
	
	dataArray = malloc(sizeof(int *) * ROWS);
	xArray = malloc(sizeof(int *) * 4);
	yArray = malloc(sizeof(int *) * 4);

	for (i = 0; i < ROWS; i++) 
	{
		// malloc space for row i's M column elements
		dataArray[i] = malloc(sizeof(int) * COLUMNS);
	}
	
	for (i = 0; i < ROWS; i++) 
	{
		for (j = 0; j < COLUMNS; j++) 
		{
			DataArray[i][j] = 0x0000;
		}
    }
	for(i = 0; i < 4; i++)
	{
		xArray[i] = 0;
		yArray[i] = 0;
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
		short **gridData;
		int *xTetrominoData, *yTetrominoData, *currentRotation;		
		int lineCount = 0, level = 0, score = 0, randTetronimoChoice = 0;
		
		srand(time(0));
		VGA_Tetris_Setup(virtual_base);
		randTetronimoChoice = Random_Number();
		VGA_Draw_Next_Tetronimo(randTetronimoChoice, NEXTPIECEGRID, virtual_base);
		initArrays(gridData, xTetrominoData, yTetrominoData);
		
		usleep(3000);
		
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
		free(dataArray);
		free(xArray);
		free(yArray);
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

	free(dataArray);
	free(xArray);
	free(yArray);
	close( fd );

	return( 0 );

}