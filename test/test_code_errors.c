#include "unity.h"
#include "code_errors.h"

void setUp(void) {}
void tearDown(void) {}

void test_code_insert_error(void)
{
    Message msg = { 0 };
    strncpy(msg.msg, "This is a test.", MAX_MSG_LENGTH);
    msg.msg_length = strlen(msg.msg);
    srand(msg.msg_length);

    // test BER 1%
    BIT_ERROR_RATE = 0.01;
    code_insert_error(&msg);
    TEST_ASSERT_EQUAL_STRING("ThiS is a test.", msg.msg);
    TEST_ASSERT_EQUAL_STRING("ThiS is a test.", msg.msg);
    // test BER 5%
    BIT_ERROR_RATE = 0.1;
    code_insert_error(&msg);
    TEST_ASSERT_EQUAL_STRING("Dh9[ 9s a |es\xF4>", msg.msg);
}

void test_add_control_bits(void)
{

}

void test_encode(void)
{
    // Matrice génératrice de P
    uint16_t G[K][N] = {
        {1,0,0,0,0,0,0,0,0,1,1,1,0,1,1,0},
        {0,1,0,0,0,0,0,0,0,0,1,1,1,0,1,1},
        {0,0,1,0,0,0,0,0,1,1,0,0,0,0,0,1},
        {0,0,0,1,0,0,0,0,1,0,1,1,1,1,0,0},
        {0,0,0,0,1,0,0,0,0,1,0,1,1,1,1,0},
        {0,0,0,0,0,1,0,0,0,0,1,0,1,1,1,1},
        {0,0,0,0,0,0,1,0,1,1,0,0,1,0,1,1},
        {0,0,0,0,0,0,0,1,1,0,1,1,1,0,0,1}
    };
    uint16_t word = encode(G, 0b0000000100000000);
    TEST_ASSERT_EQUAL_UINT16(0b0000000110111001, word);
}

void test_code_hamming_distance(void)
{
    uint16_t P = 0b110111001;
    int d = code_hamming_distance(P);
    TEST_ASSERT_EQUAL_INT(4, d);
}


void test_rem_lfsr(void)
{
    uint16_t P = 0b110111001;
    uint16_t x = 0b100000000;
    uint16_t syndrome = rem_lfsr(P, x);
    TEST_ASSERT_EQUAL_UINT16(0b10111001, syndrome);
}

int main(void)
{
    UNITY_BEGIN();
    create_syndrome_array();

    RUN_TEST(test_code_insert_error);
    RUN_TEST(test_rem_lfsr);
    RUN_TEST(test_code_hamming_distance);
    RUN_TEST(test_encode);
    return UNITY_END();
}
