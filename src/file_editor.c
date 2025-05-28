#include "../headers/file_editor.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ncurses.h>
#include <unistd.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <sys/statvfs.h>
#include <libgen.h>
#include <ctype.h>

#define PAGE_SIZE 15
#define MAX_HISTORY 20
#define MAX_PATH_LENGTH 1024

char *history[MAX_HISTORY];
int history_count = 0;
int history_pos = -1;

void add_to_history(const char *path)
{
    if (history_count > 0 && strcmp(history[history_count - 1], path) == 0)
    {
        return;
    }

    if (history_count < MAX_HISTORY)
    {
        history[history_count++] = strdup(path);
    }
    else
    {
        free(history[0]);
        for (int i = 0; i < MAX_HISTORY - 1; i++)
        {
            history[i] = history[i + 1];
        }
        history[MAX_HISTORY - 1] = strdup(path);
    }
    history_pos = history_count - 1;
}

void init_colors()
{
    start_color();
    init_pair(1, COLOR_GREEN, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_YELLOW, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(6, COLOR_BLUE, COLOR_BLACK);
}

void show_help()
{
    clear();
    attron(COLOR_PAIR(5));
    printw("=== Помощь ===\n");
    attroff(COLOR_PAIR(5));

    printw("Навигация:\n");
    printw("  Стрелки вверх/вниз - Прокрутка списка\n");
    printw("  Enter - Открыть файл/каталог\n");
    printw("  b - На уровень вверх\n");
    printw("  h - Домой (~)\n");
    printw("  < / > - Предыдущая/следующая страница\n");
    printw("  q - Выход\n\n");

    printw("Операции с файлами:\n");
    printw("  e - Редактировать файл\n");
    printw("  d - Удалить файл/каталог\n");
    printw("  c - Копировать файл\n");
    printw("  m - Переместить файл\n");
    printw("  n - Создать файл\n");
    printw("  k - Создать каталог\n\n");

    printw("Другое:\n");
    printw("  f - Поиск файлов\n");
    printw("  u - Информация о диске\n");
    printw("  ? - Эта справка\n");

    printw("\nНажмите любую клавишу для продолжения...");
    refresh();
    getch();
}

void show_disk_usage(const char *path)
{
    struct statvfs vfs;
    if (statvfs(path, &vfs) == 0)
    {
        clear();
        attron(COLOR_PAIR(5));
        printw("=== Информация о диске ===\n");
        attroff(COLOR_PAIR(5));

        unsigned long long total = vfs.f_blocks * vfs.f_frsize;
        unsigned long long free = vfs.f_bfree * vfs.f_frsize;
        unsigned long long used = total - free;
        double percent_used = (double)used / (double)total * 100.0;

        printw("Файловая система: %s\n", path);
        printw("Общий объем: %.2f GB\n", total / (1024.0 * 1024 * 1024));
        printw("Использовано: %.2f GB (%.1f%%)\n",
               used / (1024.0 * 1024 * 1024), percent_used);
        printw("Свободно: %.2f GB\n", free / (1024.0 * 1024 * 1024));

        printw("\nНажмите любую клавишу для продолжения...");
        refresh();
        getch();
    }
    else
    {
        printw("Ошибка получения информации о диске\n");
        refresh();
        getch();
    }
}

char *get_file_type(mode_t mode)
{
    if (S_ISREG(mode))
        return "Файл";
    if (S_ISDIR(mode))
        return "Каталог";
    if (S_ISCHR(mode))
        return "Символьное устройство";
    if (S_ISBLK(mode))
        return "Блочное устройство";
    if (S_ISFIFO(mode))
        return "FIFO/канал";
    if (S_ISLNK(mode))
        return "Символическая ссылка";
    if (S_ISSOCK(mode))
        return "Сокет";
    return "Неизвестно";
}

void display_file_info(const char *file_path)
{
    struct stat file_info;
    struct passwd *pw;
    struct group *gr;

    if (stat(file_path, &file_info) != 0)
    {
        printw("Ошибка получения информации о файле\n");
        refresh();
        getch();
        return;
    }

    clear();
    attron(COLOR_PAIR(5));
    printw("=== Информация о файле ===\n");
    attroff(COLOR_PAIR(5));

    printw("Имя: %s\n", file_path);
    printw("Тип: %s\n", get_file_type(file_info.st_mode));
    printw("Размер: %lld байт\n", (long long)file_info.st_size);
    printw("Права: %o\n", file_info.st_mode & 0777);

    pw = getpwuid(file_info.st_uid);
    gr = getgrgid(file_info.st_gid);
    printw("Владелец: %s (%d)\n", pw ? pw->pw_name : "?", file_info.st_uid);
    printw("Группа: %s (%d)\n", gr ? gr->gr_name : "?", file_info.st_gid);

    printw("Дата создания: %s", ctime(&file_info.st_ctime));
    printw("Дата изменения: %s", ctime(&file_info.st_mtime));
    printw("Дата доступа: %s", ctime(&file_info.st_atime));

    printw("\nДействия:\n");
    printw("e - Редактировать  d - Удалить  c - Копировать  m - Переместить\n");
    printw("\nНажмите любую клавишу для продолжения...");
    refresh();

    int ch = getch();
    switch (ch)
    {
    case 'e':
        edit_text_file(file_path);
        break;
    case 'd':
        delete_file(file_path);
        break;
    case 'c':
    {
        char dest[1024];
        printw("\nВведите путь назначения: ");
        echo();
        getnstr(dest, sizeof(dest) - 1);
        noecho();
        copy_file(file_path, dest);
        break;
    }
    case 'm':
    {
        char dest[1024];
        printw("\nВведите путь назначения: ");
        echo();
        getnstr(dest, sizeof(dest) - 1);
        noecho();
        move_file(file_path, dest);
        break;
    }
    }
}

void edit_text_file(const char *file_path)
{
    const char *ext = strrchr(file_path, '.');
    if (ext && (strcmp(ext, ".exe") == 0 || strcmp(ext, ".bin") == 0 ||
                strcmp(ext, ".so") == 0 || strcmp(ext, ".dll") == 0))
    {
        printw("Бинарные файлы нельзя редактировать!\n");
        refresh();
        getch();
        return;
    }

    FILE *file = fopen(file_path, "r");
    if (!file)
    {
        printw("Не удалось открыть файл для чтения\n");
        refresh();
        getch();
        return;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc(file_size + 1);
    size_t bytes_read = fread(content, 1, file_size, file);
    content[bytes_read] = '\0';
    fclose(file);

    clear();
    attron(COLOR_PAIR(5));
    printw("=== Редактор: %s ===\n", file_path);
    printw("F2 - Сохранить  ESC - Выход без сохранения\n");
    attroff(COLOR_PAIR(5));
    refresh();

    WINDOW *edit_win = newwin(LINES - 4, COLS - 4, 2, 2);
    keypad(edit_win, TRUE);
    scrollok(edit_win, TRUE);
    wmove(edit_win, 0, 0);
    wprintw(edit_win, "%s", content);
    wrefresh(edit_win);

    int ch;
    int cursor_x = 0, cursor_y = 0;
    bool modified = false;

    while ((ch = wgetch(edit_win)) != KEY_F(2) && ch != 27)
    {
        switch (ch)
        {
        case KEY_UP:
            if (cursor_y > 0)
                cursor_y--;
            break;
        case KEY_DOWN:
            cursor_y++;
            break;
        case KEY_LEFT:
            if (cursor_x > 0)
                cursor_x--;
            break;
        case KEY_RIGHT:
            cursor_x++;
            break;
        case KEY_BACKSPACE:
        case 127:
            if (cursor_x > 0)
            {
                for (int i = cursor_x - 1; i < strlen(content); i++)
                {
                    content[i] = content[i + 1];
                }
                cursor_x--;
                modified = true;
            }
            break;
        case '\n':
            cursor_x = 0;
            cursor_y++;
            break;
        default:
            if (isprint(ch))
            {
                memmove(content + cursor_x + 1, content + cursor_x, strlen(content) - cursor_x + 1);
                content[cursor_x] = ch;
                cursor_x++;
                modified = true;
            }
            break;
        }

        werase(edit_win);
        wprintw(edit_win, "%s", content);
        wmove(edit_win, cursor_y, cursor_x);
        wrefresh(edit_win);
    }

    if (ch == KEY_F(2))
    {
        file = fopen(file_path, "w");
        if (file)
        {
            fputs(content, file);
            fclose(file);
            printw("\nФайл сохранен\n");
        }
        else
        {
            printw("\nОшибка сохранения файла\n");
        }
        refresh();
        getch();
    }

    free(content);
    delwin(edit_win);
}

void delete_file(const char *path)
{
    clear();
    printw("Вы уверены, что хотите удалить '%s'? (y/n): ", path);
    refresh();

    int ch = getch();
    if (ch == 'y' || ch == 'Y')
    {
        if (remove(path) == 0)
        {
            printw("\nУдалено успешно\n");
        }
        else
        {
            printw("\nОшибка удаления\n");
        }
        refresh();
        getch();
    }
}

void copy_file(const char *src, const char *dest)
{
    struct stat dest_stat;
    char final_dest[2048];

    if (stat(dest, &dest_stat) == 0 && S_ISDIR(dest_stat.st_mode))
    {
        const char *filename = strrchr(src, '/');
        filename = filename ? filename + 1 : src;
        snprintf(final_dest, sizeof(final_dest), "%s/%s", dest, filename);
    }
    else
    {
        strncpy(final_dest, dest, sizeof(final_dest));
    }

    FILE *src_file = fopen(src, "rb");
    if (!src_file)
    {
        printw("\nОшибка открытия исходного файла: %s\n", src);
        refresh();
        getch();
        return;
    }

    FILE *dest_file = fopen(final_dest, "wb");
    if (!dest_file)
    {
        fclose(src_file);
        printw("\nОшибка создания файла назначения: %s\n", final_dest);
        refresh();
        getch();
        return;
    }

    char buffer[4096];
    size_t bytes;
    while ((bytes = fread(buffer, 1, sizeof(buffer), src_file)) > 0)
    {
        if (fwrite(buffer, 1, bytes, dest_file) != bytes)
        {
            printw("\nОшибка записи в файл назначения\n");
            fclose(src_file);
            fclose(dest_file);
            remove(final_dest);
            refresh();
            getch();
            return;
        }
    }

    fclose(src_file);
    fclose(dest_file);

    struct stat src_stat;
    stat(src, &src_stat);
    chmod(final_dest, src_stat.st_mode);

    printw("\nФайл успешно скопирован в: %s\n", final_dest);
    refresh();
    getch();
}

void move_file(const char *src, const char *dest)
{
    if (rename(src, dest) == 0)
    {
        printw("\nФайл перемещен успешно\n");
        refresh();
        getch();
        return;
    }

    copy_file(src, dest);
    if (remove(src) == 0)
    {
        printw("\nФайл перемещен успешно\n");
    }
    else
    {
        printw("\nФайл скопирован, но исходный не удален!\n");
    }
    refresh();
    getch();
}

void create_new_file(const char *path)
{
    char filename[1024];
    printw("Введите имя нового файла: ");
    echo();
    getnstr(filename, sizeof(filename) - 1);
    noecho();

    char fullpath[2048];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, filename);

    FILE *file = fopen(fullpath, "w");
    if (file)
    {
        fclose(file);
        printw("Файл создан: %s\n", fullpath);
    }
    else
    {
        printw("Ошибка создания файла\n");
    }
    refresh();
    getch();
}

void create_new_directory(const char *path)
{
    char dirname[1024];
    printw("Введите имя нового каталога: ");
    echo();
    getnstr(dirname, sizeof(dirname) - 1);
    noecho();

    char fullpath[2048];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, dirname);

    if (mkdir(fullpath, 0755) == 0)
    {
        printw("Каталог создан: %s\n", fullpath);
    }
    else
    {
        printw("Ошибка создания каталога\n");
    }
    refresh();
    getch();
}

