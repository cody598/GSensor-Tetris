#ifndef VGA_GSENSOR_TETRIS_H
#define VGA_GSENSOR_TETRIS_H

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
#define NEXTPIECEGRID 1
#define PLAYAREAGRID 0
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

#define SCORETEXTOFFSET 48

#define SCORETOPOFFSET 8
#define LINETOPOFFSET 10
#define LEVELTOPOFFSET 12
#define NEXTPIECETOPOFFSET 15

#define NEXTPIECEPIXELLEFTOFFSET 196
#define NEXTPIECEPIXELTOPOFFSET 144


#define SCORELEFTOFFSET SCORETEXTOFFSET + 8
#define LINELEFTOFFSET SCORETEXTOFFSET + 8
#define LEVELLEFTOFFSET SCORETEXTOFFSET + 8
#define NEXTPIECELEFTOFFSET SCORETEXTOFFSET + 7



// Define Colors
#define CYAN 0x07FF // I
#define YELLOW 0xFF33 // O
#define PURPLE 0x8010 // T
#define GREEN 0x4D04 // S
#define RED 0xF800 // Z
#define BLUE 0x001F // J
#define ORANGE 0xFD20 // L
#define BLACK 0x0000 
#define GREY 0x9D13
#define WHITE 0xFFFF

#define PHYSMEM_32(addr) (*((unsigned int *)(virtual_base + (addr & HW_REGS_MASK)))) // Char
#define PHYSMEM_16(addr) (*((unsigned short *)(virtual_base + (addr & HW_REGS_MASK)))) // Pixel

#endif /*VGA_GSENSOR_TETRIS_H*/