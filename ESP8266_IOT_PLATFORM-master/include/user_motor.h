#ifndef __USER_ESPSWITCH_H__
#define __USER_ESPSWITCH_H__

#include "driver/key.h"
#if 0
/* NOTICE---this is for 512KB spi flash.
 * you can change to other sector if you use other size spi flash. */
#define PRIV_PARAM_START_SEC        0x7C

#define PRIV_PARAM_SAVE     0
#endif

#define MOTOR_KEY_NUM            1

#define MOTOR_KEY_0_IO_MUX     PERIPHS_IO_MUX_GPIO0_U
#define MOTOR_KEY_0_IO_NUM     0
#define MOTOR_KEY_0_IO_FUNC    FUNC_GPIO0


#if 0
struct motor_saved_param {
    uint8_t status;
    uint8_t pad[3];
};
#endif
void user_motor_init(void);
BOOL user_get_key_status(void);

#endif

