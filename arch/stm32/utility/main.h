#ifndef MAIN_H
#define MAIN_H

#include <stdio.h>
#include "stm32f4_discovery.h"

#define SYSTEM_HZ                     100

#define TIM_ARR                          (uint16_t)1999
#define TIM_CCR                          (uint16_t)1000

/* MEMS Microphone SPI Interface */
#define SPI_SCK_PIN                   GPIO_Pin_10
#define SPI_SCK_GPIO_PORT             GPIOB
#define SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOB
#define SPI_SCK_SOURCE                GPIO_PinSource10
#define SPI_SCK_AF                    GPIO_AF_SPI2

#define SPI_MOSI_PIN                  GPIO_Pin_3
#define SPI_MOSI_GPIO_PORT            GPIOC
#define SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOC
#define SPI_MOSI_SOURCE               GPIO_PinSource3
#define SPI_MOSI_AF                   GPIO_AF_SPI2

#define ABS(x)         (x < 0) ? (-x) : x
#define MAX(a,b)       (a < b) ? (b) : a

void delay(unsigned int msec);
void system_beat(void);
void Fail_Handler(void);

#endif
