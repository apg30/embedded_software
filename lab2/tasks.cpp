#include "tasks.h"
#include "MCP23017.h"           // include 16-bit parallel I/O header file
#include "WattBob_TextLCD.h"    // include 2*16 character display header file 

////////////////////////////////////////////////////////////////////////////////
// @author: Aidan Gallagher
// @date: March 2017
// @project: Embedded software - Assignment2
////////////////////////////////////////////////////////////////////////////////

// Input/Output
DigitalIn square_wave(p11);       //task1
DigitalIn switch_1(p19);          //task2
DigitalOut watchdog_pin(p21);     //task3
AnalogIn analogue_1(p15);         //task4
AnalogIn analogue_2(p16);         //task4
MCP23017 par_port(p9, p10, 0x40); //task5  pointer to 16-bit parallel I/O object
WattBob_TextLCD lcd(&par_port);   //task5  pointer to 2*16 chacater LCD object 
DigitalOut myled(LED1);           //task6
Serial pc(USBTX, USBRX);          //task7
DigitalIn shutdown(p18);          //task8
DigitalOut execution_time(p14); 

// Variables
bool digital_value=0;               // Digital value from switch 1
float frequency =0;                 // Frequency of square wave
float analogue_values_1[4]= {0.0f}; // Array of last 4 values from analogue 1
float analogue_values_2[4] = {0.0f};// Array of last 4 values from analogue 2
float avg_analogue_1 = 0.0f;        // Average of last 4 analogue 1 values
float avg_analogue_2 = 0.0f;        // Average of last 4 analogue 1 values
int analogue_index = 0;             // Index of which element in array to write to

//==============================================================================
// Measure the frequency of a square wave with duty cycle of 50%. 
// Wait until the square wave flips, start timer, wait until square wave flips,
// stop timer. Take the difference and multiply by 2 for the frequency.
// If frequncy is outside the bounds ( 500 > freq > 1000) then set to NAN to 
// inform the user.
//==============================================================================
void measure_frequency()
{ 
    Timer timer;
    
    int limit = 1000;
    int square_wave_before = square_wave;            //Find out what signal is at
    while(square_wave_before==square_wave && limit>0)// Wait for transition
    {
        wait_us(1);
        limit--;
    }
        
    timer.start();                      //signal has just changed so start timer
    
    limit = 1000;
    square_wave_before = square_wave;
    while(square_wave_before==square_wave && limit>0)// Wait for transition
    {
        wait_us(1);
        limit--;
    }
    
    float elapsed_time = timer.read();
    if (elapsed_time > 0)
    {
        frequency = 1/(elapsed_time *2);
    }
    if (frequency > 1001 * 1.025 || frequency < 499 * 0.975)
    {
        frequency = NAN;
    }
}

//==============================================================================
// Read digital input (ignoring switch bounce)
// Set signal high at start and set low at end to measure function length using
// oscilloscope. 
//==============================================================================
void read_switch()
{   
    execution_time = 1;
    digital_value = switch_1;
    execution_time = 0;
}

//==============================================================================
// Output digital pulse of length 7ms. 
//==============================================================================
void watchdog_pulse()
{   
    watchdog_pin = 1;
    wait_ms(7);
    watchdog_pin = 0;
}

//==============================================================================
// Read 2 analogue inputs and store into 2 different arrays. Sum each array and 
// divide by array size to attain the average. 
// Increment the analogue count and reset if it is larger than the array size. 
//==============================================================================
void read_analogue_input()
{   
  analogue_values_1[analogue_index] = analogue_1;
  analogue_values_2[analogue_index] = analogue_2;
  
  avg_analogue_1 = 3.3 * (analogue_values_1[0] + analogue_values_1[1] + analogue_values_1[2] + analogue_values_1[3]) / 4;
  avg_analogue_2 = 3.3 * (analogue_values_2[0] + analogue_values_2[1] + analogue_values_2[2] + analogue_values_2[3]) / 4;
  
  analogue_index++;
  if (analogue_index > 3)
  {
      analogue_index =0;
  }
}

//==============================================================================
// Display to the LCD screen the 
// - frequency of the sqaure wave
// - the digital input
//==============================================================================
void display_1()
{ 
    par_port.write_bit(1,BL_BIT); // turn LCD backlight ON 
    lcd.cls();                    // clear display
    
    lcd.locate(0,0);              // set cursor to top left corner
    lcd.printf("F:%.0f D:%d", frequency, digital_value);  
}

//==============================================================================
// Display to the LCD screen the 
// - the (averaged) analogue inputs 
//==============================================================================
void display_2()
{
    lcd.locate(1,0);              // set cursor to top left corner
    lcd.printf("A:%.2f, %.2f",avg_analogue_1,avg_analogue_2);    
}
//==============================================================================
// If avg analogue_value_1 is greater than avg analogue_value_1 and switch 1
// is high then set error code.
// Where the error code is flashing a blue LED on the MBED board
// The flashing light is achieved by toggling the LED every turn if the error is 
// set, otherwise the LED is turned off. 
//==============================================================================
void error_code()
{
    if(switch_1 && avg_analogue_1 > avg_analogue_2 && switch_1 == 1)
    {   //set error code
           myled = !myled;
    }
    else
    {
        myled=0;
    }
}

//==============================================================================
// Log data in comma delimited format to the serial port. The data is
// - frequency of the sqaure wave
// - the digital input values
// - filtered analogue values 
//==============================================================================
void log_values()
{
    pc.printf("%d, %.2f, %.2f \r\n",digital_value,avg_analogue_1,avg_analogue_2);
}

//==============================================================================
// Check whether shutdown button has been selected.
// Shutdown the system by printing shutdown to LCD to inform user and detaching 
// the ticker to stop the scheduler from running.
//==============================================================================
void check_button(Ticker *slot)
{
    if(shutdown)
    {
        lcd.cls();                    // clear display
        lcd.locate(0,0);              // set cursor to top left corner
        lcd.printf("Shutdown");
        slot->detach();   
    }
}