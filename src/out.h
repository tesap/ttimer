void clearScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
}

void disableCursor() {
    write(STDOUT_FILENO, "\x1b[?25l", 6);
}

void enableCursor() {
    write(STDOUT_FILENO, "\x1b[?25h", 6);
}

void reinitScreen() {
    write(STDOUT_FILENO, "\x21" "c", 2);
}

void positionAtStart() {
    write(STDOUT_FILENO, "\x1b[H", 3);
}

void positionAt(int x, int y) {
    positionAtStart();

    char s_[16];
    // sprintf(s_, "\x1b[%d:%dH", y, x);
    // write(STDOUT_FILENO, s_, strlen(s_));

    sprintf(s_, "\x1b[%dC\x1b[%dB", x, y);
    write(STDOUT_FILENO, s_, strlen(s_));
}

char* buff_write(char* buff, char* s, int size) {
    /*
     * Writes a given chunk of memory @s of given size @size
     */
    memcpy(buff, s, size);
    return buff + size;
}

char* buff_write_str(char* buff, char* s) {
    /*
     * Writes a null-terminated string @s
     */
    buff_write(buff, s, strlen(s));
}

void drawBorders(int w, int h, char fill) {
    positionAtStart();
    int x, y;
    
    for (y = 0; y < h; ++y) {
        printf("%c", fill);
        for (x = 0; x < (w - 2); ++x) {
            if (y == 0 || y == h - 1) {
                printf("%c", fill);
            } else {
                printf(" ");
            }
        }
        printf("%c", fill);
        if (y < h - 1) {
            printf("\r\n");
        }
    }
    fflush(stdout);
}

void strcpy_nonull(const char* to, const char* from, int max_len) {
    /* 
     * Copies null-terminated string @from to @to buffer without adding null-termination
     * Additionally ensure that @from does not exceed @to buffer assuming it is of size @max_len
     */

    int len = strlen(from);
    if (len > max_len) {
        len = max_len;
    }

    memcpy(to, from, len);
}


void frame_buffer_write(const char* frameBuff, int position, const char* s) {
    int buff_len = strlen(frameBuff);
    strcpy_nonull(frameBuff + position, s, buff_len - position);
}

void draw_refresh_symbol(int w, int h) {
    positionAtStart();
    static int flag = 0;
    char* flag_str = "";
    switch (flag) {
        case 0:
            flag_str = "\\";
            break;
        case 1:
            flag_str = "/";
            break;
        case 2:
            flag_str = "-";
            break;
    }
    write(STDOUT_FILENO, flag_str, 1);
    flag = (flag + 1) % 3;
}

// TODO fix it!
int get_digits_margin_top(int h);

void draw_progress_bar(int completed_blocks, int w, int h) {
        int bar_len = (float)w * PROGRESS_BAR_LENGTH_RATIO;
        char* buff = malloc(sizeof(char) * bar_len);
        for (int i = 0; i < bar_len; ++i) {
            if (i < completed_blocks) {
                buff[i] = PROGRESS_BAR_COMPLETED_BLOCK;
            } else {
                buff[i] = PROGRESS_BAR_OTHER_BLOCK;
            }
        }

        int x = (w - bar_len) / 2;
        int y = h - get_digits_margin_top(h) / 2;

        positionAt(x, y);
        write(STDOUT_FILENO, buff, strlen(buff));
        free(buff);
}
