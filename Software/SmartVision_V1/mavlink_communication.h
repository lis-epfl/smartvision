#include <stdint.h>
#include "mavlink_bridge_header.h"
#include "mavlink.h"

// Extern USB function
extern "C"
{
	int VCP_write(const void *pBuffer, int size);
}

void mavlink_communication_init(void);
void mavlink_send_state(void);
void mavlink_send_uart_bytes(mavlink_channel_t chan, const uint8_t * ch, uint16_t length);

