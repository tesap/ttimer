
#include<assert.h>

#define X "\033[107m  \033[0m"
#define _ "  "

#define ASCII_ROWS 9
#define ASCII_COLS_DIGIT 7
#define ASCII_COLS_DOTS 3

#define SCREEN_IS_SMALL_MSG "=== SCREEN SIZE IS TOO SMALL ==="

const char* ascii_one[9] = {
    _ _ _ X _ _ _, // 1
    _ _ X X _ _ _, // 2
    _ X _ X _ _ _, // 3
    _ _ _ X _ _ _, // 4
    _ _ _ X _ _ _, // 5
    _ _ _ X _ _ _, // 6
    _ _ _ X _ _ _, // 7
    _ _ _ X _ _ _, // 8
    X X X X X X X, // 9
};

const char* ascii_two[9] = {
    _ X X X X X _, // 1
    X _ _ _ _ _ X, // 2
    _ _ _ _ _ _ X, // 3
    _ _ _ _ _ X _, // 4
    _ _ _ _ X _ _, // 5
    _ _ _ X _ _ _, // 6
    _ _ X _ _ _ _, // 7
    _ X _ _ _ _ _, // 8
    X X X X X X X, // 9
};

const char* ascii_three[9] = {
    _ X X X X X _, // 1
    X _ _ _ _ _ X, // 2
    _ _ _ _ _ _ X, // 3
    _ _ _ _ _ X _, // 4
    _ _ X X X _ _, // 5
    _ _ _ _ _ X _, // 6
    _ _ _ _ _ _ X, // 7
    X _ _ _ _ _ X, // 8
    _ X X X X X _, // 9
};

const char* ascii_four[9] = {
    X _ _ _ _ X _, // 1
    X _ _ _ _ X _, // 2
    X _ _ _ _ X _, // 3
    X _ _ _ _ X _, // 4
    X X X X X X X, // 5
    _ _ _ _ _ X _, // 6
    _ _ _ _ _ X _, // 7
    _ _ _ _ _ X _, // 8
    _ _ _ _ _ X _, // 9
};

const char* ascii_five[9] = {
    _ X X X X X X, // 1
    X _ _ _ _ _ _, // 2
    X _ _ _ _ _ _, // 3
    X X X X _ _ _, // 4
    _ _ _ _ X X _, // 5
    _ _ _ _ _ _ X, // 6
    _ _ _ _ _ _ X, // 7
    _ _ _ _ X X _, // 8
    X X X X _ _ _, // 9
};

const char* ascii_six[9] = {
    _ X X X X X _, // 1
    X _ _ _ _ _ X, // 2
    X _ _ _ _ _ _, // 3
    X _ _ _ _ _ _, // 4
    X X X X X X _, // 5
    X _ _ _ _ _ X, // 6
    X _ _ _ _ _ X, // 7
    X _ _ _ _ _ X, // 8
    _ X X X X X _, // 9
};

const char* ascii_seven[9] = {
    X X X X X X X, // 1
    _ _ _ _ _ _ X, // 2
    _ _ _ _ _ X _, // 3
    _ _ _ _ X _ _, // 4
    _ _ _ X _ _ _, // 5
    _ _ _ X _ _ _, // 6
    _ _ X _ _ _ _, // 7
    _ X _ _ _ _ _, // 8
    X _ _ _ _ _ _, // 9
};

const char* ascii_eight[9] = {
    _ X X X X X _, // 1
    X _ _ _ _ _ X, // 2
    X _ _ _ _ _ X, // 3
    X _ _ _ _ _ X, // 4
    _ X X X X X _, // 5
    X _ _ _ _ _ X, // 6
    X _ _ _ _ _ X, // 7
    X _ _ _ _ _ X, // 8
    _ X X X X X _, // 9
};

const char* ascii_nine[9] = {
    _ X X X X X _, // 1
    X _ _ _ _ _ X, // 2
    X _ _ _ _ _ X, // 3
    X _ _ _ _ _ X, // 4
    _ X X X X X X, // 5
    _ _ _ _ _ _ X, // 6
    _ _ _ _ _ _ X, // 7
    X _ _ _ _ X _, // 8
    _ X X X X _ _, // 9
};

const char* ascii_zero[9] = {
    _ X X X X X _, // 1
    X _ _ _ _ _ X, // 2
    X _ _ _ _ _ X, // 3
    X _ _ _ _ _ X, // 4
    X _ _ _ _ _ X, // 5
    X _ _ _ _ _ X, // 6
    X _ _ _ _ _ X, // 7
    X _ _ _ _ _ X, // 8
    _ X X X X X _, // 9
};

