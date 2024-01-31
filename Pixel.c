/*
 * Pixel.c
 *
 *  Created on: -----
 *      Author: -----
 */

#include "Pixel.h"



//Table for pixel dots.
//				 dots[X][Y][COLOR]
volatile uint8_t dots[8][8][3]={0};


volatile uint8_t *pixel_ctrl = (uint8_t *)0x41220008;
volatile uint8_t *pixel_chnl = (uint8_t *)0x41220000;

// Ship color
extern uint8_t r;
extern uint8_t b;
extern uint8_t g;

// Ship
extern uint8_t x;
extern uint8_t y;


// Alien
extern uint8_t alien_x;
extern uint8_t alien_y;
extern int is_going_left;


// Alien color
extern uint8_t alien_r;
extern uint8_t alien_g;
extern uint8_t alien_b;

// timer boolean
extern int timer_state;

// Timer to count alien movement
extern uint32_t timer;

// game speed
// 1000 = 1s
extern uint32_t speed;


// Projectile
extern uint8_t bullet_x;
extern uint8_t bullet_y;

extern int bullet_exist;
extern uint8_t bullet_r;
extern uint8_t bullet_g;
extern uint8_t bullet_b;



// score
extern uint8_t player_score;
extern uint8_t score_xpos;

extern uint8_t alien_score;

// Here the setup operations for the LED matrix will be performed
void setup() {
    // Clear control and channel registers
    *pixel_ctrl = 0;
    *pixel_chnl = 0;

    // Initialize pixel control with specific bit sequences
    *pixel_ctrl |= 1;        // Set bit0 to 1
    usleep(500);
    *pixel_ctrl ^= 0;        // Set bit0 to 0
    usleep(500);
    *pixel_ctrl |= 1;        // Set bit0 to 1

    *pixel_ctrl |= 0x10;     // Set bit4 to 1
    uint8_t gamma[] = {63, 63, 63};

    // Loop to set gamma values
    for(uint8_t i = 0; i < 8; i++) {
        for(uint8_t a = 0; a < 3; a++) {
            uint8_t t = gamma[a];

            // Nested loop to set gamma bits
            for(uint8_t b = 0; b < 6; b++) {
                if (t & 0b100000) {
                    *pixel_ctrl |= 0x10;
                } else {
                    *pixel_ctrl &=~ 0x10;
                }
                *pixel_ctrl &=~ 0x08;
                t <<= 1;
                *pixel_ctrl |= 0x08;
            }
        }
    }

    *pixel_ctrl |= 0x04;  // Set bit2 to 1 (Enable transmission)

    // Additional setup steps can be added here

    // Finalize setup by enabling transmission to the 8-bit register
}


//Change value of one pixel at led matrix. This function is only used for changing values of dots array
void SetPixel(uint8_t x,uint8_t y, uint8_t r, uint8_t g, uint8_t b){

	//Hint: you can invert Y-axis quite easily with 7-y
	dots[x][y][0]=b;
	dots[x][y][1]=g;
	dots[x][y][2]=r;
	//Write rest of two lines of code required to make this function work properly (green and red colors to array).


}


//Put new data to led matrix. Hint: This function is supposed to send 24-bytes and parameter x is for channel x-coordinate.
void run(uint8_t x){



	//Write code that writes data to led matrix driver (8-bit data). Use values from dots array
	//Hint: use nested loops (loops inside loops)
	//Hint2: loop iterations are 8,3,8 (pixels,color,8-bitdata)

	*pixel_ctrl &=~0x02;                //Before loops, latch should be 0 (see latch())

	for(uint8_t y=0; y<8; y++)
	{
		for(uint8_t a=0; a<3; a++)
		{
			uint8_t t = dots[x][y][a];
			for(uint8_t counter=0; counter<8;counter++)
			{
				if(t & 0x80) *pixel_ctrl |= 0x10;

				else *pixel_ctrl &=~ 0x10;

				*pixel_ctrl &=~ 0x08;

				t <<= 1;

				*pixel_ctrl |= 0x08;
			}
		}
	}
	latch();
	*pixel_ctrl &=~ 0x08;


}

//Latch signal. See colorsshield.pdf or DM163.pdf in project folder on how latching works
void latch(){
	*pixel_ctrl |= 0x02;        //Set bit1 at address to 1
	*pixel_ctrl &=~0x02;           //Set bit1 at address to 0

}


//Set one line (channel) as active, one at a time.
void open_line(uint8_t x){


	switch(x){
		case 0: *pixel_chnl |= 0x01; break;
		case 1: *pixel_chnl |= 0x02; break;
		case 2: *pixel_chnl |= 0x04; break;
		case 3: *pixel_chnl |= 0x08; break;
		case 4: *pixel_chnl |= 0x10; break;
		case 5: *pixel_chnl |= 0x20; break;
		case 6: *pixel_chnl |= 0x40; break;
		case 7: *pixel_chnl |= 0x80; break;
		default: *pixel_chnl &= ~(0xFF);
	}

}



