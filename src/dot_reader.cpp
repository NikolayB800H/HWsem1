#include <new>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <ctype.h>
#include <unistd.h>

#include "dot_reader.hpp"

static uch getch() {
    struct termios old_tio, new_tio;
    uch c;
    tcgetattr(STDIN_FILENO, &old_tio);
    new_tio = old_tio;
    new_tio.c_lflag &= (~ICANON & ~ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_tio);
    c = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &old_tio);
    return c;
}

DotReader::DotReader() : size(SIZE_ON_START), len(0), text(new uch[SIZE_ON_START]) {
}

DotReader::~DotReader() {
    delete[] text;
}

Code DotReader::getText() {
    uch getchar_ret = EOF;
    uch prev = 0;
    bool was_last_space = true;
    while (getchar_ret != '.') {
        getchar_ret = getch();
        if (getchar_ret == EOF) {
            return GET_TEXT_ERROR;
        }
        if (isspace(getchar_ret)) {
            if (was_last_space) {
                continue;
            }
            was_last_space = true;
            getchar_ret = ' ';
        } else {
            if ((!islower(getchar_ret) && getchar_ret != 127) && getchar_ret != '.') {
                continue;
            }
            was_last_space = false;
        }
        putchar(getchar_ret);
        if (getchar_ret == 127) {
            if (len) {
                --len;
                putchar('\b');
                putchar(' ');
                putchar('\b');
                if (!len || text[len - 1] == ' ') {
                    was_last_space = true;
                } else {
                    was_last_space = false;
                }
            }
        } else if (getchar_ret == '.' && prev == ' ') {
            putchar('\b');
            putchar('\b');
            putchar('.');
            putchar(' ');
            text[len - 1] = getchar_ret;
        } else {
            text[len] = getchar_ret;
            ++len;
        }
        prev = getchar_ret;
        if (sizeUpdate() != OK) {
            return GET_TEXT_ERROR;
        }
        text[len] = 0;
        fflush(stdout);
    }
    return OK;
}

Code DotReader::modify() {
    char *s_text = static_cast<char *>(static_cast<void *>(text));
    s_text[len - 1] = 0;
    char *last_pos = strrchr(s_text, ' ');
    if (last_pos) {
        ++last_pos;
    } else {
        last_pos = s_text;
    }
    size_t w_len = strlen(last_pos);
    char *last_found = s_text;
    while (((last_found = strstr(last_found, last_pos)) != nullptr && last_found != last_pos) && last_found[w_len] == ' ') {
        memset(last_found, '$', w_len + 1);
    }
    char *to = s_text;
    char *from = s_text;
    for (; *from; ++from, ++to) {
        while (*from == '$') {
            ++from;
        }
        *to = *from;
    }
    *to = 0;
    //printf("%p %p\n", last_pos, text);
    // оставить все слова, отличающиеся от последнего слова, перед печатью удалив из слова все последующие вхождения первой буквы
    return OK;
}

Code DotReader::putText() {
    if (puts(static_cast<char *>(static_cast<void *>(text))) == EOF) {
        return PUT_TEXT_ERROR;
    }
    return OK;
}

Code DotReader::sizeUpdate() {
    if (len == size - 1) {
        size *= 2;
        uch *new_text = new(std::nothrow) uch[size];
        if (new_text == nullptr) {
            return SIZE_UPDATE_ERROR;
        }
        memcpy(new_text, text, len * sizeof(uch));
        delete[] text;
        text = new_text;
    }
    return OK;
}
