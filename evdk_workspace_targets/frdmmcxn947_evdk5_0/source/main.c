/*! ***************************************************************************
 *
 * \brief     EVKD5 main application
 * \file      main.c
 * \author    Hugo Arends - HAN Embedded Vision and Machine Learning
 * \author
 * \date      October 2025
 *
 * \copyright 2025 HAN University of Applied Sciences. All Rights Reserved.
 *            \n\n
 *            Permission is hereby granted, free of charge, to any person
 *            obtaining a copy of this software and associated documentation
 *            files (the "Software"), to deal in the Software without
 *            restriction, including without limitation the rights to use,
 *            copy, modify, merge, publish, distribute, sublicense, and/or sell
 *            copies of the Software, and to permit persons to whom the
 *            Software is furnished to do so, subject to the following
 *            conditions:
 *            \n\n
 *            The above copyright notice and this permission notice shall be
 *            included in all copies or substantial portions of the Software.
 *            \n\n
 *            THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *            EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 *            OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *            NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 *            HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *            WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *            FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 *            OTHER DEALINGS IN THE SOFTWARE.
 *
 *****************************************************************************/
#include "fsl_device_registers.h"
#include "fsl_debug_console.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "gpio_input.h"
#include "gpio_output.h"
#include "ssd1306.h"

#include "uvc_camera.h"

#include "fsl_clock.h"
#include "fsl_smartdma.h"
#include "fsl_inputmux.h"
#include "fsl_inputmux_connections.h"
#include "fsl_ov7670.h"

#include "operators.h"

// -----------------------------------------------------------------------------
// Local type definitions
// -----------------------------------------------------------------------------
// Set to 1 to enable OLED.
// Set to 0 to disable OLED.
// Note. The OLED driver is blocking, so if OLED is enabled, a compatible OLED
// display (SSD1306) must be connected to the I2C bus.
#define OLED_ENABLE (0)

// -----------------------------------------------------------------------------
// Local function prototypes
// -----------------------------------------------------------------------------
void systemInit(void);
void exampleWebcamBgr888(void);
void exampleWebcamUint8(void);
void exampleWebcamBgr888TestPattern(void);
void exampleWebcamUint8TestPattern(void);
void exampleThreshold(void);
void exampleRotate(void);
void exampleTemplate(void);
void exampleFinalAssignment(void);
void exampleHuffman(void);

#if (USB_IMAGE_TYPE_UYVY == 1)
void exampleWebcamUyvy(void);
#endif

// -----------------------------------------------------------------------------
// Local variables
// -----------------------------------------------------------------------------

// Note: The heap (malloc()) is used for allocating image data
//
// uint8_pixel_t : 160 * 120 * 1 = 19200 bytes per image
// int16_pixel_t : 160 * 120 * 2 = 38400 bytes per image
// int32_pixel_t : 160 * 120 * 4 = 76800 bytes per image
// uyvy_pixel_t  : 160 * 120 * 2 = 38400 bytes per image
// bgr888_pixel_t: 160 * 120 * 3 = 57600 bytes per image
// float_pixel_t : 160 * 120 * 4 = 76800 bytes per image
//
// The heap is configured to 345888 (0x54720) bytes
// Image data for 345888 / (160 * 120) = 18.015 uint8_pixel_t images can be
// allocated. However, the images called 'cam' and 'usb' are mandatory and are
// already taking 5 (2+3) of these.

// Globally shared images. Allocation of these images is taken care of in
// systemInit(). Not static, so these can be referenced in other source files.
image_t *cam = NULL;
image_t *usb = NULL;

// Variables for timing
static volatile uint32_t ms = 0;
static volatile uint32_t ms1 = 0;
static volatile uint32_t ms2 = 0;

// Variables for SmartDMA
static volatile uint8_t smartdma_stack[64];
static volatile uint32_t smartdma_camera_image_complete = 0;
static smartdma_param_t smartdma_param;

// Variable for USB
// Not static, because referenced in uvc_camera.c
volatile uint32_t image_available_for_usb = 0;

// Camera configuration settings
static ov7670_resource_t ov7670_resource =
    {
        .xclock = kOV7670_InputClock12MHZ,
        .i2cReceiveFunc = BOARD_Camera_I2C_ReceiveSCCB,
        .i2cSendFunc = BOARD_Camera_I2C_SendSCCB,
};

static camera_device_handle_t camera_device_handle =
    {
        .ops = &ov7670_ops,
        .resource = &ov7670_resource,
};

