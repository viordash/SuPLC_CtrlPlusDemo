

#include "LogicProgram/Controller.h"
#include "board.h"
#include "driver/gpio.h"
#include "os.h"

#define LOW 0
#define HIGH 1

#define EPD_SCK_PIN GPIO_NUM_13
#define EPD_MOSI_PIN GPIO_NUM_14
#define EPD_CS_PIN GPIO_NUM_15
#define EPD_RST_PIN GPIO_NUM_26
#define EPD_DC_PIN GPIO_NUM_27
#define EPD_BUSY_PIN GPIO_NUM_25

#define EPD_OUTPUT_PIN_SEL                                                                         \
    ((1ULL << EPD_SCK_PIN) | (1ULL << EPD_MOSI_PIN) | (1ULL << EPD_CS_PIN) | (1ULL << EPD_RST_PIN) \
     | (1ULL << EPD_DC_PIN))

#define EPD_INPUT_PIN_SEL ((1ULL << EPD_BUSY_PIN))

static const char *TAG_display = "display_hw";

const uint64_t min_period_rendering_us = DISPLAY_MIN_PERIOD_RENDER_MS * 1000;

static struct t_display {
    event_t event;
    FrameBuffer frame_buffer;
    uint64_t refresh_timer;
} display = { NULL, {}, -min_period_rendering_us };

static IRAM_ATTR void EPD_BUSY_PIN_isr_handler(void *arg) {
    (void)arg;
    raise_wakeup_event_from_ISR(display.event);
}

event_t display_hw_config() {
    display.event = create_event();

    gpio_config_t inp_conf = {};
    inp_conf.intr_type = GPIO_INTR_NEGEDGE;
    inp_conf.pin_bit_mask = EPD_INPUT_PIN_SEL;
    inp_conf.mode = GPIO_MODE_INPUT;
    inp_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    ERROR_CHECK(gpio_config(&inp_conf));

    gpio_isr_handler_add(EPD_BUSY_PIN, EPD_BUSY_PIN_isr_handler, NULL);

    gpio_config_t out_conf = {};
    out_conf.intr_type = GPIO_INTR_DISABLE;
    out_conf.mode = GPIO_MODE_OUTPUT;
    out_conf.pin_bit_mask = EPD_OUTPUT_PIN_SEL;
    out_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    out_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    ERROR_CHECK(gpio_config(&out_conf));

    gpio_set_level(EPD_CS_PIN, HIGH);
    gpio_set_level(EPD_SCK_PIN, LOW);
    log_i(TAG_display, "display init");
    return display.event;
}

static void display_activate_reset() {
    gpio_set_level(EPD_RST_PIN, LOW);
}

static void display_deactivate_reset() {
    gpio_set_level(EPD_RST_PIN, HIGH);
}

static void display_send_byte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        if ((data & 0x80) == 0)
            gpio_set_level(EPD_MOSI_PIN, LOW);
        else
            gpio_set_level(EPD_MOSI_PIN, HIGH);

        data <<= 1;
        gpio_set_level(EPD_SCK_PIN, HIGH);
        gpio_set_level(EPD_SCK_PIN, LOW);
    }
}

static void display_send_command(uint8_t cmd) {
    gpio_set_level(EPD_DC_PIN, LOW);
    gpio_set_level(EPD_CS_PIN, LOW);
    display_send_byte(cmd);
    gpio_set_level(EPD_CS_PIN, HIGH);
}

static void display_send_data(uint8_t data) {
    gpio_set_level(EPD_DC_PIN, HIGH);
    gpio_set_level(EPD_CS_PIN, LOW);
    display_send_byte(data);
    gpio_set_level(EPD_CS_PIN, HIGH);
}

static void display_begin_send_data_buffer() {
    gpio_set_level(EPD_DC_PIN, HIGH);
    gpio_set_level(EPD_CS_PIN, LOW);
}

static void display_end_send_data_buffer() {
    gpio_set_level(EPD_CS_PIN, HIGH);
}