static const char* ascii_dots[9] = {
    _ _ _, // 1
    _ _ _, // 2
    _ _ _, // 3
    _ X _, // 4
    _ _ _, // 5
    _ X _, // 6
    _ _ _, // 7
    _ _ _, // 8
    _ _ _, // 9
};

void draw_ascii(const char* arr[], int w, int h, int marginLeft, int marginTop) {
    for (int y = 0; y < h; ++y) {
        positionAt(marginLeft, marginTop + y);
        write(STDOUT_FILENO, arr[y], strlen(arr[y]));
    }
}

void draw_ascii_digit(const char* arr[], int marginLeft, int marginTop) {
    draw_ascii(arr, ASCII_COLS_DIGIT, ASCII_ROWS, marginLeft, marginTop);
}

void draw_ascii_dots(int marginLeft, int marginTop) {
    draw_ascii(ascii_dots, ASCII_COLS_DOTS, ASCII_ROWS, marginLeft, marginTop);
}

const char** ascii_digits[10] = {
    ascii_zero,
    ascii_one,
    ascii_two,
    ascii_three,
    ascii_four,
    ascii_five,
    ascii_six,
    ascii_seven,
    ascii_eight,
    ascii_nine,
};

int get_digits_margin_top(int h) {
    return (h - ASCII_ROWS) / 2;
}

void getCenteredPosition(int w, int h, int sw, int sh, int* x, int* y) {
    *x = (sw - w) / 2;
    *y = (sh - h) / 2;
}

void draw_ascii_clock(int hours, int minutes, int seconds, int w, int h) {
    char hours_str[16];
    sprintf(hours_str, "%02d", hours);

    char minutes_str[16];
    sprintf(minutes_str, "%02d", minutes);

    char seconds_str[16];
    sprintf(seconds_str, "%02d", seconds);

    int digits_sep = 2;

    bool has_hours = strcmp(hours_str, "00") != 0;
    bool has_minutes = strcmp(minutes_str, "00") != 0;

    int dots_count = has_hours ? 2 : (has_minutes ? 1 : 0);
    int digits_count = 2 + dots_count * 2;

    int clock_w = digits_count * (ASCII_COLS_DIGIT * 2 + digits_sep) + dots_count * (ASCII_COLS_DOTS * 2 + digits_sep);
    if (clock_w > w) {
        int x, y;
        char* msg = SCREEN_IS_SMALL_MSG;
        getCenteredPosition(strlen(msg), 1, w, h, &x, &y);
        positionAt(x, y);
        write(STDOUT_FILENO, msg, strlen(msg));
        return;
    }
    int x = (w - clock_w) / 2;
    int marginTop = get_digits_margin_top(h);

    if (has_hours) {
        const char** ascii_h0 =  ascii_digits[hours_str[0] - '0'];
        const char** ascii_h1 =  ascii_digits[hours_str[1] - '0'];
        draw_ascii_digit(ascii_h0, x, marginTop);
        x += ASCII_COLS_DIGIT * 2 + digits_sep;

        draw_ascii_digit(ascii_h1, x, marginTop);
        x += ASCII_COLS_DIGIT * 2 + digits_sep;

        // Dots
        draw_ascii_dots(x, marginTop);
        x += ASCII_COLS_DOTS * 2 + digits_sep;
        // ====
    }

    if (has_hours || has_minutes) {
        const char** ascii_m0 =  ascii_digits[minutes_str[0] - '0'];
        const char** ascii_m1 =  ascii_digits[minutes_str[1] - '0'];
        draw_ascii_digit(ascii_m0, x, marginTop);
        x += ASCII_COLS_DIGIT * 2 + digits_sep;

        draw_ascii_digit(ascii_m1, x, marginTop);
        x += ASCII_COLS_DIGIT * 2 + digits_sep;

        // Dots
        draw_ascii_dots(x, marginTop);
        x += ASCII_COLS_DOTS * 2 + digits_sep;
        // ====
    }

    const char** ascii_s0 =  ascii_digits[seconds_str[0] - '0'];
    const char** ascii_s1 =  ascii_digits[seconds_str[1] - '0'];

    draw_ascii_digit(ascii_s0, x, marginTop);
    x += ASCII_COLS_DIGIT * 2 + digits_sep;

    draw_ascii_digit(ascii_s1, x, marginTop);
}
