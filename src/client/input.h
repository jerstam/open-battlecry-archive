#pragma once

#include "../macros.h"

typedef enum
{
    KEY_CANCEL = 0x03,
    KEY_BACK = 0x08,
    KEY_TAB = 0x09,
    KEY_CLEAR = 0x0C,
    KEY_RETURN = 0x0D,
    KEY_SHIFT = 0x10,
    KEY_CONTROL = 0x11,
    KEY_MENU = 0x12,
    KEY_PAUSE = 0x13,
    KEY_CAPITAL = 0x14,
    KEY_ESCAPE = 0x1B,
    KEY_SPACE = 0x20,
    KEY_PRIOR = 0x21,
    KEY_NEXT = 0x22,
    KEY_END = 0x23,
    KEY_HOME = 0x24,
    KEY_LEFT = 0x25,
    KEY_UP = 0x26,
    KEY_RIGHT = 0x27,
    KEY_DOWN = 0x28,
    KEY_SELECT = 0x29,
    KEY_EXECUTE = 0x2B,
    KEY_SNAPSHOT = 0x2C,
    KEY_INSERT = 0x2D,
    KEY_DELETE = 0x2E,
    KEY_HELP = 0x2F,
    KEY_0 = 0x30,
    KEY_1 = 0x31,
    KEY_2 = 0x32,
    KEY_3 = 0x33,
    KEY_4 = 0x34,
    KEY_5 = 0x35,
    KEY_6 = 0x36,
    KEY_7 = 0x37,
    KEY_8 = 0x38,
    KEY_9 = 0x39,
    KEY_A = 0x41,
    KEY_B = 0x42,
    KEY_C = 0x43,
    KEY_D = 0x44,
    KEY_E = 0x45,
    KEY_F = 0x46,
    KEY_G = 0x47,
    KEY_H = 0x48,
    KEY_I = 0x49,
    KEY_J = 0x4A,
    KEY_K = 0x4B,
    KEY_L = 0x4C,
    KEY_M = 0x4D,
    KEY_N = 0x4E,
    KEY_O = 0x4F,
    KEY_P = 0x50,
    KEY_Q = 0x51,
    KEY_R = 0x52,
    KEY_S = 0x53,
    KEY_T = 0x54,
    KEY_U = 0x55,
    KEY_V = 0x56,
    KEY_W = 0x57,
    KEY_X = 0x58,
    KEY_Y = 0x59,
    KEY_Z = 0x5A,
    KEY_LWIN = 0x5B,
    KEY_RWIN = 0x5C,
    KEY_APPS = 0x5D,
    KEY_SLEEP = 0x5F,
    KEY_NUMPAD0 = 0x60,
    KEY_NUMPAD1 = 0x61,
    KEY_NUMPAD2 = 0x62,
    KEY_NUMPAD3 = 0x63,
    KEY_NUMPAD4 = 0x64,
    KEY_NUMPAD5 = 0x65,
    KEY_NUMPAD6 = 0x66,
    KEY_NUMPAD7 = 0x67,
    KEY_NUMPAD8 = 0x68,
    KEY_NUMPAD9 = 0x69,
    KEY_MULTIPLY = 0x6A,
    KEY_ADD = 0x6B,
    KEY_SEPARATOR = 0x6C,
    KEY_SUBTRACT = 0x6D,
    KEY_DECIMAL = 0x6E,
    KEY_DIVIDE = 0x6F,
    KEY_F1 = 0x70,
    KEY_F2 = 0x71,
    KEY_F3 = 0x72,
    KEY_F4 = 0x73,
    KEY_F5 = 0x74,
    KEY_F6 = 0x75,
    KEY_F7 = 0x76,
    KEY_F8 = 0x77,
    KEY_F9 = 0x78,
    KEY_F10 = 0x79,
    KEY_F11 = 0x7A,
    KEY_F12 = 0x7B,
    KEY_NUMLOCK = 0x90,
    KEY_SCROLL = 0x91,
    KEY_COMMAND = 0x92,
    KEY_OPEN_BRACKET = 0xDB,
    KEY_CLOSE_BRACKET = 0xDD,
    KEY_SEMICOLON = 0xBA,
    KEY_APOSTRAPHE = 0xC0,
    KEY_BACK_SLASH = 0xDC,
    KEY_FORWARD_SLASH = 0xBF,
    KEY_COMMA = 0xBC,
    KEY_PERIOD = 0xBE,
    KEY_TILDE = 0xDE,
    KEY_MINUS = 0xBD,
    KEY_EQUAL = 0xBB,
    KEY_GRAVE = 223,
    KEY_WINDOWS = 91,
    KEY_COUNT = 512
} key_t;

typedef enum
{
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,
    MOUSE_BUTTON_X1,
    MOUSE_BUTTON_X2,
    MOUSE_BUTTON_X3,
    MOUSE_BUTTON_X4,
    MOUSE_BUTTON_COUNT
} mouse_button_t;

typedef struct mouse_t
{
    float x;
    float y;
    float wheel;
    u8 buttons[MOUSE_BUTTON_COUNT];
    bool double_click;
} mouse_t;

bool input_key(key_t key);
bool input_mouse_button(mouse_button_t mouse_button);

void input_set_key_down(u32 scancode);
void input_set_key_up(u32 scancode);
void input_set_mouse_button_down(mouse_button_t mouse_button);
void input_set_mouse_button_up(mouse_button_t mouse_button);

const char* input_key_name(key_t key);
