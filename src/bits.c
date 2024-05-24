#include <stdio.h>
#include <stdint.h>
#include "bits.h"

uint16_t set_nth_bit(int n, uint16_t m)
{
    return m | (1 << (16-n));
}

uint16_t get_nth_bit(int n, uint16_t m)
{ 
    return (m >> (16-n)) & 1;
}

uint16_t chg_nth_bit(int n, uint16_t m)
{
    return m ^ (1 << (16-n));
}

void print_word(int k, uint16_t m)
{
    (void)k;
    for (int i = 0; i < 16; i++)
        printf("%d", get_nth_bit(i, m));
    printf("\n");
}

// Adapted from https://en.wikipedia.org/wiki/Hamming_weight
// Use 13 arithmetic operations for a 16-bit integer
int weight(uint16_t x)
{
    x -= (x >> 1) & 0x5555;
    x = (x & 0x3333) + ((x >> 2) & 0x3333);
    x = (x + (x >> 4)) & 0x0f0f;
    x = (x + (x >> 8)) & 0x7f;
    return x;
}
