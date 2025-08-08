/**
 * @file sam_debug.c
 * @brief Implementation of the SAM debug logging system.
 * @version 1.0
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <stdlib.h>

#include "SamInc.h"
#include "SamDebug.h"
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__)
#include <sys/time.h>
#endif

/* Module names array */
static const char *module_names[SAM_MOD_MAX] = {
    "CORE", "NET", "ATC", "SOCKET", "MQTT", "HTTP", "SMS", "CALL", "FOTA"
};

/* Log level names array */
static const char *level_names[] = {
    "NONE", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"
};

/* Global debug level */
#ifdef SAM_DBG_LEVEL
static sam_dbg_level_e global_dbg_level = SAM_DBG_LEVEL;
#else
static sam_dbg_level_e global_dbg_level = SAM_DBG_LEVEL_INFO;
#endif 

/* Module-specific debug levels */
static sam_dbg_level_e module_dbg_levels[SAM_MOD_MAX];

/* Custom output function */
static sam_dbg_output_func_t custom_output_func = NULL;

/* Custom get system time function */
static sam_dbg_gettime_func_t custom_gettime_func = NULL;

/* Default buffer size for log messages */
#define SAM_DBG_BUFFER_SIZE 1024

/* Internal buffer for log messages */
static char dbg_buffer[SAM_DBG_BUFFER_SIZE];

/**
 * @brief Get the current time as a string.
 * @param buffer The buffer to store the time string.
 * @param size The size of the buffer.
 * @return 0 on success, -1 on failure.
 */
static int get_current_time(char *buffer, uint16_t size) {

    if (custom_gettime_func != NULL)
    {
        return custom_gettime_func(buffer, size);
    }

#if defined( _WIN32) || defined(__linux__)     
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int milliseconds = 0;
    
    if (t == NULL) {
        return -1;
    }

#ifdef _WIN32
    /* Get milliseconds on Windows platform */
    SYSTEMTIME st;
    GetLocalTime(&st);
    milliseconds = st.wMilliseconds;
#else
    /* Get milliseconds on Linux/Unix platform */
    struct timeval tv;
    gettimeofday(&tv, NULL);
    milliseconds = (int)(tv.tv_usec / 1000);
#endif
    
    /* Format the time string with milliseconds */
    // int len = strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
    int len = strftime(buffer, size, "%H:%M:%S", t);
    if (len <= 0 || len >= size - 4) {
        return -1;
    }
    
    /* Append milliseconds */
    return snprintf(buffer + len, size - len, ".%03d", milliseconds);
#else     
    unsigned int t;
    t = GetSysTickCnt();
    
    return snprintf(buffer, size, "%09u", t);
#endif
}

/**
 * @brief Internal output function for log messages.
 * @param msg The log message to output.
 */
static void internal_output(char *msg) {
    uint16_t len = strlen(msg);
    
    if (custom_output_func != NULL) {
        /* Use custom output function */
        custom_output_func(msg, len);
    } else {
        /* Default output to standard output */
        fwrite(msg, 1, len, stdout);
        fflush(stdout);
    }
}

/**
 * @brief Print a log message with location information.
 * @param level The log level.
 * @param module The module ID.
 * @param file The file name.
 * @param line The line number.
 * @param fmt The format string.
 * @param ap The variable argument list.
 */
