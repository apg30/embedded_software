// Aidan Gallagher
// January 2017
// Embedded Software - Assignment 1

#include "mbed.h"
#include "setup.h"

// Global variables
Parameters params = {0};     // For normal waveform cycle
Parameters mod_params = {0}; // For modified waveform cycle
bool invert = 0;             // Determine whether waveform is inverted 

// Letters
char name[] = "GALLA"; 
int numbers[5];

// Input/Output
DigitalIn switch1(p13);
DigitalIn switch2(p14);
DigitalOut signal_a(p21);
DigitalOut signal_b(p22);

// **Does initial setup**
// Print name to display. Convert letters to numbers. Calculate normal
// parameters. Calculate modified parameters.
void init(){
    lcd_display(name);                       
    int * numbers = letters_to_number(name); 
    calculate_parameters(numbers);           
    calculate_modified_parameters();         
}
    
// **Performs waveform** 
// a: base width of pule   
// b: width of space between pulses 
// c: number of pulses in a block 
// d: space between pulse blocks 
// invert: selects whether signal A should be inverted or not.
void waveform_cycle(int a, int b,int c,int d, bool invert){
    // Signal B
    signal_b = 1;
    wait_us(50);
    signal_b = 0;
    
    // Signal A
    for( int i = 0; i < c; i++)
    {
        signal_a = !invert;
        wait_us(a + (i * 50));
        signal_a = invert;
        wait_us(b);
    }    
    wait_us(d);
}

// **Main**
// Perform initial setup.
// Enter infite loop.  
int main() {
    init();        
    while(1)
    {      
        if(switch1 == 0)
        {
            // Enable stream of pulses
            if(switch2 ==0)
            {
                // Run as normal mode
                waveform_cycle(params.a, params.b, params.c, params.d, 0);
            }else
            {         
                // Run with a modified waveform cycle       
                waveform_cycle(mod_params.a, mod_params.b, mod_params.c, mod_params.d, invert);
            } 
        }   
    }
}




