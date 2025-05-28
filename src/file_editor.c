#include "../headers/file_editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>

#define PAGE_SIZE 10 // Количество файлов на странице

void list_directory(const char *path)
{
    DIR *dir = opendir(path);
    struct dirent *entry;
    char *files[1024];
    char next_path[1024];
    int count = 0;

    if (!dir)
    {
        printw("Ошибка при открытии каталога: %s\n", path);
        refresh();
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] != '.')
        {
            files[count++] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    clear();
    printw("Содержимое каталога: %s\n", path);
    printw("Нажмите 'q' для выхода, 'b' для возврата на уровень вверх, 'n' для выбора файла, 'f' для поиска файла.\n");

    int page = 0;
    int total_pages = (count + PAGE_SIZE - 1) / PAGE_SIZE;

    while (1)
    {
        clear();
        printw("Содержимое каталога: %s (Страница %d из %d)\n", path, page + 1, total_pages);

        for (int j = page * PAGE_SIZE; j < (page + 1) * PAGE_SIZE && j < count; j++)
        {
            printw("%d: %s\n", j + 1, files[j]);
        }

        printw("\nНажмите '<' для предыдущей страницы, '>' для следующей страницы...");
        refresh();

        char ch = getch(); // Ожидание ввода

        if (ch == '<' && page > 0)
        {
            page--; // Переход на предыдущую страницу
        }
        else if (ch == '>' && page < total_pages - 1)
        {
            page++; // Переход на следующую страницу
        }
        else if (ch == 'n')
        {
            // Реализация выбора файла или каталога
            char input[10];
            int idx = 0;

            printw("Введите номер каталога или файла: ");
            refresh();

            while (idx < sizeof(input) - 1)
            {
                ch = getch(); // Ожидание ввода
                if (ch >= '0' && ch <= '9')
                {
                    input[idx++] = ch;
                    printw("%c", ch);
                    refresh();
                }
                else if (ch == '\n')
                {
                    break;
                }
            }
            input[idx] = '\0';

            int choice = atoi(input);
            if (choice > 0 && choice <= count)
            {
                snprintf(next_path, sizeof(next_path), "%s/%s", path, files[choice - 1]);
                struct stat file_info;
                if (stat(next_path, &file_info) == 0 && S_ISDIR(file_info.st_mode))
                {
                    // Переход на уровень вниз
                    list_directory(next_path);
                    return; // Возврат к вызову после перехода
                }
                else
                {
                    display_file_info(next_path);
                }
            }
            printw("\nНажмите любую клавишу для продолжения...");
            refresh();
            getch();
        }
        else if (ch == 'b')
        {
            // Возврат на уровень вверх
            char *last_slash = strrchr(path, '/');
            if (last_slash != NULL && last_slash != path)
            {
                *last_slash = '\0'; // Возврат
                return;             // Возвращаемся к предыдущему каталогу
            }
            else
            {
                printw("Вы уже на верхнем уровне.\n");
                refresh();
                getch();
            }
        }
        else if (ch == 'f')
        {
            search_file(path);
        }
        else if (ch == 'q')
        {
            return; // Выход из функции
        }
    }

    for (int j = 0; j < count; j++)
    {
        free(files[j]);
    }
}

void display_file_info(const char *file_path)
{
    struct stat file_info;

    if (stat(file_path, &file_info) == 0)
    {
        clear();
        printw("Информация о файле: %s\n", file_path);
        printw("Размер: %lld байт\n", (long long)file_info.st_size);
        printw("Дата последнего изменения: %s", ctime(&file_info.st_mtime));
        printw("Тип: %s\n", S_ISDIR(file_info.st_mode) ? "Каталог" : "Файл");
        printw("Нажмите любую клавишу для продолжения...");
        refresh();
        getch();
    }
}

void search_file(const char *path)
{
    char search_term[256];
    printw("Введите имя файла для поиска: ");
    refresh();
    getnstr(search_term, sizeof(search_term) - 1);

    DIR *dir = opendir(path);
    struct dirent *entry;
    int found = 0;

    clear();
    printw("Результаты поиска для '%s':\n", search_term);

    while ((entry = readdir(dir)) != NULL)
    {
        if (strstr(entry->d_name, search_term) != NULL)
        {
            printw("%s\n", entry->d_name);
            found = 1;
        }
    }
    closedir(dir);

    if (!found)
    {
        printw("Файлы не найдены.\n");
    }
    printw("Нажмите любую клавишу для продолжения...");
    refresh();
    getch();
}
