/*

  serial.c - serial port implementation for STM32F7xx ARM processors

  Part of grblHAL

  Copyright (c) 2021 Terje Io

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <string.h>

#include "serial.h"
#include "grbl/hal.h"
#include "grbl/protocol.h"

#include "main.h"

static stream_rx_buffer_t rxbuf = {0};
static stream_tx_buffer_t txbuf = {0};
static enqueue_realtime_command_ptr enqueue_realtime_command = protocol_enqueue_realtime_command;

#ifdef SERIAL2_MOD
static stream_rx_buffer_t rxbuf2 = {0};
static stream_tx_buffer_t txbuf2 = {0};
static enqueue_realtime_command_ptr enqueue_realtime_command2 = stream_buffer_all;
#endif

#if defined(NUCLEO_F756)
  #define USART USART3
  #define USART_IRQHandler USART3_IRQHandler
#else
  #define USART USART1
  #define USART_IRQHandler USART1_IRQHandler
#endif

//
// Returns number of free characters in serial input buffer
//
static uint16_t serialRxFree (void)
{
    uint16_t tail = rxbuf.tail, head = rxbuf.head;

    return RX_BUFFER_SIZE - BUFCOUNT(head, tail, RX_BUFFER_SIZE);
}

//
// Flushes the serial input buffer
//
static void serialRxFlush (void)
{
    rxbuf.tail = rxbuf.head;
}

//
// Flushes and adds a CAN character to the serial input buffer
//
static void serialRxCancel (void)
{
    rxbuf.data[rxbuf.head] = ASCII_CAN;
    rxbuf.tail = rxbuf.head;
    rxbuf.head = BUFNEXT(rxbuf.head, rxbuf);
}

//
// Attempt to send a character bypassing buffering
//
inline static bool serialPutCNonBlocking (const char c)
{
    bool ok;

    if((ok = !(USART->CR1 & USART_CR1_TXEIE) && !(USART->ISR & USART_ISR_TXE)))
        USART->TDR = c;

    return ok;
}

//
// Writes a character to the serial output stream
//
static bool serialPutC (const char c)
{
//    if(txbuf.head != txbuf.tail || !serialPutCNonBlocking(c)) {         // Try to send character without buffering...

        uint_fast16_t next_head = BUFNEXT(txbuf.head, txbuf);           // .. if not, get pointer to next free slot in buffer

        while(txbuf.tail == next_head) {                                // While TX buffer full
            if(!hal.stream_blocking_callback())                         // check if blocking for space,
                return false;                                           // exit if not (leaves TX buffer in an inconsistent state)
        }

        txbuf.data[txbuf.head] = c;                                     // Add data to buffer,
        txbuf.head = next_head;                                         // update head pointer and
        USART->CR1 |= USART_CR1_TXEIE;                                  // enable TX interrupts
//    }

    return true;
}

//
// Writes a null terminated string to the serial output stream, blocks if buffer full
//
static void serialWriteS (const char *s)
{
    char c, *ptr = (char *)s;

    while((c = *ptr++) != '\0')
        serialPutC(c);
}

//
// Writes a number of characters from string to the serial output stream followed by EOL, blocks if buffer full
//
void serialWrite(const char *s, uint16_t length)
{
    char *ptr = (char *)s;

    while(length--)
        serialPutC(*ptr++);
}

//
// serialGetC - returns -1 if no data available
//
static int16_t serialGetC (void)
{
    uint_fast16_t bptr = rxbuf.tail;

    if(bptr == rxbuf.head)
        return -1; // no data available else EOF

    char data = rxbuf.data[bptr];       // Get next character
    rxbuf.tail = BUFNEXT(bptr, rxbuf);  // and update pointer

    return (int16_t)data;
}

static bool serialSuspendInput (bool suspend)
{
    return stream_rx_suspend(&rxbuf, suspend);
}

static bool serialSetBaudRate (uint32_t baud_rate)
{
    USART->CR1 = USART_CR1_RE|USART_CR1_TE;
    USART->CR3 = USART_CR3_OVRDIS;
    USART->BRR = UART_DIV_SAMPLING16(HAL_RCC_GetPCLK1Freq(), baud_rate);
    USART->CR1 |= (USART_CR1_UE|USART_CR1_RXNEIE);

    rxbuf.tail = rxbuf.head;
    txbuf.tail = txbuf.head;

    return true;
}

static bool serialDisable (bool disable)
{
    if(disable)
        USART->CR1 &= ~USART_CR1_RXNEIE;
    else
        USART->CR1 |= USART_CR1_RXNEIE;

    return true;
}

static enqueue_realtime_command_ptr serialSetRtHandler (enqueue_realtime_command_ptr handler)
{
    enqueue_realtime_command_ptr prev = enqueue_realtime_command;

    if(handler)
        enqueue_realtime_command = handler;

    return prev;
}

const io_stream_t *serialInit (uint32_t baud_rate)
{
    static const io_stream_t stream = {
        .type = StreamType_Serial,
        .connected = true,
        .read = serialGetC,
        .write = serialWriteS,
        .write_char = serialPutC,
        .write_all = serialWriteS,
        .get_rx_buffer_free = serialRxFree,
        .reset_read_buffer = serialRxFlush,
        .cancel_read_buffer = serialRxCancel,
        .suspend_read = serialSuspendInput,
        .disable = serialDisable,
        .set_baud_rate = serialSetBaudRate,
        .set_enqueue_rt_handler = serialSetRtHandler
    };

#if defined(NUCLEO_F756)

    __HAL_RCC_USART3_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStructure = {
        .Mode      = GPIO_MODE_AF_PP,
        .Pull      = GPIO_NOPULL,
        .Speed     = GPIO_SPEED_FREQ_VERY_HIGH,
        .Pin       = GPIO_PIN_8|GPIO_PIN_9,
        .Alternate = GPIO_AF7_USART3
    };
    HAL_GPIO_Init(GPIOD, &GPIO_InitStructure);

    serialSetBaudRate(baud_rate);

    HAL_NVIC_SetPriority(USART3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART3_IRQn);

#else

    __HAL_RCC_USART1_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStructure = {
        .Mode      = GPIO_MODE_AF_PP,
        .Pull      = GPIO_NOPULL,
        .Speed     = GPIO_SPEED_FREQ_VERY_HIGH,
        .Pin       = GPIO_PIN_9|GPIO_PIN_10,
        .Alternate = GPIO_AF7_USART1
    };
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

    serialSetBaudRate(baud_rate);

    HAL_NVIC_SetPriority(USART1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

#endif

    return &stream;
}

void USART_IRQHandler (void)
{
    if(USART->ISR & USART_ISR_RXNE) {
        char data = USART->RDR;
        if(!enqueue_realtime_command(data)) {                   // Check for and strip realtime commands,
            uint_fast16_t next_head = BUFNEXT(rxbuf.head, rxbuf);
            if(rxbuf.tail == next_head)
                rxbuf.overflow = 1;                             // flag overflow
            else {
                rxbuf.data[rxbuf.head] = data;                  // if not add data to buffer
                rxbuf.head = next_head;                         // and update pointer
            }
        }
    }

    if((USART->ISR & USART_ISR_TXE) && (USART->CR1 & USART_CR1_TXEIE)) {
        USART->TDR = txbuf.data[txbuf.tail];        // Send next character
        txbuf.tail = BUFNEXT(txbuf.tail, txbuf);    // and increment pointer
        if(txbuf.tail == txbuf.head)                // If buffer empty then
            USART->CR1 &= ~USART_CR1_TXEIE;         // disable UART TX interrupt
   }

    if(USART->ISR & USART_ISR_ORE)
        USART->ICR &= USART_ICR_ORECF;
}

#ifdef SERIAL2_MOD
#if defined(NUCLEO_F756) || defined(NUCLEO_F446)
#define UART2 USART6
#define UART2_IRQHandler USART6_IRQHandler
#else
#define UART2 USART2
#define UART2_IRQHandler USART2_IRQHandler
#endif

#if !(MODBUS_ENABLE || TRINAMIC_ENABLE == 2209)

void serialSelect (bool mpg)
{ /*
    if(mpg) {
        SERIAL_MODULE->IE = 0;
        SERIAL2_MODULE->IE = EUSCI_A_IE_RXIE;
    } else {
        SERIAL_MODULE->IE = EUSCI_A_IE_RXIE;
        SERIAL2_MODULE->IE = 0;
    } */
}

