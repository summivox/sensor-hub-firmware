#ifndef _MISC_HPP_
#define _MISC_HPP_


////////////
// token paste

#ifdef BOOST_JOIN

#define TOKEN_PASTE(A, B) BOOST_JOIN(A, B)

#else

#define TOKEN_PASTE_2(A, B) A##B
#define TOKEN_PASTE_1(A, B) TOKEN_PASTE_2(A, B)
#define TOKEN_PASTE(A, B) TOKEN_PASTE_1(A, B)

#endif//BOOST_JOIN


////////////
// misc

#define ALIGN32 __attribute__((aligned (4)))

#define SHORT_DELAY(n) for (volatile int i = n ; i --> 0 ; )



////////////
// dumbed down direct GPIO output

#define PBIT(addr, bit) (( (uint32_t)(addr) &0xF0000000)+0x02000000+(((( (uint32_t)(addr) &0xFFFFF)<<3)+ (bit)) <<2))
#define SBIT(addr, bit) (*(volatile uint32_t *)(PBIT(addr, bit)))

#define PO(port, pin) SBIT(&(GPIO##port->ODR), pin)


#endif//_MISC_HPP_
