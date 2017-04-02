// Aidan Gallagher
// January 2017
// Embedded Software - Assignment 1

#include "MCP23017.h"        // include 16-bit parallel I/O header file
#include "WattBob_TextLCD.h" // include 2*16 character display header file 
#include <ctype.h>
#include "setup.h"

#define ASCII_A 65
#define ALPHABET_HALF 13

MCP23017 *par_port;     // pointer to 16-bit parallel I/O object 
WattBob_TextLCD *lcd;   // pointer to 2*16 chacater LCD object 

Serial pc(USBTX, USBRX);     // For pc communication

// **Display information onto the lcd**
void lcd_display(char name[])
{
     par_port = new MCP23017(p9, p10, 0x40); // initialise 16-bit I/O chip
     lcd = new WattBob_TextLCD(par_port);    // initialise 2*26 char display
     par_port->write_bit(1,BL_BIT);          // turn LCD backlight ON
    
     lcd->cls();                            // clear display
     lcd->locate(0,0);                      // set cursor to l
     lcd->printf(name);                     // print string 
}

// **Converts letters to corresponding number**
// For each letter: capitalise character for standarization. Get ASCII 
// value of character. Take 65 away from value so counting starts at 1. For 
// values greater than 13, find the difference and minus the difference from 
// 13 to get value for second half of alphabet.  
int * letters_to_number(char name[]){
    for( int i = 0; i < 5; i++)
    {
        numbers[i] =  ((int)toupper(name[i]) - (ASCII_A - 1));
        // For second half of alphabet.
        if (numbers[i] > ALPHABET_HALF){
            int diff = numbers[i] -ALPHABET_HALF;
            numbers[i] = ALPHABET_HALF - diff + 1;
        }    
    }   
    
    return numbers;
}

// **Calculates parameters based on letter numbers**
// Pass in pointer to first number in array of numbers. Calculate parameters 
// using equations specified in the assignment.  
void calculate_parameters(int * numbers){
    params.a = *(numbers + 0) * 100;
    params.b = *(numbers + 1) * 100;
    params.c = *(numbers + 2) + 7;
    params.d = *(numbers + 3) * 500;
    params.mode = (*(numbers + 4) % 4) + 1;

}

// **Sets up modified parameters for modified waveform**
// Set modified parameters to normal parameters so all components have a default
// value. Based on the mode perform modifications to parameter components.
void calculate_modified_parameters(){
    mod_params = params;
    switch(params.mode){     
        case 1: // Remove 3
                mod_params.c = params.c - 3;
                break;
        case 2: // Invert
                invert = 1;
                break;
        case 3: // Add 3
                mod_params.c = params.c + 3;
                break;
        case 4: // Half d and b
                mod_params.b = params.b / 2;
                mod_params.d = params.d / 2;
                break;
        }
}