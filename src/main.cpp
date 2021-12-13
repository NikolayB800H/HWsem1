#include <stdio.h>

#include "dot_reader.hpp"

int main(int argc, char const *argv[]) {
    DotReader input;
    if (input.getText() != OK) {
        puts("getText() error");
        return -1;
    }
    if (input.modify() != OK) {
        puts("modify() error");
        return -1;
    }
    putchar('\n');
    if (input.putText() != OK) {
        puts("putText() error");
        return -1;
    }
    return 0;
}
