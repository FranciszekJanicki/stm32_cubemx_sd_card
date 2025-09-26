#include "main.h"
#include "fatfs.h"
#include "gpio.h"
#include "sd_card.h"
#include "spi.h"
#include "usart.h"

void SystemClock_Config(void);

int main(void)
{
    HAL_Init();
    SystemClock_Config();

    MX_GPIO_Init();
    MX_USART2_UART_Init();
    MX_SPI3_Init();
    MX_FATFS_Init();

    sd_card_config_t card_config = {};
    strncpy(card_config.mount_point,
            config->mount_point,
            sizeof(card_config.mount_point));
    sd_card_interface_t card_interface = {.file_system = config->filesystem,
                                          .allocate = pvPortMalloc,
                                          .deallocate = vPortFree};
    sd_card_err_t err =
        sd_card_initialize(&manager->card, &card_config, &card_interface);
    if (err != SD_CARD_ERR_OK) {
        return -1;
    }

    err = sd_card_mount(&manager->card);
    if (err != SD_CARD_ERR_OK) {
        printf("Error mounting: %s\n\r", sd_card_err_to_string(err));

        return -1;
    }

    sd_card_path_t fullpath = "0:/TEST.TXT";

    sd_card_buffer_t write_buffer = {.buffer = "dupa zbita\n\r",
                                     .buffer_len = strlen("dupa zbita")};

    err = sd_card_write(&manager->card, fullpath, &write_buffer);
    if (err != SD_CARD_ERR_OK) {
        printf("Error writing: %s\n\r", sd_card_err_to_string(err));

        return -1;
    }

    printf("Written to %s: %s\n\r", fullpath, write_buffer.buffer);

    sd_card_buffer_t read_buffer = {.buffer = NULL, .buffer_len = 64};
    err = sd_card_read(&manager->card, fullpath, &read_buffer);
    if (err != SD_CARD_ERR_OK) {
        printf("Error reading: %s\n\r", sd_card_err_to_string(err));

        return -1;
    }

    printf("Read from %s: %s\n\r", fullpath, read_buffer.buffer);
}