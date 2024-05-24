#include "unity.h"
#include "code_errors.h"

void setUp(void) {}
void tearDown(void) {}

void test_code_insert_error(void)
{
    BIT_ERROR_RATE = 0.01;
    Message msg = { 0 };
    strncpy(msg.msg, "This is a test.", MAX_MSG_LENGTH);
    msg.msg_length = strlen(msg.msg);
    code_insert_error(&msg);
    puts(msg.msg);
    TEST_ASSERT_EQUAL_STRING("Thi3 is a test.", msg.msg);
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_code_insert_error);
    return UNITY_END();
}