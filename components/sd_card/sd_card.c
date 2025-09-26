#include "ff.h"
#include "sd_card.h"
#include "sd_card_config.h"
#include <stdlib.h>
#include <string.h>

static inline void* sd_card_allocate(sd_card_t const* card, UINT size)
{
    if (card->interface.allocate == NULL) {
        return NULL;
    }

    return card->interface.allocate(size);
}

static inline void sd_card_deallocate(sd_card_t const* card, void* buffer)
{
    if (card->interface.deallocate == NULL) {
        return;
    }

    return card->interface.deallocate(buffer);
}

static inline void* sd_card_reallocate(sd_card_t const* card,
                                       void* old_buffer,
                                       UINT old_size,
                                       UINT new_size)
{
    void* new_buffer = sd_card_allocate(card, new_size);
    if (new_buffer == NULL) {
        return NULL;
    }

    memcpy(new_buffer, old_buffer, new_size < old_size ? new_size : old_size);
    sd_card_deallocate(card, old_buffer);

    return new_buffer;
}

char* sd_card_err_to_string(sd_card_err_t err)
{
    switch (err) {
        case FR_OK:
            return "SD_CARD_ERR_OK";
        case FR_DISK_ERR:
            return "SD_CARD_ERR_DISK";
        case FR_INT_ERR:
            return "SD_CARD_ERR_INT";
        case FR_NOT_READY:
            return "SD_CARD_ERR_NOT_READY";
        case FR_NO_FILE:
            return "SD_CARD_ERR_NO_FILE";
        case FR_NO_PATH:
            return "SD_CARD_ERR_NO_PATH";
        case FR_INVALID_NAME:
            return "SD_CARD_ERR_INVALID_NAME";
        case FR_DENIED:
            return "SD_CARD_ERR_DENIED";
        case FR_EXIST:
            return "SD_CARD_ERR_EXIST";
        case FR_INVALID_OBJECT:
            return "SD_CARD_ERR_INVALID_OBJECT";
        case FR_WRITE_PROTECTED:
            return "SD_CARD_ERR_WRITE_PROTECTED";
        case FR_INVALID_DRIVE:
            return "SD_CARD_ERR_INVALID_DRIVE";
        case FR_NOT_ENABLED:
            return "SD_CARD_ERR_NOT_ENABLED";
        case FR_NO_FILESYSTEM:
            return "SD_CARD_ERR_NO_FILESYSTEM";
        case FR_MKFS_ABORTED:
            return "SD_CARD_ERR_MKFS_ABORTED";
        case FR_TIMEOUT:
            return "SD_CARD_ERR_TIMEOUT";
        case FR_LOCKED:
            return "SD_CARD_ERR_LOCKED";
        case FR_NOT_ENOUGH_CORE:
            return "SD_CARD_ERR_NOT_ENOUGH_CORE";
        case FR_TOO_MANY_OPEN_FILES:
            return "SD_CARD_ERR_TOO_MANY_OPEN_FILES";
        case FR_INVALID_PARAMETER:
            return "SD_CARD_ERR_INVALID_PARAMETER";
    }
}

sd_card_err_t sd_card_initialize(sd_card_t* card,
                                 sd_card_config_t const* config,
                                 sd_card_interface_t const* interface)
{
    if (card == NULL || config == NULL || interface == NULL) {
        return FR_INVALID_PARAMETER;
    }

    memset(card, 0, sizeof(*card));
    memcpy(&card->config, config, sizeof(*config));
    memcpy(&card->interface, interface, sizeof(*interface));

    return FR_OK;
}

sd_card_err_t sd_card_deinitialize(sd_card_t* card)
{
    if (card == NULL) {
        return FR_INVALID_PARAMETER;
    }

    memset(card, 0, sizeof(*card));

    return FR_OK;
}

