////////////////////////////////////////////////////////////////////////////////
// @author: Aidan Gallagher
// @date: March 2017
// @project: Embedded software - Assignment 3
//////////////////////////////////////////////////////////////////////////////// 

void init();
void check_and_wait(int wait_time_ms, const char* func_name);
void read_pedals();
void read_engine_state();
void avg_speed();
void move_servo();
void speed_limit();
void display();
void mail_queue();
void serial_write();
void set_side_light();
void indicator();
void car_sim();