static const camera_config_t camera_config =
    {
        // uyuv_pixel_t takes 2 bytes per pixel
        .bytesPerPixel = 2,
        // Supported frame rates are: 14, 15, 25 and 30 fps
        .framePerSec = 30,
        // Although the define says YUYV, the OV7670 camera module is configured as
        // follows after initialization (also see OV7670 datasheet):
        // TSLB:  0x08 = 0b0000_1000
        // COM13: 0x88 = 0b1000_1000
        // TSLB[3], COM13[0] = 10 => UYVY
        //
        // This means pixels are stored in memory as follows (U is LSB, Y is MSB):
        //  31                                0
        // |YYYYYYYY VVVVVVVV|YYYYYYYY UUUUUUUU|
        // |     pixel 1     |     pixel 0     |
        .pixelFormat = kVIDEO_PixelFormatYUYV,
        // HSYNC/HREF, VSYNC, and PIXCLK signals are used, which is called Gated
        // Clock interface
        .interface = kCAMERA_InterfaceGatedClock,
        // QQVGA (160x120) is the supported resolution due to memory constraints
        .resolution = kVIDEO_ResolutionQQVGA,
};

// -----------------------------------------------------------------------------
// Function implementation
// -----------------------------------------------------------------------------

// Callback function that is called in the IRQ handler of the DMA controller. It
// is called when a new image is available in the cam image. Poll this flag in
// the main loop.
static void SmartDMA_camera_callback(void *param)
{
    smartdma_camera_image_complete = 1;
}

int main(void)
{
    // -------------------------------------------------------------------------
    // Initialize the system
    // -------------------------------------------------------------------------
    systemInit();

    PRINTF("Application start\r\n");

    // -------------------------------------------------------------------------
    // Start webcam mode if SW2 is pressed during startup
    // -------------------------------------------------------------------------
    if ((GPIO0->PDIR & (1 << 23)) == 0)
    {
        PRINTF("SW2 pressed - starting bgr888 webcam mode\r\n");
        exampleWebcamBgr888();
    }

    // -------------------------------------------------------------------------
    // Select one example
    // -------------------------------------------------------------------------

    // exampleWebcamBgr888();
    // exampleWebcamUint8();
    // exampleWebcamBgr888TestPattern();
    // exampleWebcamUint8TestPattern();``````````````````````````````````````
    // exampleThreshold();
    // exampleRotate();
    // exampleTemplate();
    exampleFinalAssignment();
    // exampleHuffman();

    // -------------------------------------------------------------------------
    // Should never reach this
    // -------------------------------------------------------------------------
    PRINTF("Error. Should never reach this when one of the examples is "
           "enabled\r\n");

    while (1U)
    {
        // Intentionally left blank
    }
}

