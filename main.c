// SCHKUZ002 MLKTSH012
#include <stdint.h>
#define STM32F051
#include "stm32f0xx.h"

//PROGRAM PROTOTYPES
int main(void);
void TIM6_Handler(void);

//VARIABLE DECLARATIONS
#define GPIO_IDR_0                  ((uint32_t)0x00000001)
#define GPIO_IDR_1                  ((uint32_t)0x00000002)
#define GPIO_IDR_2                  ((uint32_t)0x00000004)
#define GPIO_IDR_3                  ((uint32_t)0x00000008)
#define ADC_CHSELR_CHSEL5           ((uint32_t)0x00000020)
#define ADC_CHSELR_CHSEL6           ((uint32_t)0x00000040)
#define ADC_CFGR1_RES_1             ((uint32_t)0x00000010)
#define ADC_CFGR1_RES_2             ((uint32_t)0x00000030)
#define ADC_CR_ADCAL                ((uint32_t)0x80000000)
#define ADC_CR_ADEN                 ((uint32_t)0x00000001)
#define ADC_ISR_ADRDY               ((uint32_t)0x00000001)
#define ADC_CR_ADSTART              ((uint32_t)0x00000004)
#define ADC_ISR_EOC                 ((uint32_t)0x00000004)

//GLOBAL VARIABLE DECLARATION
static uint8_t led_patterns[] = {0x00, 0x81, 0xC3, 0xE7, 0xFF, 0x7E, 0x3C, 0x18};
static uint8_t indx = 0x00;

//REGISTER DECLARATIONS
#define GPIOB_ODR                   ((uint16_t*)0x48000414)
#define GPIOA_IDR                   ((uint32_t*)0x48000010)
#define GPIOB_MODER                 ((uint16_t*)0x48000400)
#define GPIOB_PUPDR                 ((uint16_t*)0x4800000C)
#define GPIOA_MODER                 ((uint16_t*)0x48000000)
#define ADC_CHSELR                  ((uint16_t*)0x40012428)
#define ADC_CFGR1                   ((uint16_t*)0x4001240C)
#define ADC_CR                      ((uint16_t*)0x40012408)
#define ADC_ISR                     ((uint16_t*)0x40012400)
#define ADC_DR                      ((uint16_t*)0x40012440)

#define TIM6_PSC                    ((uint32_t*)0x40001028)
#define TIM6_ARR                    ((uint32_t*)0x4000102C)
#define TIM6_DIER                   ((uint32_t*)0x4000100C)
#define TIM6_CNT                    ((uint32_t*)0x40001000)
#define NVIC_ISER                   ((uint32_t*)0xE000E100)
#define TIM6_INTERUPT_ACK           ((uint32_t*)0x40001010)


//MAIN PROGRAM ROUTINE
int main(void)
{
    uint16_t prev_state, curr_state;
    uint32_t delay_counter;
    
    *(uint32_t*)0x40021014 |= 0x60000;                                        //Clocking the GPIOB & GPIOA Ports
    *(uint32_t*)0x4002101C |= 0x10;                                           //Clocking TIM6
    *GPIOB_MODER = 0x5555;                                                    //Enabling GPIOB PINS to OUTPUT
    *GPIOA_MODER = 0x3C00;                                                    //Set Switch to input and pot PA5 & PA6 to analog
    
    *GPIOB_PUPDR = 0x55;                                                       //set sw0-1 to pullup
    *GPIOB_ODR = led_patterns[0];                                              //Writing A0 to GPIOB_ODR
    *(uint32_t*)0x40021018 |= 0x200;                                           //Clocking ADC
    
    *ADC_CHSELR |= ADC_CHSELR_CHSEL5;                                          // select channel 5
    *ADC_CFGR1 |= ADC_CFGR1_RES_1;
    *ADC_CR |= ADC_CR_ADCAL;
    
    ((*ADC_CR)) |= ADC_CR_ADEN;                                                // set ADEN=1 in the ADC_CR register
    
    while((((*(uint16_t*)0x40012400)) & ADC_ISR_ADRDY) == 0);

    *TIM6_PSC = 0x30C;
    *TIM6_ARR = 0x13FF;
    *TIM6_DIER |= 0x01;
    *NVIC_ISER |= 0x20000;
    
    prev_state = (*GPIOA_IDR & GPIO_IDR_1);
    
    while (1)
    {
        
            if ((*GPIOA_IDR & GPIO_IDR_0) == 0)
            {
                *TIM6_CNT |= 0x01;
            }
            else
            {
                *TIM6_CNT = 0x00;
            }
        
//////////////////////////////////////
        
            curr_state = (*GPIOA_IDR & GPIO_IDR_1);
        
            if (curr_state == 0 && prev_state != 0)
            
            {
                
                if (indx == ((sizeof(led_patterns)/sizeof(led_patterns[0]))-1))
                {
                    indx = 0x00;
                }
                
                else
                    
                {
                    indx += 1;
                }
                
                *GPIOB_ODR = led_patterns[indx];
                
            }
        
            prev_state = curr_state;
            for(delay_counter = 0; delay_counter < 15000; ++delay_counter);
        
//////////////////////////////////////
        
            if ((*GPIOA_IDR & GPIO_IDR_2) == 0)
            {
                *ADC_CHSELR = ADC_CHSELR_CHSEL5;
                *ADC_CR |= ADC_CR_ADSTART;
                while(((*ADC_ISR) & ADC_ISR_EOC) == 0);
                uint32_t POT0_data = (*ADC_DR);
                
                *ADC_CHSELR = ADC_CHSELR_CHSEL6;
                *ADC_CR |= ADC_CR_ADSTART;
                while(((*ADC_ISR) & ADC_ISR_EOC) == 0);
                uint32_t POT1_data = (*ADC_DR);
                
                if(POT0_data < POT1_data)
                {
                *GPIOB_ODR = POT0_data;
                }
                else
                {
                *GPIOB_ODR = POT1_data;
                }
                
            }
        else
        {
        *GPIOB_ODR = led_patterns[indx];
        }
    }
    
    return 0;
}

void TIM6_Handler(void)
{
    *TIM6_INTERUPT_ACK = 0x00;
    *GPIOB_ODR = led_patterns[indx];
    
    if (indx == ((sizeof(led_patterns)/sizeof(led_patterns[0]))-1))
    {
        indx = 0x00;
    }
    
    else
    
    {
        indx += 1;
    }
    
}