static void display_wait_busy(void) {
    const uint32_t busy_timeout_ms = 3000;
    auto wait_start_time = timer_get_time_us();
    while (gpio_get_level(EPD_BUSY_PIN) != LOW) {
        int32_t wait_time_ms = busy_timeout_ms - ((timer_get_time_us() - wait_start_time) / 1000);
        log_d(TAG_display, "display busy, wait_time_ms:%d", (int)wait_time_ms);

        if (wait_time_ms < 0) {
            log_d(TAG_display, "display busy timeout");
            break;
        }

        event_flags_t flags;
        bool timeout = !wait_event_timed(display.event, &flags, wait_time_ms);
        if (timeout) {
            log_d(TAG_display, "display busy timeout");
            break;
        }
    }
    log_d(TAG_display, "display busy release");
}

/******************************************************************************
function :	Setting the display window
parameter:
    Xstart : X-axis starting position
    Ystart : Y-axis starting position
    Xend : End position of X-axis
    Yend : End position of Y-axis
******************************************************************************/
static void
EPD_2in13_V4_SetWindows(uint16_t Xstart, uint16_t Ystart, uint16_t Xend, uint16_t Yend) {
    display_send_command(0x44); // SET_RAM_X_ADDRESS_START_END_POSITION
    display_send_data((Xstart >> 3) & 0xFF);
    display_send_data((Xend >> 3) & 0xFF);

    display_send_command(0x45); // SET_RAM_Y_ADDRESS_START_END_POSITION
    display_send_data(Ystart & 0xFF);
    display_send_data((Ystart >> 8) & 0xFF);
    display_send_data(Yend & 0xFF);
    display_send_data((Yend >> 8) & 0xFF);
}

/******************************************************************************
function :	Set Cursor
parameter:
    Xstart : X-axis starting position
    Ystart : Y-axis starting position
******************************************************************************/
static void EPD_2in13_V4_SetCursor(uint16_t Xstart, uint16_t Ystart) {
    display_send_command(0x4E); // SET_RAM_X_ADDRESS_COUNTER
    display_send_data(Xstart & 0xFF);

    display_send_command(0x4F); // SET_RAM_Y_ADDRESS_COUNTER
    display_send_data(Ystart & 0xFF);
    display_send_data((Ystart >> 8) & 0xFF);
}

/******************************************************************************
function :	Turn On Display
parameter:
******************************************************************************/
static void EPD_2in13_V4_TurnOnDisplay(void) {
    display_send_command(0x22); // Display Update Control
    display_send_data(0xf7);
    display_send_command(0x20); // Activate Display Update Sequence
}

static void EPD_2in13_V4_TurnOnDisplay_Partial(void) {
    display_send_command(0x22); // Display Update Control
    display_send_data(0xff);    // fast:0x0c, quality:0x0f, 0xcf
    display_send_command(0x20); // Activate Display Update Sequence
}

