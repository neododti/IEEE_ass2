/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Neo Dodti
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2026 Neo Dodti.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>

/** * @brief Peripheral Register Structures
 * These mirror the memory map found in the STM32F446RE reference manual
 * volatile is used to prevent the compiler from optimizing out hardware-updated registers.
 */

typedef struct
{
	volatile uint32_t MODER;   // Port mode register
	volatile uint32_t OTYPER;  // Output type register
	volatile uint32_t OSPEEDR; // Output speed register
	volatile uint32_t PUPDR;   // Pull-up/pull-down register
	volatile uint32_t IDR;	   // Input data register
	volatile uint32_t ODR;	   // Output data register
	volatile uint32_t BSRR;	   // Bit set/reset register
	volatile uint32_t LCKR;	   // Configuration lock register
	volatile uint32_t AFRL;	   // Alternate function low register
	volatile uint32_t AFRH;	   // Alternate function high register
} GPIO_TypeDef;

typedef struct
{
	volatile uint32_t CR;		// Clock control register
	volatile uint32_t PLLCFGR;	// PLL configuration register
	volatile uint32_t CFGR;		// Clock configuration register
	volatile uint32_t CIR;		// Clock interrupt register
	volatile uint32_t AHB1RSTR; // AHB1 peripheral reset register
	volatile uint32_t AHB2RSTR; // AHB2 peripheral reset register
	volatile uint32_t AHB3RSTR; // AHB3 peripheral reset register
	uint32_t RESERVED0;
	volatile uint32_t APB1RSTR; // APB1 peripheral reset register
	volatile uint32_t APB2RSTR; // APB2 peripheral reset register
	uint32_t RESERVED1[2];
	volatile uint32_t AHB1ENR; // AHB1 peripheral clock enable register
	volatile uint32_t AHB2ENR; // AHB2 peripheral clock enable register
	volatile uint32_t AHB3ENR; // AHB3 peripheral clock enable register
	uint32_t RESERVED2;
	volatile uint32_t APB1ENR; // APB1 peripheral clock enable register
} RCC_TypeDef;

typedef struct
{
	volatile uint32_t CR1;	 // Control register 1
	volatile uint32_t CR2;	 // Control register 2
	volatile uint32_t SMCR;	 // Slave mode control register
	volatile uint32_t DIER;	 // DMA/Interrupt enable register
	volatile uint32_t SR;	 // Status register
	volatile uint32_t EGR;	 // Event generation register
	volatile uint32_t CCMR1; // Capture/compare mode register 1
	volatile uint32_t CCMR2; // Capture/compare mode register 2
	volatile uint32_t CCER;	 // Capture/compare enable register
	volatile uint32_t CNT;	 // Counter register
	volatile uint32_t PSC;	 // Prescaler
	volatile uint32_t ARR;	 // Auto-reload register
} TIM_TypeDef;

typedef struct
{
	volatile uint32_t SR;	// Status Register
	volatile uint32_t DR;	// Data Register
	volatile uint32_t BRR;	// Baud Rate Register
	volatile uint32_t CR1;	// Control Register 1
	volatile uint32_t CR2;	// Control Register 2
	volatile uint32_t CR3;	// Control Register 3
	volatile uint32_t GTPR; // Guard Time and Prescaler Register
} USART_TypeDef;

/* Base Addresses for Peripherals */
#define RCC ((RCC_TypeDef *)0x40023800)
#define GPIOA ((GPIO_TypeDef *)0x40020000)
#define TIM2 ((TIM_TypeDef *)0x40000000)
#define USART2 ((USART_TypeDef *)0x40004400)

/**
 * @brief Initializes USART2 on PA2 (TX) for 9600 baud and configures TIM2 for periodic interrupts
 * @param s: Time period (in seconds)
 */
void setup(int s)
{
	// Enable clocks for GPIOA, TIM2 and USART2
	RCC->AHB1ENR |= 1;		   // GPIOAEN
	RCC->APB1ENR |= 1;		   // TIM2EN
	RCC->APB1ENR |= (1 << 17); // USART2EN

	// Configure PA2 as Alternate Function for USART2 TX
	GPIOA->MODER |= (1 << 5); // bit 5 set to 1 and bit 4 set to 0 for Alternate Function mode (10)

	// Set Alternate Function 7 (AF7) for PA2 (USART2_TX)
	GPIOA->AFRL |= (7 << 8); // 7 in binary is 111, and for AF7 we set bits 8-11 as 0111

	// Configure Baud Rate (9600 bps with 16MHz clock)
	USART2->BRR = 0x0683; // Calculated value for 9600 bps using formula BRR = f_clk / (16 * baud)

	// Configure USART2 for serial communication
	USART2->CR1 = 0; // Reset control register (stops USART)

	// Enable USART2 and its transmitter
	USART2->CR1 |= (1 << 13); // UE (USART Enable)
	USART2->CR1 |= (1 << 3);  // TE (Transmitter Enable)

	// Timer setup
	TIM2->CR1 = 0;
	TIM2->PSC = 15999;
	TIM2->ARR = (uint32_t)(s * 1000) - 1;
	TIM2->CNT = 0;
	TIM2->EGR |= 1;
	TIM2->SR &= ~1;
	TIM2->CR1 |= 1;
}

int main(void)
{
	setup(1); // Initialize system with 1s toggle rate

	while (1)
	{
		// Poll the UIF to see if timer reached its target
		if (TIM2->SR & 1) // Check if UIF is set
		{
			TIM2->SR &= ~1; // Reset flag

			// Wait until TXE (Transmit Empty) is 1
			// This is Bit 7 of the Status Register (SR)
			while (!(USART2->SR & (1 << 7)))
				;

			// Push a character directly into the Data Register (DR)
			USART2->DR = 'A';
		}
	}

	return 0;
}
