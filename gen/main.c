#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "log.h"
#include "simple.h"

int32_t main(int argc,char* argv[])
{
    int32_t ret;
    struct simple* psimple;
    ret = create_init_simple(&psimple);
    if(ret < 0){
        DBG(DBG_ERR,"create init simple error:%d\n",ret);
        return ret;
    }
    DBG(DBG_DEBUG,"create simple success\n");
    release_destroy_simple(psimple);
    DBG(DBG_DEBUG,"test success\n");
    return 0;
}