#endif

//
// Returns number of free characters in serial input buffer
//
static uint16_t serial2RxFree (void)
{
    uint32_t tail = rxbuf2.tail, head = rxbuf2.head;

    return (RX_BUFFER_SIZE - 1) - BUFCOUNT(head, tail, RX_BUFFER_SIZE);
}

//
// Returns number of characters in serial input buffer
//
uint16_t serial2RxCount (void)
{
    uint32_t tail = rxbuf2.tail, head = rxbuf2.head;

    return BUFCOUNT(head, tail, RX_BUFFER_SIZE);
}

//
// Flushes the serial input buffer
//
static void serial2RxFlush (void)
{
    rxbuf2.tail = rxbuf2.head;
}

//
// Flushes and adds a CAN character to the serial input buffer
//
static void serial2RxCancel (void)
{
    rxbuf2.data[rxbuf2.head] = ASCII_CAN;
    rxbuf2.tail = rxbuf2.head;
    rxbuf2.head = (rxbuf2.tail + 1) & (RX_BUFFER_SIZE - 1);
}

//
// Attempt to send a character bypassing buffering
//
static inline bool serial2PutCNonBlocking (const char c)
{
    bool ok;

    if((ok = !(UART2->CR1 & USART_CR1_TXEIE) && !(UART2->ISR & USART_ISR_TXE)))
        UART2->TDR = c;

    return ok;
}

