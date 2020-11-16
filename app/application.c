// #include <bc_usb_cdc.h>
#include <application.h>

#define BATTERY_UPDATE_INTERVAL (60 * 60 * 1000)
#define SERVICE_INTERVAL_INTERVAL (60 * 60 * 1000)

#define BAROMETER_TAG_PUB_NO_CHANGE_INTERVAL (15 * 60 * 1000)
#define BAROMETER_TAG_PUB_VALUE_CHANGE 10.0f
#define BAROMETER_TAG_UPDATE_NORMAL_INTERVAL (5 * 60 * 1000)
#define BAROMETER_TAG_UPDATE_SERVICE_INTERVAL (1 * 60 * 1000)

#define HUMIDITY_TAG_PUB_NO_CHANGE_INTERVAL (15 * 60 * 1000)
#define HUMIDITY_TAG_PUB_VALUE_CHANGE 5.0f
#define HUMIDITY_TAG_UPDATE_NORMAL_INTERVAL (10 * 1000)
#define HUMIDITY_TAG_UPDATE_SERVICE_INTERVAL (5 * 1000)

#define INTERNAL_TEMPERATURE_PUB_NO_CHANGE_INTERVAL (15 * 60 * 1000)
#define INTERNAL_TEMPERATURE_PUB_VALUE_CHANGE 0.1f
#define INTERNAL_TEMPERATURE_UPDATE_NORMAL_INTERVAL (10 * 1000)
#define INTERNAL_TEMPERATURE_UPDATE_SERVICE_INTERVAL (5 * 1000)

#define LUX_TAG_PUB_NO_CHANGE_INTERVAL (15 * 60 * 1000)
#define LUX_TAG_PUB_VALUE_CHANGE 5.0f
#define LUX_TAG_UPDATE_NORMAL_INTERVAL (10 * 1000)
#define LUX_TAG_UPDATE_SERVICE_INTERVAL (5 * 1000)

#define TEMPERATURE_TAG_PUB_NO_CHANGE_INTERVAL (15 * 60 * 1000)
#define TEMPERATURE_TAG_PUB_VALUE_CHANGE 0.1f
#define TEMPERATURE_TAG_UPDATE_NORMAL_INTERVAL (10 * 1000)
#define TEMPERATURE_TAG_UPDATE_SERVICE_INTERVAL (5 * 1000)

#define LUXMETER_ERROR_MSG "Error"

#if MODULE_POWER
    #define RADIO_MODE BC_RADIO_MODE_NODE_LISTENING
#else
    #define RADIO_MODE BC_RADIO_MODE_NODE_SLEEPING
    #if BATTERY_MINI
        #define BC_MODULE_BATTERY_FORMAT BC_MODULE_BATTERY_FORMAT_MINI
    #else
        #define BC_MODULE_BATTERY_FORMAT BC_MODULE_BATTERY_FORMAT_STANDARD
    #endif
#endif // #if MODULE_POWER

// LED instance
bc_led_t led;
// BUtton instance
bc_button_t button;
// Barometer tag instance
barometer_tag_t barometer;
// Humidity tag instance
humidity_tag_t humidity;
// Lux meter instance
lux_meter_tag_t lux;
// Temperature tag instance
temperature_tag_t temperature;
// Thermometer instance
internal_temp_t tmp112;
event_param_t tmp112_event_param = { .next_pub = 0 };

static void barometer_tag_init(bc_i2c_channel_t i2c_channel, barometer_tag_t *tag) {
    memset(tag, 0, sizeof(*tag));

    tag->param.channel = BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT;

    bc_tag_barometer_init(&tag->self, i2c_channel);
    bc_tag_barometer_set_update_interval(&tag->self, BAROMETER_TAG_UPDATE_SERVICE_INTERVAL);
    bc_tag_barometer_set_event_handler(&tag->self, barometer_tag_event_handler, &tag->param);
}

static void humidity_tag_init(bc_tag_humidity_revision_t revision, bc_i2c_channel_t i2c_channel, humidity_tag_t *tag) {
    memset(tag, 0, sizeof(*tag));

    if (revision == BC_TAG_HUMIDITY_REVISION_R1) {
        tag->param.channel = BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT;
    } else if (revision == BC_TAG_HUMIDITY_REVISION_R2) {
        tag->param.channel = BC_RADIO_PUB_CHANNEL_R2_I2C0_ADDRESS_DEFAULT;
    } else if (revision == BC_TAG_HUMIDITY_REVISION_R3) {
        tag->param.channel = BC_RADIO_PUB_CHANNEL_R3_I2C0_ADDRESS_DEFAULT;
    } else {
        return;
    }

    if (i2c_channel == BC_I2C_I2C1) {
        tag->param.channel |= 0x80;
    }

    bc_tag_humidity_init(&tag->self, revision, i2c_channel, BC_TAG_HUMIDITY_I2C_ADDRESS_DEFAULT);
    bc_tag_humidity_set_update_interval(&tag->self, HUMIDITY_TAG_UPDATE_SERVICE_INTERVAL);
    bc_tag_humidity_set_event_handler(&tag->self, humidity_tag_event_handler, &tag->param);
}

