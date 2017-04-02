#define NUMBER_OF_SLOTS_IN_A_SECOND 18
#define COUNTER_MAX_VALUE 900000000

#include "mbed.h"
#include "tasks.h"
#include "iostream"

////////////////////////////////////////////////////////////////////////////////
// @author: Aidan Gallagher
// @date: March 2017
// @project: Embedded software - Assignment2
// @description: Simple cyclic executive for the MBED unit. Should perform the 
//               functions below at the specified rate.
//
// T1: function=measure_frequency    rate= 1 /Second
// T2: function=read_switch          rate=3  /Second
// T3: function=watchdog_pulse       rate=3  /Second
// T4: function=read_analogue_input  rate=2  /Second
// T5: function=display              rate=0.5/Second
// T6: function=error_code           rate=2  /Second
// T7: function=log_values           rate=0.2/Second
// T8: function=check_button         rate=1  /Second
////////////////////////////////////////////////////////////////////////////////                                                              .

// Variables
Ticker slot;
int slot_counter = 1;

//==============================================================================
// Scheduler decides which task to run based on the slot_counter.
// Each second is split up into 18 timeslots of equal length(0.05s). Each
// timeslot is assigned a specific task as shown below:
//
// <-----------------------1 second---------------------->
// |1 |2 |3 |4 |  5  |  6  |7 |8 |9 |10|11|12|13|14|15|16|17|18|
// |T1|T2|T3|T4|T5_p1|T5_p2|T6|T2|T3|T8|T7|T8|T4|T2|T3|T6|T8|T8|
// 
// Some tasks run multiple times a second and therefore have multiple slots  
// equally spaced apart. Other tasks run less than once a second therefore they
// only have 1 slot. 
// The most infrequent task is the log values task which is run once every 5s,
// therefore the entire cycle repeats itself every 5 seconds. 
// Displaying to the LCD takes more than 1 slot's length of time therefore it is 
// split up into 2 functions which are run in seperate (consecutive) time slots.
//==============================================================================
void scheduler()
{
    if((slot_counter%18)==1)
    {
        //task 1
        measure_frequency();   
    }
    else if((slot_counter%6)==2)
    {
        //task 2
        read_switch();    
    }
    else if((slot_counter%6)==3)
    {
        //task 3
        watchdog_pulse();   
    }
    else if((slot_counter%9)==4)
    {
        //task 4
        read_analogue_input();    
    }
    else if((slot_counter%36)==5)
    {
        //task 5
        display_1();    
    }
    else if((slot_counter%36)==6)
    {
        //task 5
        display_2();    
    }
    else if((slot_counter%9)==7)
    {
        //task 6
        error_code();    
    }
    else if((slot_counter%90)==11)
    {
        //task 7
        log_values();   
    }
    else
    {
        //task 8
        check_button(&slot);
    }

    //Increment counter. Reset periodically to stop overflow.
    slot_counter++;
    if(slot_counter > (COUNTER_MAX_VALUE)){slot_counter=1;}
}
 
//==============================================================================
// Loops inside empty while loop.
// Breaks loop every 1/18th of a second to run Scheduler.
//==============================================================================
int main() 
{
    slot.attach(&scheduler, 1.0/NUMBER_OF_SLOTS_IN_A_SECOND); 
    while(1) {
        wait(0.02);
    }
}