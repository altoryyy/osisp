#include "../headers/file_editor.h"
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ncurses.h>
#include <unistd.h>

int main(void)
{
    // Установка локали
    setlocale(LC_ALL, "");

    // Инициализация ncurses
    initscr();            // Инициализация экрана
    cbreak();             // Включение режима "cbreak"
    noecho();             // Не отображать вводимые символы
    keypad(stdscr, TRUE); // Включение обработки клавиш

    char current_path[1024];
    char next_path[1024];
    getcwd(current_path, sizeof(current_path)); // Получаем текущий каталог

    while (1)
    {
        list_directory(current_path);

        // Устанавливаем режим не блокирующего ввода
        nodelay(stdscr, TRUE);
        char ch = getch();

        // Проверяем ввод
        if (ch == 'q')
        {
            break; // Выход
        }
        else if (ch == 'b')
        {
            // Возврат на уровень вверх
            char *last_slash = strrchr(current_path, '/');
            if (last_slash != NULL && last_slash != current_path)
            {
                *last_slash = '\0'; // Убираем последний уровень
            }
        }
        else if (ch == '\n')
        {                           // Если нажата клавиша Enter, запрашиваем выбор
            nodelay(stdscr, FALSE); // Включаем блокирующий ввод
            get_user_choice(current_path, next_path);
            strcpy(current_path, next_path);
            nodelay(stdscr, TRUE); // Возвращаем не блокирующий ввод
        }

        // Возвращаем режим блокирующего ввода
        nodelay(stdscr, FALSE);
    }

    endwin(); // Завершение работы с ncurses
    return 0;
}
