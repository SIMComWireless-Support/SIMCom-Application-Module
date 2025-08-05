/**
 * @file 	SamSub.h
 * @brief   Supplement various commonly used sub functions
 * @details 
 * 
 * @version 1.0.0
 * @date 	2025-08-01
 * @author 	Alex <fanbing.kong@sunseaaiot.com>
 * @copyright Copyright (c) 2025, SIMCom Wireless Solutions Limited. All rights reserved.
 * 
 * @note 
 *
 *		
 */
//----------------------------------------------------------------------


#ifndef __SAMSUB_H
#define __SAMSUB_H

#ifdef __cplusplus
extern "C"
{
#endif


typedef enum{
	TYPDAT_STR = 0,	//string 
	TYPDAT_DHEX,		//data hexasc
	TYPDAT_DS32,
	TYPDAT_DU32,
	TYPDAT_HU32
}TypeDataTag;
extern unsigned short Str2TypData(char *pstrs, void * ptypd, unsigned short tdlen, TypeDataTag dt);

/**
 * @brief Search for a string within another string and return the starting position.
 *
 * This function searches for the first occurrence of the second string (ptr2) within the first string (ptr1)
 * and returns the starting position of the match. If no match is found, it returns 0.
 *
 * @param ptr1 The string to search within.
 * @param ptr2 The string to search for.
 * @return The starting position of the match (1-based index), or 0 if no match is found.
 */
extern unsigned short Strsearch(char *ptr1,char *ptr2);

/**
 * @brief Extract a specific parameter from a string delimited by a separator character.
 *
 * This function parses the input string 'ptrs' and extracts the 'pcnt'-th parameter,
 * where parameters are separated by the specified 'separator' character. The extracted
 * parameter is copied to 'ptrd' with a maximum length of 'kbuflmax' to prevent buffer overflow.
 *
 * @param ptrs Pointer to the input string to be parsed.
 * @param separator Character used to delimit parameters in the input string.
 * @param pcnt The 0-based index of the parameter to extract.
 * @param ptrd Pointer to the destination buffer where the extracted parameter will be stored.
 * @param kbuflmax Maximum number of characters that can be stored in the destination buffer (including null terminator).
 * @return The length of the extracted parameter (excluding the null terminator), or 0 if the parameter was not found.
 */
extern unsigned short GetPmrStr(char *ptrs, char separator, unsigned char pcnt, char *ptrd, unsigned short kbuflmax);

/**
 * @brief Compare a string against multiple substrings separated by tabs.
 *
 * This function checks if the given string 'rets' matches any of the substrings
 * in 'exps', which are separated by tab characters ('\t'). If a match is found,
 * the function returns the 1-based index of the matching substring in 'exps'.
 * If no match is found, it returns 0.
 *
 * @param rets The string to compare against the substrings.
 * @param exps The string containing multiple substrings separated by tabs.
 * @return The 1-based index of the first matching substring, or 0 if no match.
 */
extern unsigned char StrsCmp(char * rets, char * exps);

/**
 * @brief Read a configuration value from a tab-separated string.
 *
 * This function reads a configuration value from a tab-separated string based on the specified header and index.
 *
 * @param tabs The tab-separated string containing the configuration values.
 * @param head The header string to search for.
 * @param index The index of the configuration value to read.
 * @param content The buffer where the read configuration value will be stored.
 * @return The length of the read configuration value, or 0 if the value is not found.
 */
extern unsigned char ReadCfgTab(char * tabs, char * head,  uint8 index,  char * content);

/**
 * @brief Write a configuration value to a tab-separated string.
 *
 * This function writes a configuration value to a tab-separated string based on the specified header and index.
 *
 * @param tabs The tab-separated string containing the configuration values.
 * @param head The header string to search for.
 * @param index The index of the configuration value to write.
 * @param content The configuration value to write.
 * @return The length of the written configuration value, or 0 if the write fails.
 */
extern unsigned char WriteCfgTab(char * tabs, char * head,  uint8 index,  char * content);

//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Calculates elapsed milliseconds since a starting timestamp.
 * 
 * Handles timer rollover by using unsigned integer arithmetic.
 * 
 * @param stms Starting timestamp (from GetSysTickCnt()).
 * @return unsigned int Elapsed milliseconds since stms.
 */
extern unsigned int SamGetMsCnt(unsigned int stms);


#ifdef __cplusplus
}
#endif



#endif
