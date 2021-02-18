#include "gtest/gtest.h"

char * print_binary_representation(unsigned int i, char *buffer){
    buffer[0] = '0';
    buffer[1] = 'b';
    // fill out remaining 32 bits, 1 or 0 depending on the value in the number i

    int index = 34; // size of buffer is 32 + 2 since '0b' were added initially

    while(i > 0 && index > 1) {  // using num / 2, the remainders get us the binary values in reverse order
        buffer[--index] =  (i % 2u) + '0'; // added the + '0' and '\0' to keep C from cutting off end values
        buffer[strlen(buffer)] = '\0';
        i /= 2u;
    }

    for (int j = 2; j < 34; j++) { // adds 0's where there are no 1's
        if (buffer[j] != '1') {
            buffer[j] = '0';
        }
    }
    return buffer;
}

/* PROBLEM 1: Implement a print_binary_representation function that takes an
 * unsigned integer and created as string representation of the binary values
 * of that number.
 *
 * The test below show what the expected values are for given inputs
 */

TEST(print_binary_representation, works) {
    // row 1
    char buffer[50] = {0}; // init to 0
    EXPECT_STREQ("0b00000000000000000000000000000000", print_binary_representation(0, buffer));
    EXPECT_STREQ("0b00000000000000000000000000000001", print_binary_representation(1, buffer));
    EXPECT_STREQ("0b00000000000000000000000000000010", print_binary_representation(2, buffer));
    EXPECT_STREQ("0b00000000000000000000000000000011", print_binary_representation(3, buffer));
    EXPECT_STREQ("0b00000000000000000000000000000100", print_binary_representation(4, buffer));
    EXPECT_STREQ("0b00000001110111111001101001000010", print_binary_representation(31431234, buffer));
    EXPECT_STREQ("0b00011011111000100100001101011101", print_binary_representation(467813213, buffer));
    EXPECT_STREQ("0b11111111111111111111111111111111", print_binary_representation(UINT32_MAX, buffer));
}

/* PROBLEM 2: The test below fails.  Change the signature of set_my_age and the
 * call of the function in get_my_age so that the expected value is returned.
 *
 * HINT: C is pass by value
 */

struct Person {
    char * name;
    int age;
};

void set_my_age(struct Person *p) {
    p -> age = 44; // Changed p to be a pointer in order to reference a person stored somewhere else in memory
}

int get_my_age() {
    struct Person me;
    me.name = "Carson";
    set_my_age(&me); // Inputting just "me" was sending in the pointer versus "&me" which sends in the address of the pointer
    return me.age;
}

TEST(set_my_age, works) {
    EXPECT_EQ(44, get_my_age());
}




