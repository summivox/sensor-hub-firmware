#include "stm32f1xx_hal.h"
#include "usart.h"
#include "stdlib.h"
#include "string.h"
#include <algorithm>


static void tx_blocked(void) {
    ;
}

static void rx_blocked(void) {
    ;
}

// <<< Use Configuration Wizard in Context Menu >>>

//  <h> == README ==
//      <h> Inititialization
//          <h> Init selected USART before calling IO functions
//          </h> 
//          <h> Call `retarget_init();`
//          </h> 
//      </h>
//      <h> Functions `tx_blocked` and `rx_blocked` above can be customized
//      </h>
//      <h> This library is not re-entrant nor thread-safe. These must be implemented using your RTOS.
//      </h>
//      <h> TX behavior
//          <h> Buffer will be flushed when:
//              <h> (1) full
//              </h>
//              <h> (2) auto-flush character written
//              </h>
//              <h> (3) `fflush` called
//              </h>
//          </h>
//          <h> Buffer flush may block execution (calls `tx_blocked`)
//          </h>
//      </h>
//      <h> RX behavior
//          <h> Overflow is memory safe but content will be corrupted
//          </h>
//          <h> Underflow blocks execution (calls `rx_blocked`)
//          </h>
//          <h> Character is echoed back only after it's read
//          </h>
//      </h>
//  </h>

//  <o> USART as STDIO
//      <1=> USART1
//      <2=> USART2
//      <3=> USART3
#   define USART_N 2

//  <e> TX (stdout, stderr)
#   define TX_EN 1
//      <o> depth of double buffer
#       define TX_BUF_N 256
//      <e> auto-flush
#       define TX_AUTO_FLUSH 1
//          <o> auto-flush trigger character (e.g. 0x0A '\n') <0x00-0xff>
#           define TX_AUTO_FLUSH_CHAR ((uint8_t)(0x0A))
//          <q> transmit trigger character?
#           define TX_AUTO_FLUSH_NOCONSUME 1
//      </e>
//  </e>

//  <e> RX (stdin)
#   define RX_EN 1
//      <o> buffer depth
#       define RX_BUF_N 256
//      <e> echo back *READ* chars
#       define RX_ECHO 1
//          <q> echo CR ("\r") as CRLF ("\r\n")
#           define RX_ECHO_CRLF 1
//      </e>
//  </e>

// <<< end of configuration section >>>


////////////
// select HAL USART handle

#if USART_N == 1
#   define U huart1
#elif USART_N == 2
#   define U huart2
#elif USART_N == 3
#   define U huart3
#endif


////////////
// declarations

typedef uint8_t byte;
extern "C" {
    void retarget_impl_init(void);
    void fputc_impl_nobuf(int ch);
    void fputc_impl_buf(int ch);
    void fflush_impl(void);
    int fgetc_impl(void);
    void backspace_impl(void);
}

// circular buffer helpers
#define Q_INC(i, n) do { if (++i == (n)) i = 0; } while (0)
#define Q_DEC(i, n) do { if (i-- == (0)) i = (n) - 1; } while (0)


////////////
// buffers

#ifdef TX_EN
// TX double buffer
static byte tx_buf[2][TX_BUF_N];
static byte *tx_p, *tx_p_curr, *tx_p_end; //write buffer
static byte *tx_p_next; //transfer buffer
#endif//TX_EN

#ifdef RX_EN
// RX ring buffer
static byte rx_buf[RX_BUF_N];
static uint32_t rx_head = 0; //index of element to be read
static uint32_t rx_bs_n; //number of "un-read" characters in buffer
#endif//RX_EN


////////////
// initialization

void retarget_impl_init(void) {
#ifdef TX_EN
    // TX double buffer
    tx_p_curr = tx_p = tx_buf[0];
    tx_p_end = tx_p_curr + TX_BUF_N;
    tx_p_next = tx_buf[1];

    // TX DMA : peripheral byte -> memory byte (increment)
    HAL_DMA_DeInit(U.hdmatx);
    U.hdmatx->Init.Direction = DMA_MEMORY_TO_PERIPH;
    U.hdmatx->Init.PeriphInc = DMA_PINC_DISABLE;
    U.hdmatx->Init.MemInc = DMA_MINC_ENABLE;
    U.hdmatx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    U.hdmatx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    U.hdmatx->Init.Mode = DMA_NORMAL;
    HAL_DMA_Init(U.hdmatx);
#endif//TX_EN

#ifdef RX_EN
    // RX ring buffer
    rx_head = 0;

    // RX DMA : peripheral byte -> memory byte (circular increment)
    HAL_DMA_DeInit(U.hdmarx);
    U.hdmarx->Init.Direction = DMA_PERIPH_TO_MEMORY;
    U.hdmarx->Init.PeriphInc = DMA_PINC_DISABLE;
    U.hdmarx->Init.MemInc = DMA_MINC_ENABLE;
    U.hdmarx->Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    U.hdmarx->Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    U.hdmarx->Init.Mode = DMA_CIRCULAR;
    HAL_DMA_Init(U.hdmarx);

    // start transfer right away
    HAL_UART_Receive_DMA(&U, rx_buf, RX_BUF_N);
#endif//RX_EN
}


////////////
// TX impl


// non-buffered (stderr)
void fputc_impl_nobuf(int ch) {
    static uint8_t ch_;
    ch_ = ch;
    while (HAL_UART_Transmit(&U, &ch_, 1, HAL_MAX_DELAY) != HAL_OK)
        tx_blocked();
}

// buffered (stdout)
void fputc_impl_buf(int ch) {
    if (TX_AUTO_FLUSH && ch == TX_AUTO_FLUSH_CHAR) {
        if (TX_AUTO_FLUSH_NOCONSUME) *tx_p++ = ch;
        fflush_impl();
    } else {
        *tx_p++ = ch;
        if (tx_p == tx_p_end) fflush_impl();
    }
}

void fflush_impl() {
    // wait for existing transfer to finish
    while (!(U.Instance->SR & USART_SR_TC)) tx_blocked();

    // empty buffer needs no flushing
    if (tx_p == tx_p_curr) return;

    // setup transfer
    HAL_UART_Transmit_DMA(&U, tx_p_curr, tx_p - tx_p_curr);

    // swap and reset buffer
    byte* p = tx_p_curr;
    tx_p_curr = tx_p_next;
    tx_p_next = p;
    tx_p = tx_p_curr;
    tx_p_end = tx_p_curr + TX_BUF_N;
}


////////////
// RX impl

static void echo(byte ch) {
    fputc_impl_nobuf(ch);
    // NOTE: This is a simplification of my original code (which does NOT use STM32 HAL)
    // in that synchronization between buf/nobuf is handled by the HAL library.
    if (RX_ECHO_CRLF && ch == '\r') {
        fputc_impl_nobuf('\n');
    }
}

int fgetc_impl() {
    // check DMA internal counter to see if ring buffer is empty
    while (rx_head == (RX_BUF_N - U.hdmarx->Instance->CNDTR))
        rx_blocked();

    // read from ring buffer
    byte ch = rx_buf[rx_head];
    Q_INC(rx_head, RX_BUF_N);

    // echo and backspace handling: one char is echoed only when it's first read
    if (rx_bs_n > 0) {
        --rx_bs_n;
    } else if (RX_ECHO) {
        echo(ch);
    }
    
    return ch;
}

void backspace_impl() {
    // backtrack ring buffer
    // We can do this because overflow behavior is undefined anyway
    Q_DEC(rx_head, RX_BUF_N);
    ++rx_bs_n;
}
