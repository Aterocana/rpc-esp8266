#include <stdio.h>

#include "common/cs_dbg.h"
#include "common/json_utils.h"
#include "common/platform.h"
#include "frozen/frozen.h" //JSON parser
#include "fw/src/mgos_app.h"
#include "fw/src/mgos_rpc.h"
#include "fw/src/mgos_wifi.h"
#include "fw/src/mgos_gpio.h"

static int total_sum = 0;

static void on_wifi_event(enum mgos_wifi_status event, void *data)
{
    (void) data;
    switch (event) {
        case MGOS_WIFI_IP_ACQUIRED:
            printf("IP acquired.\n");
            break;
        case MGOS_WIFI_CONNECTED:
            printf("Connected to WiFi.\n");
            break;
        case MGOS_WIFI_DISCONNECTED:
            printf("Disconnected from WiFi.\n");
            break;
    }
}

/*
    It takes the parameter and it adds it to the current total sum, then the total sum is returned.
 */
static void sum(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args)
{
    struct mbuf fb;
    struct json_out out = JSON_OUT_MBUF(&fb);
    mbuf_init(&fb, 20);

    int num = 0;
    if(json_scanf(args.p, args.len, ri->args_fmt, &num) == 1){
        printf(args.p);
        total_sum+=num;
        json_printf(&out, "{sum: %d}", total_sum);
    }
    else{
        json_printf(&out, "{error: %Q}", "num is required.");
    }
    mg_rpc_send_responsef(ri, "%.*s", fb.len, fb.buf);
    ri = NULL;
    mbuf_free(&fb);

    (void) cb_arg;
    (void) fi;
}

static void add(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args)
{
    struct mbuf fb;
    struct json_out out = JSON_OUT_MBUF(&fb);
    mbuf_init(&fb, 20);

    int addend1 = 0;
    int addend2 = 0;
    if(json_scanf(args.p, args.len, "{add1:%d, add2:%d}", &addend1, &addend2) == 2){
        printf("ADDENDS: %d, %d\n", addend1, addend2);
        json_printf(&out, "{sum: %d}", addend1+addend2);
    }
    else{
        json_printf(&out, "{error: %Q}", "both addends are required.");
    }
    mg_rpc_send_responsef(ri, "%.*s", fb.len, fb.buf);
    ri = NULL;
    mbuf_free(&fb);

    (void) cb_arg;
    (void) fi;
}

static void toggle(struct mg_rpc_request_info *ri, void *cb_arg, struct mg_rpc_frame_info *fi, struct mg_str args)
{
    struct mbuf fb;
    struct json_out out = JSON_OUT_MBUF(&fb);
    mbuf_init(&fb, 20);

    int pin = 0;
    if(json_scanf(args.p, args.len, ri->args_fmt, &pin) == 1){
        mgos_gpio_set_mode(pin, MGOS_GPIO_MODE_OUTPUT);
        json_printf(&out, "{status: %d}", mgos_gpio_toggle(pin));

    }
    else{
        json_printf(&out, "{error: %Q}", "pin is required.");
    }
    mg_rpc_send_responsef(ri, "%.*s", fb.len, fb.buf);
    ri = NULL;
    mbuf_free(&fb);

    (void) cb_arg;
    (void) fi;
}

enum mgos_app_init_result mgos_app_init(void)
{
    mgos_wifi_add_on_change_cb(on_wifi_event, 0);

    struct mg_rpc *c = mgos_rpc_get_global();
    mg_rpc_add_handler(c, "sum", "{num: %d}", sum, NULL);
    mg_rpc_add_handler(c, "add", "{add1: %d, add2: %d}", add, NULL);
    mg_rpc_add_handler(c, "pin", "{pin: %d}", toggle, NULL);

    return MGOS_APP_INIT_SUCCESS;
}
