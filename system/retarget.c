#include <stdio.h>
#include <rt_misc.h>
#include "retarget.h"

#pragma import(__use_no_semihosting_swi)

extern void retarget_impl_init(void);
extern void fputc_impl_nobuf(int ch);
extern void fputc_impl_buf(int ch);
extern void fflush_impl(void);
extern int fgetc_impl(void);
extern void backspace_impl(void);

FILE __stderr;
FILE __stdout;
FILE __stdin;

void retarget_init() {
    __stderr.handle = -1;
    __stdout.handle = -2;
    __stdin.handle = -3;
    retarget_impl_init();
}

int fputc(int ch, FILE* f) {
    switch (f->handle) {
        case -1:
            fputc_impl_nobuf(ch);
            return 0;
        case -2:
            fputc_impl_buf(ch);
            return 0;
        default:
            return -1;
    }
}
void _ttywrch(int ch) {
    fputc_impl_nobuf(ch);
}
int fflush(FILE* f) {
    switch (f->handle) {
        case -1:
        case -2:
            fflush_impl();
            return 0;
        default:
            return -1;
    }
}
int fgetc (FILE *f) {
    switch (f->handle) {
        case -3:
            return fgetc_impl();
        default:
            return -1;
    }
}
void __backspace(FILE* f) {
    switch (f->handle) {
        case -3:
            backspace_impl();
        default:
            return;
    }
}

int fclose(FILE* f) { f->handle = 0; return EOF; }
int ferror(FILE* f) { return EOF; }
int fseek (FILE* f, long nPos, int nMode) { return 0; }
void _sys_exit(int return_code) { while(1); }
