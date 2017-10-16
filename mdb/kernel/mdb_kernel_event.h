#pragma once

enum
{
    /*! @name Key and button actions
     *  @{ */
    /*! @brief The key or mouse button was released.
     *
     *  The key or mouse button was released.
     *
     *  @ingroup input
     */
    MDB_ACTION_RELEASE                = 0,
    /*! @brief The key or mouse button was pressed.
     *
     *  The key or mouse button was pressed.
     *
     *  @ingroup input
     */
    MDB_ACTION_PRESS                  = 1,
    /*! @brief The key was held down until it repeated.
     *
     *  The key was held down until it repeated.
     *
     *  @ingroup input
     */
    MDB_ACTION_REPEAT                 = 2
    /*! @} */

};

/*! @defgroup keys Keyboard keys
 *
 *  See [key input](@ref input_key) for how these are used.
 *
 *  These key codes are inspired by the _USB HID Usage Tables v1.12_ (p. 53-60),
 *  but re-arranged to map to 7-bit ASCII for printable keys (function keys are
 *  put in the 256+ range).
 *
 *  The naming of the key codes follow these rules:
 *   - The US keyboard layout is used
 *   - Names of printable alpha-numeric characters are used (e.g. "A", "R",
 *     "3", etc.)
 *   - For non-alphanumeric characters, Unicode:ish names are used (e.g.
 *     "COMMA", "LEFT_SQUARE_BRACKET", etc.). Note that some names do not
 *     correspond to the Unicode standard (usually for brevity)
 *   - Keys that lack a clear US mapping are named "WORLD_x"
 *   - For non-printable keys, custom names are used (e.g. "F4",
 *     "BACKSPACE", etc.)
 *
 *  @ingroup input
 *  @{
 */

enum
{
    /* The unknown key */
    MDB_KEY_UNKNOWN            = -1,

