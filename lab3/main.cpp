#include "mbed.h"
#include "rtos.h"
#include "features.h"

////////////////////////////////////////////////////////////////////////////////
// @author: Aidan Gallagher
// @date: March 2017
// @project: Embedded software - Assignment 3
// @description: A controller for a part of a simple car system.
// The system will use the MBED RTOS TASKS with an appropiate delay. The delay 
// is calculated by subtracting the execution time from the desired delay.
// The system will perform the following tasks at the specified frequency:
// 
// Task           Frequency(Hz)   Peroid(ms)
// read_pedals        10              100
// engine_state        2              500
// avg_speed           5              200
// move_servo          1             1000
// speed_limit         0.5           2000
// display             2              500
// mail_queue          0.2           5000
// serial_write        0.05         20000
// side_light          1             1000
// indicator           0.5           2000
// car_sim             20              50
//
//////////////////////////////////////////////////////////////////////////////// 

// Threads
Thread t_read_pedals;
Thread t_engine_state;
Thread t_avg_speed;
Thread t_move_servo;
Thread t_speed_limit;
Thread t_display;
Thread t_mail_queue;
Thread t_serial_write;
Thread t_side_light;
Thread t_indicator;
Thread t_car_sim;

int main() 
{
    init();
    t_read_pedals.start(read_pedals);
    t_engine_state.start(read_engine_state);
    t_avg_speed.start(avg_speed);
    t_move_servo.start(move_servo);
    t_speed_limit.start(speed_limit);
    t_display.start(display);
    t_mail_queue.start(mail_queue);
    t_serial_write.start(serial_write);
    t_side_light.start(set_side_light);
    t_indicator.start(indicator);
    t_car_sim.start(car_sim);
    
    while(1){}    
}