static void internal_temp_init(bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address, internal_temp_t *tag) {
    memset(tag, 0, sizeof(*tag));

    tag->param.channel = i2c_address == BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT ? BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT: BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE;

    bc_tmp112_init(&tag->self, i2c_channel, i2c_address);
    bc_tmp112_set_update_interval(&tag->self, INTERNAL_TEMPERATURE_UPDATE_SERVICE_INTERVAL);
    bc_tmp112_set_event_handler(&tag->self, internal_temperature_event_handler, &tag->param);
}

static void lux_meter_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_lux_meter_i2c_address_t i2c_address, lux_meter_tag_t *tag) {
    memset(tag, 0, sizeof(*tag));

    tag->param.channel = i2c_address == BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT ? BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT: BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE;

    bc_tag_lux_meter_init(&tag->self, i2c_channel, i2c_address);
    bc_tag_lux_meter_set_update_interval(&tag->self, LUX_TAG_UPDATE_SERVICE_INTERVAL);
    bc_tag_lux_meter_set_event_handler(&tag->self, lux_module_event_handler, &tag->param);
}

static void temperature_tag_init(bc_i2c_channel_t i2c_channel, bc_tag_temperature_i2c_address_t i2c_address, temperature_tag_t *tag) {
    memset(tag, 0, sizeof(*tag));

    tag->param.channel = i2c_address == BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT ? BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_DEFAULT: BC_RADIO_PUB_CHANNEL_R1_I2C0_ADDRESS_ALTERNATE;

    bc_tag_temperature_init(&tag->self, i2c_channel, i2c_address);
    bc_tag_temperature_set_update_interval(&tag->self, TEMPERATURE_TAG_UPDATE_SERVICE_INTERVAL);
    bc_tag_temperature_set_event_handler(&tag->self, temperature_tag_event_handler, &tag->param);
}

void barometer_tag_event_handler(bc_tag_barometer_t *self, bc_tag_barometer_event_t event, void *event_param) {
    float pascal;
    float meter;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_BAROMETER_EVENT_UPDATE) {
        return;
    }

    if (!bc_tag_barometer_get_pressure_pascal(self, &pascal)) {
        return;
    }

    if ((fabs(pascal - param->value) >= BAROMETER_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick())) {

        if (!bc_tag_barometer_get_altitude_meter(self, &meter)) {
            return;
        }

        bc_radio_pub_barometer(param->channel, &pascal, &meter);
        param->value = pascal;
        param->next_pub = bc_scheduler_get_spin_tick() + BAROMETER_TAG_PUB_NO_CHANGE_INTERVAL;

        bc_log_info("bar - %f Pa", pascal);
        bc_log_info("bar - %f m", meter);
    }
}

void battery_event_handler(bc_module_battery_event_t event, void *event_param) {
    (void) event;
    (void) event_param;

    float voltage;
    int percentage;

    if (bc_module_battery_get_voltage(&voltage)) {
        // values.battery_voltage = voltage;
        bc_radio_pub_battery(&voltage);
        bc_log_info("bat - %f V", voltage);
    }

    if (bc_module_battery_get_charge_level(&percentage)) {
        // values.battery_pct = percentage;
        bc_log_info("bat - %f %%", percentage);
    }
}

// void button_event_handler(bc_button_t *self, bc_button_event_t event, void *event_param) {
//     if (event == BC_BUTTON_EVENT_PRESS) {
//         bc_led_set_mode(&led, BC_LED_MODE_TOGGLE);
//     }

//     // Logging in action
//     bc_log_info("Button event handler - event: %i", event);
// }

void humidity_tag_event_handler(bc_tag_humidity_t *self, bc_tag_humidity_event_t event, void *event_param) {
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_HUMIDITY_EVENT_UPDATE) {
        return;
    }

    if (bc_tag_humidity_get_humidity_percentage(self, &value)) {
        if ((fabs(value - param->value) >= HUMIDITY_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick())) {
            bc_radio_pub_humidity(param->channel, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + HUMIDITY_TAG_PUB_NO_CHANGE_INTERVAL;

            bc_log_info("hum - %f %%", value);
        }
    }
}

