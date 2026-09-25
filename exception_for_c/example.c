#include<stdlib.h>
#include".exceptions.h"
#include<stdio.h>
void function1(){
    throw(ValueError);
}
int main(){
    exception_init;
    try{
        function1();
    }
    catch(ValueError){
        exception_clear;
        puts("get ValueError");
    }
    try_end;
    return 0;
}
