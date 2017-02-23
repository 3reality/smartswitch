/******************************************************************************
 * Copyright (C) 2014 -2016  Espressif System
 *
 * FileName: key.c
 *
 * Description: key driver, now can use different gpio and install different function
 *
 * Modification history:
 * 2015/7/1, v1.0 create this file.
*******************************************************************************/
#include "esp_common.h"
#include "driver/gpio.h"
#include "driver/key.h"

#define LONG_PRESS_TIME  3000 //ms
static bool reverse = 0;

LOCAL void key_intr_handler(struct keys_param *keys);

/******************************************************************************
 * FunctionName : get_key_status
 * Description  : get single key status
 * Parameters   : single_key - single key pointer parameter
 * Returns      : none
*******************************************************************************/
BOOL  
get_key_status(struct single_key_param *single_key)
{
    return GPIO_INPUT_GET(GPIO_ID_PIN(single_key->gpio_id));
}

/******************************************************************************
 * FunctionName : key_init_single
 * Description  : init single key's gpio and register function
 * Parameters   : uint8 gpio_id - which gpio to use
 *                uint32 gpio_name - gpio mux name
 *                uint32 gpio_func - gpio function
 *                key_function long_press - long press function, needed to install
 *                key_function short_press - short press function, needed to install
 * Returns      : single_key_param - single key parameter, needed by key init
*******************************************************************************/
struct single_key_param * 
key_init_single(uint8 gpio_id, uint32 gpio_name, uint8 gpio_func, key_function long_press, key_function short_press)
{
    struct single_key_param *single_key = (struct single_key_param *)zalloc(sizeof(struct single_key_param));

    single_key->gpio_id = gpio_id;
    single_key->gpio_name = gpio_name;
    single_key->gpio_func = gpio_func;
    single_key->long_press = long_press;
    single_key->short_press = short_press;

    return single_key;
}

/******************************************************************************
 * FunctionName : key_init
 * Description  : init keys
 * Parameters   : key_param *keys - keys parameter, which inited by key_init_single
 * Returns      : none
*******************************************************************************/
void  
key_init(struct keys_param *keys)
{
    u32 i;
    GPIO_ConfigTypeDef *pGPIOConfig;

    pGPIOConfig = (GPIO_ConfigTypeDef*)zalloc(sizeof(GPIO_ConfigTypeDef));
    gpio_intr_handler_register(key_intr_handler,keys);
    
    for (i = 0; i < keys->key_num; i++) {
        keys->single_key[i]->key_level = 1;
        pGPIOConfig->GPIO_IntrType = GPIO_PIN_INTR_NEGEDGE;
        pGPIOConfig->GPIO_Pullup = GPIO_PullUp_EN;
        pGPIOConfig->GPIO_Mode = GPIO_Mode_Input;
        pGPIOConfig->GPIO_Pin = (1 << keys->single_key[i]->gpio_id);//this is GPIO_Pin_0

        gpio_config(pGPIOConfig);
    }
    //enable gpio iterrupt
    _xt_isr_unmask(1<<ETS_GPIO_INUM);
}

/******************************************************************************
 * FunctionName : key_5s_cb
 * Description  : long press 5s timer callback
 * Parameters   : single_key_param *single_key - single key parameter
 * Returns      : none
*******************************************************************************/
LOCAL void  
key_5s_cb(struct single_key_param *single_key)
{
    os_timer_disarm(&single_key->key_5s);

    //turn off motor
    GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 1);
    GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);

    //enable this gpio pin interrupt
    gpio_pin_intr_state_set(GPIO_ID_PIN(single_key->gpio_id), GPIO_PIN_INTR_NEGEDGE);
}

/******************************************************************************
 * FunctionName : key_50ms_cb
 * Description  : 50ms timer callback to check it's a real key push
 * Parameters   : single_key_param *single_key - single key parameter
 * Returns      : none
*******************************************************************************/
LOCAL void  
key_50ms_cb(struct single_key_param *single_key)
{
    os_timer_disarm(&single_key->key_50ms);
    //check this gpio pin state
    if (1 == GPIO_INPUT_GET(GPIO_ID_PIN(single_key->gpio_id))) {
        os_timer_disarm(&single_key->key_5s);
        single_key->key_level = 1;
        gpio_pin_intr_state_set(GPIO_ID_PIN(single_key->gpio_id), GPIO_PIN_INTR_NEGEDGE);
        //this gpio has been in low state no more than 5s, then call short_press function
        if (single_key->short_press) {
            single_key->short_press();
        }
    } else {
        gpio_pin_intr_state_set(GPIO_ID_PIN(single_key->gpio_id), GPIO_PIN_INTR_POSEDGE);
    }
}

/******************************************************************************
 * FunctionName : key_intr_handler
 * Description  : key interrupt handler
 * Parameters   : key_param *keys - keys parameter, which inited by key_init_single
 * Returns      : none
*******************************************************************************/

LOCAL void
key_intr_handler(struct keys_param *keys)
{
    uint8 i;
    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

    for (i = 0; i < keys->key_num; i++) {
        if (gpio_status & BIT(keys->single_key[i]->gpio_id)) {
            
            //disable this gpio pin interrupt
            gpio_pin_intr_state_set(GPIO_ID_PIN(keys->single_key[i]->gpio_id), GPIO_PIN_INTR_DISABLE);
            //clear interrupt status
            GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(keys->single_key[i]->gpio_id));

            //turn off motor first 
            GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
            GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);
            GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 1);
            GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);

            if (reverse)
           {
                //reverse 
                GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 1);
                GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 0);
                GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 1);
                GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 0);
                reverse = 0;
            }
            else{
                //forward
                GPIO_OUTPUT_SET(GPIO_ID_PIN(12), 0);
                GPIO_OUTPUT_SET(GPIO_ID_PIN(13), 1);
                GPIO_OUTPUT_SET(GPIO_ID_PIN(4), 0);
                GPIO_OUTPUT_SET(GPIO_ID_PIN(14), 1);
                reverse = 1;
            }
            //run 5s for demo purpose, borrow the key_5s structure for the demo
            os_timer_disarm(&keys->single_key[i]->key_5s);
            os_timer_setfn(&keys->single_key[i]->key_5s, (os_timer_func_t *)key_5s_cb, keys->single_key[i]);
            os_timer_arm(&keys->single_key[i]->key_5s, LONG_PRESS_TIME, 0);
        }
    }
}

