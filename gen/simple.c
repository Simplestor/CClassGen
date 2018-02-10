#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "log.h"
#include "simple.h"

static int32_t simple_init(struct simple* psimple)
{
    INIT_LIST_HEAD(&(psimple->head));
    lock_init(&(psimple->lock));
    wait_init(&(psimple->wait));
    return 0;
}

static int32_t simple_release(struct simple* psimple)
{
    lock_destroy((&psimple->lock));
    wait_destroy((&psimple->wait));
    return 0;
}

static int32_t simple_register_notify(struct simple* psimple, int32_t event, simple_event_notify notify, void* object)
{
    struct simple_event_action* paction;
    if(!notify || (event <= SIMPLE_EVENT_NONE) || (event >= SIMPLE_EVENT_MAX)){
        return -EINVAL;
    }
    paction = (struct simple_event_action*)malloc(sizeof(struct simple_event_action));
    if(!paction){
        DBG(DBG_ERR,"malloc error\n");
        return -ENOMEM;
    }
    paction->notify = notify;
    paction->object = object;
    lock(&(psimple->lock));
    paction->next = psimple->paction[event];
    psimple->paction[event] = paction;
    unlock(&(psimple->lock));
    return 0;
}

static int32_t simple_unregister_notify(struct simple* psimple, int32_t event, void* object)
{
    struct simple_event_action *paction,* ptmp;
    if((event <= SIMPLE_EVENT_NONE) || (event >= SIMPLE_EVENT_MAX)){
        return -EINVAL;
    }
    lock(&(psimple->lock));
    paction = psimple->paction[event];
    if(paction->object == object){
        psimple->paction[event] = paction->next;
        free(paction);
    }else{
        while(paction->next){
            if(paction->next->object == object){
                ptmp = paction->next;
                paction->next = ptmp->next;
                free(ptmp);
                break;
            }
            paction = paction->next;
        }
    }
    unlock(&(psimple->lock));
    return 0;
}

static int32_t simple_trigger_notify(struct simple* psimple, int32_t event, void* context)
{
    struct simple_event_action* paction;
    if((event <= SIMPLE_EVENT_NONE) || (event >= SIMPLE_EVENT_MAX)){
        return -EINVAL;
    }
    paction = psimple->paction[event];
    while(paction){
        paction->notify(psimple, event, paction->object, context);
        paction = paction->next;
    }
    return 0;
}

static struct simple_operation simple_op =
{
    .init = simple_init,
    .release = simple_release,
    .register_notify = simple_register_notify,
    .unregister_notify = simple_unregister_notify,
    .trigger_notify = simple_trigger_notify,
};

int32_t create_init_simple(struct simple** psimple)
{
    int32_t ret;
    struct simple* ptmp;
    (*psimple) = (struct simple*)malloc(sizeof(struct simple));
    if(!(*psimple)){
        DBG(DBG_ERR,"malloc error\n");
        return -ENOMEM;
    }
    ptmp = *psimple;
    memset(ptmp,0,sizeof(struct simple));
    ptmp->op = &simple_op;
    ret = ptmp->op->init(ptmp);
    if(ret < 0){
        DBG(DBG_ERR,"init error\n");
        release_destroy_simple(ptmp);
        return ret;
    }
    return 0;
}

void release_destroy_simple(struct simple* psimple)
{
    if(psimple){
        psimple->op->release(psimple);
        free(psimple);
    }
}