//
// Writes a character to the serial output stream
//
static bool serial2PutC (const char c)
{
    uint32_t next_head;

//    if(txbuf2.head != txbuf2.tail || !serial2PutCNonBlocking(c)) {  // Try to send character without buffering...

        next_head = (txbuf2.head + 1) & (TX_BUFFER_SIZE - 1);       // .. if not, set and update head pointer

        while(txbuf2.tail == next_head) {                           // While TX buffer full
            if(!hal.stream_blocking_callback())                     // check if blocking for space,
                return false;                                       // exit if not (leaves TX buffer in an inconsistent state)
            UART2->CR1 |= USART_CR1_TXEIE;                          // Enable TX interrupts???
        }

        txbuf2.data[txbuf2.head] = c;                               // Add data to buffer
        txbuf2.head = next_head;                                    // and update head pointer

        UART2->CR1 |= USART_CR1_TXEIE;                              // Enable TX interrupts
//    }

    return true;
}

//
// Writes a null terminated string to the serial output stream, blocks if buffer full
//
static void serial2WriteS (const char *s)
{
    char c, *ptr = (char *)s;

    while((c = *ptr++) != '\0')
        serial2PutC(c);
}

//
// Writes a number of characters from a buffer to the serial output stream, blocks if buffer full
//
void serial2Write(const char *s, uint16_t length)
{
    char *ptr = (char *)s;

    while(length--)
        serial2PutC(*ptr++);
}

//
// Flushes the serial output buffer
//
void serial2TxFlush (void)
{
    UART2->CR1 &= ~USART_CR1_TXEIE;     // Disable TX interrupts
    txbuf2.tail = txbuf2.head;
}

//
// Returns number of characters pending transmission
//
uint16_t serial2TxCount (void)
{
    uint32_t tail = txbuf2.tail, head = txbuf2.head;

    return BUFCOUNT(head, tail, TX_BUFFER_SIZE) + (UART2->ISR & USART_ISR_TC ? 0 : 1);
}

//
// serialGetC - returns -1 if no data available
//
static int16_t serial2GetC (void)
{
    uint16_t bptr = rxbuf2.tail;

    if(bptr == rxbuf2.head)
        return -1; // no data available else EOF

    char data = rxbuf2.data[bptr++];             // Get next character, increment tmp pointer
    rxbuf2.tail = bptr & (RX_BUFFER_SIZE - 1);   // and update pointer

    return (int16_t)data;
}

static bool serial2SetBaudRate (uint32_t baud_rate)
{
    UART2->CR1 = USART_CR1_RE|USART_CR1_TE;
    UART2->CR3 = USART_CR3_OVRDIS;
    UART2->BRR = UART_DIV_SAMPLING16(HAL_RCC_GetPCLK2Freq(), baud_rate);
    UART2->CR1 |= (USART_CR1_UE|USART_CR1_RXNEIE);

    rxbuf.tail = rxbuf.head;
    txbuf.tail = txbuf.head;

    return true;
}

