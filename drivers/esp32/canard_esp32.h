/*
 * Copyright (c) 2016 UAVCAN Team
 *
 * Distributed under the MIT License, available in the file LICENSE.
 *
 */

#ifndef CANARD_ESP32_H
#define CANARD_ESP32_H

#include <canard.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * \ingroup   communication
 * \defgroup  canard_esp_interface Libcanard CAN Interface for ESP32 microcontrollers
 * \brief     Interface for Libcanard CAN interaction with ESP32 microcontrollers
 *
 * \author    Spencer Fowler <sf@spencerfowler.com>
 * \author    Integrated Solutions for Systems, Inc
 *
 * \version   0.1
 */


/**
 * @ingroup canard_esp32_interface
 * @brief Initialize CAN interface on ESP32 microcontroller.
 * @warning Enables interrupts!
 *
 * @param [in] bitrate  Set CAN bitrate (bits/sec.)
 *
 * @retval     0        Successfully initialized.
 */
int canardESP32Init(int8_t txpin, int8_t rxpin, int8_t can_stby_pin, uint32_t bitrate);


/**
 * @ingroup canard_esp32_interface
 * @brief Deinitialize CAN interface on ESP32 microcontroller.
 * @warning Not implemented
 *
 * @retval 1     Initialisation successful
 * @retval -1    Error, bitrate not supported
 */
int canardESP32Close(void);

/**
 * @ingroup canard_esp32_interface
 * @brief Transmits a CanardCANFrame to the CAN device.
 *
 * @param [in] frame  Canard CAN frame which contains the data to send
 *
 * @retval     0      No CAN send buffer free
 * @retval     -1     Error, data could not be sent
 * @retval     1      Data sent successful
 */
int canardESP32Transmit(const CanardCANFrame* frame);

/**
 * @ingroup canard_esp32_interface
 * @brief Receives a CanardCANFrame from the CAN device.
 *
 * @param [out] out_frame  Canard CAN frame which contains data received
 *
 * @retval      0          No new CAN data to be read
 * @retval      -1         Error, data could not be read
 * @retval      1          Data read successful
 */
int canardESP32Receive(CanardCANFrame* out_frame, uint32_t block_ms);

/**
 * @ingroup canard_esp32_interface
 * @brief Set hardware acceptance filters for specific node ID
 *
 * @param [in] id  node ID for hardware filter
 *
 * @retval      -1         Error, filters could no be set
 * @retval      1          Set filter successful
 */
int canardESP32ConfigureAcceptanceFilters(uint8_t node_id);

#ifdef __cplusplus
}
#endif

#endif