void internal_temperature_event_handler(bc_tmp112_t *self, bc_tmp112_event_t event, void *event_param) {
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TMP112_EVENT_UPDATE) {
        return;
    }

    if (bc_tmp112_get_temperature_celsius(self, &value)) {
        if ((fabs(value - param->value) >= INTERNAL_TEMPERATURE_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick())) {
            bc_radio_pub_temperature(param->channel, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTERVAL;

            bc_log_info("int - %f °C", value);
        }
    }
}

void lux_module_event_handler(bc_tag_lux_meter_t *self, bc_tag_lux_meter_event_t event, void *event_param) {
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_LUX_METER_EVENT_UPDATE) {
        return;
    }

    if (bc_tag_lux_meter_get_illuminance_lux(self, &value)) {
        if ((fabs(value - param->value) >= LUX_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick())) {
            bc_radio_pub_luminosity(param->channel, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + LUX_TAG_PUB_NO_CHANGE_INTERVAL;

            bc_log_info("lux - %f lx", value);
        }
    }
}

void temperature_tag_event_handler(bc_tag_temperature_t *self, bc_tag_temperature_event_t event, void *event_param) {
    float value;
    event_param_t *param = (event_param_t *)event_param;

    if (event != BC_TAG_TEMPERATURE_EVENT_UPDATE) {
        return;
    }

    if (bc_tag_temperature_get_temperature_celsius(self, &value)) {
        if ((fabs(value - param->value) >= TEMPERATURE_TAG_PUB_VALUE_CHANGE) || (param->next_pub < bc_scheduler_get_spin_tick())) {
            bc_radio_pub_temperature(param->channel, &value);
            param->value = value;
            param->next_pub = bc_scheduler_get_spin_tick() + TEMPERATURE_TAG_PUB_NO_CHANGE_INTERVAL;

            bc_log_info("temp - %f °C", value);
        }
    }
}

void switch_to_normal_mode_task(void *param) {
    bc_tag_barometer_set_update_interval(&barometer.self, BAROMETER_TAG_UPDATE_NORMAL_INTERVAL);

    bc_tag_humidity_set_update_interval(&humidity.self, HUMIDITY_TAG_UPDATE_NORMAL_INTERVAL);

    bc_tag_lux_meter_set_update_interval(&lux.self, LUX_TAG_UPDATE_NORMAL_INTERVAL);

    bc_tag_temperature_set_update_interval(&temperature.self, TEMPERATURE_TAG_UPDATE_NORMAL_INTERVAL);

    bc_tmp112_set_update_interval(&tmp112, INTERNAL_TEMPERATURE_UPDATE_NORMAL_INTERVAL);

    bc_scheduler_unregister(bc_scheduler_get_current_task_id());
}

void application_init(void) {
    bc_log_init(BC_LOG_LEVEL_DEBUG, BC_LOG_TIMESTAMP_ABS);

    bc_radio_init(RADIO_MODE);

    // Initialize LED
    bc_led_init(&led, BC_GPIO_LED, false, false);
    bc_led_set_mode(&led, BC_LED_MODE_OFF);
    // Initialize button
    // bc_button_init(&button, BC_GPIO_BUTTON, BC_GPIO_PULL_DOWN, false);
    // bc_button_set_event_handler(&button, button_event_handler, NULL);

    // Initialize temperature tag, use default address, it appears as 0:0 in node message
    temperature_tag_init(BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_DEFAULT, &temperature);
    // Initialize thermometer sensor on core module, use alternate address, it appears
    // as 0:1 in node message
    internal_temp_init(BC_I2C_I2C0, BC_TAG_TEMPERATURE_I2C_ADDRESS_ALTERNATE, &tmp112);
    // Initialize barometer
    barometer_tag_init(BC_I2C_I2C0, &barometer);
    // Initialize battery
    bc_module_battery_init();
    bc_module_battery_set_event_handler(battery_event_handler, NULL);
    bc_module_battery_set_update_interval(BATTERY_UPDATE_INTERVAL);
    // Initialize humidity
    humidity_tag_init(BC_TAG_HUMIDITY_REVISION_R3, BC_I2C_I2C1, &humidity);
    // Initialize lux
    lux_meter_tag_init(BC_I2C_I2C1, BC_TAG_LUX_METER_I2C_ADDRESS_DEFAULT, &lux);

    bc_log_info("Version: %s", VERSION);
    bc_radio_pairing_request("outdoor-monitor", VERSION);

    bc_scheduler_register(switch_to_normal_mode_task, NULL, SERVICE_INTERVAL_INTERVAL);

    bc_led_pulse(&led, 2000);
}
