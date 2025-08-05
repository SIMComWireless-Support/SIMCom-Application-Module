/**
 * @file sam_debug.h
 * @brief Header file for the SAM debug logging system.
 * @version 1.0
 */

#ifndef SAM_DEBUG_H
#define SAM_DEBUG_H

#include <stdarg.h>
#include <stdint.h>
#include <stdbool.h>

#include "SamOpts.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Log level enumeration.
 */
typedef enum {
    SAM_DBG_LEVEL_NONE  = 0,  /**< No logging */
    SAM_DBG_LEVEL_ERROR = 1,  /**< Error level */
    SAM_DBG_LEVEL_WARN  = 2,  /**< Warning level */
    SAM_DBG_LEVEL_INFO  = 3,  /**< Information level */
    SAM_DBG_LEVEL_DEBUG = 4,  /**< Debug level */
    SAM_DBG_LEVEL_TRACE = 5,  /**< Trace level */
} sam_dbg_level_e;

/**
 * @brief Module ID enumeration.
 */
typedef enum {
    SAM_MOD_CORE    = 0,  /**< Core module */
    SAM_MOD_NET,             /**< Network module */
    SAM_MOD_ATC,             /**< ATC module */
    SAM_MOD_SOCKET,       /**< SOCKET module */
    SAM_MOD_MQTT,          /**< MQTT module */
    SAM_MOD_HTTP,           /**< HTTP module */
    SAM_MOD_SMS,            /**< SMS module */
    SAM_MOD_CALL ,         /**< CALL module */
    SAM_MOD_FOTA,         /**< FOTA module */
    SAM_MOD_MAX,          /**< Maximum number of modules */
} sam_module_id_e;

/**
 * @brief Custom log output function type.
 * @param msg The log message.
 * @param len The length of the log message.
 */
typedef void (*sam_dbg_output_func_t)(char *msg, uint16_t len);

/**
 * @brief Custom get system time function type.
 * @param buffer The string buffer of system time.
 * @param len The length of the log message.
 */
typedef int (*sam_dbg_gettime_func_t)(char *buffer, uint16_t len);

/**
 * @brief Initialize the debug logging system.
 * @note Should be called at system startup.
 */
void sam_dbg_init(sam_dbg_output_func_t output, sam_dbg_gettime_func_t gettime);

/**
 * @brief Set the global debug level.
 * @param level The new global debug level.
 */
void sam_dbg_set_level(sam_dbg_level_e level);

/**
 * @brief Get the current global debug level.
 * @return The current global debug level.
 */
sam_dbg_level_e sam_dbg_get_level(void);

/**
 * @brief Set the debug level for a specific module.
 * @param module The module ID.
 * @param level The new debug level for the module.
 */
void sam_dbg_set_module_level(sam_module_id_e module, sam_dbg_level_e level);

/**
 * @brief Get the debug level for a specific module.
 * @param module The module ID.
 * @return The current debug level for the module.
 */
sam_dbg_level_e sam_dbg_get_module_level(sam_module_id_e module);

/**
 * @brief Set the custom log output function.
 * @param func The new output function, or NULL to restore the default.
 */
void sam_dbg_set_output(sam_dbg_output_func_t func);

/**
 * @brief Set the custom get system time function for log messages.
 * @param func The custom gettime function, or NULL to restore the default.
 */
void sam_dbg_set_gettime(sam_dbg_gettime_func_t func);

/**
 * @brief Print a log message with location information.
 * @param level The log level.
 * @param module The module ID.
 * @param file The file name.
 * @param line The line number.
 * @param fmt The format string.
 */
void sam_dbg_printf_loc(sam_dbg_level_e level, sam_module_id_e module, const char *file, int line, const char *fmt, ...);

/**
 * @brief Print a log message without location information.
 * @param level The log level.
 * @param fmt The format string.
 */
void sam_dbg_printf(sam_dbg_level_e level, const char *fmt, ...);

#if (SAM_CFG_DEBUG_ENABLED)
/**
 * @brief Output an error log message.
 * @param fmt The format string.
 */
#define SAM_DBG_ERROR(fmt, ...) \
    sam_dbg_printf_loc(SAM_DBG_LEVEL_ERROR, SAM_MOD_MAX, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief Output a warning log message.
 * @param fmt The format string.
 */
#define SAM_DBG_WARN(fmt, ...) \
    sam_dbg_printf_loc(SAM_DBG_LEVEL_WARN, SAM_MOD_MAX, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief Output an information log message.
 * @param fmt The format string.
 */
#define SAM_DBG_INFO(fmt, ...) \
    sam_dbg_printf_loc(SAM_DBG_LEVEL_INFO, SAM_MOD_MAX, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief Output a debug log message.
 * @param fmt The format string.
 */
#define SAM_DBG_DEBUG(fmt, ...) \
    sam_dbg_printf_loc(SAM_DBG_LEVEL_DEBUG, SAM_MOD_MAX, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief Output a trace log message.
 * @param fmt The format string.
 */
#define SAM_DBG_TRACE(fmt, ...) \
    sam_dbg_printf_loc(SAM_DBG_LEVEL_TRACE, SAM_MOD_MAX, __FILE__, __LINE__, fmt, ##__VA_ARGS__)

/**
 * @brief Output a module-specific log message.
 * @param module The module ID.
 * @param level The log level.
 * @param fmt The format string.
 */
#define SAM_DBG_MODULE(module, level, fmt, ...) \
    do { \
        if (sam_dbg_get_module_level(module) >= (level)) { \
            sam_dbg_printf_loc((level), (module), __FILE__, __LINE__, fmt, ##__VA_ARGS__); \
        } \
    } while(0)

#else
#define SAM_DBG_ERROR(fmt, ...) 
#define SAM_DBG_WARN(fmt, ...)
#define SAM_DBG_INFO(fmt, ...)
#define SAM_DBG_DEBUG(fmt, ...)
#define SAM_DBG_TRACE(fmt, ...)
#define SAM_DBG_MODULE(module, level, fmt, ...)
#endif // (SAM_CFG_DEBUG_ENABLED)

extern void DebugTrace(const char *format, ...);
extern void DebugHex(const char *buf,  unsigned short len);

#ifdef __cplusplus
}
#endif

#endif /* SAM_DEBUG_H */
