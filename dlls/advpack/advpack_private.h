/*
 * Advpack private header
 *
 * Copyright 2006 James Hawkins
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA
 */

#ifndef __ADVPACK_PRIVATE_H
#define __ADVPACK_PRIVATE_H

HRESULT do_ocx_reg(HMODULE hocx, BOOL do_reg, const WCHAR *flags, const WCHAR *param) DECLSPEC_HIDDEN;
LPWSTR get_parameter(LPWSTR *params, WCHAR separator, BOOL quoted) DECLSPEC_HIDDEN;
void set_ldids(HINF hInf, LPCWSTR pszInstallSection, LPCWSTR pszWorkingDir) DECLSPEC_HIDDEN;

HRESULT launch_exe(LPCWSTR cmd, LPCWSTR dir, HANDLE *phEXE) DECLSPEC_HIDDEN;

static inline void *heap_alloc(size_t len)
{
    return HeapAlloc(GetProcessHeap(), 0, len);
}

static inline BOOL heap_free(void *mem)
{
    return HeapFree(GetProcessHeap(), 0, mem);
}

static inline char *heap_strdupWtoA(const WCHAR *str)
{
    char *ret = NULL;

    if(str) {
        size_t size = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
        ret = heap_alloc(size);
        if(ret)
            WideCharToMultiByte(CP_ACP, 0, str, -1, ret, size, NULL, NULL);
    }

    return ret;
}

#endif /* __ADVPACK_PRIVATE_H */
