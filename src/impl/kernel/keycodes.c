#include "keycodes.h"

/*
 * QWERTZ (German) scancode → CP437 byte mapping.
 *
 * Scancode set 1, make codes only (bit 7 stripped by the keyboard driver).
 * Values are CP437 bytes, not Unicode — VGA text mode uses CP437, so
 * e.g. 'ü' is 0x81, not U+00FC.
 *
 * Index 0x00..0x39 covers the main keyboard area. Extended keys (0xE0xx)
 * are filtered out before this table is consulted.
 */

/* Base layer — no shift held */
static const uint8_t base_map[0x3A] = {
    /* 0x00 */ 0x00, 0x00, '1',  '2',  '3',  '4',  '5',  '6',
    /* 0x08 */ '7',  '8',  '9',  '0',  CP437_SHARP_S, CP437_ACUTE,
    /* 0x0E */ '\b', '\t',
    /* 0x10 */ 'q',  'w',  'e',  'r',  't',  'z',  'u',  'i',
    /* 0x18 */ 'o',  'p',  CP437_UDIAERESIS, '+',
    /* 0x1C */ '\n', 0x00, 'a',  's',
    /* 0x20 */ 'd',  'f',  'g',  'h',  'j',  'k',  'l',
    /* 0x27 */ CP437_ODIAERESIS, CP437_ADIAERESIS,
    /* 0x29 */ '^',  0x00, 0x00, CP437_SECTION,
    /* 0x2D */ 'y',  'x',  'c',  'v',
    /* 0x30 */ 'b',  'n',  'm',  ',',  '.',  '-',  0x00, '*',
    /* 0x38 */ 0x00, ' ',
};

/* Shift layer — shift held */
static const uint8_t shift_map[0x3A] = {
    /* 0x00 */ 0x00, 0x00, '!',  '"',  CP437_SECTION, '$',  '%',  '&',
    /* 0x08 */ '/',  '(',  ')',  '=',
    /* 0x0C */ '?',  '`',
    /* 0x0E */ '\b', '\t',
    /* 0x10 */ 'Q',  'W',  'E',  'R',  'T',  'Z',  'U',  'I',
    /* 0x18 */ 'O',  'P',  CP437_UDIAERESIS_UPPER, '*',
    /* 0x1C */ '\n', 0x00, 'A',  'S',
    /* 0x20 */ 'D',  'F',  'G',  'H',  'J',  'K',  'L',
    /* 0x27 */ CP437_ODIAERESIS_UPPER, CP437_ADIAERESIS_UPPER,
    /* 0x29 */ CP437_DEGREE, 0x00, 0x00, '\'',
    /* 0x2D */ 'Y',  'X',  'C',  'V',
    /* 0x30 */ 'B',  'N',  'M',  ';',  ':',  '_',  0x00, '*',
    /* 0x38 */ 0x00, ' ',
};

char to_ascii(uint16_t code, bool shift, bool caps) {
    /* Extended keys (0xE0xx) are not characters — ignore. */
    if (code & 0xFF00) {
        return 0;
    }
    /* Out of range for the main keyboard area. */
    if (code >= 0x3A) {
        return 0;
    }
    bool upper = shift ^ caps;
    return (char)(upper ? shift_map[code] : base_map[code]);
}