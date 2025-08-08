#include "serial_port.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <sys/ioctl.h>

// Internal function prototypes
static bool init_ringbuffer(ringbuffer_t *rb, uint32_t size);
static void free_ringbuffer(ringbuffer_t *rb);
static uint32_t ringbuffer_available(ringbuffer_t *rb);
static uint32_t ringbuffer_free(ringbuffer_t *rb);
static uint32_t ringbuffer_write(ringbuffer_t *rb, const uint8_t *data, uint32_t length);
static uint32_t ringbuffer_read(ringbuffer_t *rb, uint8_t *data, uint32_t length);
static bool configure_serial_port(serial_port_t *port);

/**
 * @brief Initialize ring buffer
 * @param rb Pointer to ring buffer structure
 * @param size Desired buffer size
 * @return true on success, false on failure
 */
static bool init_ringbuffer(ringbuffer_t *rb, uint32_t size) {
    rb->buffer = (uint8_t *)malloc(size);
    if (rb->buffer == NULL) {
        return false;
    }
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
    return true;
}

/**
 * @brief Free ring buffer memory
 * @param rb Pointer to ring buffer structure
 */
static void free_ringbuffer(ringbuffer_t *rb) {
    if (rb->buffer != NULL) {
        free(rb->buffer);
        rb->buffer = NULL;
    }
    rb->size = 0;
    rb->head = 0;
    rb->tail = 0;
}

/**
 * @brief Get number of bytes available in ring buffer
 * @param rb Pointer to ring buffer structure
 * @return Number of bytes available to read
 */
static uint32_t ringbuffer_available(ringbuffer_t *rb) {
    if (rb->head >= rb->tail) {
        return rb->head - rb->tail;
    }
    return rb->size - rb->tail + rb->head;
}

/**
 * @brief Get free space in ring buffer
 * @param rb Pointer to ring buffer structure
 * @return Number of bytes that can be written
 */
static uint32_t ringbuffer_free(ringbuffer_t *rb) {
    return rb->size - ringbuffer_available(rb) - 1;
}

/**
 * @brief Write data to ring buffer
 * @param rb Pointer to ring buffer structure
 * @param data Pointer to data to write
 * @param length Number of bytes to write
 * @return Number of bytes actually written
 */
static uint32_t ringbuffer_write(ringbuffer_t *rb, const uint8_t *data, uint32_t length) {
    uint32_t free_space = ringbuffer_free(rb);
    if (free_space == 0) {
        return 0;
    }
    
    length = (length > free_space) ? free_space : length;
    
    uint32_t first_part = rb->size - rb->head;
    if (first_part >= length) {
        memcpy(&rb->buffer[rb->head], data, length);
        rb->head += length;
    } else {
        memcpy(&rb->buffer[rb->head], data, first_part);
        memcpy(rb->buffer, data + first_part, length - first_part);
        rb->head = length - first_part;
    }
    
    if (rb->head >= rb->size) {
        rb->head = 0;
    }
    
    return length;
}

/**
 * @brief Read data from ring buffer
 * @param rb Pointer to ring buffer structure
 * @param data Pointer to buffer for read data
 * @param length Maximum number of bytes to read
 * @return Number of bytes actually read
 */
static uint32_t ringbuffer_read(ringbuffer_t *rb, uint8_t *data, uint32_t length) {
    uint32_t available = ringbuffer_available(rb);
    if (available == 0) {
        return 0;
    }
    
    length = (length > available) ? available : length;
    
    uint32_t first_part = rb->size - rb->tail;
    if (first_part >= length) {
        memcpy(data, &rb->buffer[rb->tail], length);
        rb->tail += length;
    } else {
        memcpy(data, &rb->buffer[rb->tail], first_part);
        memcpy(data + first_part, rb->buffer, length - first_part);
        rb->tail = length - first_part;
    }
    
    if (rb->tail >= rb->size) {
        rb->tail = 0;
    }
    
    return length;
}

/**
 * @brief Configure serial port with specified parameters
 * @param port Pointer to serial port structure
 * @return true on success, false on failure
 */
