#pragma ONCE

typedef unsigned char uch;

constexpr size_t SIZE_ON_START = 8;

enum Code {
    OK,
    GET_TEXT_ERROR,
    MODIFY_ERROR,
    PUT_TEXT_ERROR,
    SIZE_UPDATE_ERROR
};

class DotReader {
    size_t size;
    size_t len;
    uch *text;

    Code sizeUpdate();

public:
    DotReader();
    ~DotReader();
    Code getText();
    Code modify();
    Code putText();
};
