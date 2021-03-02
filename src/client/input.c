#include "input.h"

static u8 keyboard_state[KEY_COUNT];
static u8 unicode_state[KEY_COUNT];
static mouse_t mouse;
static const char* text_input;

static key_t scancode_keys[KEY_COUNT] =
{
	0, 0, 0, 0, KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J, KEY_K, KEY_L, KEY_M,
	KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T, KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z, KEY_1, KEY_2,
	KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9, KEY_0, KEY_RETURN, KEY_ESCAPE, KEY_BACK, KEY_TAB,
	KEY_SPACE, KEY_MINUS, KEY_EQUAL, KEY_OPEN_BRACKET, KEY_CLOSE_BRACKET, KEY_BACK_SLASH, 0, KEY_SEMICOLON,
	KEY_APOSTRAPHE, KEY_GRAVE, KEY_COMMA, KEY_PERIOD, KEY_FORWARD_SLASH, KEY_CAPITAL, KEY_F1, KEY_F2, KEY_F3,
	KEY_F4, KEY_F5, KEY_F6, KEY_F7, KEY_F8, KEY_F9, KEY_F10, KEY_F11, KEY_F12, 0, KEY_SCROLL, KEY_PAUSE,
	KEY_INSERT, KEY_HOME, KEY_NEXT, KEY_DELETE, KEY_END, KEY_PRIOR, KEY_RIGHT, KEY_LEFT, KEY_DOWN, KEY_UP,
	KEY_NUMLOCK, KEY_DIVIDE, KEY_MULTIPLY, KEY_MINUS, KEY_ADD, 0, KEY_NUMPAD1, KEY_NUMPAD2, KEY_NUMPAD3,
	KEY_NUMPAD4, KEY_NUMPAD5, KEY_NUMPAD6, KEY_NUMPAD7, KEY_NUMPAD8, KEY_NUMPAD9, KEY_NUMPAD0, KEY_PERIOD,
	KEY_BACK_SLASH, KEY_APPS
};

static const char* key_names[KEY_COUNT] =
{
	"", "", "", "CANCEL", "", "", "", "", "BACKSPACE", "TAB", "", "", "CLEAR", "ENTER", "", "",
	"SHIFT", "CTRL", "ALT", "PAUSE", "CAPS LOCK", "", "", "", "", "", "", "ESCAPE", "",	"", "",
	"", "SPACEBAR", "PAGE UP", "PAGE DOWN", "END", "HOME", "LEFT ARROW", "UP ARROW", "RIGHT ARROW", 
	"DOWN ARROW", "SELECT", "", "EXECUTE", "PRINT SCREEN", "INS", "DEL", "HELP", "0", "1", "2",
	"3", "4", "5", "6", "7", "8", "9", "", "", "", "", "", "", "", "A", "B", "C", "D", "E", "F",
	"G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y",
	"Z", "LEFT WINDOWS", "RIGHT WINDOWS", "APPLICATIONS", "", "SLEEP", "NUMPAD 0", "NUMPAD 1",
	"NUMPAD 2", "NUMPAD 3", "NUMPAD 4", "NUMPAD 5", "NUMPAD 6", "NUMPAD 7", "NUMPAD 8", "NUMPAD 9",
	"MULTIPLY", "ADD", "SEPARATOR", "SUBTRACT", "DECIMAL", "DIVIDE", "F1", "F2", "F3", "F4", "F5",
	"F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16", "F17", "F18", "F19",
	"F20", "F21", "F22", "F23", "F24", "", "", "", "", "", "", "", "NUM LOCK", "SCROLL LOCK", "",
	"", "", "", "", "", "", "", "", "", "", "", "", "", "LEFT SHIFT", "RIGHT SHIFT", "LEFT CONTROL",
	"RIGHT CONTROL", "LEFT MENU", "RIGHT MENU", "", "", "",	"", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "EQUAL", "COMMA", "MINUS", "PERIOD", "FORWARD SLASH",
	"TILDE", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "OPEN BRACKET", "BACK SLASH", "CLOSE BRACKET", "QUOTE",
};

bool input_key(key_t key)
{
	return keyboard_state[key];
}

bool input_mouse_button(mouse_button_t mouse_button)
{
	return mouse.buttons[mouse_button];
}

void input_set_key_down(u32 scancode)
{
	keyboard_state[scancode_keys[scancode]]++;
}

void input_set_key_up(u32 scancode)
{
	keyboard_state[scancode_keys[scancode]] = 0;
}

void input_set_mouse_button_down(mouse_button_t mouse_button)
{
	mouse.buttons[mouse_button]++;
}

void input_set_mouse_button_up(mouse_button_t mouse_button)
{
    mouse.buttons[mouse_button] = 0;
}

const char* input_key_name(key_t key)
{
	return key_names[key];
}
