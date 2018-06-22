#include <stdio.h>
#include "log.h"

int main(int argc, char *argv[])
{ 
    log_init(LOGGER_INFO,0); 
       
    info("111111111111");
    debug("22222222222");
    error("33333333333"); 
       
    log_deinit();
    
    return 0; 
}