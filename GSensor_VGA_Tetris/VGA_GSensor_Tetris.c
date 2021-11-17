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
 * Tetromino Struct
****************************************************************************************/
struct Tetromino 
{
	short color;
	int x[4];
	int y[4];
	int rotation;
};


/****************************************************************************************
 * GSensor Functions ADXL345
****************************************************************************************/
bool ADXL345_REG_WRITE(int file, uint8_t address, uint8_t value)
{
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
 * Draw Tetromino Square with Pixel Translation
****************************************************************************************/
void VGA_Draw_Tetromino_Square(int xgrid, int ygrid, short color, int choice, void *virtual_base)
// Need GRID ARRAY
{
	int xPixelTopLeftOuter, yPixelTopLeftOuter, xPixelBottomRightOuter, yPixelBottomRightOuter, xPixelTopLeftInner, yPixelTopLeftInner, xPixelBottomRightInner, yPixelBottomRightInner;
	if(xgrid < 10 && ygrid < 15 && choice == 0)
	{
		xPixelTopLeftOuter = PLAYLEFTOFFSET +(xgrid * SQUAREWIDTH);
		yPixelTopLeftOuter = ygrid * SQUAREHEIGHT;
		xPixelTopLeftInner = xPixelTopLeftOuter + 2;
		yPixelTopLeftInner = yPixelTopLeftOuter + 4;
	}
	if(xgrid < 4 && ygrid < 4 && choice == 1)
	{
		xPixelTopLeftOuter = NEXTPIECEPIXELLEFTOFFSET + (xgrid * SQUAREWIDTH);
		yPixelTopLeftOuter = NEXTPIECEPIXELTOPOFFSET + (ygrid * SQUAREHEIGHT);
		xPixelTopLeftInner = xPixelTopLeftOuter + 2;
		yPixelTopLeftInner = yPixelTopLeftOuter + 4;
	}

	
	VGA_box(xPixelTopLeftOuter, yPixelTopLeftOuter, xPixelTopLeftOuter + XBOTTOMRIGHTOUTEROFFSET, yPixelTopLeftOuter + YBOTTOMRIGHTOUTEROFFSET, WHITE, virtual_base); 
	VGA_box(xPixelTopLeftInner, yPixelTopLeftInner, xPixelTopLeftInner + XBOTTOMRIGHTINNEROFFSET, yPixelTopLeftInner + YBOTTOMRIGHTINNEROFFSET, CYAN, virtual_base); // Error With Color	
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
void VGA_SquareTetrominoBorderDraw(short color, void *virtual_base)
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
	VGA_SquareTetrominoBorderDraw(WHITE, virtual_base);	//draw border
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
 * Draw Next Tetromino Tetrominoes
****************************************************************************************/
void VGA_Draw_Next_Tetromino(int tetrominoChoice, int gridChoice, struct Tetromino * tetr, void *virtual_base)
{ 
	short color = BLACK;
	int i;
	for(i = 0; i < 4; i++)
	{
		tetr->x[i] = 0;
		tetr->y[i] = 0;
	}
	tetr->rotation = 0;

	//add random color selection!
	tetr->color = 0x249B;


	switch(tetrominoChoice)
	{
		// Need to store current tetromino square locations.
		case 0:
			// Draw I Tetromino
			color = CYAN;
			if(gridChoice == 0)
			{
				for(i = 3; i < 7; i++)
				{
					tetr->x[i-3] = i;
					tetr->y[i-3] = 0;
					tetr->color = color;
					VGA_Draw_Tetromino_Square(i,0, color, gridChoice, virtual_base);
					
				}
			}
			else if(gridChoice == 1)
			{

				for(i = 0; i < 4; i++)
				{
					//tetr->x[i];
					//->y[2];
					VGA_Draw_Tetromino_Square(i, 2, color, gridChoice, virtual_base);
				}
			}
			break;
		case 1:
			// Draw J Tetromino
			color = BLUE;
			if(gridChoice == 0)
			{
				VGA_Draw_Tetromino_Square(5, 1, color, gridChoice, virtual_base);
				for(i = 3; i < 6; i++)
				{
					VGA_Draw_Tetromino_Square(i, 0, color, gridChoice, virtual_base);
				}
			}
			else if(gridChoice == 1)
			{
				VGA_Draw_Tetromino_Square(2, 2, color, gridChoice, virtual_base);
				for(i = 0; i < 3; i++)
				{
					VGA_Draw_Tetromino_Square(i, 1, color, gridChoice, virtual_base);
				}
			}
			break;
		case 2:
			// Draw L Tetromino
			color = ORANGE;
			if(gridChoice == 0)
			{
				for(i = 3; i < 6; i++)
				{
					VGA_Draw_Tetromino_Square(i, 0, color, gridChoice, virtual_base);
				}
				VGA_Draw_Tetromino_Square(3, 1, color, gridChoice, virtual_base);
			}
			else if(gridChoice == 1)
			{
				for(i = 0; i < 3; i++)
				{
					VGA_Draw_Tetromino_Square(i, 1, color, gridChoice, virtual_base);
				}
				VGA_Draw_Tetromino_Square(0, 2, color, gridChoice, virtual_base);
			}
			break;
		case 3:
			// Draw O Tetromino
			color = YELLOW;
			if(gridChoice == 0)
			{
				for(i = 3; i < 5; i++)
				{
					VGA_Draw_Tetromino_Square(i, 0, color, gridChoice, virtual_base);
					VGA_Draw_Tetromino_Square(i, 1, color, gridChoice, virtual_base);
				}
			}
			else if(gridChoice == 1)
			{
				for(i = 0; i < 2; i++)
				{
					VGA_Draw_Tetromino_Square(i, 1, color, gridChoice, virtual_base);
					VGA_Draw_Tetromino_Square(i, 2, color, gridChoice, virtual_base);
				}
			}
			break;
		case 4:
			// Draw S Tetromino
			color = GREEN;
			if(gridChoice == 0)
			{
				for(i = 4; i < 6; i++)
				{
					VGA_Draw_Tetromino_Square(i, 0, color, gridChoice, virtual_base);
				}
				for(i = 3; i < 5; i++)
				{
					VGA_Draw_Tetromino_Square(i, 1, color, gridChoice, virtual_base);
				}
			}
			else if(gridChoice == 1)
			{
				for(i = 1; i < 3; i++)
				{
					VGA_Draw_Tetromino_Square(i, 1, color, gridChoice, virtual_base);
				}
				for(i = 0; i < 2; i++)
				{
					VGA_Draw_Tetromino_Square(i, 2, color, gridChoice, virtual_base);
				}
			}
			
			break;
		case 5:
			// Draw T Tetromino
			color = PURPLE;
			if(gridChoice == 0)
			{
				for(i = 3; i < 6; i++)
				{
					VGA_Draw_Tetromino_Square(i, 1, color, gridChoice, virtual_base);
				}
				VGA_Draw_Tetromino_Square(4, 0, color, gridChoice, virtual_base);
			}
			else if(gridChoice == 1)
			{
				for(i = 0; i < 3; i++)
				{
					VGA_Draw_Tetromino_Square(i, 2, color, gridChoice, virtual_base);
				}
				VGA_Draw_Tetromino_Square(2, 1, color, gridChoice, virtual_base);
			}
			break;
		case 6:
			// Draw Z Tetromino
			color = RED;
			if(gridChoice == 0)
			{
				for(i = 3; i < 5; i++)
				{
					VGA_Draw_Tetromino_Square(i, 0, color, gridChoice, virtual_base);
				}
				for(i = 4; i < 6; i++)
				{
					VGA_Draw_Tetromino_Square(i, 1, color, gridChoice, virtual_base);
				}
			}
			else if(gridChoice == 1)
			{
				for(i = 0; i < 2; i++)
				{
					VGA_Draw_Tetromino_Square(i, 1, color, gridChoice, virtual_base);
				}
				for(i = 1; i < 3; i++)
				{
					VGA_Draw_Tetromino_Square(i, 2, color, gridChoice, virtual_base);
				}
			}

			break;
		default:
			printf("DEFAULT CASE, we should not be here!\n");
			break;
	}
}

/****************************************************************************************
 * Delete Old Tetromino
****************************************************************************************/
void VGA_Delete_Old(int x1_old, int y1_old, int x2_old, int y2_old, int x3_old, int y3_old, int x4_old, int y4_old, short **gridArray, void *virtual_base)
{ 	
	// Delete Old
	VGA_Draw_Tetromino_Square(x1_old, y1_old, BLACK, PLAYAREAGRID, virtual_base);
	gridArray[x1_old][y1_old] = BLACK;
	VGA_Draw_Tetromino_Square(x2_old, y2_old, BLACK, PLAYAREAGRID, virtual_base);
	gridArray[x2_old][y2_old] = BLACK;
	VGA_Draw_Tetromino_Square(x3_old, y3_old, BLACK, PLAYAREAGRID, virtual_base);
	gridArray[x3_old][y3_old] = BLACK;
	VGA_Draw_Tetromino_Square(x4_old, y4_old, BLACK, PLAYAREAGRID, virtual_base);
	gridArray[x4_old][y4_old] = BLACK;
}

/****************************************************************************************
 * Draw New Tetromino
****************************************************************************************/
void VGA_Draw_New(int x1_new, int y1_new, int x2_new, int y2_new, int x3_new, int y3_new, int x4_new, int y4_new, short color, short **gridArray, void *virtual_base)
{ 
	// Draw New
	VGA_Draw_Tetromino_Square(x1_new, y1_new, color, PLAYAREAGRID, virtual_base);
	gridArray[x1_new][y1_new] = color;
	VGA_Draw_Tetromino_Square(x2_new, y2_new, color, PLAYAREAGRID, virtual_base);
	gridArray[x2_new][y2_new] = color;
	VGA_Draw_Tetromino_Square(x3_new, y3_new, color, PLAYAREAGRID, virtual_base);
	gridArray[x3_new][y3_new] = color;
	VGA_Draw_Tetromino_Square(x4_new, y4_new, color, PLAYAREAGRID, virtual_base);
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
					VGA_Draw_Tetromino_Square(j, currentRow, BLACK, PLAYAREAGRID, virtual_base);	
					VGA_Draw_Tetromino_Square(j, i, gridArray[i][j], PLAYAREAGRID, virtual_base);	
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
 * Rotate Tetrominoes CW
****************************************************************************************/
void VGA_Rotate_Tetromino(int choice, int *currentRotation, short **gridArray, struct Tetromino * movingTetptr, void *virtual_base)
{ 

}

/****************************************************************************************
 * Shift Tetromino
****************************************************************************************/
void Tetromino_Shift(short **gridArray, struct Tetromino * movingTetptr, int shiftType, void *virtual_base)
{ 
	int oldx[4], oldy[4];
	int newx[4], newy[4];
	switch(shiftType)
	{
		oldx[0] = movingTetptr->x[0];
		oldx[1] = movingTetptr->x[1];
		oldx[2] = movingTetptr->x[2];
		oldx[3] = movingTetptr->x[3];
		oldy[0] = movingTetptr->y[0];
		oldy[1] = movingTetptr->y[1];
		oldy[2] = movingTetptr->y[2];
		oldy[3] = movingTetptr->y[3];
		
		case 1:
			// Shift Left
				newx[0] = movingTetptr->x[0] - 1;
				newy[0] = movingTetptr->y[0];
				newx[1] = movingTetptr->x[1] - 1;
				newy[1] = movingTetptr->y[1];
				newx[2] = movingTetptr->x[2] - 1;
				newy[2] = movingTetptr->y[2];
				newx[3] = movingTetptr->x[3] - 1;
				newy[3]= movingTetptr->y[3];	
			break;
		case 2:
			// Shift Right
				newx[0] = movingTetptr->x[0] + 1;
				newy[0] = movingTetptr->y[0];
				newx[1] = movingTetptr->x[1] + 1;
				newy[1] = movingTetptr->y[1];
				newx[2] = movingTetptr->x[2] + 1;
				newy[2] = movingTetptr->y[2];
				newx[3] = movingTetptr->x[3] + 1;
				newy[3] = movingTetptr->y[3];	
			break;
		case 3:
			// Shift Down
				newx[0] = movingTetptr->x[0];
				newy[0] = movingTetptr->y[0] + 1;
				newx[1] = movingTetptr->x[1];
				newy[1] = movingTetptr->y[1] + 1;
				newx[2] = movingTetptr->x[2];
				newy[2] = movingTetptr->y[2] + 1;
				newx[3] = movingTetptr->x[3];
				newy[3] = movingTetptr->y[3] + 1;	
			break;
		default:
			break;
	}
	if((newx[0] && newy[0] && newx[1] && newx[1] && newx[2] && newx[2] && newx[3] && newx[3]) >= 0 
	&& (newy[0] && newy[1] && newy[2] && newx[3] ) < ROWS 
	&& (newx[0] && newx[1] && newx[2] && newx[3] ) < COLUMNS)
	{
		if(gridArray[newy[0]][newx[0]] == BLACK 
		&& gridArray[newy[1]][newx[1]] == BLACK 
		&& gridArray[newy[2]][newx[2]] == BLACK 
		&& gridArray[newy[3]][newx[3]] == BLACK)
		{
			movingTetptr->x[0] = newx[0];
			movingTetptr->y[0] = newy[0];
			movingTetptr->x[1] = newx[1];
			movingTetptr->y[1] = newy[1];
			movingTetptr->x[2] = newx[2];
			movingTetptr->y[2] = newy[2];
			movingTetptr->x[3] = newx[3];
			movingTetptr->y[3] = newy[3];
			VGA_Delete_Old(oldx[0], oldy[0], oldx[1], oldy[1], oldx[2], oldy[2], oldx[3], oldy[3], gridArray, virtual_base);
			VGA_Draw_New(newx[0], newy[0], newx[1], newy[1], newx[2], newy[2], newx[3], newy[3], movingTetptr->color, gridArray, virtual_base);
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
 * Initialize Multiple Arrays
****************************************************************************************/
void initArrays(short **dataArray)
{ 
	dataArray = malloc(sizeof(int *) * ROWS);
	int i, j;

	for (i = 0; i < ROWS; i++) 
	{
		// malloc space for row i's M column elements
		dataArray[i] = malloc(sizeof(int) * COLUMNS);
	}
	
	for (i = 0; i < ROWS; i++) 
	{
		for (j = 0; j < COLUMNS; j++) 
		{
			dataArray[i][j] = BLACK;
		}
    }
}

int main(int argc,char ** argv) {
	
    void *virtual_base;
    int fd, file, i;
	const char *filename = "/dev/i2c-0";
	uint8_t id;
	bool bSuccess;
	const int mg_per_digi = 4;
	uint16_t szXYZ[3];
	int delay = 1000000; // 1 second
	
	/* Game Variables */
	bool changed = false;
	short **gridData;			// Tetromino Square Grid
	int lineCount = 0, level = 0, score = 0, randTetrominoChoice = 0;
	struct Tetromino movingTet;	// Tetromino moving on the screen
	struct Tetromino nextTet;	// Tetromino to be dropped next

	srand(time(0)); // Start RNG
	
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
		VGA_Tetris_Setup(virtual_base);
		randTetrominoChoice = rand() % 7;
		initArrays(gridData);
		VGA_Draw_Next_Tetromino(randTetrominoChoice, NEXTPIECEGRID, &nextTet, virtual_base);
		getchar();
		//main grid test
		for(i = 0; i < 7; i++)
		{
			printf("Drawing: %d\n",i);
			VGA_Draw_Next_Tetromino(i, 0, &movingTet, virtual_base);
			getchar();
			//VGA_Delete_Old(movingTet.x[0],movingTet.y[0],movingTet.x[1],movingTet.y[1],movingTet.x[2],movingTet.y[2],movingTet.x[3],movingTety[3], gridData, virtual_base);
		}
		
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