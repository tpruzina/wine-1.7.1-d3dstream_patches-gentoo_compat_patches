/*
 * Copyright (C) 1998-1999 Francois Gouget
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

#include <rpc.h>
#include <rpcndr.h>

#ifndef _OBJBASE_H_
#define _OBJBASE_H_

/*****************************************************************************
 * Macros to define a COM interface
 */
/*
 * The goal of the following set of definitions is to provide a way to use the same
 * header file definitions to provide both a C interface and a C++ object oriented
 * interface to COM interfaces. The type of interface is selected automatically
 * depending on the language but it is always possible to get the C interface in C++
 * by defining CINTERFACE.
 *
 * It is based on the following assumptions:
 *  - all COM interfaces derive from IUnknown, this should not be a problem.
 *  - the header file only defines the interface, the actual fields are defined
 *    separately in the C file implementing the interface.
 *
 * The natural approach to this problem would be to make sure we get a C++ class and
 * virtual methods in C++ and a structure with a table of pointer to functions in C.
 * Unfortunately the layout of the virtual table is compiler specific, the layout of
 * g++ virtual tables is not the same as that of an egcs virtual table which is not the
 * same as that generated by Visual C++. There are workarounds to make the virtual tables
 * compatible via padding but unfortunately the one which is imposed to the WINE emulator
 * by the Windows binaries, i.e. the Visual C++ one, is the most compact of all.
 *
 * So the solution I finally adopted does not use virtual tables. Instead I use inline
 * non virtual methods that dereference the method pointer themselves and perform the call.
 *
 * Let's take Direct3D as an example:
 *
 *    #define INTERFACE IDirect3D
 *    DECLARE_INTERFACE_(IDirect3D,IUnknown)
 *    {
 *        // *** IUnknown methods *** //
 *        STDMETHOD_(HRESULT,QueryInterface)(THIS_ REFIID, void**) PURE;
 *        STDMETHOD_(ULONG,AddRef)(THIS) PURE;
 *        STDMETHOD_(ULONG,Release)(THIS) PURE;
 *        // *** IDirect3D methods *** //
 *        STDMETHOD(Initialize)(THIS_ REFIID) PURE;
 *        STDMETHOD(EnumDevices)(THIS_ LPD3DENUMDEVICESCALLBACK, LPVOID) PURE;
 *        STDMETHOD(CreateLight)(THIS_ LPDIRECT3DLIGHT *, IUnknown *) PURE;
 *        STDMETHOD(CreateMaterial)(THIS_ LPDIRECT3DMATERIAL *, IUnknown *) PURE;
 *        STDMETHOD(CreateViewport)(THIS_ LPDIRECT3DVIEWPORT *, IUnknown *) PURE;
 *        STDMETHOD(FindDevice)(THIS_ LPD3DFINDDEVICESEARCH, LPD3DFINDDEVICERESULT) PURE;
 *    };
 *    #undef INTERFACE
 *
 *    #ifdef COBJMACROS
 *    // *** IUnknown methods *** //
 *    #define IDirect3D_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
 *    #define IDirect3D_AddRef(p)             (p)->lpVtbl->AddRef(p)
 *    #define IDirect3D_Release(p)            (p)->lpVtbl->Release(p)
 *    // *** IDirect3D methods *** //
 *    #define IDirect3D_Initialize(p,a)       (p)->lpVtbl->Initialize(p,a)
 *    #define IDirect3D_EnumDevices(p,a,b)    (p)->lpVtbl->EnumDevice(p,a,b)
 *    #define IDirect3D_CreateLight(p,a,b)    (p)->lpVtbl->CreateLight(p,a,b)
 *    #define IDirect3D_CreateMaterial(p,a,b) (p)->lpVtbl->CreateMaterial(p,a,b)
 *    #define IDirect3D_CreateViewport(p,a,b) (p)->lpVtbl->CreateViewport(p,a,b)
 *    #define IDirect3D_FindDevice(p,a,b)     (p)->lpVtbl->FindDevice(p,a,b)
 *    #endif
 *
 * Comments:
 *  - The INTERFACE macro is used in the STDMETHOD macros to define the type of the 'this'
 *    pointer. Defining this macro here saves us the trouble of having to repeat the interface
 *    name everywhere. Note however that because of the way macros work, a macro like STDMETHOD
 *    cannot use 'INTERFACE##_VTABLE' because this would give 'INTERFACE_VTABLE' and not
 *    'IDirect3D_VTABLE'.
 *  - The DECLARE_INTERFACE declares all the structures necessary for the interface. We have to
 *    explicitly use the interface name for macro expansion reasons again. It defines the list of
 *    methods that are inheritable from this interface. It must be written manually (rather than
 *    using a macro to generate the equivalent code) to avoid macro recursion (which compilers
 *    don't like). It must start with the methods definition of the parent interface so that
 *    method inheritance works properly.
 *  - The 'undef INTERFACE' is here to remind you that using INTERFACE in the following macros
 *    will not work.
 *  - Finally the set of 'IDirect3D_Xxx' macros is a standard set of macros defined to ease access
 *    to the interface methods in C. Unfortunately I don't see any way to avoid having to duplicate
 *    the inherited method definitions there. This time I could have used a trick to use only one
 *    macro whatever the number of parameters but I preferred to have it work the same way as above.
 *  - You probably have noticed that we don't define the fields we need to actually implement this
 *    interface: reference count, pointer to other resources and miscellaneous fields. That's
 *    because these interfaces are just that: interfaces. They may be implemented more than once, in
 *    different contexts and sometimes not even in Wine. Thus it would not make sense to impose
 *    that the interface contains some specific fields.
 *
 *
 * In C this gives:
 *    typedef struct IDirect3DVtbl IDirect3DVtbl;
 *    struct IDirect3D {
 *        IDirect3DVtbl* lpVtbl;
 *    };
 *    struct IDirect3DVtbl {
 *        HRESULT (*QueryInterface)(IDirect3D* me, REFIID riid, LPVOID* ppvObj);
 *        ULONG (*AddRef)(IDirect3D* me);
 *        ULONG (*Release)(IDirect3D* me);
 *        HRESULT (*Initialize)(IDirect3D* me, REFIID a);
 *        HRESULT (*EnumDevices)(IDirect3D* me, LPD3DENUMDEVICESCALLBACK a, LPVOID b);
 *        HRESULT (*CreateLight)(IDirect3D* me, LPDIRECT3DLIGHT* a, IUnknown* b);
 *        HRESULT (*CreateMaterial)(IDirect3D* me, LPDIRECT3DMATERIAL* a, IUnknown* b);
 *        HRESULT (*CreateViewport)(IDirect3D* me, LPDIRECT3DVIEWPORT* a, IUnknown* b);
 *        HRESULT (*FindDevice)(IDirect3D* me, LPD3DFINDDEVICESEARCH a, LPD3DFINDDEVICERESULT b);
 *    };
 *
 *    #ifdef COBJMACROS
 *    // *** IUnknown methods *** //
 *    #define IDirect3D_QueryInterface(p,a,b) (p)->lpVtbl->QueryInterface(p,a,b)
 *    #define IDirect3D_AddRef(p)             (p)->lpVtbl->AddRef(p)
 *    #define IDirect3D_Release(p)            (p)->lpVtbl->Release(p)
 *    // *** IDirect3D methods *** //
 *    #define IDirect3D_Initialize(p,a)       (p)->lpVtbl->Initialize(p,a)
 *    #define IDirect3D_EnumDevices(p,a,b)    (p)->lpVtbl->EnumDevice(p,a,b)
 *    #define IDirect3D_CreateLight(p,a,b)    (p)->lpVtbl->CreateLight(p,a,b)
 *    #define IDirect3D_CreateMaterial(p,a,b) (p)->lpVtbl->CreateMaterial(p,a,b)
 *    #define IDirect3D_CreateViewport(p,a,b) (p)->lpVtbl->CreateViewport(p,a,b)
 *    #define IDirect3D_FindDevice(p,a,b)     (p)->lpVtbl->FindDevice(p,a,b)
 *    #endif
 *
 * Comments:
 *  - IDirect3D only contains a pointer to the IDirect3D virtual/jump table. This is the only thing
 *    the user needs to know to use the interface. Of course the structure we will define to
 *    implement this interface will have more fields but the first one will match this pointer.
 *  - The code generated by DECLARE_INTERFACE defines both the structure representing the interface and
 *    the structure for the jump table.
 *  - Each method is declared as a pointer to function field in the jump table. The implementation
 *    will fill this jump table with appropriate values, probably using a static variable, and
 *    initialize the lpVtbl field to point to this variable.
 *  - The IDirect3D_Xxx macros then just derefence the lpVtbl pointer and use the function pointer
 *    corresponding to the macro name. This emulates the behavior of a virtual table and should be
 *    just as fast.
 *  - This C code should be quite compatible with the Windows headers both for code that uses COM
 *    interfaces and for code implementing a COM interface.
 *
 *
 * And in C++ (with gcc's g++):
 *
 *    typedef struct IDirect3D: public IUnknown {
 *        virtual HRESULT Initialize(REFIID a) = 0;
 *        virtual HRESULT EnumDevices(LPD3DENUMDEVICESCALLBACK a, LPVOID b) = 0;
 *        virtual HRESULT CreateLight(LPDIRECT3DLIGHT* a, IUnknown* b) = 0;
 *        virtual HRESULT CreateMaterial(LPDIRECT3DMATERIAL* a, IUnknown* b) = 0;
 *        virtual HRESULT CreateViewport(LPDIRECT3DVIEWPORT* a, IUnknown* b) = 0;
 *        virtual HRESULT FindDevice(LPD3DFINDDEVICESEARCH a, LPD3DFINDDEVICERESULT b) = 0;
 *    };
 *
 * Comments:
 *  - Of course in C++ we use inheritance so that we don't have to duplicate the method definitions.
 *  - Finally there is no IDirect3D_Xxx macro. These are not needed in C++ unless the CINTERFACE
 *    macro is defined in which case we would not be here.
 */

