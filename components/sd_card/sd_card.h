#ifndef SD_CARD_SD_CARD_H
#define SD_CARD_SD_CARD_H

#include "ff.h"
#include "sd_card_config.h"
#include <stdio.h>

typedef struct {
    sd_card_config_t config;
    sd_card_interface_t interface;
} sd_card_t;

sd_card_err_t sd_card_initialize(sd_card_t* card,
                                 sd_card_config_t const* config,
                                 sd_card_interface_t const* interface);
sd_card_err_t sd_card_deinitialize(sd_card_t* card);

sd_card_err_t sd_card_mount(sd_card_t const* card);
sd_card_err_t sd_card_unmount(sd_card_t const* card);

sd_card_err_t sd_card_cat(sd_card_t const* card,
                          sd_card_path_t path,
                          sd_card_buffer_t* output);

sd_card_err_t sd_card_touch(sd_card_t const* card, sd_card_path_t path);
sd_card_err_t sd_card_unlink(sd_card_t const* card, sd_card_path_t path);

sd_card_err_t sd_card_ls(sd_card_t const* card,
                         sd_card_path_t path,
                         sd_card_buffer_t* output);

sd_card_err_t sd_card_mkdir(sd_card_t const* card, sd_card_path_t path);
sd_card_err_t sd_card_rmdir(sd_card_t const* card, sd_card_path_t path);

sd_card_err_t sd_card_rename(sd_card_t const* card,
                             sd_card_path_t old_path,
                             sd_card_path_t new_path);

sd_card_err_t sd_card_stat(sd_card_t const* card,
                           sd_card_path_t path,
                           sd_card_stat_t* stat);

sd_card_err_t sd_card_read(sd_card_t const* card,
                           sd_card_path_t path,
                           sd_card_buffer_t* output);
sd_card_err_t sd_card_write(sd_card_t const* card,
                            sd_card_path_t path,
                            sd_card_buffer_t const* input);

#endif // SD_CARD_SD_CARD_H