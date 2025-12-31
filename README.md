# Vindstryka LCD Driver

A C driver for the custom LCD panel found in the [IKEA VINDSTYRKA Air quality sensor](https://www.ikea.com/au/en/p/vindstyrka-air-quality-sensor-smart-60498233/).

This library allows you to control the segments and icons on the display, which is driven by an HT1621 controller.

## Dependencies

This project leverages the [HT1621 Driver](https://github.com/edholmes2232/ht1621) as a submodule to handle the low-level communication with the LCD controller.

Must initialise submodules when cloning:
```bash
git submodule update --init --recursive
```

## Usage

### Demo

[PercentageDemo.webm](https://github.com/user-attachments/assets/a2604ea4-0002-4e78-9e2d-d19f53ba0583)



### Example Code

```c
static void updatePercentage(uint8_t percentage)
{
    if (percentage > 100)
        percentage = 100;

    // Extract digits: hundreds, tens, units
    uint8_t hundreds = percentage / 100;
    uint8_t tens = (percentage / 10) % 10;
    uint8_t units = percentage % 10;

    // Set all 3 groups of 3 digits (digits 0-2, 3-5, 6-8)
    // Group 1: digits 0, 1, 2
    VINDSTRYKA_LCD_SetDigit(0, hundreds);
    VINDSTRYKA_LCD_SetDigit(1, tens);
    VINDSTRYKA_LCD_SetDigit(2, units);

    // Group 2: digits 3, 4, 5
    VINDSTRYKA_LCD_SetDigit(3, hundreds);
    VINDSTRYKA_LCD_SetDigit(4, tens);
    VINDSTRYKA_LCD_SetDigit(5, units);

    // Group 3: digits 6, 7, 8
    VINDSTRYKA_LCD_SetDigit(6, hundreds);
    VINDSTRYKA_LCD_SetDigit(7, tens);
    VINDSTRYKA_LCD_SetDigit(8, units);

    // Set traffic lights cumulatively
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_TL_RED, percentage > 0);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_TL_RED_YELLOW, percentage > 20);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_TL_YELLOW, percentage > 40);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_TL_YELLOW_GREEN, percentage > 60);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_TL_GREEN, percentage > 80);

    // Set brightness levels cumulatively
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_BRIGHTNESS_LEVEL_1, percentage > 0);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_BRIGHTNESS_LEVEL_2, percentage > 33);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_BRIGHTNESS_LEVEL_3, percentage > 66);

    // Set list arrows cumulatively
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_LIST_ARROW_1, percentage > 0);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_LIST_ARROW_2, percentage > 33);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_LIST_ARROW_3, percentage > 66);

    // Update the display
    VINDSTRYKA_LCD_Update();
}

int main(void)
{
    VINDSTRYKA_LCD_Init();
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_BRIGHTNESS, 1);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_LIST, 1);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_TOP_LEFT_UNDERLINE, 1);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_WRENCH, 1);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_TOP_UNDERLINE, 1);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_THERMOMETER, 1);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_CELSIUS, 1);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_CELSIUS_ARROW, 1);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_DROPLET, 1);
    VINDSTRYKA_LCD_SetIcon(VINDSTRYKA_LCD_ICON_PERCENTAGE, 1);

    VINDSTRYKA_LCD_Update();

    for (uint8_t percent = 0; percent <= 100; percent += 1)
    {
        updatePercentage(percent);
        HAL_Delay(50); // Wait half a second between updates
    }

    return 0;
}
```

## License

MIT
