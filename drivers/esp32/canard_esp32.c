/*
 * Copyright (c) 2019 Spencer Fowler
 *
 * Distributed under the MIT License, available in the file LICENSE.
 *
 */

#define HAS_CAN_CONFIG_H

#include <canard_esp32.h>
#include <string.h> // For memcpy

#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/can.h"

static const char* TAG = "canard_esp32";


int canardESP32Init(int8_t txpin, int8_t rxpin, int8_t can_stby_pin, uint32_t bitrate)
{
    // can_bitrate_t br;
    can_timing_config_t t_config;
    switch (bitrate)
    { // ESP32 Can't go lower than 25KHz due to 80MHz APB and limited divider range
    case 25000:
    {
        // t_config = CAN_TIMING_CONFIG_25KBITS();
        t_config.brp = 128; t_config.tseg_1 = 16; t_config.tseg_2 = 8; t_config.sjw = 3; t_config.triple_sampling = false;
        break;
    }
    case 33300:
    case 33333:
    {
        // 80MHz / 120 / (1+15+4) = 33333 bits/sec
        t_config.brp = 120; t_config.tseg_1 = 15; t_config.tseg_2 = 4; t_config.sjw = 3; t_config.triple_sampling = false;
        break;
    }
    case 50000:
    {
        // t_config = CAN_TIMING_CONFIG_50KBITS();
        t_config.brp = 80; t_config.tseg_1 = 15; t_config.tseg_2 = 4; t_config.sjw = 3; t_config.triple_sampling = false;
        break;
    }
    case 100000:
    {
        // t_config = CAN_TIMING_CONFIG_100KBITS();
        t_config.brp = 40; t_config.tseg_1 = 15; t_config.tseg_2 = 4; t_config.sjw = 3; t_config.triple_sampling = false;
        break;
    }
    case 125000:
    {
        // t_config = CAN_TIMING_CONFIG_125KBITS();
        t_config.brp = 32; t_config.tseg_1 = 15; t_config.tseg_2 = 4; t_config.sjw = 3; t_config.triple_sampling = false;
        break;
    }
    case 250000:
    {
        // t_config = CAN_TIMING_CONFIG_250KBITS();
        t_config.brp = 16; t_config.tseg_1 = 15; t_config.tseg_2 = 4; t_config.sjw = 3; t_config.triple_sampling = false;
        break;
    }
    case 500000:
    {
        // t_config = CAN_TIMING_CONFIG_500KBITS();
        t_config.brp = 8; t_config.tseg_1 = 15; t_config.tseg_2 = 4; t_config.sjw = 3; t_config.triple_sampling = false;
        break;
    }
    case 1000000:
    {
        // t_config = CAN_TIMING_CONFIG_1MBITS();
        t_config.brp = 4; t_config.tseg_1 = 15; t_config.tseg_2 = 4; t_config.sjw = 3; t_config.triple_sampling = false;
        break;
    }
    default:
    {
        ESP_LOGE(TAG, "Unsupported CAN Bitrate: %u", bitrate);
        return -1;
    }
    }

    can_general_config_t g_config = CAN_GENERAL_CONFIG_DEFAULT(txpin, rxpin, CAN_MODE_NORMAL);

    // create a new filter for receiving all messages
    can_filter_config_t f_config = CAN_FILTER_CONFIG_ACCEPT_ALL();

    if(can_stby_pin >= 0) {
        gpio_config_t io_conf;
        io_conf.intr_type = GPIO_PIN_INTR_DISABLE;
        io_conf.mode = GPIO_MODE_OUTPUT;
        io_conf.pin_bit_mask = (1ULL<<can_stby_pin);
        io_conf.pull_down_en = 0;
        io_conf.pull_up_en = 0;
        gpio_config(&io_conf);

        gpio_set_level(can_stby_pin, 0);
    }

    if (can_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
       ESP_LOGI(TAG, "CAN Driver installed");
        //Start CAN driver
        if (can_start() == ESP_OK) {
            ESP_LOGI(TAG, "CAN Driver started");
        } else {
            ESP_LOGE(TAG, "Failed to start CAN driver");
            return -1;
    }
    } else {
        ESP_LOGE(TAG, "Failed to install CAN driver");
        return -1;
    }

    return 0;
}

