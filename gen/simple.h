#ifndef __SIMPLE_H__ 
#define __SIMPLE_H__
#include <stdint.h>
#include "list.h"
#include "lock.h"

#include "wait.h"

struct simple;

enum SIMPLE_EVENT
{
    SIMPLE_EVENT_NONE = 0,
    SIMPLE_EVENT_MAX,
};

typedef int32_t (*simple_event_notify)(struct simple*, int32_t, void*, void*);

struct simple_event_action
{
    simple_event_notify notify;
    void* object;
    struct simple_event_action* next;
};

struct simple_operation
{
    int32_t (*init)(struct simple*);
    int32_t (*release)(struct simple*);
    int32_t (*register_notify)(struct simple*, int32_t, simple_event_notify notify, void*);
    int32_t (*unregister_notify)(struct simple*, int32_t, void*);
    int32_t (*trigger_notify)(struct simple*, int32_t, void*);
};

struct simple
{
    struct list_head head;
    wait_t wait;
    lock_t lock;
    struct simple_operation* op;
    struct simple_event_action *paction[SIMPLE_EVENT_MAX];
};

int32_t create_init_simple(struct simple** psimple);
void release_destroy_simple(struct simple* psimple);

#endif
