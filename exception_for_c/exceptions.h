#ifndef C_EXCEPTIONS_H
#define C_EXCEPTIONS_H 1
#include<setjmp.h>
#include<stdint.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
jmp_buf __try_point__[128]={};
uint8_t __curr__=0;
bool __at_exception__=false,__exception_cl__=false;
const char* __exception_name__;
#define try if(!setjmp(__try_point__[__curr__++]))
#define catch(exception) else if(__at_exception__&&(!strcmp(__exception_name__,(#exception))||!strcmp("Exception",(#exception))))
#define exception_clear {__at_exception__ = false;__exception_cl__ = true;++__curr__;}
#define throw(...) if(true){__at_exception__ = true;__exception_name__=*(#__VA_ARGS__)?#__VA_ARGS__:"Exception";if(__exception_cl__)__curr__--;__exception_cl__=false;longjmp(__try_point__[--__curr__],1);}else;
#define finally if(true)
#define try_end if(__at_exception__){longjmp(__try_point__[--__curr__],1);}else{__exception_cl__=false,--__curr__;}
#define exception_init if(setjmp(__try_point__[__curr__++])){fprintf(stderr, "unhandled exception: %s\n", __exception_name__);exit(-1);}
#endif
