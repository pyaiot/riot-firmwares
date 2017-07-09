#include <inttypes.h>
#include <stdlib.h>

#include "net/emcute.h"

#include "mqtt_utils.h"

#define ENABLE_DEBUG (1)
#include "debug.h"

int publish(uint8_t *topic, uint8_t *payload)
{
    emcute_topic_t t;
    unsigned flags = EMCUTE_QOS_1;

    DEBUG("[DEBUG] Publish with topic: %s, data: %s and flags: 0x%02x\n",
          topic, payload, (int)flags);

    t.name = (char*)topic;
    if (emcute_reg(&t) != EMCUTE_OK) {
        DEBUG("[ERROR] Unable to obtain topic %s\n", t.name);
        return 1;
    }

    /* step 2: publish data */
    if (emcute_pub(&t, (char*)payload,
                   strlen((char*)payload), flags) != EMCUTE_OK) {
        DEBUG("[ERROR] Unable to publish data to topic '%s [%i]'\n",
              t.name, (int)t.id);
        return 1;
    }

    DEBUG("[DEBUG] Published %i bytes to topic '%s [%i]'\n",
          (int)strlen((char*)payload), t.name, t.id);

    return 0;
}
