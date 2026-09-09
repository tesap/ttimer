#include <ctype.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <getopt.h>
#include <limits.h>

#define CTRL_KEY(k) ((k) & 0x1f)
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define BLINK_DURATION_SEC 0.5
#define MAX_TIMER_DURATION 99U * 3600U + 59U * 60U + 59U

#define CONFIG_DIR "/.config/ttimer"
#define CONFIG_FILE_LAST_SAVED CONFIG_DIR "/last_saved.txt"

#define PROGRESS_BAR_LENGTH_RATIO 0.7f
#define PROGRESS_BAR_COMPLETED_BLOCK '|'
#define PROGRESS_BAR_OTHER_BLOCK '-'
#define SHOW_REFRESH_SYMBOL false

typedef enum {
    TIMER_ACTIVE,
    TIMER_ENDED,
    TIMER_PAUSE,
} StateType;

struct CmdArgs {
    const char* time;
    bool is_increasing;
    const char* end_script;
};

static struct CmdArgs cmd_args = {
    .time = NULL,
    .is_increasing = false,
    .end_script = NULL,
};

struct endedState {
    float last_blink_swap;
    bool is_blink;
};

struct pauseState {
    float last_blink_swap;
    bool is_blink;
    float pause_start;
};

struct appState {
    int width;
    int height;
    float timer_start;
    float timer_length;
    int display_seconds;
    bool draw_borders_flag;
    StateType type;
    struct endedState endedState;
    struct pauseState pauseState;
    int display_completed_blocks;
    bool is_increasing;
};

volatile sig_atomic_t screen_resized = true;

#include "out.h"
#include "digits.h"
#include "term.h"


void handle_resize(int sig) {
    screen_resized = true;
}

void exit_help_message() {
    fprintf(stderr, "Usage: ./ttimer -t <TIME> [-b <PATH>]\n\r");
    fprintf(stderr, "Options:\n\r");
    fprintf(stderr, "\t-t, --time <TIME>\tTimer length. Examples: \"1h25m30s\", \"100\"\n\r");
    fprintf(stderr, "\t-i, --increasing\tShow passed time instead of left\n\r");
    fprintf(stderr, "\t-b, --end-script <PATH>\tFile that will be executed at the end of timer\n\r");
    exit(0);
}

void parse_cmd_args(int argc, char* argv[], struct CmdArgs* args) {
    bool t_flag = false;
    bool i_flag = false;
    bool b_flag = false;

    while (1) {
        static struct option long_options[] = {
            {"help",                no_argument,       0, 'h'},
            {"time",                required_argument, 0, 't'},
            {"increasing",                no_argument, 0, 'i'},
            {"end-script",          required_argument, 0, 'b'},
            {0, 0, 0, 0}
        };

        int longindex;
        int c = getopt_long(argc, argv, "t:b:ih", long_options, &longindex);
        if (c == -1) {
            break;
        }

        switch (c) {
            case 0:
                break;
            case 'h':
                exit_help_message();
                break;
            case 't':
                if (t_flag) {
                    exit_help_message();
                }
                t_flag = true;
                args->time = optarg;
                break;
            case 'i':
                if (i_flag) {
                    exit_help_message();
                }
                i_flag = true;
                args->is_increasing = true;
                break;
            case 'b':
                if (b_flag) {
                    exit_help_message();
                }
                b_flag = true;
                args->end_script = optarg;
                break;
            case '?':
                exit_help_message();
            default:
                printf("==== default\n");
        }
    }
}

int parseTimeArg(const char* s) {
    /*
     * Parses input argument --time and returns time duration in seconds
     * Examples:
     *     "10m"   -> 600
     *     "1h"    -> 3600
     *     "1000s" -> 1000
     *     "1h10m" -> 4200
     */
    bool state_number_active = false;

    int64_t res = 0;
    int i_n_start = 0;
    int i_n_end = 0;
    for (int i = 0; i < strlen(s); ++i) {
        if (state_number_active) {
            if (!isdigit(s[i])) {
                i_n_end = i;
                assert(i_n_end - i_n_start > 0);
                char s_[16] = {0};
                memcpy(s_, s + i_n_start, i_n_end - i_n_start);
                int n = atoi(s_);

                switch (s[i]) {
                    case 'h':
                        res += n * 3600;
                        break;
                    case 'm':
                        res += n * 60;
                        break;
                    case 's':
                        res += n;
                        break;
                }
                state_number_active = false;
            }
        } else {
            if (isdigit(s[i])) {
                state_number_active = true;
                i_n_start = i;
            }
        }
    }

    return res;
}

