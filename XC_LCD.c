#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "LCD.h"
#include "XGUI.h"


// SPI Defines
// We are going to use SPI 0, and allocate it to the following GPIO pins
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments

static GUI_HANDLE gui;
static XC_DATA gui_buffer[4096 * 2];
extern XGUI_DRIVER g_LCD_ST7789_DRV;

extern const XC_FONT g_FontralewayBold32;
extern const XC_FONT g_FontOpenSansLight48;
extern const XC_FONT g_sFontCalibri10;
extern const XC_FONT g_sFontCalibri24;
extern const XC_FONT g_FontSevenSegment32;

static volatile int g_bLcdReady = 1;
static void main_cbGuiFrameEnd( void );
static void main_cbLcdTransferDone( void );

int main()
{
    LCD_SPI_PINS pins;

    stdio_init_all();

    // SPI initialisation. This example will use SPI at 1MHz.
    spi_init(spi0, 20 * 1000 * 1000);
    spi_set_format(spi0, 8, SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);

    gpio_set_function(PIN_MISO, GPIO_FUNC_SPI);
    gpio_set_function(PIN_SCK,  GPIO_FUNC_SPI);
    gpio_set_function(PIN_MOSI, GPIO_FUNC_SPI);
    
  // Keep CS under software control because commands and pixel blocks
    // need separate transfers.

    gpio_init(PIN_CS);
    gpio_set_dir(PIN_CS, GPIO_OUT);

    gpio_init(PIN_DC);
    gpio_set_dir(PIN_DC, GPIO_OUT);

    gpio_init(PIN_RST);
    gpio_set_dir(PIN_RST, GPIO_OUT);

    gpio_init(PIN_BL);
    gpio_set_dir(PIN_BL, GPIO_OUT);

    gpio_put(PIN_CS, 1);
    gpio_put(PIN_DC, 1);
    gpio_put(PIN_RST, 1);
    gpio_put(PIN_BL, 1);

    pins.cs = PIN_CS;
    pins.dc = PIN_DC;
    pins.miso = PIN_MISO;
    pins.mosi = PIN_MOSI;
    pins.rst = PIN_RST;
    pins.sck = PIN_SCK;
    
    LCD_SetPinsSPI(&pins);

    // For more examples of SPI use see https://github.com/raspberrypi/pico-examples/tree/master/spi

    if (GUI_Init(&gui, gui_buffer, sizeof(gui_buffer),
                 GUI_PIXEL_FORMAT_RGB565,
                 &g_LCD_ST7789_DRV, spi0,
                 LCD_PORTRAIT) != XC_GUI_OK)
    {
        while (true) { tight_loop_contents(); }
    }

    GUI_ATH(&gui, "www.gigasitron.com");
    GUI_16BitPerPixel(0);  /* Core prepares byte-oriented RGB565 output. */
    GUI_Clear( XC_Blue );

    GUI_SetFontBackColor( XC_Blue );
    GUI_SetFont(&g_FontOpenSansLight48);
	
	GUI_AddCbBlockEnd( main_cbLcdTransferDone );
	
	GUI_AddCbFrameEnd( main_cbGuiFrameEnd );

    while (true)
    {
        if( 0 != g_bLcdReady )
        {
            g_bLcdReady = 0;
            GUI_Draw_Exe();
        }
       
    }
}


static uint32_t g_nCount = 0;
static uint8_t g_Buf[128];
void GUI_AppDraw(size_t bFrameStart)
{
    if( 0 != bFrameStart )
	{
        g_nCount++;
         sprintf(g_Buf, "%02d", g_nCount);
    }
    GUI_Clear(XC_Blue);
    GUI_DrawRect(10, 10, 100, 80, XC_White);
    GUI_PrintString(g_Buf, XC_White, 10,168);
}


static void main_cbLcdTransferDone( void )
{
    g_bLcdReady = 1;
}


static void main_cbGuiFrameEnd( void )
{
    g_bLcdReady = 1;
}