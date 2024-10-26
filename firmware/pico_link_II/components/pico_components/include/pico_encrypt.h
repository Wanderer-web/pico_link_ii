/***
 * @Author: Wanderer
 * @Date: 2023-05-16 18:59:41
 * @LastEditors: Wanderer
 * @LastEditTime: 2023-05-16 19:00:22
 * @FilePath: \pico_link_pro\components\pico_components\include\pico_encrypt.h
 * @Description:
 */

#ifndef __PICO_ENCRYPT_H__
#define __PICO_ENCRYPT_H__

#include "pico_headfile.h"

typedef struct
{
    uint32_t a;
    uint32_t b;
    uint32_t c;
    uint32_t d;
} __attribute__((packed)) pico_encrypt_par_t;

extern pico_encrypt_par_t encrypt_par;

void pico_encrypt_check(void);
void pico_write_encrypt_and_check(void);

#endif