void systemInit(void)
{
    status_t status;

    // -------------------------------------------------------------------------
    // Board
    BOARD_InitBootPins();
    BOARD_PowerMode_OD();
    BOARD_InitBootClocks();
    BOARD_InitDebugConsole();
    CLOCK_SetupExtClocking(BOARD_XTAL0_CLK_HZ);

    PRINTF("\r\n\r\n");
    PRINTF("EVDK5.0 - %s %s\r\n", __DATE__, __TIME__);
    PRINTF("Application init\r\n");

    // -------------------------------------------------------------------------
    // Image memory allocation for static images required by camera and USB.
    // Allocation must be done before DMA initialization!
    cam = newUyvyImage(EVDK5_WIDTH, EVDK5_HEIGHT);

#if (USB_IMAGE_TYPE_BGR888 == 1)
    usb = newBgr888Image(EVDK5_WIDTH, EVDK5_HEIGHT);
#endif

#if (USB_IMAGE_TYPE_UYVY == 1)
    usb = newUyvyImage(EVDK5_WIDTH, EVDK5_HEIGHT);
#endif

    if (usb == NULL)
    {
        PRINTF("Error. Could not allocate image memory\r\n");
        while (1)
        {
        }
    }

    // -------------------------------------------------------------------------
    // SysTick
    status = SysTick_Config(SystemCoreClock / 1000);

    if (status != 0)
    {
        PRINTF("Error. SysTick not started\r\n");
        while (1)
        {
        }
    }

    // Lowest priority for the SysTick timer
    NVIC_SetPriority(SysTick_IRQn, 7);

    // -------------------------------------------------------------------------
    // GPIOs
    gpio_output_init();
    gpio_input_init();

    // -------------------------------------------------------------------------
    // OV7670 camera
    GPIO1->PCOR = (1 << 19); // CAMERA_RST: 0 normal mode; 1 reset mode

    // 10ms settling time for the camera module
    ms = 0;
    while (ms < 10)
    {
    }

    GPIO1->PSOR = (1 << 19); // CAMERA_RST: 0 normal mode; 1 reset mode

    // Set alternative 7 (SmartDMA_PIOn) for all data pins
    PORT1->PCR[4] = PORT_PCR_MUX(7) | PORT_PCR_IBE(1);
    PORT1->PCR[5] = PORT_PCR_MUX(7) | PORT_PCR_IBE(1);
    PORT1->PCR[6] = PORT_PCR_MUX(7) | PORT_PCR_IBE(1);
    PORT1->PCR[7] = PORT_PCR_MUX(7) | PORT_PCR_IBE(1);
    PORT3->PCR[4] = PORT_PCR_MUX(7) | PORT_PCR_IBE(1);
    PORT3->PCR[5] = PORT_PCR_MUX(7) | PORT_PCR_IBE(1);
    PORT1->PCR[10] = PORT_PCR_MUX(7) | PORT_PCR_IBE(1);
    PORT1->PCR[11] = PORT_PCR_MUX(7) | PORT_PCR_IBE(1);

    // Enable and set clocks for LPFLEXCOMM7 and LPI2C7
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM7);
    CLOCK_EnableClock(kCLOCK_LPFlexComm7);
    CLOCK_EnableClock(kCLOCK_LPI2c7);
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom7Clk, 1u);

    // Initialize LPI2C7
    BOARD_Camera_I2C_Init();

    status = CAMERA_DEVICE_Init(&camera_device_handle, &camera_config);

    if (status != kStatus_Success)
    {
        PRINTF("Error. Camera not initialized\r\n");
        while (1)
        {
        }
    }

    // -------------------------------------------------------------------------
    // SmartDMA
    // SmartDMA firmware is copied from FLASH to SRAMX
    SMARTDMA_Init(SMARTDMA_CAMERA_MEM_ADDR, s_smartdmaCameraFirmware,
                  SMARTDMA_CAMERA_FIRMWARE_SIZE);

    // Set the callback function. This function will be called when an entire
    // frame from the camera is available.
    SMARTDMA_InstallCallback(SmartDMA_camera_callback, NULL);

    // Enable SmartDMA interrupts with highest priority
    NVIC_SetPriority(SMARTDMA_IRQn, 0);
    NVIC_EnableIRQ(SMARTDMA_IRQn);

    // SmartDMA core needs its own stack. The file fsl_smartdma_mcxn.h
    // describes that is shall be at least 64 bytes.
    smartdma_param.cameraParam.smartdma_stack = (uint32_t *)smartdma_stack;
    // Configure pointer for storing camera data
    smartdma_param.cameraParam.p_buffer = (uint32_t *)(cam->data);
    // Boot the SMARTDMA to run program.
    SMARTDMA_Boot(kSMARTDMA_FlexIO_CameraWholeFrame, &smartdma_param, 0x2);

    // -------------------------------------------------------------------------
    // USB
    USB_DeviceApplicationInit();

#if (USB_IMAGE_TYPE_UYVY == 1)
    // In UYVY webcam mode, always start the example here. This keeps main nice
    // and clean
    exampleWebcamUyvy();
#endif

#if (OLED_ENABLE == 1)
    // -------------------------------------------------------------------------
    // SSD1306 Oled display
    ssd1306_init();
    ssd1306_setorientation(1);
    ssd1306_clearscreen();
    ssd1306_drawbitmap(made_by_ese);
    ssd1306_update();
#endif
}

// System tick timer IRQ handler. Is called every millisecond.
void SysTick_Handler(void)
{
    ms++;
}

// -----------------------------------------------------------------------------
// BGR888 examples
// -----------------------------------------------------------------------------

#if (USB_IMAGE_TYPE_BGR888 == 1)
void exampleWebcamBgr888(void)
{
    PRINTF("%s\r\n", __func__);

    while (1U)
    {
        // ---------------------------------------------------------------------
        // Wait for camera image complete
        // ---------------------------------------------------------------------
        while (smartdma_camera_image_complete == 0)
        {
        }

        smartdma_camera_image_complete = 0;

        // Copy timestamp
        ms1 = ms;

        // ---------------------------------------------------------------------
        // Image processing pipeline
        // ---------------------------------------------------------------------
        // Convert uyvy_pixel_t camera image to bgr888_pixel_t image for USB
        convertUyvyToBgr888(cam, usb);

#if 0
        // ---------------------------------------------------------------------
        // Optionally wait while frame is being handled by USB.
        // This will wait forever if there is no USB connection!
        // ---------------------------------------------------------------------
        while(image_available_for_usb == 1)
        {}
#endif

        // ---------------------------------------------------------------------
        // Set flag for USB interface that a new frame is available
        // ---------------------------------------------------------------------
        image_available_for_usb = 1;

        // Copy timestamp
        ms2 = ms;

        // Print debug info
        PRINTF("%d | delta: %d ms\r\n", ms1, ms2 - ms1);
    }
}

