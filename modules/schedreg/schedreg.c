#include <errno.h>
#include <inttypes.h>
#include <stdio.h>

#include "utlist.h"

#include "schedreg.h"
#include "thread.h"

#define ENABLE_DEBUG (0)
#include "debug.h"

#define SCHEDREG_QUEUE_SIZE  (4U)

static msg_t _schedreg_msg_queue[SCHEDREG_QUEUE_SIZE];
static char schedreg_stack[THREAD_STACKSIZE_DEFAULT];

/**
 * @brief   Keep the head of the registers thread as global variable
 */
schedreg_t *schedreg = NULL;

static schedreg_t *_schedreg_find_nth(int pos)
{
    schedreg_t *tmp = schedreg;

    for (int i = 0; (i < pos) && tmp; i++) {
        tmp = tmp->next;
    }
    return tmp;
}

static int _schedreg_num(void)
{
    int num = 0;
    schedreg_t *tmp = schedreg;
    while(tmp != NULL) {
        tmp = tmp->next;
        num++;
    }
    return num;
}

static void _schedreg_update(void)
{
    schedreg_t *tmp = schedreg;
    int count = _schedreg_num();

    for (int i = 0; (i < count); i++) {
        tmp->msg->type = i + SCHEDREG_TYPE; 
        tmp = tmp->next;
    }
}

int schedreg_register(schedreg_t *entry, kernel_pid_t pid)
{
    int count = _schedreg_num();
    if( count >= 0xFF) {
        DEBUG("[DEBUG] schedreg: registry full \n");        
        return 1;
    }
    entry->msg->type = SCHEDREG_TYPE + count;
    LL_APPEND(schedreg, entry);
    schedreg_resched(count, pid);
    return 0;
}

void schedreg_unregister(schedreg_t *entry)
{
    LL_DELETE(schedreg, entry);
    _schedreg_update();
}

int schedreg_resched(int n, kernel_pid_t pid)
{
    schedreg_t *tmp = _schedreg_find_nth(n);
    if(tmp) {
        DEBUG("[DEBUG] schedreg: re-scheduling entry %d in %lu \n", n, tmp->period);        
        xtimer_set_msg (tmp->xtimer, tmp->period, tmp->msg, pid);
        tmp->cb(tmp->arg);
        return 0;
    }
    else {
        DEBUG("[DEBUG] schedreg:  entry %d not found\n", n);        
        return 1;
    }
}

static void *schedreg_thread(void *args)
{
    (void) args;
    msg_init_queue(_schedreg_msg_queue, SCHEDREG_QUEUE_SIZE);
    msg_t msg;

    while(msg_receive(&msg))
    {
        DEBUG("[DEBUG] schedreg: msg received \n");
        if(msg.type >= SCHEDREG_TYPE ) {
            schedreg_resched(msg.type - SCHEDREG_TYPE, thread_getpid());
        }
        else {
            DEBUG("[DEBUG] schedreg: unknown msg typereceived\n");        
        }
    }

    return NULL;
}

int init_schedreg_thread(void)
{
    int schedreg_pid = thread_create(schedreg_stack, sizeof(schedreg_stack),
                                   THREAD_PRIORITY_MAIN - 1,
                                   THREAD_CREATE_STACKTEST, schedreg_thread,
                                   NULL, "Schedreg thread");
    if (schedreg_pid == -EINVAL || schedreg_pid == -EOVERFLOW) {
        puts("Error: failed to create schedreg thread, exiting\n");
        return schedreg_pid;
    }
    else {
        puts("Successfuly created schedreg thread !\n");
        return schedreg_pid;
    }
}