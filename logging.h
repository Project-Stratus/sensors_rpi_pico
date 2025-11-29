// int setup_fs();
// void write_log(char *msg);

#ifndef LOG_H
#define LOG_H

void write_result(float uv, float direction, float temperature);
void setup_fs();

typedef struct
{
    float uv;
    float direction;
    float temperature;
} Log;

#endif