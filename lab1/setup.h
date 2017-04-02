// Aidan Gallagher
// January 2017
// Embedded Software - Assignment 1

#ifndef SETUP_H
#define SETUP_H

// Struct to hold parameters
struct Parameters {
  int a;
  int b;
  int c;
  int d;
  int mode;
};

// Global variables
extern bool invert;
extern Parameters params;
extern Parameters mod_params;
extern int numbers[5];

// Setup functions
void lcd_display(char name[]);
int * letters_to_number(char name[]);
void calculate_parameters(int numbers[5]);
void calculate_modified_parameters();

#endif