void search_file(const char *path)
{
    char search_term[256];
    printw("Введите имя файла для поиска (можно использовать * и ?): ");
    echo();
    getnstr(search_term, sizeof(search_term) - 1);
    noecho();

    char command[1024];
    snprintf(command, sizeof(command), "find \"%s\" -name \"%s\" 2>/dev/null", path, search_term);

    clear();
    printw("Результаты поиска для '%s':\n", search_term);
    printw("--------------------------------\n");

    FILE *pipe = popen(command, "r");
    if (pipe)
    {
        char result[1024];
        int count = 0;
        while (fgets(result, sizeof(result), pipe))
        {
            result[strcspn(result, "\n")] = '\0';
            printw("%s\n", result);
            count++;
        }
        pclose(pipe);
        printw("\nНайдено файлов: %d\n", count);
    }
    else
    {
        printw("Ошибка выполнения поиска\n");
    }

    printw("\nНажмите любую клавишу для продолжения...");
    refresh();
    getch();
}

void list_directory(const char *path)
{
    add_to_history(path);

    DIR *dir = opendir(path);
    if (!dir)
    {
        printw("Ошибка при открытии каталога: %s\n", path);
        refresh();
        getch();
        return;
    }

    struct dirent *entry;
    char *files[1024];
    int count = 0;

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            files[count++] = strdup(entry->d_name);
        }
    }
    closedir(dir);

    int page = 0;
    int total_pages = (count + PAGE_SIZE - 1) / PAGE_SIZE;
    int selected = 0;

    while (1)
    {
        clear();
        attron(COLOR_PAIR(5));
        printw("=== %s === (Страница %d/%d)\n", path, page + 1, total_pages);
        attroff(COLOR_PAIR(5));

        printw("n-Создать файл  e-Редактировать  d-Удалить  c-Копировать  m-Перемещение  f-Поиск  ");
        printw("u-Диск  ?-Помощь  q-Выход\n\n");

        int start = page * PAGE_SIZE;
        int end = (page + 1) * PAGE_SIZE;
        if (end > count)
            end = count;

        for (int i = start; i < end; i++)
        {
            char fullpath[2048];
            snprintf(fullpath, sizeof(fullpath), "%s/%s", path, files[i]);

            struct stat file_info;
            stat(fullpath, &file_info);

            if (i == start + selected)
            {
                attron(A_REVERSE);
            }

            if (S_ISDIR(file_info.st_mode))
            {
                attron(COLOR_PAIR(1));
            }
            else if (file_info.st_mode & S_IXUSR)
            {
                attron(COLOR_PAIR(4));
            }
            else if (S_ISLNK(file_info.st_mode))
            {
                attron(COLOR_PAIR(3));
            }
            else
            {
                attron(COLOR_PAIR(2));
            }

            char *name = basename(files[i]);
            int name_width = 30;
            if (strlen(name) > name_width)
            {
                name[name_width] = '\0';
            }
            printw("\n%-*s", name_width, name);
            attroff(COLOR_PAIR(1) | COLOR_PAIR(2) | COLOR_PAIR(3) | COLOR_PAIR(4) | A_REVERSE);

            char date_str[20];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d %H:%M", localtime(&file_info.st_mtime));
            printw("  %10lld bytes  %s", (long long)file_info.st_size, date_str);
        }

        printw("\nСтрелки: Навигация  Enter: Выбрать  < >: Страницы  b: Назад  h: Домой\n");
        refresh();

        int ch = getch();
        switch (ch)
        {
        case KEY_UP:
            if (selected > 0)
                selected--;
            break;
        case KEY_DOWN:
            if (selected < end - start - 1)
                selected++;
            break;
        case KEY_LEFT:
            if (page > 0)
            {
                page--;
                selected = 0;
            }
            break;
        case KEY_RIGHT:
            if (page < total_pages - 1)
            {
                page++;
                selected = 0;
            }
            break;
        case '\n':
        {
            int idx = page * PAGE_SIZE + selected;
            char next_path[2048];
            snprintf(next_path, sizeof(next_path), "%s/%s", path, files[idx]);

            struct stat file_info;
            stat(next_path, &file_info);

            if (S_ISDIR(file_info.st_mode))
            {
                list_directory(next_path);
                return;
            }
            else
            {
                display_file_info(next_path);
            }
            break;
        }
        case 'b':
        {
            char parent[2048];
            strncpy(parent, path, sizeof(parent));
            char *last_slash = strrchr(parent, '/');
            if (last_slash && last_slash != parent)
            {
                *last_slash = '\0';
                list_directory(parent);
                return;
            }
            break;
        }
        case 'h':
            list_directory(getenv("HOME"));
            return;
        case 'f':
            search_file(path);
            break;
        case 'u':
            show_disk_usage(path);
            break;
        case '?':
            show_help();
            break;
        case 'n':
        {
            create_new_file(path);
            return;
        }
        case 'k':
        {
            create_new_directory(path);
            return;
        }
        case 'q':
            return;
        default:
            break;
        }
    }

    for (int i = 0; i < count; i++)
    {
        free(files[i]);
    }
}
