#include "vindstryka_lcd.h"
#include "ht1621.h"

// Number of nibbles (4-bit segments) in the Vindstryka LCD
#define VINDSTRYKA_LCD_NUM_NIBBLES 27

// Buffer to hold the nibble data to be sent
static uint8_t _lcd_buffer[VINDSTRYKA_LCD_NUM_NIBBLES] = {0};

// Each digit maps across 2 nibbles
// nibble 1 controls EFG, nibble 2 controls ABCD
static const uint16_t _lcd_digit_map[] = {
    // 0 is A,B,C,D,E,F [ABCD in nibble 1, EF in nibble 2]
    0x050F, //< '0': 0b0000010100001111 - ABCDEF
    0x0006, //< '1': 0b0000000000000110 - BC
    0x030D, //< '2': 0b0000001100001101 - ABDEG
    0x020F, //< '3': 0b0000010000001111 - ABCDG
    0x0606, //< '4': 0b0000001000000110 - BCFG
    0x060B, //< '5': 0b0000011000001011 - ACDFG
    0x070B, //< '6': 0b0000011100001011 - ACDEFG
    0x000E, //< '7': 0b0000000000001110 - ABC
    0x070F, //< '8': 0b0000011100001111 - ABCDEFG
    0x060F, //< '9': 0b0000011000001111 - ABCDFG
    0x070E, //< 'A': 0b0000001100001110 - ABCEFG
    0x0703, //< 'b': 0b0000011100000011 - CDEFG
    0x0509, //< 'C': 0b0000010100001001 - ADEF
    0x0307, //< 'd': 0b0000001100000111 - BCDEG
    0x0709, //< 'E': 0b0000011100001001 - ADEFG
    0x0708, //< 'F': 0b0000001100001000 - AEFG
};

typedef struct
{
    uint8_t nibble_index;
    uint8_t bit_mask;
} icon_map_t;

// Each icon maps to a specific nibble and bit within that nibble
static const icon_map_t _icon_map[] = {
    [VINDSTRYKA_LCD_ICON_ROOM] = {.nibble_index = 2, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_WINDOW] = {.nibble_index = 6, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_HOUSE] = {.nibble_index = 26, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_TREES] = {.nibble_index = 26, .bit_mask = 0x04},
    [VINDSTRYKA_LCD_ICON_WRENCH] = {.nibble_index = 26, .bit_mask = 0x02},
    [VINDSTRYKA_LCD_ICON_ADD_PURIFIER] = {.nibble_index = 26, .bit_mask = 0x01},
    [VINDSTRYKA_LCD_ICON_PURIFIER] = {.nibble_index = 25, .bit_mask = 0x02},
    [VINDSTRYKA_LCD_ICON_LINKED] = {.nibble_index = 25, .bit_mask = 0x01},
    [VINDSTRYKA_LCD_ICON_TOP_LEFT_UNDERLINE] = {.nibble_index = 4, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_TOP_UNDERLINE] = {.nibble_index = 25, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_PM25] = {.nibble_index = 25, .bit_mask = 0x04},
    [VINDSTRYKA_LCD_ICON_TL_RED] = {.nibble_index = 24, .bit_mask = 0x01},
    [VINDSTRYKA_LCD_ICON_TL_RED_YELLOW] = {.nibble_index = 24, .bit_mask = 0x02},
    [VINDSTRYKA_LCD_ICON_TL_YELLOW] = {.nibble_index = 24, .bit_mask = 0x04},
    [VINDSTRYKA_LCD_ICON_TL_YELLOW_GREEN] = {.nibble_index = 24, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_TL_GREEN] = {.nibble_index = 15, .bit_mask = 0x8},
    [VINDSTRYKA_LCD_ICON_BRIGHTNESS] = {.nibble_index = 8, .bit_mask = 0x01},
    [VINDSTRYKA_LCD_ICON_BRIGHTNESS_LEVEL_1] = {.nibble_index = 8, .bit_mask = 0x02},
    [VINDSTRYKA_LCD_ICON_BRIGHTNESS_LEVEL_2] = {.nibble_index = 8, .bit_mask = 0x04},
    [VINDSTRYKA_LCD_ICON_BRIGHTNESS_LEVEL_3] = {.nibble_index = 8, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_AUTO_OFF] = {.nibble_index = 11, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_THERMOMETER] = {.nibble_index = 9, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_FARENHEIT] = {.nibble_index = 13, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_FARENHEIT_ARROW] = {.nibble_index = 15, .bit_mask = 0x04},
    [VINDSTRYKA_LCD_ICON_CELSIUS] = {.nibble_index = 15, .bit_mask = 0x01},
    [VINDSTRYKA_LCD_ICON_CELSIUS_ARROW] = {.nibble_index = 15, .bit_mask = 0x02},
    [VINDSTRYKA_LCD_ICON_DROPLET] = {.nibble_index = 16, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_PERCENTAGE] = {.nibble_index = 23, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_TVOC] = {.nibble_index = 20, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_FLOWER] = {.nibble_index = 18, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_ARROW_UP] = {.nibble_index = 22, .bit_mask = 0x04},
    [VINDSTRYKA_LCD_ICON_ARROW_DOWN] = {.nibble_index = 22, .bit_mask = 0x02},
    [VINDSTRYKA_LCD_ICON_ARROW_RIGHT] = {.nibble_index = 22, .bit_mask = 0x01},
    [VINDSTRYKA_LCD_ICON_LIST_ARROW_1] = {.nibble_index = 23, .bit_mask = 0x01},
    [VINDSTRYKA_LCD_ICON_LIST_ARROW_2] = {.nibble_index = 23, .bit_mask = 0x02},
    [VINDSTRYKA_LCD_ICON_LIST_ARROW_3] = {.nibble_index = 22, .bit_mask = 0x08},
    [VINDSTRYKA_LCD_ICON_LIST] = {.nibble_index = 23, .bit_mask = 0x04},
};

