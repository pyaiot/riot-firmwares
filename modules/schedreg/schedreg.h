
#ifndef SCHEDREG_H
#define SCHEDREG_H

#include <inttypes.h>

#include "msg.h"
#include "xtimer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Message offset for schedreg msg.type
 */
#define SCHEDREG_TYPE           (0xC00)          

/**
 * @brief Signature for the sched callback
 *
 * @param[in] arg           optional argument which is passed to the
 *                          callback
 */
typedef void (*sched_cb_t)(void *arg);

/**
 * @brief   Schedule entry
 */
typedef struct schedreg {
    struct schedreg* next;         /**< pointer to next entry */
    sched_cb_t cb;                 /**< cb to execute upon message*/
    void* arg;                     /**< cb args */
    msg_t* msg;                    /**< msg to send */
    xtimer_t* xtimer;              /**< xtimer to schedule msg send */
    uint32_t period;               /**< message period */
} schedreg_t;

/**
 * @brief   Registers a thread to the registry.
 *
 * @param[in] entry     An entry you want to add to the registry.
 * @param[in] pid    the PID of that will handle scheduling
 *
 * @warning Call schedreg_unregister() *before* you leave the context you
 *          allocated @p entry in. Otherwise it might get overwritten.
 *
 * @return  0 on success
 * @return  -EINVAL if invalid entry
 */
int schedreg_register(schedreg_t *entry, kernel_pid_t pid);

/**
 * @brief   Removes an entry from registry
 *
 * @param[in] entry     An entry you want to remove from the registry.
 */
void schedreg_unregister(schedreg_t *entry);


/**
 * @name    Static entry initialization macros
 * @anchor  schedreg_init_static
 * @{
 */
/**
 * @brief   Initializes a schedreg entry
 *
 * @param[in] type      The @ref schedreg_type_t for the schedreg entry
 * @param[in] pid       The PID of the registering thread
 *
 * @return  An initialized schedreg entry
 */
#define SCHEDREG_INIT(cb, arg, msg, xtimer, period)  { NULL, cb, arg, msg, xtimer, period}

/**
 * @name    Dynamic entry initialization functions
 * @anchor  schedreg_init_dyn
 * @{
 */
/**
 * @brief   Initializes a schedreg entry dynamically with PID
 *
 * @param[out] entry    A schedreg entry
 * @param[in] cb        The cb to be executed
 * @param[in] arg       The args for the callback
 * @param[in] msg       Pointer to msg_t to send to self
 * @param[in] xtimer    Pointer to xtimer that will handle sheduling msg send
 * @param[in] period    The time period in us for the cb execution
 * 
 */
static inline void schedreg_init_pid(schedreg_t *entry,
                                         sched_cb_t cb,
                                         void* arg,
                                         msg_t* msg,
                                         xtimer_t* xtimer,
                                         uint32_t period)
{
    entry->next = NULL;
    entry->cb = cb;
    entry->arg = arg;
    entry->msg = msg;
    entry->xtimer = xtimer;
    entry->period = period;
}

/**
 * @brief   Executes the n element callback and re-schedule
 *
 * @param[in] n      nth element in linked list
 * @param[in] pid    the PID of that will handle scheduling
 *
 */
int schedreg_resched(int n, kernel_pid_t pid);

/**
 * @brief   Inits schedreg as main thread
 * 
 * @param[out] pid    pid of thread
 */
int init_schedreg_thread(void);

#ifdef __cplusplus
}
#endif

#endif
