#include<iostream>
#include"telegram.h"
int main(){
    curl_global_init(CURL_GLOBAL_ALL);
    std::cout<<"HELLO\n";
    TG tg;
    
    tg.Loop();
    curl_global_cleanup();
    return 0;
}