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
    while (getchar_ret != '.') {
        getchar_ret = getch();
        if (getchar_ret == EOF) {
            return GET_TEXT_ERROR;
        }
        if (((!isspace(getchar_ret) && !islower(getchar_ret)) && getchar_ret != 127) && getchar_ret != '.') {
            continue;
        }
        putchar(getchar_ret);
        if (getchar_ret == 127) {
            if (len) {
                --len;
                putchar('\b');
                putchar(' ');
                putchar('\b');
            }
        } else {
            text[len] = getchar_ret;
            ++len;
        }
        if (sizeUpdate() != OK) {
            return GET_TEXT_ERROR;
        }
        text[len] = 0;
        fflush(stdout);
    }
    return OK;
}

// оставить все слова, отличающиеся от последнего слова, перед печатью удалив из слова все последующие вхождения первой буквы
Code DotReader::modify() {
    formatSpaces();
    char *s_text = static_cast<char *>(static_cast<void *>(text));
    char *last_pos = strrchr(s_text, ' ');
    if (last_pos) {
        ++last_pos;
    } else {
        last_pos = s_text;
    }
    size_t w_len = strlen(last_pos);
    char *last_found = s_text;
    while (((last_found = strstr(last_found, last_pos)) != nullptr && last_found != last_pos)) {
        if (last_found[w_len] == ' ' && (last_found == s_text || (last_found[-1] == ' ' || last_found[-1] == '$'))) {
            memset(last_found, '$', w_len + 1);
        } else {
            ++last_found;
        }
    }
    last_found = s_text - 1;
    char first = 0;
    do {
        first = last_found[1];
        if (first != '$') {
            char *now = last_found + 2;
            for (; *now && *now != ' '; ++now) {
                if (*now == first) {
                    *now = '$';
                }
            }
            last_found = now;
        } else {
            do {
                ++last_found;
            } while (*last_found == '$');
            --last_found;
        }
    } while (*last_found != 0);
    char *to = s_text;
    char *from = s_text;
    for (; *from; ++from, ++to) {
        while (*from == '$') {
            ++from;
        }
        *to = *from;
    }
    if (to == s_text) {
        ++to;
    }
    if (*(--to)) {
        ++to;
    }
    *to = '.';
    to[1] = 0;
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

void DotReader::formatSpaces() {
    bool was_last_space = true;
    uch *from_spc = text;
    uch *to_spc = text;
    for (; from_spc != text + len; ++from_spc) {
        if (isspace(*from_spc)) {
            if (was_last_space) {
                continue;
            }
            was_last_space = true;
            *(to_spc++) = ' ';
        } else {
            was_last_space = false;
            *(to_spc++) = *from_spc;
        }
    }
    to_spc[-1] = '.';
    len = to_spc - text;
    if (to_spc > text + 1 && to_spc[-2] == ' ') {
        to_spc[-2] = '.';
        --len;
    }
    text[len] = 0;
    --len;
    printf("%s|%d\n", text, len);
}
