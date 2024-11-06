#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Definición de comandos y configuraciones de control de la pantalla LCD
#define LCD_CMD_CLEAR 0x01             // Comando para limpiar la pantalla
#define LCD_CMD_ENTRY_MODE 0x04        // Configura el modo de entrada
#define LCD_CMD_DISPLAY_CTRL 0x08      // Controla el encendido de la pantalla
#define LCD_CMD_FUNCTION_SET 0x20      // Configuración de funciones de la pantalla
#define LCD_OPT_TWO_LINE 0x08          // Configuración de modo de dos líneas
#define LCD_OPT_DISPLAY_ON 0x04        // Habilita el display
#define LCD_OPT_BACKLIGHT 0x08         // Activa la luz de fondo
#define LCD_ENABLE_SIGNAL 0x04         // Bit de habilitación de señal

// Dirección del dispositivo I2C para el controlador LCD
static const int lcd_i2c_address = 0x27;

#define LCD_MODE_CHARACTER 1           // Modo de carácter para enviar datos
#define LCD_MODE_COMMAND 0             // Modo de comando para enviar instrucciones
#define LCD_MAX_LINES 2                // Número máximo de líneas en el LCD
#define LCD_MAX_COLUMNS 16             // Número máximo de columnas por línea

// Función para enviar un byte a través de I2C
void i2c_send_byte(uint8_t data) {
#ifdef i2c_default
    i2c_write_blocking(i2c_default, lcd_i2c_address, &data, 1, false);
#endif
}

// Alterna la señal de habilitación para la pantalla LCD
void lcd_toggle(uint8_t data) {
    sleep_us(600);
    i2c_send_byte(data | LCD_ENABLE_SIGNAL);  // Activa el bit de habilitación
    sleep_us(600);
    i2c_send_byte(data & ~LCD_ENABLE_SIGNAL); // Desactiva el bit de habilitación
    sleep_us(600);
}

// Enviar un byte al LCD en modo de comando o carácter
void lcd_send(uint8_t data, int mode) {
    uint8_t high_nibble = mode | (data & 0xF0) | LCD_OPT_BACKLIGHT;
    uint8_t low_nibble = mode | ((data << 4) & 0xF0) | LCD_OPT_BACKLIGHT;

    i2c_send_byte(high_nibble);
    lcd_toggle(high_nibble);
    i2c_send_byte(low_nibble);
    lcd_toggle(low_nibble);
}

// Borra el contenido de la pantalla LCD
void lcd_clear() {
    lcd_send(LCD_CMD_CLEAR, LCD_MODE_COMMAND);
}

// Posiciona el cursor en la pantalla LCD
void lcd_set_cursor(int line, int pos) {
    int address = (line == 0) ? 0x80 + pos : 0xC0 + pos; // Determina la posición en base a la línea
    lcd_send(address, LCD_MODE_COMMAND);
}

// Muestra un carácter en la posición actual del cursor
void lcd_show_char(char character) {
    lcd_send(character, LCD_MODE_CHARACTER);
}

// Muestra una cadena de caracteres en la pantalla LCD
void lcd_show_string(const char *str) {
    while (*str) {
        lcd_show_char(*str++); // Envía cada carácter de la cadena
    }
}

// Inicializa la pantalla LCD en modo de dos líneas
void lcd_initialize() {
    // Secuencia de inicialización de la pantalla LCD
    lcd_send(0x03, LCD_MODE_COMMAND);
    lcd_send(0x03, LCD_MODE_COMMAND);
    lcd_send(0x03, LCD_MODE_COMMAND);
    lcd_send(0x02, LCD_MODE_COMMAND);

    lcd_send(LCD_CMD_ENTRY_MODE, LCD_MODE_COMMAND);           // Configuración del modo de entrada
    lcd_send(LCD_CMD_FUNCTION_SET | LCD_OPT_TWO_LINE, LCD_MODE_COMMAND);  // Configuración en modo de dos líneas
    lcd_send(LCD_CMD_DISPLAY_CTRL | LCD_OPT_DISPLAY_ON, LCD_MODE_COMMAND); // Enciende la pantalla
    lcd_clear();                                              // Limpia la pantalla
}

int main() {
#if !defined(i2c_default) || !defined(PICO_DEFAULT_I2C_SDA_PIN) || !defined(PICO_DEFAULT_I2C_SCL_PIN)
    #warning "Este ejemplo requiere una placa con pines I2C configurados"
#else
    // Inicializa el I2C en la frecuencia deseada
    i2c_init(i2c_default, 100 * 1000);
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    lcd_initialize();  // Llama a la función de inicialización del LCD

    // Muestra mensajes en la pantalla LCD
    lcd_set_cursor(0, 3);    // Posiciona el cursor en la primera línea, columna 3
    lcd_show_string("Flores Dominguez");

    lcd_set_cursor(1, 3);    // Posiciona el cursor en la segunda línea, columna 3
    lcd_show_string("Eduardo");

    while (1);  // Bucle infinito para mantener la ejecución
#endif
}
