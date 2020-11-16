#ifndef _APPLICATION_H
#define _APPLICATION_H

#ifndef VERSION
#define VERSION "vdev"
#endif

#include <bcl.h>

typedef struct {
    uint8_t channel;
    float value;
    bc_tick_t next_pub;
} event_param_t;

typedef struct {
    bc_tag_barometer_t self;
    event_param_t param;
} barometer_tag_t;

typedef struct {
    bc_tag_humidity_t self;
    event_param_t param;
} humidity_tag_t;

typedef struct {
    bc_tmp112_t self;
    event_param_t param;
} internal_temp_t;

typedef struct {
    bc_tag_lux_meter_t self;
    event_param_t param;
} lux_meter_tag_t;

typedef struct {
    bc_tag_temperature_t self;
    event_param_t param;
} temperature_tag_t;

typedef struct {
    bc_tag_voc_lp_t self;
    event_param_t param;
} voc_lp_tag_t;


static void barometer_tag_init(bc_i2c_channel_t i2c_channel, barometer_tag_t *tag);
static void humidity_tag_init(bc_tag_humidity_revision_t revision, bc_i2c_channel_t i2c_channel, humidity_tag_t *tag);
static void internal_temp_init(bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address, internal_temp_t *tag);
static void lux_meter_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_lux_meter_i2c_address_t i2c_address, lux_meter_tag_t *tag);
static void temperature_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address, temperature_tag_t *tag);
static void voc_lp_tag_init(bc_i2c_channel_t i2c_channel, voc_lp_tag_t *tag);

void barometer_tag_event_handler(bc_tag_barometer_t *self, bc_tag_barometer_event_t event, void *event_param);
void battery_event_handler(bc_module_battery_event_t event, void *event_param);
void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param);
void humidity_tag_event_handler(bc_tag_humidity_t *self, bc_tag_humidity_event_t event, void *event_param);
void internal_temperature_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param);
void lux_module_event_handler(bc_tag_lux_meter_t *self, bc_tag_lux_meter_event_t event, void *event_param);
void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param);
void voc_lp_tag_event_handler(bc_tag_voc_lp_t *self, bc_tag_voc_lp_event_t event, void *event_param);

#endif // _APPLICATION_H