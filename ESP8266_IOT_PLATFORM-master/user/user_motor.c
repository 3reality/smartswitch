/******************************************************************************
 * Copyright (C) 2014 -2016  3Reality
 *
 * FileName: user_motor.c
 *
 * Description: motor demo's function realization
 *
 * Modification history:
 * 2017/2/1, v1.0 create this file.
*******************************************************************************/
#include "esp_common.h"
#include "user_config.h"
#if MOTOR_DEVICE
#include "user_motor.h"

//LOCAL struct motor_saved_param motor_param;
LOCAL struct keys_param keys;
LOCAL struct single_key_param *single_key[MOTOR_KEY_NUM];


/******************************************************************************
 * FunctionName : user_get_key_status
 * Description  : a
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
BOOL  
user_get_key_status(void)
{
    return get_key_status(single_key[0]);
}



/******************************************************************************
 * FunctionName : user_motor_short_press
 * Description  : key's short press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void  
user_motor_short_press(void)
{
    return;
}

/******************************************************************************
 * FunctionName : user_motor_long_press
 * Description  : key's long press function, needed to be installed
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
LOCAL void  
user_motor_long_press(void)
{
    return;
}


/******************************************************************************
 * FunctionName : user_motor_init
 * Description  : init plug's key function and relay output
 * Parameters   : none
 * Returns      : none
*******************************************************************************/
void  
user_motor_init(void)
{

    printf("user_motor_init start!\n");

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);

    //GPIO0 in our system
    single_key[0] = key_init_single(MOTOR_KEY_0_IO_NUM, MOTOR_KEY_0_IO_MUX, MOTOR_KEY_0_IO_FUNC,
                                    user_motor_long_press, user_motor_short_press);

    keys.key_num = MOTOR_KEY_NUM;
    keys.single_key = single_key;

    key_init(&keys);
#if 0
    spi_flash_read((PRIV_PARAM_START_SEC + PRIV_PARAM_SAVE) * SPI_FLASH_SEC_SIZE,
                (uint32 *)&motor_param, sizeof(struct motor_saved_param));
#endif				
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 1);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);

}
#endif