static bool configure_serial_port(serial_port_t *port) {
    struct termios options;
    
    // Get current port settings
    if (tcgetattr(port->fd, &options) < 0) {
        perror("tcgetattr failed");
        return false;
    }
    
    // Set input/output baud rate
    speed_t baud;
    switch (port->config.baudrate) {
        case 9600:   baud = B9600;   break;
        case 19200:  baud = B19200;  break;
        case 38400:  baud = B38400;  break;
        case 57600:  baud = B57600;  break;
        case 115200: baud = B115200; break;
        case 230400: baud = B230400; break;
        case 460800: baud = B460800; break;
        case 921600: baud = B921600; break;
        default:
            fprintf(stderr, "Unsupported baud rate\n");
            return false;
    }
    cfsetispeed(&options, baud);
    cfsetospeed(&options, baud);
    
    // Enable receiver and set local mode
    options.c_cflag |= (CLOCAL | CREAD);
    
    // Set data bits
    options.c_cflag &= ~CSIZE;
    switch (port->config.data_bits) {
        case 5: options.c_cflag |= CS5; break;
        case 6: options.c_cflag |= CS6; break;
        case 7: options.c_cflag |= CS7; break;
        case 8: options.c_cflag |= CS8; break;
        default:
            fprintf(stderr, "Unsupported data bits\n");
            return false;
    }
    
    // Set parity
    switch (port->config.parity) {
        case 'N': // No parity
            options.c_cflag &= ~PARENB;
            break;
        case 'E': // Even parity
            options.c_cflag |= PARENB;
            options.c_cflag &= ~PARODD;
            break;
        case 'O': // Odd parity
            options.c_cflag |= PARENB;
            options.c_cflag |= PARODD;
            break;
        default:
            fprintf(stderr, "Unsupported parity\n");
            return false;
    }
    
    // Set stop bits
    if (port->config.stop_bits == 2) {
        options.c_cflag |= CSTOPB;
    } else if (port->config.stop_bits == 1) {
        options.c_cflag &= ~CSTOPB;
    } else {
        fprintf(stderr, "Unsupported stop bits\n");
        return false;
    }
    
    // Set flow control
    if (port->config.flow_control) {
        options.c_cflag |= CRTSCTS;
    } else {
        options.c_cflag &= ~CRTSCTS;
    }
    
    // Set raw input mode (no canonical processing)
    //options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    options.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ISIG);
    
    // Disable software flow control
    //options.c_iflag &= ~(IXON | IXOFF | IXANY);
    options.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP 
                       | INLCR | IGNCR | ICRNL | IXON);

    // Raw output mode
    options.c_oflag &= ~OPOST;
    
    // Set minimum characters and timeout
    options.c_cc[VMIN]  = 1;  // Non-blocking read
    options.c_cc[VTIME] = 10; // 1 second timeout
    
    // Apply settings
    if (tcsetattr(port->fd, TCSANOW, &options) < 0) {
        perror("tcsetattr failed");
        return false;
    }
    
    // Flush buffers
    tcflush(port->fd, TCIOFLUSH);
    
    return true;
}

/**
 * @brief Initialize serial port
 * @param port Pointer to serial port structure
 * @param port_name Device path (e.g., "/dev/ttyS0")
 * @param config Pointer to configuration structure
 * @return true on success, false on failure
 */