// Move the aliens in the right direction
void MoveAlien(){
	// Remove the old alien
	SetPixel(alien_x,alien_y,0,0,0);

	// If the alien is on the right edge and heading right
	if(alien_x == 6 && is_going_left == 0){
		alien_y += 1;
		is_going_left = 1;

	// If the alien is on the left edge and traveling on the left
	}else if(alien_x == 0 && is_going_left == 1){
		alien_y += 1;
		is_going_left = 0;
		// The alien travels to the right
	}else if(is_going_left == 0){
		alien_x += 1;
		// The alien travels to the left
	}else if(is_going_left == 1){
		alien_x -= 1;
	}
	// Draw a new alien and reset the timer
	SetPixel(alien_x,alien_y,alien_r,alien_g,alien_b);
}

// Move or remove the bullet
void MoveBullet(){
    if(bullet_y != 0){
    	// The projectile is not yet on the roof
        SetPixel(bullet_x,bullet_y,0,0,0);
        bullet_y -= 1;
        SetPixel(bullet_x,bullet_y,bullet_r,bullet_g,bullet_b);
    }else{
    	// Bullet on the ceiling
        SetPixel(bullet_x,bullet_y,0,0,0);
        bullet_exist = 0;
    }
}

// Draw the ship
void DrawShip(){
	// Draw ship
	SetPixel(x+1, y,r,g,b);
	SetPixel(x, y,r,g,b);
	SetPixel(x-1, y,r,g,b);
	SetPixel(x, y-1,r,g,b);
}

// Reset the scoreboard
void ResetScore(){
	for(uint8_t i = 0; i<8; ++i){
		SetPixel(score_xpos, i, 0,0,0);
	}
}

// Start the game from the beginning
void ResetGame(){
	// Clears the board (e.g. a smiley face)
	for(int x = 0; x<8; ++x){
		for(int y = 0; y<8; ++y){
			SetPixel(x,y,0,0,0);
		}
	}
	SetPixel(alien_x,alien_y,0,0,0);
	SetPixel(bullet_x,bullet_y,0,0,0);
	alien_x = 0;
	alien_y = 0;

	is_going_left = 0;
	timer = 0;
	timer_state = 1;
	bullet_exist = 0;
	bullet_x = 0;
	bullet_y =0;

	player_score = 0;
	alien_score = 0;

	SetPixel(alien_x,alien_y,alien_r,alien_g,alien_b);

	ResetScore();
}

// Recreate the alien
void RespawnAlien(){
	SetPixel(alien_x,alien_y,0,0,0);
	SetPixel(bullet_x,bullet_y,0,0,0);
	alien_x = 0;
	alien_y = 0;

	is_going_left = 0;
	timer = 0;

	bullet_exist = 0;
	bullet_x = 0;
	bullet_y =0;

	SetPixel(alien_x,alien_y,alien_r,alien_g,alien_b);
}

// Removes the ship
void DeleteShip(){
	SetPixel(x+1,y,0,0,0);
	SetPixel(x,y,0,0,0);
	SetPixel(x-1,y,0,0,0);
	SetPixel(x, y-1,0,0,0);
}

// Draws a smiley face
void PrintWin(){
	DeleteShip();
	SetPixel(0,0,0,0,0);

	SetPixel(1,1,0,255,0);
	SetPixel(1,2,0,255,0);

	SetPixel(2,1,0,255,0);
	SetPixel(2,2,0,255,0);

	SetPixel(5,1,0,255,0);
	SetPixel(5,2,0,255,0);

	SetPixel(4,1,0,255,0);
	SetPixel(4,2,0,255,0);

	SetPixel(0,4,0,255,0);
	SetPixel(5,4,0,255,0);

	SetPixel(1,5,0,255,0);
	SetPixel(4,5,0,255,0);

	SetPixel(2,6,0,255,0);
	SetPixel(3,6,0,255,0);
}

// Draws a sad face
void PrintLoss(){
	DeleteShip();
	SetPixel(0,0,0,0,0);

	SetPixel(1,1,255,0,0);
	SetPixel(1,2,255,0,0);
	SetPixel(2,1,255,0,0);
	SetPixel(2,2,255,0,0);

	SetPixel(5,1,255,0,0);
	SetPixel(5,2,255,0,0);

	SetPixel(4,1,255,0,0);
	SetPixel(4,2,255,0,0);

	SetPixel(2,4,255,0,0);
	SetPixel(3,4,255,0,0);

	SetPixel(1,5,255,0,0);
	SetPixel(4,5,255,0,0);

	SetPixel(0,6,255,0,0);
	SetPixel(5,6,255,0,0);

}








