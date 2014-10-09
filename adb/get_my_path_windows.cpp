/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <limits.h>
#include <assert.h>
#include <windows.h>

#include "utils.h"

void get_my_path(char *exe, size_t maxLen)
{
    char  *r;

    /* XXX: should be GetModuleFileNameA */
	wchar_t buffer[MAX_PATH] = { 0 };
	if (GetModuleFileName(NULL, buffer, maxLen) > 0) {
		std::string s = WideToMultiByte(buffer);
		memcpy(exe, &s[0], s.size()*sizeof(char));
		r = strrchr(exe, '\\');
        if (r != NULL)
            *r = '\0';
    } else {
        exe[0] = '\0';
    }
}

