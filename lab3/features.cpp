#define AVERAGE_NUMBER 3  // Number of reading to take average from
#define MAX_ACC 15        // Miles per hour/second
#define MAX_BRAKE 55      // Miles per hour/second
#define MAX_SPEED 130     // Miles per hour
#define DELTA_TIME 0.05 
#define MAX_ANALOGUE_VALUE 1
#define MAX_SERVO_VALUE 1
#define SPEED_DECAY 0.999

#include "mbed.h"
#include "Servo.h"
#include "features.h"
#include "rtos.h"

#include "MCP23017.h"           // include 16-bit parallel I/O header file
#include "WattBob_TextLCD.h"    // include 2*16 character display header file 

////////////////////////////////////////////////////////////////////////////////
// @author: Aidan Gallagher
// @date: March 2017
// @project: Embedded software - Assignment 3
// @notes: Thread::wait( n ms) parameter is given in milliseconds 
//////////////////////////////////////////////////////////////////////////////// 

// Input and Output
////////////////////////////////////////////////////////////////////////////////
//  Signal      Direction    Purpose                    Pin  Name
//  A/D 1       In           Accelerator pedal          p15  acc_pedal
//  A/D 2       In           Brake pedal                p16  brake_pedal
//  Dig 1       In           Side light on/off switch   p11  side_light_switch
//  Dig 2       In           Left indicator switch      p12  left_indicator_switch
//  Dig 3       In           Right indicator switch     p13  right_indicator_switch
//  Dig 4       In           Engine on/off switch       p14  engine_switch
//  Servo       Out          Average speed              p21  avg_speed_servo
//  MBED LED 1  Out          Side light indicator       LED1 side_light
//  MBED LED 2  Out          Left turn indicator        LED2 left_indicator
//  MBED LED 3  Out          Right turn indicator       LED3 right_indicator
//  MBED LED 4  Out          Engine on/off indicator    LED4 engine_state
//  RedBox LED  Out          Overspeed indicator        p22  overspeed
////////////////////////////////////////////////////////////////////////////////
AnalogIn acc_pedal(p15); 
AnalogIn brake_pedal(p16); 
DigitalIn side_light_switch(p11); 
DigitalIn left_indicator_switch(p12); 
DigitalIn right_indicator_switch(p13); 
DigitalIn engine_switch(p14); 
Servo avg_speed_servo(p21);
DigitalOut side_light(LED1);
DigitalOut left_indicator(LED2);
DigitalOut right_indicator(LED3);
DigitalOut engine_state(LED4);
DigitalOut overspeed(p22);

MCP23017 par_port(p9, p10, 0x40); //  pointer to 16-bit parallel I/O object
WattBob_TextLCD lcd(&par_port);  //   pointer to 2*16 chacater LCD object 
Serial pc(USBTX, USBRX); // tx, rx

// Structures
typedef struct {
  float    speed_mail;
  float    acc_mail; 
  float    brake_mail; 
} mail_t;

// Variables
float brake_var=0;
float acc_var=0;
float speed_var=0;
float odomotor_var=0;
float avg_speed_var=0;
float speeds_var[AVERAGE_NUMBER]={0};
int avg_speed_counter=0;
bool engine_var=0;
bool finished=0;
bool prev_left_indicator=false;
bool prev_right_indicator=false;

//Mutexes
Mutex brake_mutex; 
Mutex acc_mutex;
Mutex speed_mutex;
Mutex odomotor_mutex;
Mutex avg_speed_mutex;
Mutex engine_var_mutex;

Mail<mail_t, 100> mail_box;

////////////////////////////////////////////////////////////////////////////////
// Initial setup. Print to terminal and calirate servo.
////////////////////////////////////////////////////////////////////////////////
void init()
{
    pc.printf("\nStarting car \r\n");
    avg_speed_servo.calibrate(0.0009f,45.0f);
}

////////////////////////////////////////////////////////////////////////////////
// Check if wait time is greater than 0 then wait that time else display error 
// to terminal.
////////////////////////////////////////////////////////////////////////////////
void check_and_wait(int wait_time_ms, const char* func_name)
{
        if(wait_time_ms<0){
            pc.printf("Error in Task %s. Time diff: %d ms \r\n", func_name, wait_time_ms );
        }else{           
            Thread::wait(wait_time_ms);
        }
}

