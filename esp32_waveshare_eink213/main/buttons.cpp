#include "buttons.h"
#include "Buttons/button.h"
#include "board.h"
#include "os.h"
#include <vector>

static struct {
    std::vector<button> buttons{
        { button("button UP",
                 BUTTON_UP_IO_CLOSE,
                 BUTTON_UP_IO_OPEN,
                 ButtonsPressType::UP_PRESSED,
                 ButtonsPressType::UP_LONG_PRESSED) },
        { button("button DOWN",
                 BUTTON_DOWN_IO_CLOSE,
                 BUTTON_DOWN_IO_OPEN,
                 ButtonsPressType::DOWN_PRESSED,
                 ButtonsPressType::DOWN_LONG_PRESSED) },
        { button("button SELECT",
                 BUTTON_SELECT_IO_CLOSE,
                 BUTTON_SELECT_IO_OPEN,
                 ButtonsPressType::SELECT_PRESSED,
                 ButtonsPressType::SELECT_LONG_PRESSED) },
    };
} buttons;

ButtonsPressType handle_buttons(event_flags_t flags) {
    for (auto &button : buttons.buttons) {
        switch (button.handle(flags)) {
            case button::state::btDown:
                log_d(button.TAG, "process, flags:0x%08X btDown", (unsigned int)flags);
                break;
            case button::state::btShortPressed:
                log_d(button.TAG, "process, flags:0x%08X btShortPressed", (unsigned int)flags);
                break;
            case button::state::btPressed:
                log_d(button.TAG, "process, flags:0x%08X btPressed", (unsigned int)flags);
                return button.pressed_type;
                break;
            case button::state::btLongPressed:
                log_d(button.TAG, "process, flags:0x%08X btLongPressed", (unsigned int)flags);
                return button.long_pressed_type;
                break;

            default:
                break;
        }
    }
    return ButtonsPressType::NOTHING_PRESSED;
}
