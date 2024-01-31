/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 *
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

// Main program for exercise

//****************************************************
//By default, every output used in this exercise is 0
//****************************************************
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "sleep.h"
#include "xgpiops.h"
#include "xttcps.h"
#include "xscugic.h"
#include "xparameters.h"
#include "Pixel.h"
#include "Interrupt_setup.h"

//********************************************************************
//***************TRY TO READ COMMENTS*********************************
//********************************************************************

//***Hint: Use sleep(x)  or usleep(x) if you want some delays.****
//***To call assembler code found in blinker.S, call it using: blinker();***


//Comment this if you want to disable all interrupts
#define enable_interrupts




/***************************************************************************************
Name: Abdullah Sagir
Student number: 151945467

Name:
Student number:

Name:
Student number:

Tick boxes that you have coded

Led-matrix driver		Game		    Assembler
	[x]					[x]					[]

Brief description:

In the game, the user shoots an alien moving downwards with a side-controlled ship.
When the player hits the alien, the player gets a point. When the alien hits to the ship, the alien gets a point.
The points are shown on the right side.
The first to get four points wins.

BTN3: Steer left
BTN2: Steer right
BTN1: Shoot the ship
BTN0: Start the game over

SW1: Changes the game refresh rate from 10Hz <-> 20Hz
SW0: Changes the difficulty level of the game, i.e. the speed of the alien

*****************************************************************************************/

volatile uint32_t channel = 0;

// Coordinates of the ship
volatile uint8_t x = 3;
volatile uint8_t y = 7;

// Ship color
volatile uint8_t r = 255;
volatile uint8_t b = 65;
volatile uint8_t g = 65;

// Color of projectile
volatile uint8_t bullet_r = 255;
volatile uint8_t bullet_g = 255;
volatile uint8_t bullet_b = 0;

// Alien color
volatile uint8_t alien_r = 0;
volatile uint8_t alien_g = 255;
volatile uint8_t alien_b = 0;


// Coordinates of the alien
volatile uint8_t alien_x = 0;
volatile uint8_t alien_y = 0;
// Direction of the alien
volatile int is_going_left = 0;

// Coordinates of the projectile
volatile uint8_t bullet_x = 0;
volatile uint8_t bullet_y = 0;


// Timer to calculate the alien's movement
volatile uint32_t timer = 0;
volatile int timer_state = 1;


// Game difficulty
volatile uint32_t difficulty = 2;
volatile uint32_t freq = 5;


// The truth value of the projectile
volatile int bullet_exist = 0;




// Game score
volatile uint8_t player_score = 0;
volatile uint8_t score_xpos = 7;
volatile uint8_t alien_score = 0;



int main()
{
	//**DO NOT REMOVE THIS****
	    init_platform();
	//************************


#ifdef	enable_interrupts
	    init_interrupts();
#endif


	    //setup screen
	    setup();



	    Xil_ExceptionEnable();



	    //Try to avoid writing any code in the main loop.
		while(1){


		}


		cleanup_platform();
		return 0;
}


//Timer interrupt handler for led matrix update. Frequency is 800 Hz
void TickHandler(void *CallBackRef){
	//Don't remove this
	uint32_t StatusEvent;

	// Exceptions must be disabled when updating screen
	Xil_ExceptionDisable();



	//****Write code here ****


	 // To update the matrix, check that the channel is not greater than 7
	if (channel>7)  {
		channel=0;
		}

	// Close all channels with the default case of open_line()
	open_line(8);

	// Run the current channel and open it, then increment the channel
   // run(channel);
	run(channel);
	open_line(channel);
	channel++;






	//****END OF OWN CODE*****************

	//*********clear timer interrupt status. DO NOT REMOVE********
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);
	//*************************************************************
	//enable exceptions
	Xil_ExceptionEnable();
}


// Timer interrupt for moving alien, shooting... Frequency is 10 Hz by default
//Timer interrupt for moving alien, shooting... Frequency is 10 Hz by default
void TickHandler1(void *CallBackRef){

	//Don't remove this
	uint32_t StatusEvent;

	//****Write code here ****
	// Updates the ship
		DrawShip();

		// Movement of projectile and alien
		// Moves the projectile faster than the alien depending on the difficulty level
		if(timer_state == 1){
			if(bullet_exist == 1){
				MoveBullet();
			}

			timer++;
			if(timer >= difficulty){
				MoveAlien();
				timer = 0;
			}
		}


		// Projectile and alien collide
		if(alien_x == bullet_x && alien_y == bullet_y && alien_x != 0 && alien_y !=0){
			SetPixel(bullet_x,bullet_y,0,0,0);
			player_score += 1;
			SetPixel(score_xpos, 4-player_score, 0,255,0);

			RespawnAlien();
		}

		// Alien and ship collide
		if(x == alien_x && y == alien_y+1){
			alien_x = 0;
			alien_y = 0;

			SetPixel(bullet_x,bullet_y,0,0,0);

			alien_score += 1;
			SetPixel(score_xpos, 3+alien_score, 255,0,0);

			RespawnAlien();
		}

		// Checking whether the game was won or lost
		if(player_score == 4 || alien_score == 4){
			timer_state = 0;

			if(player_score == 4){
				PrintWin();
			}else{
				PrintLoss();
			}


		}




	//****END OF OWN CODE*****************
	//clear timer interrupt status. DO NOT REMOVE
	StatusEvent = XTtcPs_GetInterruptStatus((XTtcPs *)CallBackRef);
	XTtcPs_ClearInterruptStatus((XTtcPs *)CallBackRef, StatusEvent);

}


//Interrupt handler for switches and buttons.
//Reading Status will tell which button or switch was used
//Bank information is useless in this exercise
void ButtonHandler(void *CallBackRef, u32 Bank, u32 Status) {
    // Extracted constants for button and switch statuses
    const u32 BTN0_MASK = 0x01;
    const u32 BTN1_MASK = 0x02;
    const u32 BTN2_MASK = 0x04;
    const u32 BTN3_MASK = 0x08;
    const u32 SW0_MASK  = 0x10;
    const u32 SW1_MASK  = 0x20;

    //****Start of your code****

    // Handle button and switch events
    if (Status & BTN0_MASK) {
        // BTN0 is clicked
        ResetGame();
    } else if (Status & BTN1_MASK) {
        // BTN1 is clicked
        if (timer_state == 1 && bullet_exist == 0) {
            bullet_x = x;
            bullet_y = y - 2;
            bullet_exist = 1;

            // Initialize bullet
            SetPixel(bullet_x, bullet_y, bullet_r, bullet_g, bullet_b);
        }
    } else if (Status & BTN2_MASK) {
        // BTN2 is clicked
        if (x < 6) {
            // Move the ship
            DeleteShip();
            x += 1;
        }
    } else if (Status & BTN3_MASK) {
        // BTN3 is clicked
        if (x > 0) {
            // Move the ship
            DeleteShip();
            x -= 1;
        }
    } else if (Status & SW0_MASK) {
        // SW0 position is changed
        // Toggle difficulty level (alien speed)
        difficulty = (difficulty == 2) ? 4 : 2;
    } else if (Status & SW1_MASK) {
        // SW1 position is changed
        // Toggle update frequency
        freq = (freq == 20) ? 10 : 20;
        change_freq(freq);
    }

    //****End of your code****
}


