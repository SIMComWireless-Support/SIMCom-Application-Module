#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include <stdint.h>
#include <stdbool.h>

// Ring buffer structure for serial data buffering
typedef struct {
    uint8_t *buffer;    // Pointer to buffer memory
    uint32_t size;      // Total size of the buffer
    uint32_t head;      // Index for writing new data
    uint32_t tail;      // Index for reading old data
} ringbuffer_t;

// Serial port configuration structure
typedef struct {
    int baudrate;       // Baud rate (e.g., 9600, 115200)
    char parity;        // 'N' for none, 'O' for odd, 'E' for even
    uint8_t data_bits;  // Number of data bits (5, 6, 7, 8)
    uint8_t stop_bits;  // Number of stop bits (1, 2)
    bool flow_control;  // Hardware flow control enabled/disabled
} serial_config_t;

// Serial port device structure
typedef struct {
    int fd;                     // File descriptor for the port
    char port_name[64];         // Device path (e.g., "/dev/ttyS0")
    serial_config_t config;      // Current configuration
    ringbuffer_t rx_buffer;      // Receive ring buffer
    ringbuffer_t tx_buffer;      // Transmit ring buffer
    bool is_open;               // Flag indicating if port is open
} serial_port_t;

/**
 * @brief Initialize serial port with given configuration
 * @param port Pointer to serial port structure
 * @param port_name Device path (e.g., "/dev/ttyS0")
 * @param config Pointer to configuration structure
 * @return true on success, false on failure
 */
bool serial_init(serial_port_t *port, const char *port_name, const serial_config_t *config);

/**
 * @brief Close serial port and free resources
 * @param port Pointer to serial port structure
 */
void serial_close(serial_port_t *port);

/**
 * @brief Write data to serial port (buffered)
 * @param port Pointer to serial port structure
 * @param data Pointer to data to send
 * @param length Number of bytes to send
 * @return Number of bytes actually written, -1 on error
 */
int serial_write(serial_port_t *port, const uint8_t *data, uint32_t length);

/**
 * @brief Read data from serial port buffer
 * @param port Pointer to serial port structure
 * @param data Pointer to buffer for received data
 * @param length Maximum number of bytes to read
 * @return Number of bytes actually read, -1 on error
 */
int serial_read(serial_port_t *port, uint8_t *data, uint32_t length);

/**
 * @brief Get number of bytes available in receive buffer
 * @param port Pointer to serial port structure
 * @return Number of bytes available to read
 */
uint32_t serial_available(serial_port_t *port);

/**
 * @brief Flush transmit buffer (send all buffered data)
 * @param port Pointer to serial port structure
 */
void serial_flush(serial_port_t *port);

#endif // SERIAL_PORT_H