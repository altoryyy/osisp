#ifndef FILE_EDITOR_H
#define FILE_EDITOR_H

// Функция для отображения содержимого указанного каталога
void list_directory(const char *path);

// Функция для получения выбора пользователя для перехода в каталог
void get_user_choice(const char *path, char *next_path);

#endif // FILE_EDITOR_H
