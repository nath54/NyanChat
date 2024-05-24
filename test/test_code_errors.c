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
    TEST_ASSERT_EQUAL_INT16(0b0000000110111001, word);
}

void test_code_hamming_distance(void)
{
    uint16_t P = 0b110111001;
    int d = code_hamming_distance(P);
    TEST_ASSERT_EQUAL_INT(4, d);
}


void test_lfsr(void)
{
    uint16_t P = 11;
    uint16_t x = 229;
    uint16_t encoded = rem_lfsr(P, x);
    TEST_ASSERT_EQUAL(1, encoded);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_code_insert_error);
    RUN_TEST(test_lfsr);
    RUN_TEST(test_code_hamming_distance);
    RUN_TEST(test_encode);
    return UNITY_END();
}
