#ifndef __UTILITY_H__
#define __UTILITY_H__

#include "color_code.h"

#ifndef CARRIAGE_RETURN
#define CARRIAGE_RETURN     0
#endif // CARRIAGE_RETURN

void reverse_byte(uint8_t *in, uint8_t *out, int words)
{
  for (int i = 0; i < words; i++)
  {
    for (int j = 0; j < 4; j++)
    {
      out[i*4 + j] = in[i*4 + 3 - j];
    }
  }
}

void newline(int n)
{
    if (CARRIAGE_RETURN)
        for (int i = 0; i < n; i++)
            printf("\n\r");
    else 
        for (int i = 0; i < n; i++)
            printf("\n");
}

void print_time(unsigned int elapsed)
{
    if (elapsed > 30)
        printf("%u us", elapsed);
    else
        printf("<30 us");
}

void print_title(char* title)
{
    config_font(BHCYN);
    printf("Starting %s test", title);
    reset_font();
    newline(2);
}

void print_result(rt_perf_t *perf, char* title, int error, int max_error, int bytes)
{
    int cycles = rt_perf_get(perf, RT_PERF_CYCLES);
    float elapsed = (float)cycles * 50 / 1000;
    printf("Encryption time: %f us", elapsed);
    newline(1);
    printf("Encryption throughput: %f MBytes/s\t", (float)bytes / elapsed);
    printf("%f Bytes/cycle", (float)bytes / cycles);
    newline(1);
    printf("Total cycles: %d", cycles);
    newline(1);
    printf("Instructions: %d", rt_perf_get(perf, RT_PERF_INSTR));
    newline(1);
    printf("Errors: %d / %d", error, max_error);
    newline(1);
    printf("%s test: ", title);
    if (error)
    {
        config_font(RED);
        printf("FAIL");
    }
    else
    {
        config_font(GRN);
        printf("CLEAR");
    }
    reset_font();
    newline(2);
}


#endif // __UTILITY_H__
