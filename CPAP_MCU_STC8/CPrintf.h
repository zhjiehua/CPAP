#ifndef __CPRINTF_H_
#define __CPRINTF_H_

#define LCDSIM  0//Ê¹ÓÃÄ£ÄâÒº¾§ÆÁ

//#if (LCDSIM == 0)
//#define _CDebug
//#endif

#define _CDebug

#ifdef _CDebug
	#ifdef __cplusplus
		void myCPrintf(const char *fmt, ...);
		#define cDebug(format, ...)     myCPrintf(format, ##__VA_ARGS__)
		//#define cDebug(format, args...)    printf(format, ##args)
	#else
		#include "uart.h"
		#define cDebug printf
	#endif
#else
	#ifdef __cplusplus
		#define cDebug(format, ...)
	#else
		#include "uart.h"
		//#define cDebug uart_printf_none
		#define cDebug
        #define Send2PC printf
	#endif
#endif
#endif
