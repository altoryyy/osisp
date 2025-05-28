#ifndef FILE_EDITOR_H
#define FILE_EDITOR_H

#include <ncurses.h>

void list_directory(const char *path);
void display_file_info(const char *file_path);
void edit_text_file(const char *file_path);
void search_file(const char *path);
void delete_file(const char *path);
void copy_file(const char *src, const char *dest);
void move_file(const char *src, const char *dest);
void show_help();
void show_disk_usage(const char *path);
void create_new_file(const char *path);
void create_new_directory(const char *path);
void init_colors();

#endif // FILE_EDITOR_H