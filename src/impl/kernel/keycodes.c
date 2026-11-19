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
    [0x02] = '1',  [0x03] = '2',  [0x04] = '3',  [0x05] = '4',
    [0x06] = '5',  [0x07] = '6',  [0x08] = '7',  [0x09] = '8',
    [0x0A] = '9',  [0x0B] = '0',
    [0x0C] = CP437_SHARP_S, [0x0D] = CP437_ACUTE,
    [0x0E] = '\b', [0x0F] = '\t',
    [0x10] = 'q',  [0x11] = 'w',  [0x12] = 'e',  [0x13] = 'r',
    [0x14] = 't',  [0x15] = 'z',  [0x16] = 'u',  [0x17] = 'i',
    [0x18] = 'o',  [0x19] = 'p',
    [0x1A] = CP437_UDIAERESIS, [0x1B] = '+',
    [0x1C] = '\n',
    [0x1E] = 'a',  [0x1F] = 's',
    [0x20] = 'd',  [0x21] = 'f',  [0x22] = 'g',  [0x23] = 'h',
    [0x24] = 'j',  [0x25] = 'k',  [0x26] = 'l',
    [0x27] = CP437_ODIAERESIS, [0x28] = CP437_ADIAERESIS,
    [0x29] = '^',
    [0x2B] = CP437_SECTION,
    [0x2C] = 'y',  [0x2D] = 'x',  [0x2E] = 'c',  [0x2F] = 'v',
    [0x30] = 'b',  [0x31] = 'n',  [0x32] = 'm',  [0x33] = ',',
    [0x34] = '.',  [0x35] = '-',
    [0x37] = '*',
    [0x39] = ' ',
};


/* Shift layer — shift held */
static const uint8_t shift_map[0x3A] = {
    [0x02] = '!',  [0x03] = '"',  [0x04] = CP437_SECTION,
    [0x05] = '$',  [0x06] = '%',  [0x07] = '&',
    [0x08] = '/',  [0x09] = '(',  [0x0A] = ')',  [0x0B] = '=',
    [0x0C] = '?',  [0x0D] = '`',
    [0x0E] = '\b', [0x0F] = '\t',
    [0x10] = 'Q',  [0x11] = 'W',  [0x12] = 'E',  [0x13] = 'R',
    [0x14] = 'T',  [0x15] = 'Z',  [0x16] = 'U',  [0x17] = 'I',
    [0x18] = 'O',  [0x19] = 'P',
    [0x1A] = CP437_UDIAERESIS_UPPER, [0x1B] = '*',
    [0x1C] = '\n',
    [0x1E] = 'A',  [0x1F] = 'S',
    [0x20] = 'D',  [0x21] = 'F',  [0x22] = 'G',  [0x23] = 'H',
    [0x24] = 'J',  [0x25] = 'K',  [0x26] = 'L',
    [0x27] = CP437_ODIAERESIS_UPPER, [0x28] = CP437_ADIAERESIS_UPPER,
    [0x29] = CP437_DEGREE,
    [0x2B] = '\'',
    [0x2C] = 'Y',  [0x2D] = 'X',  [0x2E] = 'C',  [0x2F] = 'V',
    [0x30] = 'B',  [0x31] = 'N',  [0x32] = 'M',  [0x33] = ';',
    [0x34] = ':',  [0x35] = '_',
    [0x37] = '*',
    [0x39] = ' ',
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