void exampleWebcamUint8(void)
{
    PRINTF("%s\r\n", __func__);

    // -------------------------------------------------------------------------
    // Local image memory allocation
    // -------------------------------------------------------------------------
    image_t *src = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);
    image_t *dst = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);

    if (dst == NULL)
    {
        PRINTF("Could not allocate image memory\r\n");
        while (1)
        {
        }
    }

    while (1U)
    {
        // ---------------------------------------------------------------------
        // Wait for camera image complete
        // ---------------------------------------------------------------------
        while (smartdma_camera_image_complete == 0)
        {
        }

        smartdma_camera_image_complete = 0;

        // Copy timestamp
        ms1 = ms;

        // ---------------------------------------------------------------------
        // Image processing pipeline
        // ---------------------------------------------------------------------
        // Convert uyvy_pixel_t camera image to uint8_pixel_t image
        convertUyvyToUint8(cam, src);

        // Examples, select one!
        copyUint8Image(src, dst);
        // scale(src, dst);
        // brightness(src, dst, 100);
        // contrast(src, dst, 10.0f);

        // Convert uint8_pixel_t image to bgr888_pixel_t image for USB
        convertUint8ToBgr888(dst, usb);

        // ---------------------------------------------------------------------
        // Set flag for USB interface that a new frame is available
        // ---------------------------------------------------------------------
        image_available_for_usb = 1;

        // Copy timestamp
        ms2 = ms;

        // Print debug info
        PRINTF("%d | delta: %d ms\r\n", ms1, ms2 - ms1);
    }
}

void exampleWebcamBgr888TestPattern(void)
{
    PRINTF("%s\r\n", __func__);

    // -------------------------------------------------------------------------
    // Local image memory allocation
    // -------------------------------------------------------------------------
    image_t *bgr888 = newBgr888Image(EVDK5_WIDTH, EVDK5_HEIGHT);

    const bgr888_pixel_t colorLut[] =
        {
            {.b = 0x00, .g = 0x00, .r = 0xFF}, // Red
            {.b = 0x00, .g = 0xFF, .r = 0x00}, // Green
            {.b = 0xFF, .g = 0x00, .r = 0x00}, // Blue
            {.b = 0x00, .g = 0xFF, .r = 0xFF}, // Yellow
            {.b = 0xFF, .g = 0x00, .r = 0xFF}, // Magenta
            {.b = 0xFF, .g = 0xFF, .r = 0x00}, // Cyan
            {.b = 0xFF, .g = 0xFF, .r = 0xFF}, // White
            {.b = 0x00, .g = 0x00, .r = 0x00}, // Black
        };

    const uint32_t numberOfColors = sizeof(colorLut) / sizeof(bgr888_pixel_t);
    bgr888_pixel_t color = {0x00, 00, 00};
    uint32_t cnt = 0;

    while (1U)
    {
        // ---------------------------------------------------------------------
        // Wait for camera image complete
        // ---------------------------------------------------------------------
        while (smartdma_camera_image_complete == 0)
        {
        }

        smartdma_camera_image_complete = 0;

        // ---------------------------------------------------------------------
        // Image processing pipeline
        // ---------------------------------------------------------------------
        // Change color every second
        if (ms1 < ms)
        {
            ms1 = ms + 1000;

            // Set color
            color = colorLut[cnt];

            // Calculate next index
            cnt = (cnt == (numberOfColors - 1)) ? 0 : cnt + 1;

            PRINTF("Color: B=0x%02X G=0x%02X R=0x%02X\r\n",
                   color.b, color.g, color.r);
        }

        // \todo Copy-and-paste week 1 code here

        // Set all pixels to color
        bgr888_pixel_t *bgr888_pixel = (bgr888_pixel_t *)bgr888->data;
        int32_t len = bgr888->rows * bgr888->cols;
        while (len > 0)
        {
            *bgr888_pixel = color;
            bgr888_pixel++;
            len--;
        }

        // Convert bgr888_pixel_t image to USB
        copyBgr888Image(bgr888, usb);

        // ---------------------------------------------------------------------
        // Set flag for USB interface that a new frame is available
        // ---------------------------------------------------------------------
        image_available_for_usb = 1;
    }
}

