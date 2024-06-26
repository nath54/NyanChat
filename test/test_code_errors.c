#include "unity.h"
#include "code_errors.h"
#include "bits.h"

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

void test_encode_lfsr(void)
{
    uint16_t P = 0b110111001;
    uint16_t x8 = 0x0100;
    TEST_ASSERT_EQUAL_UINT8(0b10111001, (uint8_t)encode_lfsr(P, x8));

    uint16_t x9 = 0x0200;
    TEST_ASSERT_EQUAL_UINT8(0b11001011, (uint8_t)encode_lfsr(P, x9));
}

static void print_detected_errors(Message* msg)
{
    printf("Potential errors : ");
    for (uint32_t i = 0; i < msg->msg_length; i++) {
        if (msg->error[i])
            printf("\033[0;31m%c\033[0m", msg->msg[i]);
        else
            printf("\033[0;32m%c\033[0m", msg->msg[i]);
    }
    printf("\n");
}

void test_code_correct_error(void)
{
    BIT_ERROR_RATE = 0.01;

    Message msg = { 0 };
    strncpy(msg.msg, "Les oiseaux chantent au printemps pour profiter du beau temps.", MAX_MSG_LENGTH);
    msg.msg_length = strlen(msg.msg);
    srand(msg.msg_length);
    add_control_bits(&msg);
    code_insert_error(&msg);
    printf("Received message : %s\n", msg.msg);
    int rc = code_correct_error(&msg);
    print_detected_errors(&msg);
    TEST_ASSERT_EQUAL_INT(-1, rc);
}

int main(void)
{
    UNITY_BEGIN();
    create_syndrome_array();

    RUN_TEST(test_code_insert_error);
    RUN_TEST(test_encode_lfsr);
    RUN_TEST(test_code_hamming_distance);
    RUN_TEST(test_encode);
    RUN_TEST(test_code_correct_error);

    return UNITY_END();
}
