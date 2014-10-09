#pragma once

#define WIN32_LEAN_AND_MEAN 


#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NONSTDC_NO_DEPRECATE

#pragma warning(disable: 4996)
#pragma warning(disable: 4013)
#pragma warning(disable: 4133)

#include <windows.h>
#include <stdio.h>
#include <io.h>
#include <direct.h>
#include <stdlib.h>
#include <stdint.h>

#define __inline__    _inline

//#define unlink    _unlink
// #define chmod	  _chmod
// #define close	  _close
#define snprintf   _snprintf
#define PATH_MAX       MAX_PATH
