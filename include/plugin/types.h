#pragma once

typedef struct{
    char** exts;
    int ext_count;
    char* id;
    bool (*classify_func)(const char* file);
} fa_file_t;

typedef struct{
    char* id;
    char* version;
    
} fa_plugin;