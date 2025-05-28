#include <locale.h>
#include <ncurses.h>
#include <unistd.h>
#include <stdlib.h>
#include "../headers/file_editor.h"

int main(void)
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0); // Скрываем курсор

    init_colors();

    char *home = getenv("HOME");
    list_directory(home ? home : ".");

    endwin();
    return 0;
}