sd_card_err_t sd_card_mount(sd_card_t const* card)
{
    if (card == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FRESULT result =
        f_mount(card->interface.file_system, card->config.mount_point, 1);
    if (result != FR_OK) {
        return result;
    }

    return FR_OK;
}

sd_card_err_t sd_card_unmount(sd_card_t const* card)
{
    if (card == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FRESULT result = f_mount(NULL, card->config.mount_point, 1);
    if (result != FR_OK) {
        return result;
    }

    return FR_OK;
}

sd_card_err_t sd_card_cat(sd_card_t const* card,
                          sd_card_path_t path,
                          sd_card_buffer_t* output)
{
    if (card == NULL || path == NULL || output == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FIL file;
    FRESULT result = f_open(&file, path, FA_READ | FA_OPEN_EXISTING);
    if (result != FR_OK) {
        return result;
    }

    UINT buffer_len = 1024UL;
    char* buffer = sd_card_allocate(card, buffer_len);
    if (buffer == NULL) {
        f_close(&file);

        return result;
    }

    UINT used_len = 0UL;
    UINT bytes_read = 0U;

    while (1) {
        if (used_len >= buffer_len) {
            UINT new_buffer_len = buffer_len;
            while (used_len >= new_buffer_len) {
                new_buffer_len *= 2UL;
            }

            char* new_buffer =
                sd_card_reallocate(card, buffer, buffer_len, new_buffer_len);
            if (new_buffer == NULL) {
                f_close(&file);
                sd_card_deallocate(card, buffer);

                return result;
            }

            buffer = new_buffer;
            buffer_len = new_buffer_len;
        }

        result = f_read(&file,
                        buffer + used_len,
                        buffer_len - used_len,
                        &bytes_read);
        if (result != FR_OK) {
            f_close(&file);
            sd_card_deallocate(card, buffer);

            return result;
        }

        if (bytes_read == 0U) {
            break;
        }

        used_len += bytes_read;
    }

    buffer[used_len] = '\0';

    f_close(&file);

    output->buffer = buffer;
    output->buffer_len = buffer_len;

    return FR_OK;
}

sd_card_err_t sd_card_touch(sd_card_t const* card, sd_card_path_t path)
{
    if (card == NULL || path == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FIL file;
    FRESULT result = f_open(&file, path, FA_OPEN_ALWAYS | FA_WRITE);
    if (result != FR_OK) {
        return result;
    }

    f_close(&file);

    return FR_OK;
}

sd_card_err_t sd_card_ls(sd_card_t const* card,
                         sd_card_path_t path,
                         sd_card_buffer_t* output)
{
    if (card == NULL || path == NULL || output == NULL) {
        return FR_INVALID_PARAMETER;
    }

    DIR dir;
    FRESULT result = f_opendir(&dir, path);
    if (result != FR_OK) {
        return result;
    }

    UINT buffer_len = 1024UL;
    char* buffer = sd_card_allocate(card, buffer_len);
    if (buffer == NULL) {
        f_closedir(&dir);
        return result;
    }

    UINT used_len = 0UL;
    while (1) {
        if (used_len >= buffer_len) {
            UINT new_buffer_len = buffer_len;
            while (used_len >= new_buffer_len) {
                new_buffer_len *= 2;
            }

            char* new_buffer =
                sd_card_reallocate(card, buffer, buffer_len, new_buffer_len);
            if (new_buffer == NULL) {
                sd_card_deallocate(card, buffer);
                f_closedir(&dir);

                return result;
            }

            buffer = new_buffer;
            buffer_len = new_buffer_len;
        }

        FILINFO entry;
        result = f_readdir(&dir, &entry);
        if (result != FR_OK) {
            sd_card_deallocate(card, buffer);
            f_closedir(&dir);

            return result;
        }

        if (entry.fname[0] == '\0') {
            break;
        }

        snprintf(buffer + used_len, buffer_len - used_len, "%s\n", entry.fname);
        used_len += strlen(entry.fname) + 2UL;
    }

    f_closedir(&dir);

    output->buffer = buffer;
    output->buffer_len = used_len + 1;

    return FR_OK;
}

sd_card_err_t sd_card_mkdir(sd_card_t const* card, sd_card_path_t path)
{
    if (card == NULL || path == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FRESULT result = f_mkdir(path);
    if (result != FR_OK) {
        return result;
    }

    return FR_OK;
}

sd_card_err_t sd_card_rmdir(sd_card_t const* card, sd_card_path_t path)
{
    if (card == NULL || path == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FRESULT result = f_rmdir(path);
    if (result != FR_OK) {
        return result;
    }

    return FR_OK;
}

sd_card_err_t sd_card_rename(sd_card_t const* card,
                             sd_card_path_t old_path,
                             sd_card_path_t new_path)
{
    if (card == NULL || old_path == NULL || new_path == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FRESULT result = f_rename(old_path, new_path);
    if (result != FR_OK) {
        return result;
    }

    return FR_OK;
}

sd_card_err_t sd_card_unlink(sd_card_t const* card, sd_card_path_t path)
{
    if (card == NULL || path == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FRESULT result = f_unlink(path);
    if (result != FR_OK) {
        return result;
    }

    return FR_OK;
}

sd_card_err_t sd_card_stat(sd_card_t const* card,
                           sd_card_path_t path,
                           sd_card_stat_t* stat)
{
    if (card == NULL || path == NULL || stat == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FILINFO info;
    FRESULT result = f_stat(path, &info);
    if (result != FR_OK) {
        return result;
    }

    memcpy(&stat->info, &info, sizeof(stat->info));

    return FR_OK;
}

sd_card_err_t sd_card_read(sd_card_t const* card,
                           sd_card_path_t path,
                           sd_card_buffer_t* output)
{
    if (card == NULL || path == NULL || output == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FIL file;
    FRESULT result = f_open(&file, path, FA_READ | FA_OPEN_EXISTING);
    if (result != FR_OK) {
        return result;
    }

    UINT requested = output->buffer_len;
    char* buffer = sd_card_allocate(card, requested);
    if (buffer == NULL) {
        f_close(&file);
        return result;
    }

    UINT bytes_read = 0U;
    result = f_read(&file, buffer, requested, &bytes_read);
    if (result != FR_OK) {
        f_close(&file);
        sd_card_deallocate(card, buffer);
        return result;
    }

    buffer[bytes_read] = '\0';

    f_close(&file);

    output->buffer = buffer;
    output->buffer_len = bytes_read + 1UL;

    return FR_OK;
}

sd_card_err_t sd_card_write(sd_card_t const* card,
                            sd_card_path_t path,
                            sd_card_buffer_t const* input)
{
    if (card == NULL || path == NULL || input == NULL) {
        return FR_INVALID_PARAMETER;
    }

    FIL file;
    FRESULT result = f_open(&file, path, FA_WRITE | FA_CREATE_ALWAYS);
    if (result != FR_OK) {
        return result;
    }

    char* buffer = input->buffer;
    UINT buffer_len = input->buffer_len;

    UINT bytes_written = 0U;
    result = f_write(&file, buffer, buffer_len, &bytes_written);
    if (result != FR_OK || bytes_written != buffer_len) {
        f_close(&file);
        return result;
    }

    f_close(&file);

    return FR_OK;
}
