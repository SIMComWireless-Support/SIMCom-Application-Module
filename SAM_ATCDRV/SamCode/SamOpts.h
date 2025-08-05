/**
 * @file sam_opts.h
 * @brief SAM configuration file.
 * @version 1.0
 */

#ifndef SAM_OPTS_H
#define SAM_OPTS_H

/**
 * @brief Debug logging system configuration.
 */

/* Global debug level */
#define SAM_DBG_LEVEL          SAM_DBG_LEVEL_INFO

/* Module-specific debug levels */
#define SAM_MQTT_DBG_LEVEL     SAM_DBG_LEVEL_DEBUG
#define SAM_HTTP_DBG_LEVEL     SAM_DBG_LEVEL_INFO
#define SAM_SMS_DBG_LEVEL      SAM_DBG_LEVEL_WARN

/* Log buffer size */
#define SAM_DBG_BUFFER_SIZE    1024

/* Enable/disable the debug logging system */
#define SAM_CFG_DEBUG_ENABLED  1

#endif /* SAM_OPTS_H */