int canardESP32Close(void)
{

    //Stop the CAN driver
    if (can_stop() == ESP_OK) {
        ESP_LOGI(TAG, "CAN Driver stopped");
    } else {
        ESP_LOGE(TAG, "Failed to stop CAN driver");
        return -1;
    }

    //Uninstall the CAN driver
    if (can_driver_uninstall() == ESP_OK) {
        ESP_LOGI(TAG, "CAN Driver uninstalled");
    } else {
        ESP_LOGE(TAG, "Failed to uninstall CAN driver");
        return -1;
    }

    return 0;
}

int canardESP32Recover(){
    can_status_info_t status_info;
    esp_err_t ret = can_get_status_info(&status_info);
    if (ret == ESP_OK) {
        if (status_info.state == CAN_STATE_BUS_OFF) {
            ESP_LOGW(TAG, "Initiating CAN recovery");
            can_initiate_recovery();
            return 0;
        } else if (status_info.state == CAN_STATE_STOPPED) {
            ESP_LOGW(TAG, "Starting CAN interface");
            if (ESP_OK == can_start()) {
                return 1;
            } else { return -1; }
        } else if (status_info.state == CAN_STATE_RECOVERING) {
            ESP_LOGW(TAG, "CAN recovery pending");
            return 0;
        } else {
            ESP_LOGW(TAG, "CAN Recovery called but bus is OK");
            return 1;
        }

    } else if (ret == ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Tried to recover bus but driver not installed");
        return -1;
    }

    return 0;

}

int canardESP32Transmit(const CanardCANFrame* frame)
{

    //Configure message to transmit
    can_message_t message;
    message.identifier = frame->id & CANARD_CAN_EXT_ID_MASK;
    if ((frame->id & CANARD_CAN_FRAME_EFF) != 0) {
        message.flags = CAN_MSG_FLAG_EXTD;
    } else {
        message.flags = 0;
    }
    message.data_length_code = frame->data_len;
    memcpy(message.data, frame->data, frame->data_len);
    //message.data[i] = 0;


    //Queue message for transmission
    esp_err_t res = can_transmit(&message, 0); // block for 0 ticks
    if (res == ESP_OK) {
        ESP_LOGV(TAG, "Message queued for transmit");
        return 1;
    } else if (res == ESP_ERR_TIMEOUT){
        return -1;
    }
    else if (res == ESP_FAIL){
        return -2;
    }
    else if (res == ESP_ERR_INVALID_STATE){
        canardESP32Recover();
        return -3;
    }

    return 0;

}

int canardESP32Receive(CanardCANFrame* out_frame, uint32_t block_ms)
{

    //Wait for message to be received
    can_message_t message;
    if (can_receive(&message, pdMS_TO_TICKS(block_ms)) == ESP_OK) {
        ESP_LOGI(TAG, "RX");
    } else {
        ESP_LOGV(TAG, "no CAN RX");
        return 0;
    }

    //Process received message

    out_frame->id = message.identifier;
    out_frame->data_len = message.data_length_code;
    memcpy(out_frame->data, message.data, message.data_length_code);

    if (message.flags & CAN_MSG_FLAG_EXTD) {
        ESP_LOGV(TAG, "Message is in Extended Format");
        out_frame->id |= CANARD_CAN_FRAME_EFF;
    } else {
        ESP_LOGV(TAG, "Message is in Standard Format");
    }

    return 1;
}

int canardESP32ConfigureAcceptanceFilters(uint8_t node_id)
    // TODO: Inplement acceptance filtering (must be done on driver creation)
{
    static const uint32_t DefaultFilterMsgMask = 0x80;
    static const uint32_t DefaultFilterMsgID = 0x0;
    static const uint32_t DefaultFilterSrvMask = 0x7F80;
    uint8_t res = 1;

    // // create a new filter for receiving messages
    // can_filter_t filter_Msg = {
    //     .id = DefaultFilterMsgID,
    //     .mask = DefaultFilterMsgMask,
    //     .flags = {
    //         .rtr = 0,
    //         .extended = 3
    //     }
    // };

    // // create a new filter for receiving services
    // can_filter_t filter_Srv = {
    //     .id = ((uint32_t)node_id << 8) | 0x80,
    //     .mask = DefaultFilterSrvMask,
    //     .flags = {
    //         .rtr = 0,
    //         .extended = 3
    //     }
    // };

    // // setup 2 MOb's to receive, 12 MOb's are used as send buffer
    // if (!can_set_filter(0, &filter_Msg))
    // {
    //     res = -1;
    // }

    // if (!can_set_filter(1, &filter_Srv))
    // {
    //     res = -1;
    // }

    return res;
}
