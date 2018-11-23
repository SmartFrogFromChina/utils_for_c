#include <stdio.h>
//#include "log.h"
#include "pretty_log.h"

int main(int argc, char *argv[])
#if 0
{ 
    log_init(LOGGER_INFO,0); 
    
       
    info("111111111111");
    debug("22222222222");
    error("33333333333"); 
       
    log_deinit();
    
    return 0; 
}
#else
{
    pretty_log_init(argc,argv);
    
    debug("111111111111");
    info("222222222222");
    warn("333333333333"); 
    error("444444444444");
    fatal("555555555555");
    raw("666666666666\n");
    
    return 0;
}
#endif