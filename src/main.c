#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>
#include "../headers/file_editor.h"

int main(void)
{
    setlocale(LC_ALL, "");
    initscr();
    cbreak();             // Включаем режим cbreak
    noecho();             // Отключаем вывод вводимых символов
    keypad(stdscr, TRUE); // Включаем поддержку дополнительных клавиш

    char current_path[1024];
    getcwd(current_path, sizeof(current_path));

    while (1)
    {
        list_directory(current_path);
        char ch = getch(); // Ожидание ввода

        if (ch == 'q')
        {
            break; // Выход
        }
        else if (ch == 'b')
        {
            char *last_slash = strrchr(current_path, '/');
            if (last_slash != NULL && last_slash != current_path)
            {
                *last_slash = '\0'; // Возврат на уровень вверх
            }
        }
        else if (ch == 'f')
        {
            search_file(current_path);
        }
    }

    endwin(); // Завершение работы ncurses
    return 0;
}