void exampleWebcamUint8TestPattern(void)
{
    PRINTF("%s\r\n", __func__);

    // -------------------------------------------------------------------------
    // Local image memory allocation
    // -------------------------------------------------------------------------
    image_t *img0 = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);

    while (1U)
    {
        // ---------------------------------------------------------------------
        // Wait for camera image complete
        // ---------------------------------------------------------------------
        while (smartdma_camera_image_complete == 0)
        {
        }

        smartdma_camera_image_complete = 0;

        // ---------------------------------------------------------------------
        // Image processing pipeline
        // ---------------------------------------------------------------------
        // Set test pattern: pixel value is row number
        for (int r = 0; r < img0->rows; r++)
        {
            memset(img0->data + (r * img0->cols), r, img0->cols);
        }

        // Convert uint8_pixel_t image to bgr888_pixel_t image for USB
        convertToBgr888(img0, usb);

        // ---------------------------------------------------------------------
        // Set flag for USB interface that a new frame is available
        // ---------------------------------------------------------------------
        image_available_for_usb = 1;
    }
}
#endif

// -----------------------------------------------------------------------------
// UYVY examples
// -----------------------------------------------------------------------------

#if (USB_IMAGE_TYPE_UYVY == 1)
void exampleWebcamUyvy(void)
{
    PRINTF("%s\r\n", __func__);

    while (1U)
    {
        // Wait for camera image complete
        while (smartdma_camera_image_complete == 0)
        {
        }

        // Reset the flag
        smartdma_camera_image_complete = 0;

        // Copy the image. To speed thing up even more, the usb and cam images
        // can be made to point to the same image data buffer.
        copyUyvyImage(cam, usb);

        // Set flag for USB interface that a new frame is available
        image_available_for_usb = 1;
    }
}
#endif

// -----------------------------------------------------------------------------
// Other examples
// -----------------------------------------------------------------------------

void exampleThreshold(void)
{
    PRINTF("%s\r\n", __func__);

    // -------------------------------------------------------------------------
    // Local image memory allocation
    // -------------------------------------------------------------------------
    image_t *src = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);
    image_t *dst = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);

    if (dst == NULL)
    {
        PRINTF("Could not allocate image memory\r\n");
        while (1)
        {
        }
    }

    while (1U)
    {
        // ---------------------------------------------------------------------
        // Wait for camera image complete
        // ---------------------------------------------------------------------
        while (smartdma_camera_image_complete == 0)
        {
        }

        smartdma_camera_image_complete = 0;

        // Copy timestamp
        ms1 = ms;

        // ---------------------------------------------------------------------
        // Image processing pipeline
        // ---------------------------------------------------------------------
        // Convert uyvy_pixel_t camera image to uint8_pixel_t image
        convertToUint8(cam, src);

        threshold(src, dst, 0, 64);

#if (OLED_ENABLE == 1)
        // Show binary image on OLED display
        ssd1306_showimage(dst);
        ssd1306_update();
#endif

        // Convert uint8_pixel_t image to bgr888_pixel_t image for USB
        convertToBgr888(dst, usb);

        // Copy timestamp
        ms2 = ms;

        // ---------------------------------------------------------------------
        // Set flag for USB interface that a new frame is available
        // ---------------------------------------------------------------------
        image_available_for_usb = 1;

        // Print debug info
        PRINTF("%d | delta: %d ms\r\n", ms1, ms2 - ms1);
    }
}

void exampleRotate(void)
{
    PRINTF("%s\r\n", __func__);

    // Update SysTick to increase precision to 0.01ms (=10us)
    uint32_t status = SysTick_Config(SystemCoreClock / 100000);

    if (status != 0)
    {
        PRINTF("SysTick update failed\r\n");
        while (1)
        {
        }
    }

    PRINTF("SysTick updated successfully\r\n");

    // -------------------------------------------------------------------------
    // Local image memory allocation
    // -------------------------------------------------------------------------
    image_t *src = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);
    image_t *dst = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);

    if (dst == NULL)
    {
        PRINTF("Could not allocate image memory\r\n");
        while (1)
        {
        }
    }

    while (1U)
    {
        // ---------------------------------------------------------------------
        // Wait for camera image complete
        // ---------------------------------------------------------------------
        while (smartdma_camera_image_complete == 0)
        {
        }

        smartdma_camera_image_complete = 0;

        // ---------------------------------------------------------------------
        // Image processing pipeline
        // ---------------------------------------------------------------------
        convertToUint8(cam, src);

        // Prepare
        clearUint8Image(dst);

        ms1 = ms;
        rotate(src, dst, 3.1415f, (point_t){src->cols / 2, src->rows / 2});
        ms2 = ms;
        PRINTF("%06d us", (ms2 - ms1) * 10);

        // Prepare
        copyUint8Image(src, dst);

        ms1 = ms;
        rotate180_c(dst);
        ms2 = ms;
        PRINTF(" | %03d us", (ms2 - ms1) * 10);

        // Prepare
        copyUint8Image(src, dst);

        ms1 = ms;
        rotate180_arm(dst);
        ms2 = ms;
        PRINTF(" | %03d us", (ms2 - ms1) * 10);

        // Prepare
        copyUint8Image(src, dst);

        ms1 = ms;
        rotate180_cm33(dst);
        ms2 = ms;
        PRINTF(" | %03d us", (ms2 - ms1) * 10);

        PRINTF("\r\n");

        convertToBgr888(dst, usb);

        // ---------------------------------------------------------------------
        // Set flag for USB interface that a new frame is available
        // ---------------------------------------------------------------------
        image_available_for_usb = 1;
    }
}

