#pragma once
#include <stdio.h>
#include <stdint.h>

typedef enum
{
	LogType_Info,
	LogType_Success,
	LogType_Error,
	LogType_Warning

} LogType;

static void log(LogType type, const char* msg)
{
	const char* prefix;
	switch (type)
	{
	case LogType_Info:    
		prefix = "[*] "; 
		break;
	case LogType_Success:  
		prefix = "[+] "; 
		break;
	case LogType_Error:    
		prefix = "[-] "; 
		break;
	case LogType_Warning:  
		prefix = "[!] "; 
		break;
	default:              
		prefix = "[?] "; 
		break;
	}
	printf("%s%s\n", prefix, msg);
}