#if defined(__cplusplus) && !defined(CINTERFACE)

/* C++ interface */

#define STDMETHOD(method)        virtual HRESULT STDMETHODCALLTYPE method
#define STDMETHOD_(type,method)  virtual type STDMETHODCALLTYPE method
#define STDMETHODV(method)       virtual HRESULT STDMETHODVCALLTYPE method
#define STDMETHODV_(type,method) virtual type STDMETHODVCALLTYPE method

#define PURE   = 0
#define THIS_
#define THIS   void

#define interface struct
#define DECLARE_INTERFACE(iface)        interface DECLSPEC_NOVTABLE iface
#define DECLARE_INTERFACE_(iface,ibase) interface DECLSPEC_NOVTABLE iface : public ibase
#define DECLARE_INTERFACE_IID_(iface, ibase, iid) interface DECLSPEC_UUID(iid) DECLSPEC_NOVTABLE iface : public ibase

#define BEGIN_INTERFACE
#define END_INTERFACE

#else  /* __cplusplus && !CINTERFACE */

/* C interface */

#define STDMETHOD(method)        HRESULT (STDMETHODCALLTYPE *method)
#define STDMETHOD_(type,method)  type (STDMETHODCALLTYPE *method)
#define STDMETHODV(method)       HRESULT (STDMETHODVCALLTYPE *method)
#define STDMETHODV_(type,method) type (STDMETHODVCALLTYPE *method)

