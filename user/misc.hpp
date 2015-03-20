#ifndef _MISC_HPP_
#define _MISC_HPP_

#define ALIGN32 __attribute__((aligned (4)))

#define SHORT_DELAY(n) for (volatile int i = n ; i --> 0 ; )
#define PBIT(addr, bit) (( (uint32_t)(addr) &0xF0000000)+0x02000000+(((( (uint32_t)(addr) &0xFFFFF)<<3)+ (bit)) <<2))
#define SBIT(addr, bit) (*(volatile uint32_t *)(PBIT(addr, bit)))


#define PO(port, pin) SBIT(&(GPIO##port->ODR), pin)

#define DBG0 PO(A, 0)
#define DBG1 PO(A, 1)
#define DBG3 PO(A, 3)

#define DBG_NSS DBG3

#endif//_MISC_HPP_
