#include <stdlib.h>
#include <memory.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include <pushover.h>

struct pushover_s {
    CURL *curl;
    struct curl_slist *http_header;
    cJSON *request;
    cJSON *response;
    cJSON *errors; /* no need to free */
    char *req_buf;
    char resp_buf[2048];
    int len;
};

static size_t curl_write_cb(char *ptr, size_t size, size_t nmemb, void *priv);
static void pushover_reset_request_buffer(pushover_t *c);

void pushover_global_init(void) {
    curl_global_init(CURL_GLOBAL_ALL);
}

void pushover_global_cleanup(void) {
    curl_global_cleanup();
}

pushover_t *pushover_new(void) {
    pushover_t *c;
    CURLcode ret;

    c = (pushover_t *)malloc(sizeof *c);
    memset(c, 0, sizeof *c);

    c->http_header = NULL;
    c->http_header = curl_slist_append(c->http_header, "Content-Type: application/json");
    if (!c->http_header) {
        goto __curl_slist_err;
    }

    c->curl = curl_easy_init();
    if (!c->curl) {
        goto __curl_handle_err;
    }
    ret = curl_easy_setopt(c->curl, CURLOPT_URL, PUSHOVER_DEFAULT_API_URL);
    if (ret != CURLE_OK) {
        goto __curl_err;
    }
    ret = curl_easy_setopt(c->curl, CURLOPT_WRITEFUNCTION, curl_write_cb);
    if (ret != CURLE_OK) {
        goto __curl_err;
    }
    ret = curl_easy_setopt(c->curl, CURLOPT_WRITEDATA, (void *)c);
    if (ret != CURLE_OK) {
        goto __curl_err;
    }
    ret = curl_easy_setopt(c->curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (ret != CURLE_OK) {
        goto __curl_err;
    }
    ret = curl_easy_setopt(c->curl, CURLOPT_HTTPHEADER, c->http_header);
    if (ret != CURLE_OK) {
        goto __curl_err;
    }

    c->request = cJSON_CreateObject();
    if (!c->request) {
        goto __cjson_req_err;
    }

    return c;

__cjson_req_err:
__curl_err:
    curl_easy_cleanup(c->curl);
__curl_handle_err:
    curl_slist_free_all(c->http_header);
__curl_slist_err:
    return NULL;
}

void pushover_drop(pushover_t *c) {
    if (!c) {
        return;
    }

    curl_easy_cleanup(c->curl);
    curl_slist_free_all(c->http_header);
    cJSON_Delete(c->request);
    if (c->response) {
        cJSON_Delete(c->response);
    }
    pushover_reset_request_buffer(c);
    free(c);
}

void pushover_send(pushover_t *c, int *http_ok) {
    CURLcode ret;
    cJSON *node;

    *http_ok = 0;
    if (!c->req_buf) {
        c->req_buf = cJSON_PrintUnformatted(c->request);
    }
    ret = curl_easy_setopt(c->curl, CURLOPT_POSTFIELDSIZE, strlen(c->req_buf));
    if (ret != CURLE_OK) {
        return;
    }
    ret = curl_easy_setopt(c->curl, CURLOPT_POSTFIELDS, c->req_buf);
    if (ret != CURLE_OK) {
        return;
    }

    ret = curl_easy_perform(c->curl);
    if (ret != CURLE_OK) {
        return;
    }

    if (c->response) {
        cJSON_Delete(c->response);
        c->response = NULL;
    }
    c->response = cJSON_Parse(c->resp_buf);
    if (!c->response) {
        return;
    }

    node = cJSON_GetObjectItemCaseSensitive(c->response, "request");
    if (!node || !cJSON_IsString(node)) {
        return;
    }
    node = cJSON_GetObjectItemCaseSensitive(c->response, "status");
    if (!node || !cJSON_IsNumber(node)) {
        return;
    }
    c->errors = NULL;
    if (node->valueint != 1) {
        c->errors = cJSON_GetObjectItemCaseSensitive(c->response, "errors");
        if (!(c->errors && cJSON_IsArray(c->errors) &&
              (cJSON_GetArraySize(c->errors) > 0) &&
              cJSON_IsString(cJSON_GetArrayItem(c->errors, 0)))) {
            return;
        }
    }
    *http_ok = 1;
}

int pushover_last_errors_length(pushover_t *c) {
    if (c->errors) {
        return cJSON_GetArraySize(c->errors);
    }
    return 0;
}

const char *pushover_last_errors(pushover_t *c, int idx) {
    if (c->errors) {
        return cJSON_GetArrayItem(c->errors, idx)->valuestring;
    }
    return NULL;
}

const char *pushover_last_request_id(pushover_t *c) {
    return cJSON_GetObjectItemCaseSensitive(c->response, "request")->valuestring;
}

#define DEFINE_PUSHOVER_SET_FUNCTION(arg) \
    void pushover_set_ ## arg (pushover_t *c, const char *arg) { \
        pushover_reset_request_buffer(c); \
        if (!arg || strlen(arg) == 0) { \
            cJSON_DeleteItemFromObject(c->request, #arg); \
        } else { \
            cJSON_AddStringToObject(c->request, #arg, arg); \
        } \
    }

DEFINE_PUSHOVER_SET_FUNCTION(token);
DEFINE_PUSHOVER_SET_FUNCTION(user);
DEFINE_PUSHOVER_SET_FUNCTION(message);

DEFINE_PUSHOVER_SET_FUNCTION(url);
DEFINE_PUSHOVER_SET_FUNCTION(url_title);
DEFINE_PUSHOVER_SET_FUNCTION(title);
DEFINE_PUSHOVER_SET_FUNCTION(device);
DEFINE_PUSHOVER_SET_FUNCTION(sound);

#undef DEFINE_PUSHOVER_SET_FUNCTION

void pushover_set_priority(pushover_t *c, PUSHOVER_PRIOR prior) {
    pushover_reset_request_buffer(c);
    cJSON_AddNumberToObject(c->request, "priority", (int)prior);
}

void pushover_set_timestamp(pushover_t *c, long timestamp) {
    pushover_reset_request_buffer(c);
    cJSON_AddNumberToObject(c->request, "timestamp", timestamp);
}

void pushover_set_api_url(pushover_t *c, const char *url) {
    curl_easy_setopt(c->curl, CURLOPT_URL, url);
}

static void pushover_reset_request_buffer(pushover_t *c) {
    if (c->req_buf) {
        free(c->req_buf);
        c->req_buf = NULL;
    }
}

static size_t curl_write_cb(char *ptr, size_t sz, size_t nmemb, void *priv) {
    pushover_t *c = (pushover_t *)priv;
    size_t total = sz * nmemb;
    strncat(c->resp_buf, ptr, (sizeof c->resp_buf) - c->len);
    c->len = strlen(c->resp_buf);
    return total;
}
