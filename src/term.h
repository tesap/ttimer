#include <sys/ioctl.h>

struct termios orig_termios;

void die(const char *s) {
    // TODO Add refresh+positionTop
    perror(s);
    exit(1);
}

void disableRawMode() {
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1) {
        die("tcsetattr");
    }
    enableCursor();
    reinitScreen();
    positionAtStart();
}

void enableRawMode() {
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(disableRawMode);

    struct termios raw;
    int res = tcgetattr(STDIN_FILENO, &raw);
    if (res == -1 ) die("tcgetattr");
    // ICANON - disable canonical mode, i.e read byte-by-byte
    // ECHO - disable echoing
    // ISIG - Disable auto handling Ctrl-C and Ctrl-Z
    // IEXTEN - Disable Ctrl-V
    raw.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    // IXON - Disable Ctrl-Q and Ctrl-S
    // ICRNL - Disable Ctrl-M translation
    raw.c_iflag &= ~(IXON | ICRNL);
    // OPOST - Disable translation \n to \r\n
    raw.c_oflag &= ~(OPOST);

    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    res = tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
    if (res == -1 ) die("tcsetattr");
}


float uptimeSeconds() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void secsToTimestamp(int secs, int* hours, int* minutes, int* seconds) {
    *hours = secs / 3600;
    *minutes = (secs - *hours * 3600) / 60;
    *seconds = (secs - *hours * 3600 - *minutes * 60);
}


bool mainLoop(struct appState* s) {
    static bool redraw = true;
    char c = '\0';
    int res = read(STDIN_FILENO, &c, 1);
    if (res == -1 ) die("read");

    if (c == CTRL_KEY('c')) return false;
    if (c == 'b') {
        s->draw_borders_flag = !s->draw_borders_flag;
    }
    if (c == 'p') {
        if (s->type == TIMER_ACTIVE) {
            s->type = TIMER_PAUSE;
            s->pauseState.pause_start = uptimeSeconds();
            redraw = true;
        } else if (s->type == TIMER_PAUSE) {
            s->type = TIMER_ACTIVE;
            float pause_duration = uptimeSeconds() - s->pauseState.pause_start;
            s->timer_start += pause_duration;
            redraw = true;
        }
    }
    if (c == CTRL_KEY('c')) return false;

    float cur_time = uptimeSeconds();
    if (s->type == TIMER_ACTIVE) {
        float seconds_passed = cur_time - s->timer_start;
        float seconds_left = s->timer_length - seconds_passed;
        float new_display_seconds = (s->is_increasing) ? seconds_passed : seconds_left;

        if ((int)s->display_seconds != (int)new_display_seconds) {
            s->display_seconds = (int)new_display_seconds;
            redraw = true;
        }

        if (seconds_left < 0) {
            s->type = TIMER_ENDED;
            redraw = true;
        }

        float progressRatio = (float)(cur_time - s->timer_start) / s->timer_length;
        int new_display_completed_blocks = progressRatio * s->width * PROGRESS_BAR_LENGTH_RATIO;
        if (new_display_completed_blocks != s->display_completed_blocks) {
            s->display_completed_blocks = new_display_completed_blocks;
            redraw = true;
        }
    } else if (s->type == TIMER_ENDED) {
        if (cur_time - s->endedState.last_blink_swap > BLINK_DURATION_SEC) {
            s->endedState.last_blink_swap = cur_time;
            s->endedState.is_blink = !s->endedState.is_blink;
            redraw = true;
        }
    } else if (s->type == TIMER_PAUSE) {
        if (cur_time - s->pauseState.last_blink_swap > BLINK_DURATION_SEC) {
            s->pauseState.last_blink_swap = cur_time;
            s->pauseState.is_blink = !s->pauseState.is_blink;
            redraw = true;
        }
    }

    if (!redraw) {
        return true;
    }

    // === DRAW ===
    clearScreen();
    if (s->draw_borders_flag) {
        drawBorders(s->width, s->height, '~');
    }

    if (SHOW_REFRESH_SYMBOL) {
        draw_refresh_symbol(s->width, s->height);
    }

    switch (s->type) {
        case (TIMER_PAUSE):
        case (TIMER_ACTIVE):
        case (TIMER_ENDED): {
            int hours, minutes, seconds;
            secsToTimestamp(s->display_seconds, &hours, &minutes, &seconds);
            if (s->type == TIMER_ACTIVE ||
               (s->type == TIMER_PAUSE && s->pauseState.is_blink) ||
               (s->type == TIMER_ENDED && s->endedState.is_blink)) {
                draw_ascii_clock(hours, minutes, seconds, s->width, s->height);
            }
            break;
        }
    }

    draw_progress_bar(s->display_completed_blocks, s->width, s->height);
    redraw = false;
    return true;
}

int getWindowSize(int *cols, int *rows) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        return -1;
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
    // TODO add fallback method
}
