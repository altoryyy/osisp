#include "../headers/file_editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <ncurses.h>
#include <unistd.h>

void list_directory(const char *path)
{
    DIR *dir = opendir(path);
    struct dirent *entry;

    if (!dir)
    {
        printw("Ошибка при открытии каталога: %s\n", path);
        refresh();
        return;
    }

    clear();
    printw("Содержимое каталога: %s\n", path);
    printw("Нажмите 'q' для выхода, 'b' для возврата на уровень вверх.\n");
    printw("Выберите каталог для перехода вниз (нажмите номер):\n");

    int index = 1; // Индекс для отображения
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_DIR && entry->d_name[0] != '.')
        { // Только директории
            printw("%d: %s\n", index++, entry->d_name);
        }
    }
    closedir(dir);
    refresh();
}

void get_user_choice(const char *path, char *next_path)
{
    char input[10];
    int idx = 0;
    char ch;

    printw("Введите номер каталога: ");
    refresh();

    while (idx < sizeof(input) - 1)
    {
        ch = getch();
        if (ch >= '0' && ch <= '9')
        {
            input[idx++] = ch; // Добавляем цифру в строку
            printw("%c", ch);  // Отображаем введенный символ
            refresh();
        }
        else if (ch == '\n')
        {
            break; // Завершение ввода
        }
    }
    input[idx] = '\0'; // Завершаем строку

    int choice = atoi(input); // Преобразуем ввод в число

    if (choice > 0)
    {
        DIR *dir = opendir(path);
        struct dirent *entry;
        int index = 1;

        rewinddir(dir); // Сброс указателя на начало каталога
        while ((entry = readdir(dir)) != NULL)
        {
            if (entry->d_type == DT_DIR && entry->d_name[0] != '.')
            {
                if (index++ == choice)
                {
                    snprintf(next_path, 1024, "%s/%s", path, entry->d_name);
                    closedir(dir);
                    return;
                }
            }
        }
        closedir(dir);
    }
    strcpy(next_path, path); // Если выбор некорректен, остаемся в текущем каталоге
}
