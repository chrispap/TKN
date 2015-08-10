#define MAX_COMPORT_NAME 20

#ifdef __linux__

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

char ** listSerialPorts()
{
    DIR* dir;
    struct dirent* entry;
    struct stat st;
    char entry_path[PATH_MAX + 1];
    size_t path_len;
    char* dir_path = "/dev/";
    char **list_ser = NULL;

    /* Copy the directory path into entry_path.  */
    strncpy (entry_path, dir_path, sizeof (entry_path));
    path_len = strlen (dir_path);

    /* If the directory path doesn't end with a slash, append a slash.  */
    if (entry_path[path_len - 1] != '/') {
        entry_path[path_len] = '/';
        entry_path[path_len + 1] = '\0';
        ++path_len;
    }

    /* Start the listing operation of the directory specified on the command line.  */
    dir = opendir (dir_path);


    /* Loop over all directory entries.  */
    list_ser = (char **) malloc(sizeof(char *) * (50+1));
    int list_i=0;

    while ((entry = readdir (dir)) != NULL) {
        /* Build the path to the directory entry by appending the entry name to the path name.  */
        strncpy (entry_path + path_len, entry->d_name, sizeof (entry_path) - path_len);

        /* Ensure that its a characted device */
        lstat (entry_path, &st);
        if (!S_ISCHR (st.st_mode))
            continue;

        char *serialPortNamePAttern = "ttyUSB";

        if ( strncmp(entry->d_name, serialPortNamePAttern, strlen(serialPortNamePAttern)) != 0)
            continue;

        char * serPath = malloc(sizeof(char) * (strlen(entry_path)+1));
        strcpy(serPath, entry_path);
        list_ser[list_i++] = serPath;
    }

    list_ser[list_i] = NULL;
    closedir (dir);
    return list_ser;
}

#else

#pragma comment(lib, "advapi32")

#include <windows.h>

#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

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

#endif
