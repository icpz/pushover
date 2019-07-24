#ifndef __PUSHOVER_CLIENT_H__
#define __PUSHOVER_CLIENT_H__

#define PUSHOVER_DEFAULT_API_URL "https://api.pushover.net/1/messages.json"

typedef struct pushover_s pushover_t;

typedef enum {
    PUSHOVER_PRIOR_LOWEST = -2,
    PUSHOVER_PRIOR_LOW    = -1,
    PUSHOVER_PRIOR_NORMAL =  0,
    PUSHOVER_PRIOR_HIGH   =  1,
    PUSHOVER_PRIOR_EMERG  =  2,
} PUSHOVER_PRIOR;

void pushover_global_init(void);
pushover_t *pushover_new(void);
void pushover_drop(pushover_t *c);
void pushover_global_cleanup(void);

void pushover_send(pushover_t *c, int *http_ok);
int pushover_last_errors_length(pushover_t *c);
const char *pushover_last_errors(pushover_t *c, int idx);
const char *pushover_last_request_id(pushover_t *c);

/* required fields */
void pushover_set_token(pushover_t *c, const char *token);
void pushover_set_user(pushover_t *c, const char *user);
void pushover_set_message(pushover_t *c, const char *message);

/* optional fields */
void pushover_set_url(pushover_t *c, const char *url);
void pushover_set_url_title(pushover_t *c, const char *url_title);
void pushover_set_title(pushover_t *c, const char *title);
void pushover_set_device(pushover_t *c, const char *device);
void pushover_set_priority(pushover_t *c, PUSHOVER_PRIOR prior);
void pushover_set_sound(pushover_t *c, const char *sound);
void pushover_set_timestamp(pushover_t *c, long timestamp);
void pushover_set_api_url(pushover_t *c, const char *url);

#endif /* __PUSHOVER_CLIENT_H__ */