void exampleTemplate(void)
{
    PRINTF("%s\r\n", __func__);

    // Update SysTick to have better precision
    SysTick_Config(SystemCoreClock / 100000);

    // -------------------------------------------------------------------------
    // Local image memory allocation
    // -------------------------------------------------------------------------
    // Create additional int16_pixel_t images
    image_t *src_int16 = newInt16Image(EVDK5_WIDTH, EVDK5_HEIGHT);
    image_t *dst_int16 = newInt16Image(EVDK5_WIDTH, EVDK5_HEIGHT);
    image_t *msk_int16 = newEmptyInt16Image(3, 3);

    // Prepare images
    clearInt16Image(src_int16);
    clearInt16Image(dst_int16);

    // Set convolution mask
    int16_pixel_t msk_data[3 * 3] =
        {
            // Identity
            0,
            0,
            0,
            0,
            1,
            0,
            0,
            0,
            0,
            // Edge enhancement
            // -1,-1,-1,
            // -1, 8,-1,
            // -1,-1,-1,
            // Sharpen
            // 0,-1, 0,
            // -1, 5,-1,
            // 0,-1, 0,
        };

    msk_int16->data = (uint8_t *)msk_data;

    image_t *src = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);
    image_t *dst = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);

    if (dst == NULL)
    {
        PRINTF("Could not allocate image memory\r\n");
        while (1)
        {
        }
    }

    while (1U)
    {
        // ---------------------------------------------------------------------
        // Wait for camera image complete
        // ---------------------------------------------------------------------
        while (smartdma_camera_image_complete == 0)
        {
        }

        smartdma_camera_image_complete = 0;

        // ---------------------------------------------------------------------
        // Image processing pipeline
        // ---------------------------------------------------------------------
        // Convert uyvy_pixel_t camera image to uint8_pixel_t image
        convertToUint8(cam, src_int16);

        // Copy timestamp
        ms1 = ms;

        // Use this as a playground for testing image processing functions. As
        // an example, the following function scales the image for better
        // visualization.

        // scale(src, dst); // 3520 us
        // scaleFast(src, dst); // 1870 us

        // clearUint8Image(dst); // 0130 us
        // clearUint8Image_cm33(dst); // 0060 us

        // convolve(src_int16, dst_int16, msk_int16); // 35740 us
        // convolveFast(src_int16, dst_int16, msk_int16); // 5550 us

        // mean(src, dst, 3); // 55610 us
        // meanFast(src, dst); // 8100 us

        // sobel(src_int16, dst_int16, NULL); // 67640 us
        // sobelFast(src_int16, dst_int16); // 14610 us

        // threshold2Means(src_int16, dst_int16, BRIGHTNESS_DARK);
        thresholdOtsu(src_int16, dst_int16, BRIGHTNESS_DARK);

        // fillHolesTwoPass(src_int16, dst_int16);

        // Copy timestamp
        ms2 = ms;

        // Scale for visualisation
        scaleInt16ToUint8(dst_int16, dst);

        // Convert uint8_pixel_t image to bgr888_pixel_t image for USB
        convertToBgr888(dst, usb);

        // ---------------------------------------------------------------------
        // Set flag for USB interface that a new frame is available
        // ---------------------------------------------------------------------
        image_available_for_usb = 1;

        // Print debug info
        PRINTF("%d | delta: %04d us\r\n", ms1, (ms2 - ms1) * 10);
    }
}

void downscale2x_fast(const image_t *src, image_t *dst)
{
    uint8_t *s_row = (uint8_t *)src->data;
    uint8_t *d = (uint8_t *)dst->data;

    // We skip every other row, so the stride is 2 full source rows
    uint32_t src_row_stride = src->cols * 2;

    for (uint32_t y = 0; y < dst->rows; y++)
    {
        uint8_t *s_pixel = s_row;
        for (uint32_t x = 0; x < dst->cols; x++)
        {
            *d++ = *s_pixel; // Grab pixel and move destination pointer
            s_pixel += 2;    // Skip one pixel in the source row
        }
        s_row += src_row_stride; // Skip one full row in the source
    }
}

