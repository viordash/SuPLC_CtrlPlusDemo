

#include "driver/gpio.h"
#include "board.h"
#include "esp_adc/adc_oneshot.h"
#include "os.h"

#define GPIO_OUTPUT_IO_0 GPIO_NUM_19
#define GPIO_OUTPUT_IO_1 GPIO_NUM_21
#define GPIO_OUTPUT_IO_2 GPIO_NUM_22
#define GPIO_OUTPUT_IO_3 GPIO_NUM_23
#define GPIO_OUTPUT_PIN_SEL                                                                        \
    ((1ULL << GPIO_OUTPUT_IO_0) | (1ULL << GPIO_OUTPUT_IO_1) | (1ULL << GPIO_OUTPUT_IO_2)          \
     | (1ULL << GPIO_OUTPUT_IO_3))

#define BUTTON_UP_IO GPIO_NUM_4
#define BUTTON_DOWN_IO GPIO_NUM_5
#define BUTTON_SELECT_IO GPIO_NUM_0

#define INPUT_0_IO GPIO_NUM_12 /*GPIO_NUM_34*/
#define INPUT_1_IO GPIO_NUM_16
#define INPUT_2_IO GPIO_NUM_17
#define INPUT_3_IO GPIO_NUM_18

#define GPIO_INPUT_PIN_SEL                                                                         \
    ((1ULL << BUTTON_UP_IO) | (1ULL << BUTTON_DOWN_IO) | (1ULL << BUTTON_SELECT_IO)                \
     | (1ULL << INPUT_0_IO) | (1ULL << INPUT_1_IO) | (1ULL << INPUT_2_IO) | (1ULL << INPUT_3_IO))

#define ADC_VBAT_CHAN ADC_CHANNEL_0
#define ADC_VBAT_ATTEN ADC_ATTEN_DB_12
#define ADC_0_CHAN ADC_CHANNEL_4
#define ADC_0_ATTEN ADC_ATTEN_DB_12
#define ADC_1_CHAN ADC_CHANNEL_5
#define ADC_1_ATTEN ADC_ATTEN_DB_12
#define ADC_2_CHAN ADC_CHANNEL_7
#define ADC_2_ATTEN ADC_ATTEN_DB_12

#define GPIO_ACTIVE 1
#define GPIO_PASSIVE 0
#define INPUT_NC_VALUE 0
#define INPUT_NO_VALUE 1

static const char *TAG_gpio = "gpio";

static struct t_gpio {
    event_t event;
    adc_oneshot_unit_handle_t adc1_handle;
} gpio;

static void analog_init();

static void outputs_init() {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT;
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    set_digital_output_value(OUTPUT_0, false);
    set_digital_output_value(OUTPUT_1, false);
    set_digital_output_value(OUTPUT_2, false);
    set_digital_output_value(OUTPUT_3, false);
}

static IRAM_ATTR void BUTTON_UP_IO_isr_handler(void *arg) {
    (void)arg;
    event_flags_t flags = gpio_get_level(BUTTON_UP_IO) == INPUT_NC_VALUE //
                            ? BUTTON_UP_IO_CLOSE
                            : BUTTON_UP_IO_OPEN;
    raise_event_from_ISR(gpio.event, flags);
}
static IRAM_ATTR void BUTTON_DOWN_IO_isr_handler(void *arg) {
    (void)arg;
    event_flags_t flags = gpio_get_level(BUTTON_DOWN_IO) == INPUT_NC_VALUE //
                            ? BUTTON_DOWN_IO_CLOSE
                            : BUTTON_DOWN_IO_OPEN;
    raise_event_from_ISR(gpio.event, flags);
}

static IRAM_ATTR void BUTTON_SELECT_IO_isr_handler(void *arg) {
    (void)arg;
    event_flags_t flags = gpio_get_level(BUTTON_SELECT_IO) == INPUT_NC_VALUE //
                            ? BUTTON_SELECT_IO_CLOSE
                            : BUTTON_SELECT_IO_OPEN;
    raise_event_from_ISR(gpio.event, flags);
}

static IRAM_ATTR void INPUT_0_IO_isr_handler(void *arg) {
    (void)arg;
    event_flags_t flags = gpio_get_level(INPUT_0_IO) == INPUT_NC_VALUE //
                            ? INPUT_0_IO_CLOSE
                            : INPUT_0_IO_OPEN;
    raise_event_from_ISR(gpio.event, flags);
}

static IRAM_ATTR void INPUT_1_IO_isr_handler(void *arg) {
    (void)arg;
    event_flags_t flags = gpio_get_level(INPUT_1_IO) == INPUT_NC_VALUE //
                            ? INPUT_1_IO_CLOSE
                            : INPUT_1_IO_OPEN;
    raise_event_from_ISR(gpio.event, flags);
}

static IRAM_ATTR void INPUT_2_IO_isr_handler(void *arg) {
    (void)arg;
    event_flags_t flags = gpio_get_level(INPUT_2_IO) == INPUT_NC_VALUE //
                            ? INPUT_2_IO_CLOSE
                            : INPUT_2_IO_OPEN;
    raise_event_from_ISR(gpio.event, flags);
}

static IRAM_ATTR void INPUT_3_IO_isr_handler(void *arg) {
    (void)arg;

    event_flags_t flags = gpio_get_level(INPUT_3_IO) == INPUT_NC_VALUE //
                            ? INPUT_3_IO_CLOSE
                            : INPUT_3_IO_OPEN;
    raise_event_from_ISR(gpio.event, flags);
}

