#include <stdio.h>
#include <stdint.h>
#include "bits.h"

uint16_t set_nth_bit(int n, uint16_t m)
    { return m | (1 << n); }

uint16_t get_nth_bit(int n, uint16_t m)
    { return (m >> n) & 1; }

uint16_t chg_nth_bit(int n, uint16_t m)
    { return (get_nth_bit(n, m) == 1) ? m & ~(1 << n) : set_nth_bit(n, m); }

void print_word(int k, uint16_t m)
{
    for (int i = k - 1; i >= 0; i--)
        printf("%d", get_nth_bit(i, m));
    printf("\n");
}

int card_word_bits(uint16_t m)
{
    int count = 0;
    while (m > 0) {
        count += m & 1;
        m >>= 1;
    }
    return count;
}