void upscale2x_fast(const image_t *src, image_t *dst)
{
    uint8_t *s_row = (uint8_t *)src->data;
    uint8_t *d_data = (uint8_t *)dst->data;

    for (uint32_t y = 0; y < src->rows; y++)
    {
        // Pointer to the start of the two rows in the destination we need to fill
        uint8_t *d_row0 = &d_data[(y * 2) * dst->cols];
        uint8_t *d_row1 = d_row0 + dst->cols;

        for (uint32_t x = 0; x < src->cols; x++)
        {
            uint8_t val = *s_row++; // Grab the small pixel

            // Write it twice horizontally in the first row
            *d_row0++ = val;
            *d_row0++ = val;

            // Write it twice horizontally in the second row
            *d_row1++ = val;
            *d_row1++ = val;
        }
    }
}

typedef struct
{
    int32_t x;
    int32_t y;
    bgr888_pixel_t color;
    bool detected;
} detection_result_t;

detection_result_t processBlobAnalysis(image_t *lbl_small, uint32_t numBlobs)
{
    bgr888_pixel_t GREEN = {0, 255, 0};
    bgr888_pixel_t BLUE = {255, 0, 0};
    bgr888_pixel_t YELLOW = {0, 255, 255};
    bgr888_pixel_t RED = {0, 0, 255};

    detection_result_t result;
    result.x = -1;
    result.y = -1;
    result.color = RED;
    result.detected = false;

    if (numBlobs > 0)
    {
        blobinfo_t firstBlob;
        memset(&firstBlob, 0, sizeof(blobinfo_t));

        centroid(lbl_small, &firstBlob, 1);
        circularity(lbl_small, &firstBlob, 1);
        huInvariantMoments(lbl_small, &firstBlob, 1);

        result.x = (int32_t)(firstBlob.centroid.x * 2.0f);
        result.y = (int32_t)(firstBlob.centroid.y * 2.0f);
        result.detected = true;

        // PRINTF("------- BLOB ANALYSIS -------\n");
        // PRINTF("Circularity    : %.4f\n", firstBlob.circularity);
        // PRINTF("-----------------------------\n");

        // for (int i = 0; i < 4; i++)
        // {
        //     PRINTF("Hu Moment phi%d : %.6f\n", i + 1, firstBlob.hu_moments[i]);
        // }

        // PRINTF("-----------------------------\n\n");

        /*
         * SHAPE CLASSIFICATION REFERENCE TABLE
         * ---------------------------------------------------------------------------
         * Feature          | Circle (Green)   | Square (Blue)    | Triangle (Yellow)
         * ---------------------------------------------------------------------------
         * Circularity      | 0.995 - 1.016    | 0.797 - 0.836    | 0.635 - 0.674
         * Hu Moment phi1   | 0.1591 - 0.1592  | 0.1657 - 0.1669  | 0.1890 - 0.1933
         * Hu Moment phi2   | ~0.000001        | ~0.000004        | ~0.000030
         * Hu Moment phi3   | ~0.000000        | ~0.000003        | 0.0039 - 0.0047
         * Hu Moment phi4   | 0.000000         | 0.000000         | ~0.000001
         * ---------------------------------------------------------------------------
         */

        float phi1 = firstBlob.hu_moments[0];

        if (phi1 < 0.16f && firstBlob.circularity > 0.900f)
        {
            result.color = GREEN; // Circle
        }
        else if (phi1 < 0.18f && firstBlob.circularity > 0.750f)
        {
            result.color = BLUE; // Square
        }
        else if (phi1 < 0.20f && firstBlob.circularity > 0.600f)
        {
            result.color = YELLOW; // Triangle
        }
        else
        {
            result.color = RED; // Unknown
        }
    }

    return result;
}

