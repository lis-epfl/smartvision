#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "mavlink_bridge_header.h"
#include "mavlink.h"


// Extern USB function
extern "C"
{
	int VCP_write(const void *pBuffer, int size);
	int VCP_read(void *pBuffer, int size);
}

void send_calibration_image(uint8_t ** image, uint32_t image_size, uint16_t image_row, uint16_t image_column);
void mavlink_communication_init(void);
void mavlink_send_state(void);
void mavlink_send_uart_bytes(mavlink_channel_t chan, const uint8_t * ch, uint16_t length);
void communication_receive(void);
void handle_mavlink_message(mavlink_channel_t chan, mavlink_message_t * msg);
void communication_parameter_send(void);

