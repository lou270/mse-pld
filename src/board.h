/*************
* Project : MSE Avionics
* Board : PAYLOAD
* Description : Board description
**************/

// ==== PIN

// RGB LED
#define RGB_LED_R_PIN 17
#define RGB_LED_G_PIN 18
#define RGB_LED_B_PIN 16

// I2C0
#define I2C0_SDA 20
#define I2C0_SCL 21

// SPI1
#define SPI1_MISO 12
#define SPI1_MOSI 11
#define SPI1_SCLK 10

// Specific radio module
#define RADIO_CS_PIN 13
#define RADIO_DIO0_PIN 3
#define RADIO_RESET_PIN 14
#define RADIO_DIO1_PIN 2

// UART SEQ <-> PLD
#define SEQ_PLD_UART_TX_PIN 0
#define SEQ_PLD_UART_RX_PIN 1

// UART GNSS
#define GNSS_UART_TX_PIN 8
#define GNSS_UART_RX_PIN 9

// Voltage acquisition
#define VBAT_M_PIN A2 // Pin 28 ADC2
#define VD_DIV (2.0/(10.0+2.0))

// Buzzer
#define BUZZER_PIN 10

// PICO Board
#define PICO_LED_PIN 25
#define PICO_BUTTON_PIN 24

// LED
#define PICO_LED_PIN 25
