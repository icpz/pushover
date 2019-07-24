#include <stdio.h>
#include <stdlib.h>

#include "pushover.h"

int main() {
    int http_ok;
    int i, errnum;
    pushover_t *p;

    pushover_global_init();

    p = pushover_new();

    pushover_set_user(p, getenv("PUSHOVER_USER"));
    pushover_set_token(p, getenv("PUSHOVER_TOKEN"));
    pushover_set_message(p, "test client");

    pushover_set_device(p, "iphone-xs");
    pushover_set_priority(p, PUSHOVER_PRIOR_NORMAL);

    pushover_send(p, &http_ok);
    if (!http_ok) {
        fprintf(stderr, "http failed\n");
    } else {
        fprintf(stderr, "request: %s\n", pushover_last_request_id(p));
        errnum = pushover_last_errors_length(p);
        if (!errnum) {
            fprintf(stderr, "post succ\n");
        }
        for (i = 0; i < errnum; ++i) {
            fprintf(stderr, "error %d: %s\n", i, pushover_last_errors(p, i));
        }
    }

    pushover_drop(p);

    pushover_global_cleanup();

    return 0;
}