#define PURE
#define THIS_ INTERFACE *This,
#define THIS  INTERFACE *This

#define interface struct

#ifdef __WINESRC__
#define CONST_VTABLE
#endif

#ifdef CONST_VTABLE
#undef CONST_VTBL
#define CONST_VTBL const
#define DECLARE_INTERFACE(iface) \
         typedef interface iface { const struct iface##Vtbl *lpVtbl; } iface; \
         typedef struct iface##Vtbl iface##Vtbl; \
         struct iface##Vtbl
#else
#undef CONST_VTBL
#define CONST_VTBL
#define DECLARE_INTERFACE(iface) \
         typedef interface iface { struct iface##Vtbl *lpVtbl; } iface; \
         typedef struct iface##Vtbl iface##Vtbl; \
         struct iface##Vtbl
#endif
#define DECLARE_INTERFACE_(iface,ibase) DECLARE_INTERFACE(iface)
#define DECLARE_INTERFACE_IID_(iface, ibase, iid) DECLARE_INTERFACE_(iface, ibase)

#define BEGIN_INTERFACE
#define END_INTERFACE

#endif  /* __cplusplus && !CINTERFACE */

#ifndef __IRpcStubBuffer_FWD_DEFINED__
#define __IRpcStubBuffer_FWD_DEFINED__
typedef interface IRpcStubBuffer IRpcStubBuffer;
#endif
#ifndef __IRpcChannelBuffer_FWD_DEFINED__
#define __IRpcChannelBuffer_FWD_DEFINED__
typedef interface IRpcChannelBuffer IRpcChannelBuffer;
#endif

#ifndef RC_INVOKED
/* For compatibility only, at least for now */
#include <stdlib.h>
#endif

#include <wtypes.h>
#include <unknwn.h>
#include <objidl.h>

#include <guiddef.h>
#ifndef INITGUID
#include <cguid.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NONAMELESSSTRUCT
#define LISet32(li, v)   ((li).HighPart = (v) < 0 ? -1 : 0, (li).LowPart = (v))
#define ULISet32(li, v)  ((li).HighPart = 0, (li).LowPart = (v))
#else
#define LISet32(li, v)   ((li).u.HighPart = (v) < 0 ? -1 : 0, (li).u.LowPart = (v))
#define ULISet32(li, v)  ((li).u.HighPart = 0, (li).u.LowPart = (v))
#endif

/*****************************************************************************
 *	Standard API
 */
DWORD WINAPI CoBuildVersion(void);

typedef enum tagCOINIT
{
    COINIT_APARTMENTTHREADED  = 0x2, /* Apartment model */
    COINIT_MULTITHREADED      = 0x0, /* OLE calls objects on any thread */
    COINIT_DISABLE_OLE1DDE    = 0x4, /* Don't use DDE for Ole1 support */
    COINIT_SPEED_OVER_MEMORY  = 0x8  /* Trade memory for speed */
} COINIT;

HRESULT WINAPI CoInitialize(LPVOID lpReserved);
HRESULT WINAPI CoInitializeEx(LPVOID lpReserved, DWORD dwCoInit);
void WINAPI CoUninitialize(void);
DWORD WINAPI CoGetCurrentProcess(void);

HINSTANCE WINAPI CoLoadLibrary(LPOLESTR lpszLibName, BOOL bAutoFree);
void WINAPI CoFreeAllLibraries(void);
void WINAPI CoFreeLibrary(HINSTANCE hLibrary);
void WINAPI CoFreeUnusedLibraries(void);
void WINAPI CoFreeUnusedLibrariesEx(DWORD dwUnloadDelay, DWORD dwReserved);

HRESULT WINAPI CoCreateInstance(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, REFIID iid, LPVOID *ppv);
HRESULT WINAPI CoCreateInstanceEx(REFCLSID      rclsid,
				  LPUNKNOWN     pUnkOuter,
				  DWORD         dwClsContext,
				  COSERVERINFO* pServerInfo,
				  ULONG         cmq,
				  MULTI_QI*     pResults);

HRESULT WINAPI CoGetInstanceFromFile(COSERVERINFO* pServerInfo, CLSID* pClsid, IUnknown* punkOuter, DWORD dwClsCtx, DWORD grfMode, OLECHAR* pwszName, DWORD dwCount, MULTI_QI* pResults);
HRESULT WINAPI CoGetInstanceFromIStorage(COSERVERINFO* pServerInfo, CLSID* pClsid, IUnknown* punkOuter, DWORD dwClsCtx, IStorage* pstg, DWORD dwCount, MULTI_QI* pResults);

HRESULT WINAPI CoGetMalloc(DWORD dwMemContext, LPMALLOC* lpMalloc);
LPVOID WINAPI CoTaskMemAlloc(ULONG size) __WINE_ALLOC_SIZE(1);
void WINAPI CoTaskMemFree(LPVOID ptr);
LPVOID WINAPI CoTaskMemRealloc(LPVOID ptr, ULONG size);

HRESULT WINAPI CoRegisterMallocSpy(LPMALLOCSPY pMallocSpy);
HRESULT WINAPI CoRevokeMallocSpy(void);

HRESULT WINAPI CoGetContextToken( ULONG_PTR *token );

/* class registration flags; passed to CoRegisterClassObject */
typedef enum tagREGCLS
{
    REGCLS_SINGLEUSE = 0,
    REGCLS_MULTIPLEUSE = 1,
    REGCLS_MULTI_SEPARATE = 2,
    REGCLS_SUSPENDED = 4,
    REGCLS_SURROGATE = 8
} REGCLS;

HRESULT WINAPI CoGetClassObject(REFCLSID rclsid, DWORD dwClsContext, COSERVERINFO *pServerInfo, REFIID iid, LPVOID *ppv);
HRESULT WINAPI CoRegisterClassObject(REFCLSID rclsid,LPUNKNOWN pUnk,DWORD dwClsContext,DWORD flags,LPDWORD lpdwRegister);
HRESULT WINAPI CoRevokeClassObject(DWORD dwRegister);
HRESULT WINAPI CoGetPSClsid(REFIID riid,CLSID *pclsid);
HRESULT WINAPI CoRegisterPSClsid(REFIID riid, REFCLSID rclsid);
HRESULT WINAPI CoRegisterSurrogate(LPSURROGATE pSurrogate);
HRESULT WINAPI CoSuspendClassObjects(void);
HRESULT WINAPI CoResumeClassObjects(void);
ULONG WINAPI CoAddRefServerProcess(void);
ULONG WINAPI CoReleaseServerProcess(void);

/* marshalling */
HRESULT WINAPI CoCreateFreeThreadedMarshaler(LPUNKNOWN punkOuter, LPUNKNOWN* ppunkMarshal);
HRESULT WINAPI CoGetInterfaceAndReleaseStream(LPSTREAM pStm, REFIID iid, LPVOID* ppv);
HRESULT WINAPI CoGetMarshalSizeMax(ULONG* pulSize, REFIID riid, LPUNKNOWN pUnk, DWORD dwDestContext, LPVOID pvDestContext, DWORD mshlflags);
HRESULT WINAPI CoGetStandardMarshal(REFIID riid, LPUNKNOWN pUnk, DWORD dwDestContext, LPVOID pvDestContext, DWORD mshlflags, LPMARSHAL* ppMarshal);
HRESULT WINAPI CoMarshalHresult(LPSTREAM pstm, HRESULT hresult);
HRESULT WINAPI CoMarshalInterface(LPSTREAM pStm, REFIID riid, LPUNKNOWN pUnk, DWORD dwDestContext, LPVOID pvDestContext, DWORD mshlflags);
HRESULT WINAPI CoMarshalInterThreadInterfaceInStream(REFIID riid, LPUNKNOWN pUnk, LPSTREAM* ppStm);
HRESULT WINAPI CoReleaseMarshalData(LPSTREAM pStm);
HRESULT WINAPI CoDisconnectObject(LPUNKNOWN lpUnk, DWORD reserved);
HRESULT WINAPI CoUnmarshalHresult(LPSTREAM pstm, HRESULT* phresult);
HRESULT WINAPI CoUnmarshalInterface(LPSTREAM pStm, REFIID riid, LPVOID* ppv);
HRESULT WINAPI CoLockObjectExternal(LPUNKNOWN pUnk, BOOL fLock, BOOL fLastUnlockReleases);
BOOL WINAPI CoIsHandlerConnected(LPUNKNOWN pUnk);

/* security */
HRESULT WINAPI CoInitializeSecurity(PSECURITY_DESCRIPTOR pSecDesc, LONG cAuthSvc, SOLE_AUTHENTICATION_SERVICE* asAuthSvc, void* pReserved1, DWORD dwAuthnLevel, DWORD dwImpLevel, void* pReserved2, DWORD dwCapabilities, void* pReserved3);
HRESULT WINAPI CoGetCallContext(REFIID riid, void** ppInterface);
HRESULT WINAPI CoSwitchCallContext(IUnknown *pContext, IUnknown **ppOldContext);
HRESULT WINAPI CoQueryAuthenticationServices(DWORD* pcAuthSvc, SOLE_AUTHENTICATION_SERVICE** asAuthSvc);

HRESULT WINAPI CoQueryProxyBlanket(IUnknown* pProxy, DWORD* pwAuthnSvc, DWORD* pAuthzSvc, OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel, RPC_AUTH_IDENTITY_HANDLE* pAuthInfo, DWORD* pCapabilities);
HRESULT WINAPI CoSetProxyBlanket(IUnknown* pProxy, DWORD dwAuthnSvc, DWORD dwAuthzSvc, OLECHAR* pServerPrincName, DWORD dwAuthnLevel, DWORD dwImpLevel, RPC_AUTH_IDENTITY_HANDLE pAuthInfo, DWORD dwCapabilities);
HRESULT WINAPI CoCopyProxy(IUnknown* pProxy, IUnknown** ppCopy);

HRESULT WINAPI CoImpersonateClient(void);
HRESULT WINAPI CoQueryClientBlanket(DWORD* pAuthnSvc, DWORD* pAuthzSvc, OLECHAR** pServerPrincName, DWORD* pAuthnLevel, DWORD* pImpLevel, RPC_AUTHZ_HANDLE* pPrivs, DWORD* pCapabilities);
HRESULT WINAPI CoRevertToSelf(void);

/* misc */
HRESULT WINAPI CoGetTreatAsClass(REFCLSID clsidOld, LPCLSID pClsidNew);
HRESULT WINAPI CoTreatAsClass(REFCLSID clsidOld, REFCLSID clsidNew);
HRESULT WINAPI CoAllowSetForegroundWindow(IUnknown *pUnk, LPVOID lpvReserved);
HRESULT WINAPI CoGetObjectContext(REFIID riid, LPVOID *ppv);

HRESULT WINAPI CoCreateGuid(GUID* pguid);
BOOL WINAPI CoIsOle1Class(REFCLSID rclsid);

BOOL WINAPI CoDosDateTimeToFileTime(WORD nDosDate, WORD nDosTime, FILETIME* lpFileTime);
BOOL WINAPI CoFileTimeToDosDateTime(FILETIME* lpFileTime, WORD* lpDosDate, WORD* lpDosTime);
HRESULT WINAPI CoFileTimeNow(FILETIME* lpFileTime);
HRESULT WINAPI CoRegisterMessageFilter(LPMESSAGEFILTER lpMessageFilter,LPMESSAGEFILTER *lplpMessageFilter);
HRESULT WINAPI CoRegisterChannelHook(REFGUID ExtensionGuid, IChannelHook *pChannelHook);

typedef enum tagCOWAIT_FLAGS
{
    COWAIT_WAITALL   = 0x00000001,
    COWAIT_ALERTABLE = 0x00000002
} COWAIT_FLAGS;

HRESULT WINAPI CoWaitForMultipleHandles(DWORD dwFlags,DWORD dwTimeout,ULONG cHandles,LPHANDLE pHandles,LPDWORD lpdwindex);

/*****************************************************************************
 *	GUID API
 */
HRESULT WINAPI StringFromCLSID(REFCLSID id, LPOLESTR*);
HRESULT WINAPI CLSIDFromString(LPCOLESTR, LPCLSID);
HRESULT WINAPI CLSIDFromProgID(LPCOLESTR progid, LPCLSID riid);
HRESULT WINAPI ProgIDFromCLSID(REFCLSID clsid, LPOLESTR *lplpszProgID);

INT WINAPI StringFromGUID2(REFGUID id, LPOLESTR str, INT cmax);

/*****************************************************************************
 *	COM Server dll - exports
 */
HRESULT WINAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID * ppv) DECLSPEC_HIDDEN;
HRESULT WINAPI DllCanUnloadNow(void) DECLSPEC_HIDDEN;

/* shouldn't be here, but is nice for type checking */
#ifdef __WINESRC__
HRESULT WINAPI DllRegisterServer(void) DECLSPEC_HIDDEN;
HRESULT WINAPI DllUnregisterServer(void) DECLSPEC_HIDDEN;
#endif


/*****************************************************************************
 *	Data Object
 */
HRESULT WINAPI CreateDataAdviseHolder(LPDATAADVISEHOLDER* ppDAHolder);
HRESULT WINAPI CreateDataCache(LPUNKNOWN pUnkOuter, REFCLSID rclsid, REFIID iid, LPVOID* ppv);

/*****************************************************************************
 *	Moniker API
 */
HRESULT WINAPI BindMoniker(LPMONIKER pmk, DWORD grfOpt, REFIID iidResult, LPVOID* ppvResult);
HRESULT WINAPI CoGetObject(LPCWSTR pszName, BIND_OPTS *pBindOptions, REFIID riid, void **ppv);
HRESULT WINAPI CreateAntiMoniker(LPMONIKER * ppmk);
HRESULT WINAPI CreateBindCtx(DWORD reserved, LPBC* ppbc);
HRESULT WINAPI CreateClassMoniker(REFCLSID rclsid, LPMONIKER* ppmk);
HRESULT WINAPI CreateFileMoniker(LPCOLESTR lpszPathName, LPMONIKER* ppmk);
HRESULT WINAPI CreateGenericComposite(LPMONIKER pmkFirst, LPMONIKER pmkRest, LPMONIKER* ppmkComposite);
HRESULT WINAPI CreateItemMoniker(LPCOLESTR lpszDelim, LPCOLESTR  lpszItem, LPMONIKER* ppmk);
HRESULT WINAPI CreateObjrefMoniker(LPUNKNOWN punk, LPMONIKER * ppmk);
HRESULT WINAPI CreatePointerMoniker(LPUNKNOWN punk, LPMONIKER * ppmk);
HRESULT WINAPI GetClassFile(LPCOLESTR filePathName,CLSID *pclsid);
HRESULT WINAPI GetRunningObjectTable(DWORD reserved, LPRUNNINGOBJECTTABLE *pprot);
HRESULT WINAPI MkParseDisplayName(LPBC pbc, LPCOLESTR szUserName, ULONG * pchEaten, LPMONIKER * ppmk);
HRESULT WINAPI MonikerCommonPrefixWith(IMoniker* pmkThis,IMoniker* pmkOther,IMoniker** ppmkCommon);
HRESULT WINAPI MonikerRelativePathTo(LPMONIKER pmkSrc, LPMONIKER pmkDest, LPMONIKER * ppmkRelPath, BOOL dwReserved);

/*****************************************************************************
 *	Storage API
 */
#define STGM_DIRECT		0x00000000
#define STGM_TRANSACTED		0x00010000
#define STGM_SIMPLE		0x08000000
#define STGM_READ		0x00000000
#define STGM_WRITE		0x00000001
#define STGM_READWRITE		0x00000002
#define STGM_SHARE_DENY_NONE	0x00000040
#define STGM_SHARE_DENY_READ	0x00000030
#define STGM_SHARE_DENY_WRITE	0x00000020
#define STGM_SHARE_EXCLUSIVE	0x00000010
#define STGM_PRIORITY		0x00040000
#define STGM_DELETEONRELEASE	0x04000000
#define STGM_CREATE		0x00001000
#define STGM_CONVERT		0x00020000
#define STGM_FAILIFTHERE	0x00000000
#define STGM_NOSCRATCH		0x00100000
#define STGM_NOSNAPSHOT		0x00200000
#define STGM_DIRECT_SWMR	0x00400000

#define STGFMT_STORAGE		0
#define STGFMT_FILE 		3
#define STGFMT_ANY 		4
#define STGFMT_DOCFILE 	5

typedef struct tagSTGOPTIONS
{
    USHORT usVersion;
    USHORT reserved;
    ULONG ulSectorSize;
    const WCHAR* pwcsTemplateFile;
} STGOPTIONS;

HRESULT WINAPI StgCreateDocfile(LPCOLESTR pwcsName,DWORD grfMode,DWORD reserved,IStorage **ppstgOpen);
HRESULT WINAPI StgCreateStorageEx(const WCHAR*,DWORD,DWORD,DWORD,STGOPTIONS*,void*,REFIID,void**);
HRESULT WINAPI StgIsStorageFile(LPCOLESTR fn);
HRESULT WINAPI StgIsStorageILockBytes(ILockBytes *plkbyt);
HRESULT WINAPI StgOpenStorage(const OLECHAR* pwcsName,IStorage* pstgPriority,DWORD grfMode,SNB snbExclude,DWORD reserved,IStorage**ppstgOpen);
HRESULT WINAPI StgOpenStorageEx(const WCHAR* pwcwName,DWORD grfMode,DWORD stgfmt,DWORD grfAttrs,STGOPTIONS *pStgOptions, void *reserved, REFIID riid, void **ppObjectOpen);

HRESULT WINAPI StgCreateDocfileOnILockBytes(ILockBytes *plkbyt,DWORD grfMode, DWORD reserved, IStorage** ppstgOpen);
HRESULT WINAPI StgOpenStorageOnILockBytes(ILockBytes *plkbyt, IStorage *pstgPriority, DWORD grfMode, SNB snbExclude, DWORD reserved, IStorage **ppstgOpen);
HRESULT WINAPI StgSetTimes( OLECHAR const *lpszName, FILETIME const *pctime, FILETIME const *patime, FILETIME const *pmtime);

#ifdef __cplusplus
}
#endif

#ifndef __WINESRC__
# include <urlmon.h>
#endif
#include <propidl.h>

#ifndef __WINESRC__

#define FARSTRUCT
#define HUGEP

#define WINOLEAPI        STDAPI
#define WINOLEAPI_(type) STDAPI_(type)

#endif /* __WINESRC__ */

#endif /* _OBJBASE_H_ */