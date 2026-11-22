# XC_GUI for Raspberry Pi Pico 2 W

## Pico SDK integration guide (C)

<div style="text-align:center; break-after:page; page-break-after:always;">
  <img src="https://chatgpt.com/api/library/files/libfile_27b9728e81208191a0cb8f6d7fe9e2e3/download" alt="XC-IPS-2A TFT LCD module: top view, bottom view, and pin labels" style="width:50%; height:auto; max-height:75vh; object-fit:contain;" />
</div>

This guide creates a C project for the Raspberry Pi Pico 2 W, links the supplied XC_GUI library, and displays a simple ST7789 screen. The example uses SPI0 and GPIO assignments chosen in the application. Call `LCD_SetPinsSPI()` before `GUI_Init()` to pass the chosen pins to the LCD driver.

### What you need

- Raspberry Pi Pico 2 W, a supported ST7789 display, and a USB cable.
- Raspberry Pi Pico C/C++ SDK, Arm GNU toolchain, CMake, and Ninja. The [Raspberry Pi Pico VS Code extension](https://github.com/raspberrypi/pico-vscode) can install and configure these tools.
- These four XC_GUI files: `XC_GUI_PICO2W.a`, `XGUI.h`, `LCD.h`, and `XC_GUI_Types.h`.

Use a Pico SDK setup targeting **Pico 2 W (`pico2_w`) on RP2350 ARM**. This library is a precompiled ARM archive, so it cannot be used as a MicroPython module or linked into an RP2040 or RP2350 RISC-V build. The example was checked with Pico SDK 2.3.0 and Arm GNU Toolchain 15.2.1.

### 1. Create the project

In VS Code, install the Raspberry Pi Pico extension. Choose **Create Pico Project** → **New C/C++ Project**, set the board to **Pico 2 W**, and create a project named `xc_gui_demo`. Choose a C example or a blank project and keep the generated `pico_sdk_import.cmake` in its root directory.

If creating the project without VS Code, create the `xc_gui_demo` directory and copy `external/pico_sdk_import.cmake` from your installed Pico SDK into it. Set `PICO_SDK_PATH` to the SDK directory before configuring CMake. Raspberry Pi's [Pico SDK quick start](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html) describes this project layout.

Your finished project should look like this:

```text
xc_gui_demo/
├── CMakeLists.txt
├── main.c
├── pico_sdk_import.cmake
└── xc_gui/
    ├── XC_GUI_PICO2W.a
    ├── XGUI.h
    ├── LCD.h
    └── XC_GUI_Types.h
```

Copy the four supplied XC_GUI files into the `xc_gui` directory. Keep the three header files together: `XGUI.h` includes the other two.

### 2. Check the display connections

The example uses these GPIO pins. Check your display module's pin labels before wiring it. In another project, you may choose different pins that are valid for the SPI controller you select.

| Display signal | Pico 2 W GPIO | XC_GUI name |
| --- | ---: | --- |
| SPI clock / SCK | GP18 | `PIN_SCK` |
| SPI data in / DIN / MOSI | GP19 | `PIN_MOSI` |
| Chip select / CS | GP17 | `PIN_CS` |
| Data/command / DC | GP20 | `PIN_DC` |
| Reset / RST | GP21 | `PIN_RST` |
| Backlight / BL | Module jumper (always on) | Controlled by the application if needed |
| Display data out / MISO, if present | GP16 | `PIN_MISO` |
| Ground | GND | — |

Connect the module's power input according to **its own voltage specification**. Pico GPIO signals are 3.3 V. MISO is unnecessary for a write-only display. The example still configures GP16 for SPI; it does not require a MISO wire. Fit the module's BL jumper if you want the backlight always on. If you remove the jumper, configure its BL signal separately in your application; BL is not a member of `LCD_SPI_PINS`.

### 3. Set up `CMakeLists.txt`

Replace the generated root `CMakeLists.txt` with the following minimal example. If your VS Code project has SDK/toolchain setup lines above `project(...)`, keep those lines and insert the imported XC_GUI target after `pico_sdk_init()`.

```cmake
cmake_minimum_required(VERSION 3.13)

set(PICO_BOARD pico2_w CACHE STRING "Board type")
include(pico_sdk_import.cmake)  # Must be before project()
project(xc_gui_demo C CXX ASM)
pico_sdk_init()

add_library(xc_gui STATIC IMPORTED)
set_target_properties(xc_gui PROPERTIES
    IMPORTED_LOCATION "${CMAKE_CURRENT_LIST_DIR}/xc_gui/XC_GUI_PICO2W.a"
    INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_LIST_DIR}/xc_gui"
)

add_executable(xc_gui_demo main.c)
target_link_libraries(xc_gui_demo PRIVATE
    xc_gui
    pico_stdlib
    hardware_spi
    hardware_gpio
)
pico_add_extra_outputs(xc_gui_demo)
```

`CMAKE_CURRENT_LIST_DIR` means the directory containing this `CMakeLists.txt`. The archive path must match the **actual filename** you received. The include directory is passed automatically to `main.c` when the application links `xc_gui`.

The project uses C for `main.c` and XC_GUI. Pico SDK may still enable C++ internally when creating the application; you do not need to write C++ code.

### 4. Add `main.c`

Create `main.c` in the project root:

```c
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "XGUI.h"

extern XGUI_DRIVER g_LCD_ST7789_DRV;
extern const XC_FONT g_FontOpenSansLight48;

static GUI_HANDLE gui;
static XC_DATA gui_buffer[4096 * 2];
static volatile int lcd_ready = 1;
static unsigned int frame_count;
static char frame_text[16] = "0";

static void transfer_done(void) { lcd_ready = 1; }
static void frame_done(void) { lcd_ready = 1; }

/* XC_GUI calls this function to draw each block of the display. */
void GUI_AppDraw(size_t frame_start)
{
    if (frame_start != 0) {
        snprintf(frame_text, sizeof(frame_text), "%u", ++frame_count);
    }

    GUI_Clear(XC_Blue);
    GUI_DrawRect(10, 10, 100, 80, XC_White);
    GUI_PrintString(frame_text, XC_White, 10, 168);
}

int main(void)
{
    LCD_SPI_PINS pins;

    stdio_init_all();

    spi_init(spi0, 20 * 1000 * 1000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK, GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);

    /* CS stays a normal GPIO: the LCD driver controls it in software. */
    const unsigned int control_pins[] = {PIN_CS, PIN_DC, PIN_RST};
    for (unsigned int i = 0; i < 3; ++i) {
        gpio_init(control_pins[i]);
        gpio_set_dir(control_pins[i], GPIO_OUT);
        gpio_put(control_pins[i], 1);
    }

    pins.miso = PIN_MISO;
    pins.sck  = PIN_SCK;
    pins.mosi = PIN_MOSI;
    pins.cs   = PIN_CS;
    pins.dc   = PIN_DC;
    pins.rst  = PIN_RST;
    if (LCD_SetPinsSPI(&pins) != 0) {
        while (true) { tight_loop_contents(); }
    }

    if (GUI_Init(&gui, gui_buffer, sizeof(gui_buffer),
                 GUI_PIXEL_FORMAT_RGB565,
                 &g_LCD_ST7789_DRV, spi0, LCD_PORTRAIT) != XC_GUI_OK) {
        while (true) { tight_loop_contents(); }
    }

    GUI_ATH(&gui, "www.gigasitron.com");
    GUI_16BitPerPixel(0);  /* Byte-oriented RGB565 over 8-bit SPI. */
    GUI_SetFont(&g_FontOpenSansLight48);
    GUI_AddCbBlockEnd(transfer_done);
    GUI_AddCbFrameEnd(frame_done);

    while (true) {
        if (lcd_ready) {
            lcd_ready = 0;
            GUI_Draw_Exe();
        }
    }
}
```

The drawing code belongs in `GUI_AppDraw()`. The `frame_start` flag lets you update changing values once per new frame. `LCD_SetPinsSPI()` copies the pin configuration into the driver before display initialization. The SPI handle passed to `GUI_Init()` is `spi0`, which the ST7789 driver uses for display transfers. The two callbacks allow the example to start the next drawing operation when the current block or frame completes.

### 5. Configure and build

In VS Code, open the project folder, confirm that the extension has selected **Pico 2 W**, then run its **Compile Project** command. The output file is `build/xc_gui_demo.uf2` (or the UF2 in the extension's configured build directory).

For a command-line build, open a PowerShell session with the Pico SDK and ARM compiler configured. For example, set `PICO_SDK_PATH` to your installed SDK directory and put its `arm-none-eabi-gcc.exe` toolchain directory on `PATH`. Then run from the project root:

```powershell
cmake -S . -B build -G Ninja -DPICO_BOARD=pico2_w -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel 4
```

On Windows, **specify `-G Ninja` when first creating the build directory** so CMake does not select Visual Studio/MSVC. If an existing `build` directory uses another generator, choose a new directory such as `build-ninja`; CMake cannot change a build directory's generator in place. If CMake cannot locate an already installed picotool, provide `-Dpicotool_DIR="C:/path/to/picotool-config-directory"` when configuring.

### 6. Load the firmware

Disconnect the Pico 2 W. Hold **BOOTSEL** while connecting it to your computer by USB, then copy `xc_gui_demo.uf2` to the `RPI-RP2` drive. The board will reboot and run the display example.

### Common issues

| Symptom | Check |
| --- | --- |
| `XGUI.h: No such file or directory` | The three headers must be in `xc_gui/`, and `INTERFACE_INCLUDE_DIRECTORIES` must point to that folder. |
| Archive missing during link | Check `IMPORTED_LOCATION` against the archive's exact path and filename. |
| `undefined reference to g_LCD_ST7789_DRV` | Confirm that you received the XC_GUI archive that includes the ST7789 driver and linked the `xc_gui` target. |
| `undefined reference to GUI_AppDraw` | Define `GUI_AppDraw(size_t)` in your application, as shown above. |
| `undefined reference to LCD_SetPinsSPI` | Use an archive and `LCD.h` from the same XC_GUI release; older archives lack the configurable-pin function. |
| CMake reports MSVC or a missing boot-stage ELF | Configure a **new** build directory with `-G Ninja` and ensure the Arm GNU toolchain is available. |
| Blank screen | Check power, GND, your chosen SPI/CS/DC/RST wiring, that `LCD_SetPinsSPI()` is called before `GUI_Init()`, and the module's backlight jumper. |

To use different pins, change the pin definitions used by `main.c`, wire the display accordingly, and pass those values to `LCD_SetPinsSPI()` before `GUI_Init()`. The SPI clock, MOSI, and optional MISO pins must support the same SPI instance you pass to `GUI_Init()`; CS, DC, and RST are ordinary output GPIOs.

### References

- [Raspberry Pi Pico C/C++ SDK setup](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html)
- [Raspberry Pi Pico VS Code extension](https://github.com/raspberrypi/pico-vscode)
- [CMake imported static libraries](https://cmake.org/cmake/help/latest/guide/importing-exporting/index.html)
