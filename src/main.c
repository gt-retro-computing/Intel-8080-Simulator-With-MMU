#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pthread.h>
#include <string.h>
#include "8080emu.h"
#include "unistd.h"

void finish() {
    endwin();
    exit(0);
}

static WINDOW* regDisplayWindow;

#define COLOR_PAIR_NORMAL 1
#define COLOR_PAIR_ACTIVE_FLAG 2

void initScreen() {
    initscr();
    keypad(stdscr, TRUE);
    raw();
    cbreak();
    noecho();
    curs_set(0);
    timeout(20);

    regDisplayWindow = newwin(10, 25, 0, 0);
    refresh();

    if (has_colors()) {
        start_color();
        init_pair(COLOR_PAIR_NORMAL, COLOR_WHITE, COLOR_BLACK);
        init_pair(COLOR_PAIR_ACTIVE_FLAG, COLOR_WHITE, COLOR_RED);
    }
}

void updateRegisters(WINDOW *regDisplayWin, State8080 *instance);

void refreshScreen(State8080 *instance) {
    updateRegisters(regDisplayWindow, instance);
}

void updateRegisters(WINDOW *regDisplayWin, State8080 *instance) {
    const char *title = "Registers";
    int w = getmaxx(regDisplayWin), h = getmaxy(regDisplayWin);
    box(regDisplayWin, 0, 0);
    mvwprintw(regDisplayWin, 0, (int) (w - strlen(title)) / 2, title);
    mvwprintw(regDisplayWin, 1, 1, "A  : %02X", instance->a);
    mvwprintw(regDisplayWin, 2, 1, "B|C: %02X|%02X", instance->b, instance->c);
    mvwprintw(regDisplayWin, 3, 1, "D|E: %02X|%02X", instance->d, instance->e);
    mvwprintw(regDisplayWin, 4, 1, "H|L: %02X|%02X", instance->h, instance->l);
    mvwprintw(regDisplayWin, 5, 1, "SP : %04X", instance->sp);
    mvwhline(regDisplayWin, 6, 1, 0, w - 2);
    mvwprintw(regDisplayWin, 7, 1, "PC : %04X", instance->pc);
    mvwprintw(regDisplayWin, 8, 1, "FLAGS ");

    // Neg Flag
    if (instance->cc.s) {
        wattron(regDisplayWin, COLOR_PAIR(2));
    }
    mvwaddch(regDisplayWin, 8, 7, 'S');
    wattroff(regDisplayWin, COLOR_PAIR(2));

    // Zero Flag
    if (instance->cc.z) {
        wattron(regDisplayWin, COLOR_PAIR(2));
    }
    mvwaddch(regDisplayWin, 8, 9, 'Z');
    wattroff(regDisplayWin, COLOR_PAIR(2));


    // Aux Carry Flag
    if (instance->cc.ac) {
        wattron(regDisplayWin, COLOR_PAIR(2));
    }
    mvwaddch(regDisplayWin, 8, 11, 'A');
    wattroff(regDisplayWin, COLOR_PAIR(2));

    // Parity
    if (instance->cc.p) {
        wattron(regDisplayWin, COLOR_PAIR(2));
    }
    mvwaddch(regDisplayWin, 8, 13, 'P');
    wattroff(regDisplayWin, COLOR_PAIR(2));


    // Carry
    if (instance->cc.cy) {
        wattron(regDisplayWin, COLOR_PAIR(2));
    }
    mvwaddch(regDisplayWin, 8, 15, 'C');
    wattroff(regDisplayWin, COLOR_PAIR(2));
    wrefresh(regDisplayWin);
}

State8080* init8080(void) {
    State8080* state = calloc(1, sizeof(State8080));
    state->memory = malloc(sizeof(uint8_t) * 0x10000);  //64K
    return state;
}

void* simloop(void* arg) {
    State8080* state = (State8080*) arg;
    for(;;){
        if (!state->halt) {
            emulate8080(state);
        } else {
            usleep(10000);
        }
    }
    return NULL;
}

int main() {
    initScreen();
    State8080* shared = init8080();
    shared->halt = true;

    // Load a loop
    shared -> memory[0] = 0b11000011;
    shared -> memory[1] = 0b00000000;
    shared -> memory[2] = 0b00000000;

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, simloop, shared);

    int lastPress = 0;
    for (;;) {
        int c = getch();
        if (c != lastPress) {
            lastPress = c;

            switch (c) {
            case 's':
                emulate8080(shared);
                break;
            case 'R':
                reset8080(shared);
                break;
            case 'r':
                shared->halt = !shared->halt;
                break;
            case 'q':
                finish();
            }
        }
        refreshScreen(shared);
    }
    return 0;
}