static void vprintf_loc(sam_dbg_level_e level, sam_module_id_e module, const char *file, int line, const char *fmt, va_list ap) {
    char time_str[32];
    const char *base_file = strrchr(file, '/');
    if (base_file == NULL) {
        base_file = strrchr(file, '\\');
        if (base_file == NULL) {
            base_file = file;
        } else {
            base_file++;
        }
    } else {
        base_file++;
    }
	
#if 0    
    /* ��ȡ��ǰʱ�� */
    if (get_current_time(time_str, sizeof(time_str)) <= 0) {
        strcpy(time_str, "0000-00-00 00:00:00");
    }
#else	
    /* ��ȡ��ǰʱ�䣨���뼶�� */
    if (get_current_time(time_str, sizeof(time_str)) <= 0) {
        strcpy(time_str, "0000-00-00 00:00:00.000");
    }
#endif 	
    
    /* Generate the log message prefix */
    int prefix_len = 0;
    if (module < SAM_MOD_MAX) {
	prefix_len = snprintf(dbg_buffer, SAM_DBG_BUFFER_SIZE, 
                             "[%s] [%s] [%s] [%s:%d] ", 
                             time_str, module_names[module], level_names[level], base_file, line);
    }
    else {
	prefix_len = snprintf(dbg_buffer, SAM_DBG_BUFFER_SIZE, 
                             "[%s] [%s] [%s:%d] ", 
                             time_str, level_names[level], base_file, line);
    }
    
    if (prefix_len < 0 || prefix_len >= SAM_DBG_BUFFER_SIZE) {
        return;
    }
    
    /* Append the formatted message */
    vsnprintf(dbg_buffer + prefix_len, SAM_DBG_BUFFER_SIZE - prefix_len, fmt, ap);
    
    /* Ensure the message ends with a newline */
    int total_len = strlen(dbg_buffer);
    if (total_len < SAM_DBG_BUFFER_SIZE - 1 && dbg_buffer[total_len - 1] != '\n') {
        dbg_buffer[total_len] = '\n';
        dbg_buffer[total_len + 1] = '\0';
    }
    
    /* Output the log message */
    internal_output(dbg_buffer);
}

/**
 * @brief Print a log message without location information.
 * @param level The log level.
 * @param fmt The format string.
 * @param ap The variable argument list.
 */
static void vprintf_dbg(sam_dbg_level_e level, const char *fmt, va_list ap) {
    char time_str[32];

#if 0    
    /* ��ȡ��ǰʱ�� */
    if (get_current_time(time_str, sizeof(time_str)) <= 0) {
        strcpy(time_str, "0000-00-00 00:00:00");
    }
#else	
    /* ��ȡ��ǰʱ�䣨���뼶�� */
    if (get_current_time(time_str, sizeof(time_str)) <= 0) {
        strcpy(time_str, "0000-00-00 00:00:00.000");
    }
#endif	
    
    /* Generate the log message prefix */
    int prefix_len = snprintf(dbg_buffer, SAM_DBG_BUFFER_SIZE, 
                             "[%s][%s] ", 
                             time_str, level_names[level]);
    
    if (prefix_len < 0 || prefix_len >= SAM_DBG_BUFFER_SIZE) {
        return;
    }
    
    /* Append the formatted message */
    vsnprintf(dbg_buffer + prefix_len, SAM_DBG_BUFFER_SIZE - prefix_len, fmt, ap);
    
    /* Ensure the message ends with a newline */
    int total_len = strlen(dbg_buffer);
    if (total_len < SAM_DBG_BUFFER_SIZE - 1 && dbg_buffer[total_len - 1] != '\n') {
        dbg_buffer[total_len] = '\n';
        dbg_buffer[total_len + 1] = '\0';
    }
    
    /* Output the log message */
    internal_output(dbg_buffer);
}

/* Implement the functions declared in the header file */
/**
 * @brief Set the global debug level.
 * @param level The new global debug level.
 */
void sam_dbg_set_level(sam_dbg_level_e level) {
    global_dbg_level = level;
}

/**
 * @brief Get the current global debug level.
 * @return The current global debug level.
 */
sam_dbg_level_e sam_dbg_get_level(void) {
    return global_dbg_level;
}

/**
 * @brief Set the debug level for a specific module.
 * @param module The module ID.
 * @param level The new debug level for the module.
 */
void sam_dbg_set_module_level(sam_module_id_e module, sam_dbg_level_e level) {
    if (module < SAM_MOD_MAX) {
        module_dbg_levels[module] = level;
    }
}

/**
 * @brief Get the debug level for a specific module.
 * @param module The module ID.
 * @return The current debug level for the module.
 */