bool readConfig(int* out) {
    if (system("mkdir -p ~/" CONFIG_DIR) != 0) {
        return false;
    }

    char* home_path = getenv("HOME");
    char path[128];
    snprintf(path, sizeof(path), "%s%s", home_path, CONFIG_FILE_LAST_SAVED);

    FILE* fin = fopen(path, "r");
    if (!fin) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s%s", "Could not open file: ", path);
        perror(msg);
        return false;
    }

    char buff[64];
    char* res = fgets(buff, sizeof(buff), fin);
    if (res == NULL) {
        printf("Could not read duration from %s\n\r", path);
        return false;
    }

    *out = atoi(buff);

    fclose(fin);
    return true;
}

bool writeConfig(int n) {
    if (system("mkdir -p ~/" CONFIG_DIR) != 0) {
        return false;
    }

    char* home_path = getenv("HOME");
    char path[128];
    snprintf(path, sizeof(path), "%s%s", home_path, CONFIG_FILE_LAST_SAVED);

    FILE* fout = fopen(path, "w+");
    if (!fout) {
        char msg[128];
        snprintf(msg, sizeof(msg), "%s%s", "Could not open file: ", path);
        perror(msg);
        return false;
    }

    fprintf(fout, "%d", n);
    fclose(fout);

    return true;
}


void readTimeArgInteractive(float* out) {
    char buff[32] = {0};
    fgets(buff, sizeof(buff), stdin);
    *out = parseTimeArg(buff);
}

bool askRestoreConfigInteractive(int timer_dur) {
    printf("Found interrupted timer with duration: %d\n\rDo you want to restore it?\n\r-> [Y/n] ", timer_dur);
    fflush(stdout);
    char c = 0;
    read(STDIN_FILENO, &c, 1);
    if (c == 'y' || c == 'Y' || c == '\n') {
        return true;
    }
    return false;
}

int main(int argc, char* argv[]) {
    static_assert(MAX_TIMER_DURATION < UINT_MAX);
    parse_cmd_args(argc, argv, &cmd_args);
    // Disable buffering
    // setvbuf(stdout, NULL, _IONBF, 0);

    static struct appState s;

    if (cmd_args.time) {
        s.timer_length = parseTimeArg(cmd_args.time);
        if (s.timer_length <= 0) {
            fprintf(stderr, ANSI_COLOR_RED "Wrong --time option.\n\r" ANSI_COLOR_RESET);
            exit_help_message();
        }
    } else {
        int configValue = 0;
        bool ok = readConfig(&configValue);
        if (ok) {
            if (askRestoreConfigInteractive(configValue)) {
                s.timer_length = configValue;
            } else {
                readTimeArgInteractive(&s.timer_length);
            }
        }
        if (s.timer_length <= 0) {
            fprintf(stderr, ANSI_COLOR_RED "You should provide time duration via --time option\n\r" ANSI_COLOR_RESET);
            exit_help_message();
        }
    }
    
    if (s.timer_length > MAX_TIMER_DURATION) {
        fprintf(stderr, ANSI_COLOR_RED "Timer duration is too big. Max duration is %d seconds\n\r" ANSI_COLOR_RESET, MAX_TIMER_DURATION);
        exit(1);
    }
    s.timer_start = uptimeSeconds();
    s.display_seconds = 0;
    s.type = TIMER_ACTIVE;
    s.draw_borders_flag = true;
    s.endedState.is_blink = false;
    s.endedState.last_blink_swap = 0;
    s.pauseState.is_blink = false;
    s.pauseState.last_blink_swap = 0;
    s.pauseState.pause_start = 0;
    s.display_completed_blocks = 0;
    s.is_increasing = cmd_args.is_increasing;

    enableRawMode();
    clearScreen();
    disableCursor();

    struct sigaction sa;
    sa.sa_handler = handle_resize;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    sigaction(SIGWINCH, &sa, NULL);


    while (1) {
        if (screen_resized) {
            screen_resized = false;
            getWindowSize(&s.width, &s.height);
        }
        if (!mainLoop(&s)) {
            if (s.display_seconds > 0) {
                writeConfig(s.display_seconds);
            }
            break;
        }
    }

    disableRawMode();

    return 0;
}
