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
    TEST_ASSERT_EQUAL_STRING("Thi3 is a test.", msg.msg);
    // test BER 5%
    BIT_ERROR_RATE = 0.1;
    code_insert_error(&msg);
    TEST_ASSERT_EQUAL_STRING("th\xC9# \xC9s a dert\x0F", msg.msg);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_code_insert_error);
    return UNITY_END();
}