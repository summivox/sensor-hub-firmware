#ifndef _RETARGET_RETARGET_H_
#define _RETARGET_RETARGET_H_

#include <stdio.h>




#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

    struct __FILE {
        int handle;
    };

    extern FILE __stderr;
    extern FILE __stdout;
    extern FILE __stdin;
    
    void retarget_init(void);

#ifdef __cplusplus
}
#endif//__cplusplus


#endif /* _RETARGET_RETARGET_H_ */