static bool serial2Disable (bool disable)
{
    if(disable)
        UART2->CR1 &= ~USART_CR1_RXNEIE;
    else
        UART2->CR1 |= USART_CR1_RXNEIE;

    return true;
}

static enqueue_realtime_command_ptr serial2SetRtHandler (enqueue_realtime_command_ptr handler)
{
    enqueue_realtime_command_ptr prev = enqueue_realtime_command2;

    if(handler)
        enqueue_realtime_command2 = handler;

    return prev;
}

const io_stream_t *serial2Init (uint32_t baud_rate)
{
    static const io_stream_t stream = {
        .type = StreamType_Serial,
        .connected = true,
        .read = serial2GetC,
        .write = serial2WriteS,
        .write_n =  serial2Write,
        .write_char = serial2PutC,
        .write_all = serial2WriteS,
        .get_rx_buffer_free = serial2RxFree,
        .get_rx_buffer_count = serial2RxCount,
        .get_tx_buffer_count = serial2TxCount,
        .reset_write_buffer = serial2TxFlush,
        .reset_read_buffer = serial2RxFlush,
        .cancel_read_buffer = serial2RxCancel,
    //    .suspend_read = serial2SuspendInput,
        .disable = serial2Disable,
        .set_baud_rate = serial2SetBaudRate,
        .set_enqueue_rt_handler = serial2SetRtHandler
    };

#if defined(NUCLEO_F756) || defined(NUCLEO_F446)

    __HAL_RCC_USART6_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStructure = {
        .Mode      = GPIO_MODE_AF_PP,
        .Pull      = GPIO_NOPULL,
        .Speed     = GPIO_SPEED_FREQ_VERY_HIGH,
        .Pin       = GPIO_PIN_6|GPIO_PIN_7,
        .Alternate = GPIO_AF8_USART6
    };
    HAL_GPIO_Init(GPIOC, &GPIO_InitStructure);

    serial2SetBaudRate(baud_rate);

    HAL_NVIC_SetPriority(USART6_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART6_IRQn);

#else

    __HAL_RCC_USART2_CLK_ENABLE();

    GPIO_InitTypeDef GPIO_InitStructure = {
      .Mode      = GPIO_MODE_AF_PP,
      .Pull      = GPIO_NOPULL,
      .Speed     = GPIO_SPEED_FREQ_VERY_HIGH,
      .Pin       = GPIO_PIN_2|GPIO_PIN_3,
      .Alternate = GPIO_AF7_USART2
    };
    HAL_GPIO_Init(GPIOA, &GPIO_InitStructure);

    serial2SetBaudRate(baud_rate);

    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

#endif

#if MODBUS_ENABLE
//  UART2->IE = EUSCI_A_IE_RXIE;
#endif

    return &stream;
}

void UART2_IRQHandler (void)
{
    if(UART2->ISR & USART_ISR_RXNE) {

        uint16_t next_head = (rxbuf2.head + 1) & (RX_BUFFER_SIZE - 1);  // Get and increment buffer pointer

        if(rxbuf2.tail == next_head) {                                  // If buffer full
            rxbuf.overflow = 1;                                         // flag overflow
            next_head = UART2->RDR;                                     // and do dummy read to clear interrupt
        } else {
            char data = UART2->RDR;
            if(!enqueue_realtime_command2(data)) {                      // Check and strip realtime commands,
                rxbuf2.data[rxbuf2.head] = data;                        // if not add data to buffer
                rxbuf2.head = next_head;                                // and update pointer
            }
        }
    }

    if((UART2->ISR & USART_ISR_TXE) && (UART2->CR1 & USART_CR1_TXEIE)) {

        uint16_t tail = txbuf2.tail;            // Get buffer pointer

        UART2->TDR = txbuf2.data[tail++];       // Send next character and increment pointer

        if(tail == TX_BUFFER_SIZE)              // If at end
            tail = 0;                           // wrap pointer around

        txbuf2.tail = tail;                     // Update global pointer

        if(tail == txbuf2.head)                 // If buffer empty then
            UART2->CR1 &= ~USART_CR1_TXEIE;     // disable UART TX interrupt
   }

    if(UART2->ISR & USART_ISR_FE)
        UART2->ICR &= USART_ICR_FECF;

    UART2->ICR = UART2->ISR;
}
#endif
