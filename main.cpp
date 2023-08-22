#include<iostream>
#include"telegram.h"
int main(){
    std::cout<<"HELLO\n";
    TG tg;
    
    tg.Loop();
    
    return 0;
}