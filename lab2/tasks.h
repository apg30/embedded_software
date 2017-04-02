#ifndef TASKS_H
#define TASKS_H

#include "mbed.h"

////////////////////////////////////////////////////////////////////////////////
// @author: Aidan Gallagher
// @date: March 2017
// @project: Embedded software - Assignment2
////////////////////////////////////////////////////////////////////////////////

// Tasks
void measure_frequency();
void read_switch();
void watchdog_pulse();
void read_analogue_input();
void display_1();
void display_2();
void error_code();
void log_values();
void check_button(Ticker *slot);

#endif