//! Clear the LCD buffer (Must be followed by Update() to apply)
void VINDSTRYKA_LCD_Clear(void)
{
    // Clear the LCD buffer
    for (int i = 0; i < VINDSTRYKA_LCD_NUM_NIBBLES; i++)
    {
        _lcd_buffer[i] = 0;
    }
}

/**
 * @brief Update the Vindstryka LCD with the current buffer contents
 */
void VINDSTRYKA_LCD_Update(void)
{
    // Apply the buffer to the HT1621 display
    HT1621_SuccessiveWrite(_lcd_buffer, 0, VINDSTRYKA_LCD_NUM_NIBBLES);
}

/**
 * @brief Initialize the Vindstryka LCD
 */
void VINDSTRYKA_LCD_Init(void)
{
    HT1621_Init();

    // System Enable
    HT1621_WriteCommand(HT1621_CMD_SYSEN);
    // LCD ON
    HT1621_WriteCommand(HT1621_CMD_LCDON);

    // Clear display
    VINDSTRYKA_LCD_Clear();
    VINDSTRYKA_LCD_Update();

    // Set BIAS
    HT1621_WriteCommand(HT1621_CMD_BIAS);
}

/**
 * @brief Set a digit on the Vindstryka LCD.
 *
 * @param digit Digit index (0-VINDSTRYKA_LCD_NUM_DIGITS-1)
 * @param value Hex value to set (0x0-0xF)
 */
void VINDSTRYKA_LCD_SetDigit(uint8_t digit, uint8_t value)
{
    if ((digit >= VINDSTRYKA_LCD_NUM_DIGITS) || (value > 0xF))
    {
        return;
    }

    uint8_t first_nibble = 0;
    switch (digit)
    {
    case 0:
        // Digit 0 = Nibble 2+3,
        first_nibble = 2;
        break;
    case 1:
        // Digit 1 = Nibble 4+5,
        first_nibble = 4;
        break;
    case 2:
        // Digit 2 = Nibble 6+7,
        first_nibble = 6;
        break;
    case 3:
        // Digit 3 = Nibble 9+10
        first_nibble = 9;
        break;
    case 4:
        // Digit 4 = Nibble 11+12,
        first_nibble = 11;
        break;
    case 5:
        // Digit 5 = Nibble 13+14,
        first_nibble = 13;
        break;
    case 6:
        // Digit 6 = Nibble 16+17
        first_nibble = 16;
        break;
    case 7:
        // Digit 7 = Nibble 18+19,
        first_nibble = 18;
        break;
    case 8:
        // Digit 8 = Nibble 20+21
        first_nibble = 20;
        break;
    default:
        return;
    }

    // Clear any previous digits by XORing with 0x0F, so as not to affect icons
    _lcd_buffer[first_nibble] ^= 0x0F;
    _lcd_buffer[first_nibble + 1] ^= 0x0F;

    _lcd_buffer[first_nibble] = (_lcd_digit_map[value] >> 8) & 0xFF; // Upper byte (EFGH)
    _lcd_buffer[first_nibble + 1] = _lcd_digit_map[value] & 0xFF;    // Lower byte (ABCD)
}

/**
 * @brief Set or clear an icon on the Vindstryka LCD.
 *
 * @param icon Icon to set/clear
 * @param state 1 to set, 0 to clear
 */
void VINDSTRYKA_LCD_SetIcon(enum VINSTRYKA_LCD_ICON icon, uint8_t state)
{
    if (state)
    {
        _lcd_buffer[_icon_map[icon].nibble_index] |= _icon_map[icon].bit_mask; // Set the bit
    }
    else
    {
        _lcd_buffer[_icon_map[icon].nibble_index] &= ~_icon_map[icon].bit_mask; // Clear the bit
    }
}