static void inputs_init() {
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_ANYEDGE;
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_install_isr_service(0);

    gpio_isr_handler_add(BUTTON_UP_IO, BUTTON_UP_IO_isr_handler, NULL);
    gpio_isr_handler_add(BUTTON_DOWN_IO, BUTTON_DOWN_IO_isr_handler, NULL);
    gpio_isr_handler_add(BUTTON_SELECT_IO, BUTTON_SELECT_IO_isr_handler, NULL);
    gpio_isr_handler_add(INPUT_0_IO, INPUT_0_IO_isr_handler, NULL);
    gpio_isr_handler_add(INPUT_1_IO, INPUT_1_IO_isr_handler, NULL);
    gpio_isr_handler_add(INPUT_2_IO, INPUT_2_IO_isr_handler, NULL);
    gpio_isr_handler_add(INPUT_3_IO, INPUT_3_IO_isr_handler, NULL);
}

event_t gpio_init() {
    outputs_init();

    gpio.event = create_event();

    inputs_init();
    analog_init();
    return gpio.event;
}

bool get_digital_output_value(gpio_output gpio) {
    switch (gpio) {
        case OUTPUT_0:
            return gpio_get_level(GPIO_OUTPUT_IO_0) == GPIO_ACTIVE;
        case OUTPUT_1:
            return gpio_get_level(GPIO_OUTPUT_IO_1) == GPIO_ACTIVE;
        case OUTPUT_2:
            return gpio_get_level(GPIO_OUTPUT_IO_2) == GPIO_ACTIVE;
        case OUTPUT_3:
            return gpio_get_level(GPIO_OUTPUT_IO_3) == GPIO_ACTIVE;
    }
    log_e(TAG_gpio, "get_digital_value, err:0x%X", ESP_ERR_NOT_FOUND);
    return false;
}

void set_digital_output_value(gpio_output gpio, bool value) {
    esp_err_t err = ESP_ERR_NOT_FOUND;
    int gpio_val = value ? GPIO_ACTIVE : GPIO_PASSIVE;
    switch (gpio) {
        case OUTPUT_0:
            err = gpio_set_level(GPIO_OUTPUT_IO_0, gpio_val);
            break;
        case OUTPUT_1:
            err = gpio_set_level(GPIO_OUTPUT_IO_1, gpio_val);
            break;
        case OUTPUT_2:
            err = gpio_set_level(GPIO_OUTPUT_IO_2, gpio_val);
            break;
        case OUTPUT_3:
            err = gpio_set_level(GPIO_OUTPUT_IO_3, gpio_val);
            break;
    }
    if (err != ESP_OK) {
        log_e(TAG_gpio, "set_digital_value, err:0x%X", err);
    }
}

bool get_digital_input_value(gpio_input gpio) {
    gpio_num_t gpio_num;
    switch (gpio) {
        case INPUT_0:
            gpio_num = INPUT_0_IO;
            break;
        case INPUT_1:
            gpio_num = INPUT_1_IO;
            break;
        case INPUT_2:
            gpio_num = INPUT_2_IO;
            break;
        case INPUT_3:
            gpio_num = INPUT_3_IO;
            break;
        default:
            log_e(TAG_gpio, "get_digital_input_value, err:0x%X", ESP_ERR_NOT_FOUND);
            return false;
    }

    bool input_closed = gpio_get_level(gpio_num) == INPUT_NC_VALUE;
    return input_closed;
}

bool maintenance_button_pressed() {
    bool state = gpio_get_level(BUTTON_UP_IO) == 0;
    log_d(TAG_gpio, "select_button_pressed, state:%d", state);
    return state;
}

static void analog_init() {
    adc_oneshot_unit_init_cfg_t init_config1 = { .unit_id = ADC_UNIT_1,
                                                 .clk_src = ADC_RTC_CLK_SRC_DEFAULT,
                                                 .ulp_mode = ADC_ULP_MODE_DISABLE };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &gpio.adc1_handle));

    adc_oneshot_chan_cfg_t config_vbat = {
        .atten = ADC_VBAT_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(gpio.adc1_handle, ADC_VBAT_CHAN, &config_vbat));

    adc_oneshot_chan_cfg_t config_chan_0 = {
        .atten = ADC_0_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(gpio.adc1_handle, ADC_0_CHAN, &config_chan_0));
    adc_oneshot_chan_cfg_t config_chan_1 = {
        .atten = ADC_1_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(gpio.adc1_handle, ADC_1_CHAN, &config_chan_1));
    adc_oneshot_chan_cfg_t config_chan_2 = {
        .atten = ADC_2_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(gpio.adc1_handle, ADC_2_CHAN, &config_chan_2));
}

int get_adc_value(adc_input adc) {
    int val = 0x0;
    esp_err_t err = ESP_ERR_NOT_FOUND;
    switch (adc) {
        case VBAT:
            err = adc_oneshot_read(gpio.adc1_handle, ADC_VBAT_CHAN, &val);
            break;
        case CHAN_0:
            err = adc_oneshot_read(gpio.adc1_handle, ADC_0_CHAN, &val);
            break;
        case CHAN_1:
            err = adc_oneshot_read(gpio.adc1_handle, ADC_1_CHAN, &val);
            break;
        case CHAN_2:
            err = adc_oneshot_read(gpio.adc1_handle, ADC_2_CHAN, &val);
            break;
    }
    if (err != ESP_OK) {
        log_e(TAG_gpio, "get_adc_value, err:0x%X", err);
    }
    return val;
}