/******************************************************************************
function :	Initialize the e-Paper register
parameter:
******************************************************************************/
void display_hw_init(void) {
    display_activate_reset();
    sleep_ms(1);
    display_deactivate_reset();
    sleep_ms(1);
    display_wait_busy();

    display_send_command(0x12); //SWRESET

    sleep_ms(11);
    display_wait_busy();
    display_send_command(0x01); //Driver output control
    display_send_data(DISPLAY_HW_HEIGHT - 1);
    display_send_data(0x00);
    display_send_data(0x00);

    display_send_command(0x11); //data entry mode
    display_send_data(0x03);

    EPD_2in13_V4_SetWindows(0, 0, DISPLAY_HW_WIDTH - 1, DISPLAY_HW_HEIGHT - 1);
    EPD_2in13_V4_SetCursor(0, 0);

    display_send_command(0x3C); //BorderWavefrom
    display_send_data(0x05);

    display_send_command(0x21); //  Display update control
    display_send_data(0x00);
    display_send_data(0x80);

    display_send_command(0x18); //Read built-in temperature sensor
    display_send_data(0x80);
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and displays
parameter:
    Image : Image data
******************************************************************************/
static IRAM_ATTR void display_hw_send_buffer(uint8_t *image) {
    uint16_t Width, Height;
    Width = (DISPLAY_HW_WIDTH % 8 == 0) ? (DISPLAY_HW_WIDTH / 8) : (DISPLAY_HW_WIDTH / 8 + 1);
    Height = DISPLAY_HW_HEIGHT;

    display_wait_busy();
    display_send_command(0x24);
    display_begin_send_data_buffer();
    for (uint16_t j = 0; j < Height; j++) {
        for (uint16_t i = 0; i < Width; i++) {
            display_send_byte(image[i + j * Width]);
        }
    }
    display_end_send_data_buffer();

    EPD_2in13_V4_TurnOnDisplay();
}

/******************************************************************************
function :	Sends the image buffer in RAM to e-Paper and partial refresh
parameter:
    Image : Image data
******************************************************************************/
static IRAM_ATTR void display_hw_send_buffer_partial(uint8_t *image) {
    uint16_t Width, Height;
    Width = (DISPLAY_HW_WIDTH % 8 == 0) ? (DISPLAY_HW_WIDTH / 8) : (DISPLAY_HW_WIDTH / 8 + 1);
    Height = DISPLAY_HW_HEIGHT;

    display_wait_busy();
    //Reset
    display_activate_reset();
    sleep_ms(1);
    display_deactivate_reset();
    sleep_ms(1);

    display_wait_busy();
    display_send_command(0x3C); //BorderWavefrom
    display_send_data(0x80);

    display_send_command(0x01); //Driver output control
    display_send_data(DISPLAY_HW_HEIGHT - 1);
    display_send_data(0x00);
    display_send_data(0x00);

    display_send_command(0x11); //data entry mode
    display_send_data(0x03);

    EPD_2in13_V4_SetWindows(0, 0, DISPLAY_HW_WIDTH - 1, DISPLAY_HW_HEIGHT - 1);
    EPD_2in13_V4_SetCursor(0, 0);

    display_send_command(0x24); //Write Black and White image to RAM
    display_begin_send_data_buffer();
    for (uint16_t j = 0; j < Height; j++) {
        for (uint16_t i = 0; i < Width; i++) {
            display_send_byte(image[i + j * Width]);
        }
    }
    display_end_send_data_buffer();
    EPD_2in13_V4_TurnOnDisplay_Partial();
}

IRAM_ATTR FrameBuffer *begin_render() {
    memset(display.frame_buffer.buffer, DISPLAY_PIXELS_OFF, sizeof(display.frame_buffer.buffer));
    display.frame_buffer.has_changes = false;
    return &display.frame_buffer;
}

bool skip_frequent_rendering() {
    uint64_t current_time = timer_get_time_us();
    int64_t timespan = current_time - display.refresh_timer;
    bool skip = timespan < min_period_rendering_us;
    if (!skip) {
        display.refresh_timer = current_time;
    }
    log_d(TAG_display, "------------ skip: %u, timespan:%lld, ", (unsigned)skip, timespan);
    return skip;
}

IRAM_ATTR void end_render(FrameBuffer *fb) {
    if (fb == FRAMEBUFFER_STUB) {
        return;
    }
    if (!fb->has_changes) {
        return;
    }
    display_hw_send_buffer(fb->buffer);
}
IRAM_ATTR void end_render_partial(FrameBuffer *fb) {
    if (fb == FRAMEBUFFER_STUB) {
        return;
    }
    if (!fb->has_changes) {
        return;
    }
    if (skip_frequent_rendering()) {
        Controller::DoDeferredRender();
        return;
    }
    display_hw_send_buffer_partial(fb->buffer);
}

FrameBuffer *peek_framebuffer() {
    return &display.frame_buffer;
}