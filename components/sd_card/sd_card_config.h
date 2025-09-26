#ifndef SD_CARD_SD_CARD_CONFIG_H
#define SD_CARD_SD_CARD_CONFIG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

#define SD_CARD_MOUNT_POINT_MAXLEN (20U)
#define SD_CARD_ERR_OK (FR_OK)

typedef FRESULT sd_card_err_t;

typedef struct {
    TCHAR* buffer;
    UINT buffer_len;
} sd_card_buffer_t;

typedef struct {
    FILINFO info;
} sd_card_stat_t;

typedef TCHAR const* sd_card_path_t;

typedef struct {
    TCHAR mount_point[SD_CARD_MOUNT_POINT_MAXLEN];
} sd_card_config_t;

typedef struct {
    FATFS* file_system;

    void* (*allocate)(UINT);
    void (*deallocate)(void*);
} sd_card_interface_t;

char* sd_card_err_to_string(sd_card_err_t err);

#undef SD_CARD_MOUNT_POINT_MAXLEN

#endif // SD_CARD_SD_CARD_CONFIG_H