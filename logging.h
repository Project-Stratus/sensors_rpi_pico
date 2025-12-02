// int setup_fs();
// void write_log(char *msg);

#ifndef LOG_H
#define LOG_H

typedef struct
{
    float uv;
    long press_data;
    int direction;   // int
    int temperature; // int
    // long
} log_t;

void write_result(log_t *);
void setup_fs();

#endif