bool serial_init(serial_port_t *port, const char *port_name, const serial_config_t *config) {
    // Initialize port structure
    memset(port, 0, sizeof(serial_port_t));
    strncpy(port->port_name, port_name, sizeof(port->port_name) - 1);
    port->config = *config;
    port->is_open = false;
    
    // Initialize ring buffers
    if (!init_ringbuffer(&port->rx_buffer, 4096) || !init_ringbuffer(&port->tx_buffer, 4096)) {
        fprintf(stderr, "Failed to allocate ring buffers\n");
        free_ringbuffer(&port->rx_buffer);
        free_ringbuffer(&port->tx_buffer);
        return false;
    }
    
    // Open serial port (non-blocking)
    port->fd = open(port_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (port->fd < 0) {
        perror("open serial port failed");
        free_ringbuffer(&port->rx_buffer);
        free_ringbuffer(&port->tx_buffer);
        return false;
    }
    
    // Configure serial port
    if (!configure_serial_port(port)) {
        close(port->fd);
        free_ringbuffer(&port->rx_buffer);
        free_ringbuffer(&port->tx_buffer);
        return false;
    }
    
    port->is_open = true;
    return true;
}

/**
 * @brief Close serial port and free resources
 * @param port Pointer to serial port structure
 */
void serial_close(serial_port_t *port) {
    if (port->is_open) {
        close(port->fd);
        port->is_open = false;
    }
    free_ringbuffer(&port->rx_buffer);
    free_ringbuffer(&port->tx_buffer);
}

/**
 * @brief Write data to serial port (buffered)
 * @param port Pointer to serial port structure
 * @param data Pointer to data to send
 * @param length Number of bytes to send
 * @return Number of bytes actually written, -1 on error
 */
int serial_write(serial_port_t *port, const uint8_t *data, uint32_t length) {
    if (!port->is_open) {
        return -1;
    }
    
    // First try to write any pending data in TX buffer
    serial_flush(port);
    
    // If TX buffer is empty, try direct write
    if (ringbuffer_available(&port->tx_buffer) == 0) {
        int bytes_written = write(port->fd, data, length);
        if (bytes_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                bytes_written = 0;
            } else {
                perror("write failed");
                return -1;
            }
        }
        
        // If we couldn't write all data, buffer the remainder
        if (bytes_written < length) {
            uint32_t remaining = length - bytes_written;
            uint32_t buffered = ringbuffer_write(&port->tx_buffer, data + bytes_written, remaining);
            return bytes_written + buffered;
        }
        return bytes_written;
    }
    
    // TX buffer not empty, buffer all new data
    return ringbuffer_write(&port->tx_buffer, data, length);
}

/**
 * @brief Read data from serial port buffer
 * @param port Pointer to serial port structure
 * @param data Pointer to buffer for received data
 * @param length Maximum number of bytes to read
 * @return Number of bytes actually read, -1 on error
 */
int serial_read(serial_port_t *port, uint8_t *data, uint32_t length) {
    if (!port->is_open) {
        return -1;
    }
    
    // First try to read from RX buffer
    uint32_t bytes_read = ringbuffer_read(&port->rx_buffer, data, length);
    if (bytes_read > 0) {
        return bytes_read;
    }
    
    // RX buffer empty, read directly from port
    int bytes_received = read(port->fd, data, length);
    if (bytes_received < 0) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0; // No data available
        }
        perror("read failed");
        return -1;
    }
    
    return bytes_received;
}

/**
 * @brief Get number of bytes available in receive buffer
 * @param port Pointer to serial port structure
 * @return Number of bytes available to read
 */
uint32_t serial_available(serial_port_t *port) {
    if (!port->is_open) {
        return 0;
    }
    
    // Check if there's data in RX buffer
    uint32_t available = ringbuffer_available(&port->rx_buffer);
    if (available > 0) {
        return available;
    }
    
    // Check if there's data available from port
    int bytes_available;
    if (ioctl(port->fd, FIONREAD, &bytes_available) < 0) {
        perror("ioctl FIONREAD failed");
        return 0;
    }
    
    return bytes_available;
}

/**
 * @brief Flush transmit buffer (send all buffered data)
 * @param port Pointer to serial port structure
 */
void serial_flush(serial_port_t *port) {
    if (!port->is_open || ringbuffer_available(&port->tx_buffer) == 0) {
        return;
    }
    
    uint8_t temp_buf[256];
    uint32_t bytes_to_write;
    
    while ((bytes_to_write = ringbuffer_available(&port->tx_buffer)) > 0) {
        // Read from TX buffer (without removing)
        bytes_to_write = (bytes_to_write > sizeof(temp_buf)) ? sizeof(temp_buf) : bytes_to_write;
        
        // Handle wrap-around
        uint32_t first_part = port->tx_buffer.size - port->tx_buffer.tail;
        if (first_part >= bytes_to_write) {
            memcpy(temp_buf, &port->tx_buffer.buffer[port->tx_buffer.tail], bytes_to_write);
        } else {
            memcpy(temp_buf, &port->tx_buffer.buffer[port->tx_buffer.tail], first_part);
            memcpy(temp_buf + first_part, port->tx_buffer.buffer, bytes_to_write - first_part);
        }
        
        // Try to write to port
        int bytes_written = write(port->fd, temp_buf, bytes_to_write);
        if (bytes_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // Try again later
            }
            perror("write during flush failed");
            break;
        }
        
        // Advance tail pointer
        port->tx_buffer.tail += bytes_written;
        if (port->tx_buffer.tail >= port->tx_buffer.size) {
            port->tx_buffer.tail -= port->tx_buffer.size;
        }
    }
}