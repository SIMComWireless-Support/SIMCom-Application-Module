/**
 * @file 	SamSub.c
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

#define __SAMSUB_C

#include "include.h"


unsigned short Str2TypData(char *pstrs, void * ptypd, unsigned short tdlen, TypeDataTag dt)
{
	unsigned char * strbuf = NULL;
	unsigned char t, c;
	unsigned int i, j;
	int * pn = NULL;
	int n;
	unsigned int * pm = NULL;
	unsigned int m;
	
	if(ptypd == NULL|| pstrs == NULL) return(0);
	
	if(dt == TYPDAT_STR)
	{// STRING
		strbuf = (unsigned char *)ptypd;
		for(i = 0; i < tdlen;  i++)
		{
			strbuf[i] = pstrs[i];
			if(pstrs[i] == 0) break;
		}
		return(i);
	}
	else if(dt == TYPDAT_DHEX)
	{ //HEX STRING TO DATA ARRAY
		strbuf = (unsigned char *)ptypd;
		for(i = 0, j = 0; i < tdlen;  i++)
		{
			c = pstrs[i*2];
			if(c >= '0' && c <= '9') 
			{
				c -= '0';	
			}
			else if(c >= 'A' && c <= 'F')
			{
				c -= 'A';
				c += 0x0A;
			}
			else if(c >= 'a' && c <= 'f')
			{
				c -= 'a';
				c += 0x0A;
			}
			else
			{
				break;
			}
			t = c;
			c = pstrs[i*2+1];
			if(c >= '0' && c <= '9') 
			{
				c -= '0';	
			}
			else if(c >= 'A' && c <= 'F')
			{
				c -= 'A';
				c += 0x0A;
			}
			else if(c >= 'a' && c <= 'f')
			{
				c -= 'a';
				c += 0x0A;
			}
			else
			{
				break;
			}
			t = (t & 0x0F)<<4;
			t |= (c & 0x0F);
			strbuf[i] = t;
		}
		return(i);
	}
	else if(dt == TYPDAT_DU32 && tdlen == 4)
	{// DEC STR TO U32
		pm = (unsigned int *)ptypd;
		m = 0;
		for(i=0; (pstrs[i] >= '0' && pstrs[i] <= '9'); i++)
		{
			m *= 10;
			m += (unsigned int)(pstrs[i] - '0');
		}
		*pm = m;
		return(4);
	}
	else if(dt == TYPDAT_DU32 && tdlen == 4)
	{// DEC STR TO S32
		pn = (int *)ptypd;
		n = 0;
		i = 0;
		j = 0;
		if(pstrs[0] == '-')
		{
			j = 1;
			i++;
		}
		else if(pstrs[0] == '+')
		{
			j = 0;
			i++;
		}
		for( ; (pstrs[i] >= '0' && pstrs[i] <= '9'); i++)
		{
			n *= 10;
			n += (int)(pstrs[i] - '0');
		}
		if(j != 0) n = 0 - n;
		*pn = n;
		return(i);
	}
	else if(dt == TYPDAT_HU32 && tdlen == 4)
	{// HEX STR TO U32
		pm = (unsigned int *)ptypd;
		m = 0;
		j = 0; 
		for(i=0; i<8; i++)
		{
			c = 0;
			if(pstrs[i] >= '0' && pstrs[i] <= '9')
			{
				c = pstrs[i] - '0';	
			}
			else if(pstrs[i] >= 'a' && pstrs[i] <= 'f')
			{
				c = pstrs[i] - 'a';
				c += 0x0A;	
			}
			else if(pstrs[i] >= 'A' && pstrs[i] <= 'F')
			{
				c = pstrs[i] - 'A';
				c += 0x0A;
			}
			else if((pstrs[i]=='X' || pstrs[i]=='x') && m == 0)
			{
				continue;
			}
			else 
			{
				break;
			}
			m <<= 4;
			m |= (c & 0x0F);
		}
		*pm = m; 
		return(i);
	}
	
	return(0);
}


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
uint16 Strsearch(char *ptr1,char *ptr2)
{
	uint16 max_length;
	char * old_ptr2;
	uint16 i;
	uint16 flag;
	max_length=strlen((char *)ptr1)+1;
	old_ptr2=ptr2;
	flag=0;
	for(i=0;i<max_length;i++)
	{
		if(*ptr2==0)
		{
			return(flag);
		}
		else if(*(ptr1+i)!=*ptr2)
		{
			if(flag!=0)
			{
				i=(flag-1); // about for i++ in next research
				flag=0;
				ptr2=old_ptr2;
			}
		}
		else
		{
			if(flag==0) flag=i+1;
			ptr2++;
		}
	}
	return(0);
}

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
unsigned short GetPmrStr(char *ptrs, char separator, unsigned char pcnt, char *ptrd, unsigned short kbuflmax)
{
    uint16 i, j, k;
    k = 0;  // Counter for the number of separators encountered
    i = 0;  // Start index of the current parameter
    j = 0;  // Current position in the input string
    
    // Scan the input string to find the start and end of the desired parameter
    while(ptrs[j] != 0)
    {
        if(ptrs[j] == separator)
        {
            if(k >= pcnt)
            {
                break;  // Found the end of the desired parameter
            }
            else
            {
                k++;    // Increment separator counter
                i = j+1; // Move to the start of the next parameter
            }
        }
        j++;
    }
    
    // Check if the desired parameter was found
    if(k >= pcnt)
    {
        // Copy the parameter to the destination buffer, respecting buffer limits
        for(k=0; i<j && k<kbuflmax-1; i++)  // Adjusted to leave room for null terminator
        {
            if(ptrs[i] == '\r' || ptrs[i] == '\n' || ptrs[i] == 0)
            {
                break;  // Stop at line endings
            }
            ptrd[k] = ptrs[i];
            if((ptrd[k] == ' '||ptrd[k] == '\t') && k == 0) continue;
            k++;
        }
        ptrd[k] = 0;  // Null-terminate the destination string
        return k;     // Return the length of the extracted parameter
    }
    else
    {
        ptrd[0] = 0;  // Null-terminate empty string
        return 0;     // Parameter not found
    }
}


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
//in<rets : sr1 
//in<exps : se1\tse2\tse3....
//out>  se1 isin sr1 -> 1, se2 isin sr1 -> 2
//out>  se1 or se2 or se3 .... all is not sr1 -> 0x00
uint8 StrsCmp(char * rets, char * exps)
{
	uint16 i, j;
	uint8 n;
	char  buf[130];

    if(rets[0] == 0 || exps[0] == 0) return(0);

	n = 1;
	j = 0;
	do{
		for(i=0; i < 130; i++, j++)
		{
			buf[i] = exps[j];
			if(buf[i] == '\t' || buf[i] == 0)
			{
				buf[i] = 0;
				if(Strsearch(rets, buf) == 1)
				{
					return(n);
				}
				else if(exps[j] != 0)
				{
					n++;
					j++;
					break;
				}
				else
				{
					return(0);
				}
			}
		}
	}while(exps[j] != 0);
    return(0);
}


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
//Configuration Table : \vCFG_APN1\tIPV4V6\tcmiot\tusr\tpwd
unsigned char ReadCfgTab(char * tabs, char * head,  uint8 index,  char * content)
{
	uint16 i, j;
	uint8 n;
	char buf[64];
    j = 0;
	buf[j++] = '\v';
	if(head != NULL && head[0] != 0)
	{
		for(i=0; j<127; j++)
		{
			buf[j] = head[i++];
			if(buf[j] == 0 ||buf[j] == '\t') break;
		}
	}
	buf[j] = 0;
	n = 0;
	i = Strsearch(tabs,  buf);
	if(i != 0)
	{
		j += (i-1);
		while(n < index && tabs[j] != 0)
		{
			if(tabs[j++] == '\t') n++;
		}
		i = 0;
		content[i] = 0;
		while(tabs[j] != 0 && tabs[j] != '\t' && tabs[j] != '\v')
		{
			content[i++] = tabs[j++];
		}
		content[i] = 0;
		if(i > 0)
		{
			return(i);
		}
	}
	return(0);
}


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
//Configuration Table (max : 1020)
//\vCFG_APN1\tIPV4V6\tcmiot\tusr\tpwd
unsigned char WriteCfgTab(char * tabs, char * head,  uint8 index,  char * content)
{
	uint16 i, j, k, h;
	uint8 n;
	char buf[1024];
    j = 0;
	buf[j++] = '\v';
	if(head != NULL && head[0] != 0)
	{
		for(i=0; j<127; j++)
		{
			buf[j] = head[i++];
			if(buf[j] == 0 ||buf[j] == '\t') break;
		}
	}
	buf[j] = 0;

	if(index == 0)
	{//FIRST 
		tabs[0] = '\v';
	}
	
	i = Strsearch(tabs,  buf);
    if(i == 0) return(0);
    h = j;
    while(tabs[h] != 0) h++;
    k = 0;
    while(content[k] != 0)
    {
		if(content[k] == '\t' || content[k] == '\v') 
		{
			content[k] = 0;
			break;
		}
		k++;
    }
    if(k == 0 || (h + k) >= 1000) return(0);

    j += (i-1);
    n = 0;
    while(n < index && j<1020)
    {
        if(tabs[j] == 0 || tabs[j] == '\v')
		{//add tab
			tabs[j++] = '\t';
			tabs[j] = 0;
			n++;
		} 
		else if(tabs[j++] == '\t') 
		{
			n++;	
		}
    }
	if(n != index) return(0);
    
    i = j;
    while(tabs[j] != 0 && tabs[j] != '\t' && tabs[j] != '\v')
    {
        j++;
    }

    k = 0;
    while(tabs[j] != 0)
    {
        buf[k++] = tabs[j++];
    }
    buf[k++] = 0;
    j = 0;
    while(content[j] != 0)
    {
        tabs[i++] = content[j++];
    }
    k = 0;
    while(buf[k] != 0)
    {
        tabs[i++] = buf[k++];
    }
    tabs[i++] = 0;
	return(j);
}


//////////////////////////////////////////////////////////////////////////////
/**
 * @brief Calculates elapsed milliseconds since a starting timestamp.
 * 
 * Handles timer rollover by using unsigned integer arithmetic.
 * 
 * @param stms Starting timestamp (from GetSysTickCnt()).
 * @return unsigned int Elapsed milliseconds since stms.
 */
unsigned int SamGetMsCnt(unsigned int stms)
{
    unsigned int   cms;
	unsigned int   dtim;
	cms = GetSysTickCnt();
	if(cms >= stms)
	{
		dtim = cms - stms;
	}
	else
	{
		dtim = 0x0FFFFFFFF - stms;
		dtim += cms;
	}
	return(dtim);
}