////////////////////////////////////////////////////////////////////////////////
// Read brake and accelerator values from variable resistors.
// Scale brake and accelerator values.
////////////////////////////////////////////////////////////////////////////////
void read_pedals()
{
    float tmp_brake_var =0;
    float tmp_acc_var =0;
    
    Timer t;
    t.start();
    int wait_time_ms=0;
    
    while(!finished)
    {
        t.reset();
            
        tmp_brake_var=(brake_pedal / MAX_ANALOGUE_VALUE) * MAX_BRAKE;
        brake_mutex.lock();
        brake_var=tmp_brake_var;       
        brake_mutex.unlock();
        
        tmp_acc_var=(acc_pedal / MAX_ANALOGUE_VALUE) * MAX_ACC ;
        acc_mutex.lock();
        acc_var=tmp_acc_var;
        acc_mutex.unlock();
        
        wait_time_ms = 100 - t.read_ms();
        check_and_wait(wait_time_ms, __func__);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Read engine on/off switch and show current state on an LED. 
////////////////////////////////////////////////////////////////////////////////
void read_engine_state()
{
    Timer t;
    t.start();
    int wait_time_ms=0;
    
    while(!finished)
    {
        t.reset();
        
        engine_var_mutex.lock();
        engine_var=engine_switch;
        if(engine_var==1)
        {
            engine_state=1;
        }
        else
        {
            engine_state=0;
        }
        engine_var_mutex.unlock();
        
        wait_time_ms = 500 - t.read_ms();
        check_and_wait(wait_time_ms, __func__);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Filter speed with averaging filter 
////////////////////////////////////////////////////////////////////////////////
void avg_speed()
{
    float tmp_avg_speed_var =0;
    
    Timer t;
    t.start();
    int wait_time_ms=0;
    
    while(!finished)
    {
        t.reset();
        
        // Add current speed to speeds array
        speed_mutex.lock();
        speeds_var[avg_speed_counter] = speed_var;
        speed_mutex.unlock();
        
        // Calcuate average
        tmp_avg_speed_var = (speeds_var[0] +
                           speeds_var[1] +
                           speeds_var[2]) / 3;  
        avg_speed_mutex.lock();
        avg_speed_var = tmp_avg_speed_var;
        avg_speed_mutex.unlock();
                                 
        // Increment counter and reset if passed max value
        avg_speed_counter++;
        if(avg_speed_counter > 2) {avg_speed_counter=0;}
        
        wait_time_ms = 200 - t.read_ms();
        check_and_wait(wait_time_ms, __func__);
        
    }
}

////////////////////////////////////////////////////////////////////////////////
// Move servo to represent average speed. Use 1 - value to make it go clockwise.
////////////////////////////////////////////////////////////////////////////////
void move_servo()
{
    Timer t;
    t.start();
    int wait_time_ms=0;
    
    while(!finished)
    {
        t.reset();
        
        // Scale value  
        avg_speed_mutex.lock();                 
        avg_speed_servo =  1- ((avg_speed_var/MAX_SPEED) * MAX_SERVO_VALUE);
        avg_speed_mutex.unlock();
        
        wait_time_ms = 1000 - t.read_ms();
        check_and_wait(wait_time_ms, __func__);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Monitor speed to check if it goes over 70 mph. If so set a digitalout high.
////////////////////////////////////////////////////////////////////////////////
void speed_limit()
{
    Timer t;
    t.start();
    int wait_time_ms=0;
    
    while(!finished)
    {
        t.reset();
        
        // Read speed value
        speed_mutex.lock();
        float tmp_speed_var = speed_var;
        speed_mutex.unlock();
        
        // Set overspeed LED
        if(tmp_speed_var <= 70)
        {
            overspeed=0;
        }
        else
        {
            overspeed=1;
        }

        wait_time_ms = 2000 - t.read_ms();
        check_and_wait(wait_time_ms, __func__);
        
    }
}

////////////////////////////////////////////////////////////////////////////////
// Display on MBED display odometer value and average speed. 
////////////////////////////////////////////////////////////////////////////////
void display()
{
    Timer t;
    t.start();
    int wait_time_ms=0;
    
    float tmp_odomotor_var = 0;
    float tmp_speed_var=0;
    
    while(!finished)
    {
        t.reset();
        
        par_port.write_bit(1,BL_BIT); // turn LCD backlight ON 
        lcd.cls();                    // clear display
        
        // Find odomotor value   
        odomotor_mutex.lock();
        tmp_odomotor_var = odomotor_var;
        odomotor_mutex.unlock();
        
        // Print odomotor value
        lcd.locate(0,0);   
        lcd.printf("Odo: %.2f ",tmp_odomotor_var);  

        // Find speed value            
        speed_mutex.lock();
        tmp_speed_var = speed_var;
        speed_mutex.unlock();
        
        // Print speed value
        lcd.locate(1,0);  
        lcd.printf("Spd: %.2f",tmp_speed_var);
               
        wait_time_ms = 500 - t.read_ms();
        check_and_wait(wait_time_ms, __func__);
    }
}

////////////////////////////////////////////////////////////////////////////////
// Send speed, accelerometer and brake values to a 100 element MAIL queue 
////////////////////////////////////////////////////////////////////////////////
void mail_queue()
{
    Timer t;
    t.start();
    int wait_time_ms =0;
    
    while(!finished)
    {
       t.reset();
       
       mail_t *mail = mail_box.alloc();
       speed_mutex.lock();
       mail->speed_mail = speed_var;
       speed_mutex.unlock();
        
       acc_mutex.lock();
       mail->acc_mail = acc_var;
       acc_mutex.unlock();
       
       brake_mutex.lock();
       mail->brake_mail = brake_var;
       brake_mutex.unlock();
       
       mail_box.put(mail);
       
       wait_time_ms = 5000 - t.read_ms();
       check_and_wait(wait_time_ms, __func__);
    }
    
}

////////////////////////////////////////////////////////////////////////////////
// Dump contents of MAIL queue to the serial connection. 
////////////////////////////////////////////////////////////////////////////////
void serial_write()
{
    Timer t;
    t.start();
    int wait_time_ms =0;
    
    while (!finished)
    {
        wait_time_ms = 20000 - t.read_ms();
        check_and_wait(wait_time_ms, __func__);
        
        for(int i=0; i<4; i++){
            osEvent evt = mail_box.get();
            if (evt.status ==osEventMail)
            {
                mail_t *mail = (mail_t*)evt.value.p;
                pc.printf("{ \"Speed\": %.2f, \"Acc\": %.2f, \"Brake\": %.2f }\r\n", mail->speed_mail, mail->acc_mail, mail->brake_mail );            
                mail_box.free(mail);           
            }
        }
        
        t.reset();
    }
}

////////////////////////////////////////////////////////////////////////////////
// Read a single side light switch and set side lights accordingly.  
////////////////////////////////////////////////////////////////////////////////
void set_side_light()
{
    Timer t;
    t.start();
    int wait_time_ms =0;
    
    while(!finished)
    {
        t.reset();
        
        if(side_light_switch==1)
        {
            side_light=1;
        }
        else
        {
            side_light=0;
        }
        
        wait_time_ms = 1000 - t.read_ms();
        check_and_wait(wait_time_ms, __func__); 
    }   
}

////////////////////////////////////////////////////////////////////////////////
// Read the two turn indicator switches and flash appropriate indicator
// LEDs at a rate of 1Hz. If both switches are switched on then flash
// both indicator LEDs at a rate of 2Hz (hazard mode).   
////////////////////////////////////////////////////////////////////////////////
void indicator()
{    
    Timer t;
    t.start();
    int wait_time_ms =0;
    
    while(!finished)
    {
       t.reset();
       
       if(left_indicator_switch && right_indicator_switch)
       {
           for(int i =0; i < 4; i++)
           {
               left_indicator = 1;
               right_indicator = 1;
               Thread::wait(250);
               left_indicator = 0;
               right_indicator = 0;
               Thread::wait(250);
           }
       }
       else if(left_indicator_switch ==1)
       {
           right_indicator=0;
           for(int i =0; i < 2; i++)
           {
               left_indicator=1;
               Thread::wait(500);
               left_indicator=0;
               Thread::wait(500);
           }
       }
       else if(right_indicator_switch ==1)
       {
           left_indicator=0;
           for(int i =0; i < 2; i++)
           {
               right_indicator = 1;
               Thread::wait(500);
               right_indicator = 0;
               Thread::wait(500);
           }
       }
       else
       {
           left_indicator=0;
           right_indicator=0;
           wait_time_ms = 2000 - t.read_ms();
           check_and_wait(wait_time_ms, __func__);
       }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Calculate speed and distance.
// Speed is in miles per hour so divide by 60 for miles per minute and again by
// 60 for miles per second. The speed in miles per second is then multiplied by 
// the number of seconds since the calculation was last run. This time is known 
// as delta time.
////////////////////////////////////////////////////////////////////////////////
void car_sim()
{
    Timer t;
    t.start();
    signed int wait_time_ms =0;
    
    float tmp_odomotor_var=0;
    float tmp_speed_var = 0;
    bool tmp_engine_var=0;
    while(!finished)
    {
        t.reset();
        
        engine_var_mutex.lock();
        tmp_engine_var=engine_var;
        engine_var_mutex.unlock();
        
        // Check if engine is off. Don't allow acceleration is engine is off.
        if(tmp_engine_var ==0)
        {     
            acc_var=0;
        }
           
        // Odomotor calcuation
        speed_mutex.lock();
        tmp_odomotor_var = tmp_odomotor_var + (((speed_var/60)/60) * DELTA_TIME);
        speed_mutex.unlock();
        
        // Set odomotor value
        odomotor_mutex.lock();
        odomotor_var=tmp_odomotor_var;
        odomotor_mutex.unlock();
        
        // Speed calculation               +
        tmp_speed_var = (tmp_speed_var + (acc_var*DELTA_TIME) - (brake_var*DELTA_TIME) ) * SPEED_DECAY;
        if (tmp_speed_var < 0){ tmp_speed_var=0;}
        if (tmp_speed_var > MAX_SPEED){ tmp_speed_var=MAX_SPEED; acc_var=0;}     
        
        // Set speed value
        speed_mutex.lock();
        speed_var=tmp_speed_var;
        speed_mutex.unlock();

        wait_time_ms = 50 - t.read_ms();
        check_and_wait(wait_time_ms, __func__);
    }
}