    /* Printable keys */
    MDB_KEY_SPACE              = 32,
    MDB_KEY_APOSTROPHE         = 39  ,/* ' */
    MDB_KEY_COMMA              = 44  ,/* , */
    MDB_KEY_MINUS              = 45  ,/* - */
    MDB_KEY_PERIOD             = 46  ,/* . */
    MDB_KEY_SLASH              = 47  ,/* / */
    MDB_KEY_0                  = 48,
    MDB_KEY_1                  = 49,
    MDB_KEY_2                  = 50,
    MDB_KEY_3                  = 51,
    MDB_KEY_4                  = 52,
    MDB_KEY_5                  = 53,
    MDB_KEY_6                  = 54,
    MDB_KEY_7                  = 55,
    MDB_KEY_8                  = 56,
    MDB_KEY_9                  = 57,
    MDB_KEY_SEMICOLON          = 59  ,/* ; */
    MDB_KEY_EQUAL              = 61  ,/* = */
    MDB_KEY_A                  = 65,
    MDB_KEY_B                  = 66,
    MDB_KEY_C                  = 67,
    MDB_KEY_D                  = 68,
    MDB_KEY_E                  = 69,
    MDB_KEY_F                  = 70,
    MDB_KEY_G                  = 71,
    MDB_KEY_H                  = 72,
    MDB_KEY_I                  = 73,
    MDB_KEY_J                  = 74,
    MDB_KEY_K                  = 75,
    MDB_KEY_L                  = 76,
    MDB_KEY_M                  = 77,
    MDB_KEY_N                  = 78,
    MDB_KEY_O                  = 79,
    MDB_KEY_P                  = 80,
    MDB_KEY_Q                  = 81,
    MDB_KEY_R                  = 82,
    MDB_KEY_S                  = 83,
    MDB_KEY_T                  = 84,
    MDB_KEY_U                  = 85,
    MDB_KEY_V                  = 86,
    MDB_KEY_W                  = 87,
    MDB_KEY_X                  = 88,
    MDB_KEY_Y                  = 89,
    MDB_KEY_Z                  = 90,
    MDB_KEY_LEFT_BRACKET       = 91  ,/* [ */
    MDB_KEY_BACKSLASH          = 92  ,/* \ */
    MDB_KEY_RIGHT_BRACKET      = 93  ,/* ] */
    MDB_KEY_GRAVE_ACCENT       = 96  ,/* ` */
    MDB_KEY_WORLD_1            = 161 ,/* non-US #1 */
    MDB_KEY_WORLD_2            = 162 ,/* non-US #2 */
    MDB_KEY_ESCAPE             = 256,
    MDB_KEY_ENTER              = 257,
    MDB_KEY_TAB                = 258,
    MDB_KEY_BACKSPACE          = 259,
    MDB_KEY_INSERT             = 260,
    MDB_KEY_DELETE             = 261,
    MDB_KEY_RIGHT              = 262,
    MDB_KEY_LEFT               = 263,
    MDB_KEY_DOWN               = 264,
    MDB_KEY_UP                 = 265,
    MDB_KEY_PAGE_UP            = 266,
    MDB_KEY_PAGE_DOWN          = 267,
    MDB_KEY_HOME               = 268,
    MDB_KEY_END                = 269,
    MDB_KEY_CAPS_LOCK          = 280,
    MDB_KEY_SCROLL_LOCK        = 281,
    MDB_KEY_NUM_LOCK           = 282,
    MDB_KEY_PRINT_SCREEN       = 283,
    MDB_KEY_PAUSE              = 284,
    MDB_KEY_F1                 = 290,
    MDB_KEY_F2                 = 291,
    MDB_KEY_F3                 = 292,
    MDB_KEY_F4                 = 293,
    MDB_KEY_F5                 = 294,
    MDB_KEY_F6                 = 295,
    MDB_KEY_F7                 = 296,
    MDB_KEY_F8                 = 297,
    MDB_KEY_F9                 = 298,
    MDB_KEY_F10                = 299,
    MDB_KEY_F11                = 300,
    MDB_KEY_F12                = 301,
    MDB_KEY_F13                = 302,
    MDB_KEY_F14                = 303,
    MDB_KEY_F15                = 304,
    MDB_KEY_F16                = 305,
    MDB_KEY_F17                = 306,
    MDB_KEY_F18                = 307,
    MDB_KEY_F19                = 308,
    MDB_KEY_F20                = 309,
    MDB_KEY_F21                = 310,
    MDB_KEY_F22                = 311,
    MDB_KEY_F23                = 312,
    MDB_KEY_F24                = 313,
    MDB_KEY_F25                = 314,
    MDB_KEY_KP_0               = 320,
    MDB_KEY_KP_1               = 321,
    MDB_KEY_KP_2               = 322,
    MDB_KEY_KP_3               = 323,
    MDB_KEY_KP_4               = 324,
    MDB_KEY_KP_5               = 325,
    MDB_KEY_KP_6               = 326,
    MDB_KEY_KP_7               = 327,
    MDB_KEY_KP_8               = 328,
    MDB_KEY_KP_9               = 329,
    MDB_KEY_KP_DECIMAL         = 330,
    MDB_KEY_KP_DIVIDE          = 331,
    MDB_KEY_KP_MULTIPLY        = 332,
    MDB_KEY_KP_SUBTRACT        = 333,
    MDB_KEY_KP_ADD             = 334,
    MDB_KEY_KP_ENTER           = 335,
    MDB_KEY_KP_EQUAL           = 336,
    MDB_KEY_LEFT_SHIFT         = 340,
    MDB_KEY_LEFT_CONTROL       = 341,
    MDB_KEY_LEFT_ALT           = 342,
    MDB_KEY_LEFT_SUPER         = 343,
    MDB_KEY_RIGHT_SHIFT        = 344,
    MDB_KEY_RIGHT_CONTROL      = 345,
    MDB_KEY_RIGHT_ALT          = 346,
    MDB_KEY_RIGHT_SUPER        = 347,
    MDB_KEY_MENU               = 348,

    MDB_KEY_LAST               = MDB_KEY_MENU

};

enum
{
    MDB_MOD_SHIFT           = 0x0001,

    /* If this bit is set one or more Control keys were held down. */
    MDB_MOD_CONTROL         = 0x0002,

    /* If this bit is set one or more Alt keys were held down. */
    MDB_MOD_ALT             = 0x0004,

    /* If this bit is set one or more Super keys were held down. */
    MDB_MOD_SUPER           = 0x0008
};



enum
{
    MDB_EVENT_KEYBOARD = 0x100
};



struct mdb_event_keyboard
{
    int key;
    int scancode;
    int action;
    int mods;
};
