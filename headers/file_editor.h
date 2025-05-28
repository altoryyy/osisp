#ifndef FILE_EDITOR_H
#define FILE_EDITOR_H

// Функция для отображения содержимого указанного каталога
void list_directory(const char *path);

// Функция для получения выбора пользователя для перехода в каталог
int get_user_choice(const char *path, char *next_path);

// Функция для отображения информации о файле
void display_file_info(const char *file_path);

// Функция для редактирования текстового файла
void edit_text_file(const char *file_path);

// Функция для поиска файла
void search_file(const char *path);

#endif // FILE_EDITOR_H
