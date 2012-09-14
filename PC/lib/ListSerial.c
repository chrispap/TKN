#include <windows.h>
//#include <stdio.h>
//#include <tchar.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383
#define MAX_COMPORT_NAME 20

char ** listSerialPorts()
{
    HKEY hKey;

    char **list_ser = NULL;

    if( !RegOpenKeyEx( HKEY_LOCAL_MACHINE, TEXT("HARDWARE\\DEVICEMAP\\SERIALCOMM"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
        return NULL;

    TCHAR    achClass[MAX_PATH] = TEXT("");  // buffer for class name 
    DWORD    cchClassName = MAX_PATH;  // size of class string 
    DWORD    cSubKeys=0;               // number of subkeys 
    DWORD    cbMaxSubKey;              // longest subkey size 
    DWORD    cchMaxClass;              // longest class string 
    DWORD    cValues;              // number of values for key 
    DWORD    cchMaxValue;          // longest value name 
    DWORD    cbMaxValueData;       // longest value data 
    DWORD    cbSecurityDescriptor; // size of security descriptor 
    FILETIME ftLastWriteTime;      // last write time 

    DWORD i, retCode; 

    TCHAR  achValue[MAX_VALUE_NAME]; 
    DWORD cchValue = MAX_VALUE_NAME; 

    // Get the class name and the value count. 
    retCode = RegQueryInfoKeyA(
                hKey,                    // key handle
                achClass,                // buffer for class name
                &cchClassName,           // size of class string
                NULL,                    // reserved
                &cSubKeys,               // number of subkeys
                &cbMaxSubKey,            // longest subkey size
                &cchMaxClass,            // longest class string
                &cValues,                // number of values for this key
                &cchMaxValue,            // longest value name
                &cbMaxValueData,         // longest value data
                &cbSecurityDescriptor,   // security descriptor
                &ftLastWriteTime);       // last write time

    // Enumerate the key values. 
    if (cValues) 
    {
        list_ser = (char **) malloc(sizeof(char *) * (cValues+1));
        int list_i;

        for (i=0, list_i = 0, retCode=ERROR_SUCCESS; i<cValues; i++) 
        {
            cchValue = MAX_VALUE_NAME; 
            achValue[0] = '\0'; 
            char *entryData = malloc((MAX_COMPORT_NAME+1) * sizeof(char));
            memset(entryData, 0, (MAX_COMPORT_NAME+1) * sizeof(char));

            entryData[0] = '\0';
            DWORD entryDataSize = MAX_COMPORT_NAME;
            
            retCode = RegEnumValueA(hKey,
                                   i,
                                   achValue,
                                   &cchValue,
                                   NULL,
                                   NULL,
                                   entryData,
                                   &entryDataSize);

            if (retCode == ERROR_SUCCESS ) 
            { 
                list_ser[list_i++] = entryData;
            }
            else
            {
                free(entryData);
            }
        }

        list_ser[list_i] = NULL;
    }

    RegCloseKey(hKey);
    return list_ser;
}