void exampleFinalAssignment(void)
{
    PRINTF("%s\r\n", __func__);

    // -------------------------------------------------------------------------
    // Local image memory allocation
    // -------------------------------------------------------------------------
    image_t *src = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);
    image_t *dst = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);

    image_t *src_small = newUint8Image(EVDK5_WIDTH / 2, EVDK5_HEIGHT / 2);
    image_t *thr_small = newUint8Image(EVDK5_WIDTH / 2, EVDK5_HEIGHT / 2);
    image_t *rbb_small = newUint8Image(EVDK5_WIDTH / 2, EVDK5_HEIGHT / 2);
    image_t *lbl_small = newUint8Image(EVDK5_WIDTH / 2, EVDK5_HEIGHT / 2);

    clearUint8Image(src);
    clearUint8Image(dst);
    clearUint8Image(src_small);
    clearUint8Image(thr_small);
    clearUint8Image(rbb_small);
    clearUint8Image(lbl_small);

    if (src_small == NULL)
    {
        PRINTF("Could not allocate image memory\r\n");
        while (1)
        {
        }
    }

    while (1U)
    {
        // Wait for camera image complete
        while (smartdma_camera_image_complete == 0)
        {
        }

        smartdma_camera_image_complete = 0;

        // ---------------------------------------------------------------------
        // Image processing pipeline
        // ---------------------------------------------------------------------
        ms1 = ms;

        // TIMING: 1ms
        convertUyvyToUint8(cam, src);

        // TIMING: 0-1ms
        downscale2x_fast(src, src_small);

        // TIMING: 0-1ms
        threshold(src_small, thr_small, 0, 60);

        // TIMING: 2ms
        removeBorderBlobsTwoPass(thr_small, rbb_small, CONNECTED_FOUR, 100);

        // TIMING: 2ms
        uint32_t numBlobs = labelTwoPass(rbb_small, lbl_small, CONNECTED_FOUR, 100);

        // TIMING: 3-4ms
        detection_result_t result = processBlobAnalysis(lbl_small, numBlobs);

        // TIMING: 0-1ms
        uint8_t *d_data = (uint8_t *)rbb_small->data;

        for (int i = 0; i < (rbb_small->rows * rbb_small->cols); i++)
        {
            if (d_data[i] > 0)
            {
                d_data[i] = 255;
            }
        }

        // TIMING: 0-1ms
        upscale2x_fast(rbb_small, dst);

        // TIMING: 1-2ms
        convertUint8ToBgr888(dst, usb);

        // TIMING: 0-1ms
        if (result.detected && result.x >= 0 && result.x < usb->cols && result.y >= 0 && result.y < usb->rows)
        {
            int size = 12;
            point_t h_start = {result.x - size, result.y};
            point_t h_end = {result.x + size, result.y};
            point_t v_start = {result.x, result.y - size};
            point_t v_end = {result.x, result.y + size};

            drawLineBgr888(usb, h_start, h_end, result.color);
            drawLineBgr888(usb, v_start, v_end, result.color);
        }

        ms2 = ms;

        // TOTAL TIMING: 10-12ms

        // ---------------------------------------------------------------------
        // Set flag for USB interface that a new frame is available
        // ---------------------------------------------------------------------
        image_available_for_usb = 1;

        PRINTF("delta: %d ms\r\n", ms2 - ms1);
    }
}

void exampleHuffman(void)
{
    PRINTF("%s\r\n", __func__);

    // Update SysTick to have better precision
    SysTick_Config(SystemCoreClock / 100000);

    // -------------------------------------------------------------------------
    // Local image memory allocation
    // -------------------------------------------------------------------------
    image_t *src = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);
    image_t *dst = newUint8Image(EVDK5_WIDTH, EVDK5_HEIGHT);

    clearUint8Image(src);
    clearUint8Image(dst);

    if (dst == NULL)
    {
        PRINTF("Could not allocate image memory\r\n");
        while (1)
        {
        }
    }

    PRINTF("Starting Huffman Snapshot Test...\r\n");

    while (smartdma_camera_image_complete == 0)
        ;
    smartdma_camera_image_complete = 0;

    convertUyvyToUint8(cam, src);

    uint32_t hist[256] = {0};
    histogram(src, hist);

    LinkedListNode *pq_head = make_huffman_pq(hist);
    TreeNode *huffman_root = make_huffman_tree(pq_head);

    size_t encoded_size = 0;
    uint8_t *encoded_data = encode_image(src, huffman_root, &encoded_size);

    decode_image(encoded_data, encoded_size, huffman_root, dst);

    size_t original_size = src->rows * src->cols;
    PRINTF("Huffman Results: Original: %u bytes | Encoded: %u bytes\r\n", original_size, (uint32_t)encoded_size);
    PRINTF("Compression Ratio: %d%%\r\n", (int)((encoded_size * 100) / original_size));

    PRINTF("Original [0]:  0x%02X\r\n", src->data[0]);
    PRINTF("Encoded [0]:   0x%02X\r\n", encoded_data[0]);
    PRINTF("Decoded [0]:   0x%02X\r\n", dst->data[0]);

    destroy_huffman_tree(&huffman_root);
    free(encoded_data);

    while (1U)
    {
    }
}