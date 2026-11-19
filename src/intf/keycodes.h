#pragma once

#include <stdint.h>
#include <stdbool.h>

/* Letters — QWERTZ: Y and Z are swapped relative to US QWERTY. */
#define KEY_CODE_A 0x1E
#define KEY_CODE_B 0x30
#define KEY_CODE_C 0x2E
#define KEY_CODE_D 0x20
#define KEY_CODE_E 0x12
#define KEY_CODE_F 0x21
#define KEY_CODE_G 0x22
#define KEY_CODE_H 0x23
#define KEY_CODE_I 0x17
#define KEY_CODE_J 0x24
#define KEY_CODE_K 0x25
#define KEY_CODE_L 0x26
#define KEY_CODE_M 0x32
#define KEY_CODE_N 0x31
#define KEY_CODE_O 0x18
#define KEY_CODE_P 0x19
#define KEY_CODE_Q 0x10
#define KEY_CODE_R 0x13
#define KEY_CODE_S 0x1F
#define KEY_CODE_T 0x14
#define KEY_CODE_U 0x16
#define KEY_CODE_V 0x2F
#define KEY_CODE_W 0x11
#define KEY_CODE_X 0x2D
#define KEY_CODE_Y 0x2C   /* QWERTZ: Y is at the US-Z position */
#define KEY_CODE_Z 0x15   /* QWERTZ: Z is at the US-Y position */

/* Digits (top row) */
#define KEY_CODE_0 0x0B
#define KEY_CODE_1 0x02
#define KEY_CODE_2 0x03
#define KEY_CODE_3 0x04
#define KEY_CODE_4 0x05
#define KEY_CODE_5 0x06
#define KEY_CODE_6 0x07
#define KEY_CODE_7 0x08
#define KEY_CODE_8 0x09
#define KEY_CODE_9 0x0A

/* Control keys */
#define KEY_CODE_BACKSPACE   0x0E
#define KEY_CODE_TAB         0x0F
#define KEY_CODE_ENTER       0x1C
#define KEY_CODE_LEFT_SHIFT  0x2A
#define KEY_CODE_RIGHT_SHIFT 0x36
#define KEY_CODE_LEFT_CTRL   0x1D
#define KEY_CODE_LEFT_ALT    0x38
#define KEY_CODE_CAPS_LOCK   0x3A
#define KEY_CODE_SPACE       0x39

/* QWERTZ punctuation (US positions differ) */
#define KEY_CODE_COMMA       0x33   /* , (shift: ;) */
#define KEY_CODE_PERIOD      0x34   /* . (shift: :) */
#define KEY_CODE_MINUS       0x35   /* - (shift: _) — '/' on US */
#define KEY_CODE_UDIAERESIS  0x1A   /* ü (shift: Ü) — '[' on US */
#define KEY_CODE_PLUS        0x1B   /* + (shift: *) — ']' on US */
#define KEY_CODE_ODIAERESIS  0x27   /* ö (shift: Ö) — ';' on US */
#define KEY_CODE_ADIAERESIS  0x28   /* ä (shift: Ä) — '\'' on US */
#define KEY_CODE_HASH        0x2B   /* # (shift: ') — '\\' on US */
#define KEY_CODE_SHARP_S     0x0C   /* ß (shift: ?) — '-' on US */
#define KEY_CODE_ACUTE       0x0D   /* ´ (shift: `) — '=' on US */
#define KEY_CODE_CIRCUMFLEX  0x29   /* ^ (shift: °) — '`' on US */

/* CP437 glyph byte values (VGA text mode uses CP437, not Unicode). */
#define CP437_UDIAERESIS        0x81   /* ü */
#define CP437_UDIAERESIS_UPPER  0x9A   /* Ü */
#define CP437_ODIAERESIS        0x94   /* ö */
#define CP437_ODIAERESIS_UPPER  0x99   /* Ö */
#define CP437_ADIAERESIS        0x84   /* ä */
#define CP437_ADIAERESIS_UPPER  0x8E   /* Ä */
#define CP437_SHARP_S           0xE1   /* ß */
#define CP437_ACUTE             0xEF   /* ´ */
#define CP437_DEGREE            0xF8   /* ° */
#define CP437_SECTION           0x15   /* § */

char to_ascii(uint16_t code, bool shift, bool caps);
