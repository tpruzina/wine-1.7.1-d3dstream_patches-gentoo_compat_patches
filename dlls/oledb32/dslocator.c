/* Data Links
 *
 * Copyright 2013 Alistair Leslie-Hughes
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
#include <stdarg.h>
#include <string.h>

#define COBJMACROS

#include "windef.h"
#include "winbase.h"
#include "objbase.h"
#include "oleauto.h"
#include "winerror.h"
#include "oledb.h"
#include "oledberr.h"
#include "msdasc.h"

#include "oledb_private.h"

#include "wine/debug.h"

WINE_DEFAULT_DEBUG_CHANNEL(oledb);


typedef struct DSLocatorImpl
{
    IDataSourceLocator     IDataSourceLocator_iface;
    LONG ref;

} DSLocatorImpl;

static inline DSLocatorImpl *impl_from_IDataSourceLocator( IDataSourceLocator *iface )
{
    return CONTAINING_RECORD(iface, DSLocatorImpl, IDataSourceLocator_iface);
}


static HRESULT WINAPI dslocator_QueryInterface(IDataSourceLocator *iface, REFIID riid, void **ppvoid)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);
    TRACE("(%p)->(%s, %p)\n", This, debugstr_guid(riid),ppvoid);

    *ppvoid = NULL;

    if (IsEqualIID(riid, &IID_IUnknown) ||
        IsEqualIID(riid, &IID_IDispatch) ||
        IsEqualIID(riid, &IID_IDataSourceLocator))
    {
      *ppvoid = &This->IDataSourceLocator_iface;
    }

    if(*ppvoid)
    {
      IUnknown_AddRef( (IUnknown*)*ppvoid );
      return S_OK;
    }

    FIXME("interface %s not implemented\n", debugstr_guid(riid));
    return E_NOINTERFACE;
}

static ULONG WINAPI dslocator_AddRef(IDataSourceLocator *iface)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);
    TRACE("(%p)->%u\n",This,This->ref);
    return InterlockedIncrement(&This->ref);
}

static ULONG WINAPI dslocator_Release(IDataSourceLocator *iface)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);
    ULONG ref = InterlockedDecrement(&This->ref);

    TRACE("(%p)->%u\n",This,ref+1);

    if (!ref)
    {
        heap_free(This);
    }

    return ref;
}

static HRESULT WINAPI dslocator_GetTypeInfoCount(IDataSourceLocator *iface, UINT *pctinfo)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);

    FIXME("(%p)->()\n", This);

    return E_NOTIMPL;
}

static HRESULT WINAPI dslocator_GetTypeInfo(IDataSourceLocator *iface, UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);

    FIXME("(%p)->(%u %u %p)\n", This, iTInfo, lcid, ppTInfo);

    return E_NOTIMPL;
}

static HRESULT WINAPI dslocator_GetIDsOfNames(IDataSourceLocator *iface, REFIID riid, LPOLESTR *rgszNames,
    UINT cNames, LCID lcid, DISPID *rgDispId)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);

    FIXME("(%p)->(%s %p %u %u %p)\n", This, debugstr_guid(riid), rgszNames, cNames, lcid, rgDispId);

    return E_NOTIMPL;
}

static HRESULT WINAPI dslocator_Invoke(IDataSourceLocator *iface, DISPID dispIdMember, REFIID riid,
    LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);

    FIXME("(%p)->(%d %s %d %d %p %p %p %p)\n", This, dispIdMember, debugstr_guid(riid),
           lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);

    return E_NOTIMPL;
}

static HRESULT WINAPI dslocator_get_hWnd(IDataSourceLocator *iface, LONG *phwndParent)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);

    FIXME("(%p)->(%p)\n",This, phwndParent);

    return E_NOTIMPL;
}

static HRESULT WINAPI dslocator_put_hWnd(IDataSourceLocator *iface, LONG phwndParent)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);

    FIXME("(%p)->(%d)\n",This, phwndParent);

    return E_NOTIMPL;
}

static HRESULT WINAPI dslocator_PromptNew(IDataSourceLocator *iface, IDispatch **ppADOConnection)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);

    FIXME("(%p)->(%p)\n",This, ppADOConnection);

    return E_NOTIMPL;
}

static HRESULT WINAPI dslocator_PromptEdit(IDataSourceLocator *iface, IDispatch **ppADOConnection, VARIANT_BOOL *success)
{
    DSLocatorImpl *This = impl_from_IDataSourceLocator(iface);

    FIXME("(%p)->(%p %p)\n",This, ppADOConnection, success);

    return E_NOTIMPL;
}

static const IDataSourceLocatorVtbl DSLocatorVtbl =
{
    dslocator_QueryInterface,
    dslocator_AddRef,
    dslocator_Release,
    dslocator_GetTypeInfoCount,
    dslocator_GetTypeInfo,
    dslocator_GetIDsOfNames,
    dslocator_Invoke,
    dslocator_get_hWnd,
    dslocator_put_hWnd,
    dslocator_PromptNew,
    dslocator_PromptEdit
};

HRESULT create_dslocator(IUnknown *outer, void **obj)
{
    DSLocatorImpl *This;

    TRACE("(%p, %p)\n", outer, obj);

    *obj = NULL;

    if(outer) return CLASS_E_NOAGGREGATION;

    This = heap_alloc(sizeof(*This));
    if(!This) return E_OUTOFMEMORY;

    This->IDataSourceLocator_iface.lpVtbl = &DSLocatorVtbl;
    This->ref = 1;

    *obj = &This->IDataSourceLocator_iface;

    return S_OK;
}