sam_dbg_level_e sam_dbg_get_module_level(sam_module_id_e module) {
    if (module < SAM_MOD_MAX) {
        return module_dbg_levels[module];
    }
    return global_dbg_level;
}

/**
 * @brief Set the custom output function for log messages.
 * @param func The custom output function, or NULL to restore the default.
 */
void sam_dbg_set_output(sam_dbg_output_func_t func) {
    custom_output_func = func;
}

/**
 * @brief Set the custom get system time function for log messages.
 * @param func The custom gettime function, or NULL to restore the default.
 */
void sam_dbg_set_gettime(sam_dbg_gettime_func_t func) {
    custom_gettime_func = func;
}

/**
 * @brief Print a log message with location information.
 * @param level The log level.
 * @param module The module ID.
 * @param file The file name.
 * @param line The line number.
 * @param fmt The format string.
 */
void sam_dbg_printf_loc(sam_dbg_level_e level, sam_module_id_e module, const char *file, int line, const char *fmt, ...) {
    if (level <= global_dbg_level && level != SAM_DBG_LEVEL_NONE) {
        va_list ap;
        va_start(ap, fmt);
        vprintf_loc(level, module, file, line, fmt, ap);
        va_end(ap);
    }
}

/**
 * @brief Print a log message without location information.
 * @param level The log level.
 * @param fmt The format string.
 */
void sam_dbg_printf(sam_dbg_level_e level, const char *fmt, ...) {
    if (level <= global_dbg_level && level != SAM_DBG_LEVEL_NONE) {
        va_list ap;
        va_start(ap, fmt);
        vprintf_dbg(level, fmt, ap);
        va_end(ap);
    }
}

/**
 * @brief Initialize the debug logging system.
 * @note Should be called at system startup.
 */
void sam_dbg_init(sam_dbg_output_func_t output, sam_dbg_gettime_func_t gettime) {
    /* set customer function */
    custom_output_func = output;
    custom_gettime_func = gettime;
    
    /* Set default module debug levels to the global level */
    for (int i = 0; i < SAM_MOD_MAX; i++) {
        module_dbg_levels[i] = global_dbg_level;
    }

#ifdef SAM_MQTT_DBG_LEVEL
	module_dbg_levels[SAM_MOD_MQTT] = SAM_MQTT_DBG_LEVEL;
#endif

#ifdef SAM_HTTP_DBG_LEVEL
	module_dbg_levels[SAM_MOD_HTTP] = SAM_HTTP_DBG_LEVEL;
#endif

#ifdef SAM_SMS_DBG_LEVEL
	module_dbg_levels[SAM_MOD_SMS] = SAM_SMS_DBG_LEVEL;
#endif

}


/* DebugTrace */
#include <stdarg.h>
void DebugTrace(const char *format, ...)
{
	unsigned int t;
	char tbuf[16];
	char buf[1025];

	//if (SAM_DBG_LEVEL_TRACE > global_dbg_level ) return; 

	t = GetSysTickCnt();
	snprintf(tbuf, 16, "%09u:", t);
	
	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, 1024, format, ap);
	va_end(ap);
	
	
	SendtoCom(DBGCH_A, tbuf, strlen(tbuf));
	SendtoCom(DBGCH_A, buf, strlen(buf));
}

void DebugHex(const char *buf,  unsigned short len)
{
	char chardp[1028], tempchar;
	unsigned short j;
	if(len > 512) len = 512;
	for(j=0; j<len ;j++)
	{
		tempchar = buf[j];
		if((tempchar>>4) <= 0x09)
		{
			chardp[j<<1] = (tempchar>>4) + '0';
		}
		else
		{
			chardp[j<<1] = ((tempchar>>4)-0x0A) + 'A';
		}
		if((tempchar&0x0F) <= 0x09)
		{
			chardp[((j<<1)+1)] = (tempchar&0x0F) + '0';
		}
		else
		{
			chardp[((j<<1)+1)] = ((tempchar&0x0F)-0x0A) + 'A';
		}
	}
	chardp[(j<<1)] = 0;
	SendtoCom(DBGCH_A, chardp, j*2);
}

