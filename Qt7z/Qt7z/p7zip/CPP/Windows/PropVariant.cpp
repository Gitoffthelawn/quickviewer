// Windows/PropVariant.cpp

#include "../myWindows/StdAfx.h"

#include "../Common/Defs.h"

#include "PropVariant.h"

namespace NWindows {
namespace NCOM {

BSTR AllocBstrFromAscii(const char *s) throw()
{
  if (!s)
    return NULL;
  UINT len = (UINT)strlen(s);
  BSTR p = ::SysAllocStringLen(NULL, len);
  if (p)
  {
    for (UINT i = 0; i <= len; i++)
      p[i] = (Byte)s[i];
  }
  return p;
}

HRESULT PropVarEm_Alloc_Bstr(PROPVARIANT *p, unsigned numChars) throw()
{
  p->bstrVal = ::SysAllocStringLen(NULL, numChars);
  if (!p->bstrVal)
  {
    p->vt = VT_ERROR;
    p->scode = E_OUTOFMEMORY;
    return E_OUTOFMEMORY;
  }
  p->vt = VT_BSTR;
  return S_OK;
}

HRESULT PropVarEm_Set_Str(PROPVARIANT *p, const char *s) throw()
{
  p->bstrVal = AllocBstrFromAscii(s);
  if (p->bstrVal)
  {
    p->vt = VT_BSTR;
    return S_OK;
  }
  p->vt = VT_ERROR;
  p->scode = E_OUTOFMEMORY;
  return E_OUTOFMEMORY;
}

CPropVariant::CPropVariant(const PROPVARIANT &varSrc)
{
  vt = VT_EMPTY;
  InternalCopy(&varSrc);
}

CPropVariant::CPropVariant(const CPropVariant &varSrc)
{
  vt = VT_EMPTY;
  InternalCopy(&varSrc);
}

CPropVariant::CPropVariant(BSTR bstrSrc)
{
  vt = VT_EMPTY;
  *this = bstrSrc;
}

CPropVariant::CPropVariant(LPCOLESTR lpszSrc)
{
  vt = VT_EMPTY;
  *this = lpszSrc;
}

CPropVariant& CPropVariant::operator=(const CPropVariant &varSrc)
{
  InternalCopy(&varSrc);
  return *this;
}

CPropVariant& CPropVariant::operator=(const PROPVARIANT &varSrc)
{
  InternalCopy(&varSrc);
  return *this;
}

CPropVariant& CPropVariant::operator=(BSTR bstrSrc)
{
  *this = (LPCOLESTR)bstrSrc;
  return *this;
}

static const char * const kMemException = "out of memory";

CPropVariant& CPropVariant::operator=(LPCOLESTR lpszSrc)
{
  InternalClear();
  vt = VT_BSTR;
  wReserved1 = 0;
  bstrVal = ::SysAllocString(lpszSrc);
  if (!bstrVal && lpszSrc)
  {
    throw kMemException;
    // vt = VT_ERROR;
    // scode = E_OUTOFMEMORY;
  }
  return *this;
}

CPropVariant& CPropVariant::operator=(const UString &s)
{
  InternalClear();
  vt = VT_BSTR;
  wReserved1 = 0;
  bstrVal = ::SysAllocStringLen(s, s.Len());
  if (!bstrVal)
    throw kMemException;
  return *this;
}

CPropVariant& CPropVariant::operator=(const UString2 &s)
{
  /*
  if (s.IsEmpty())
    *this = L"";
  else
  */
  {
    InternalClear();
    vt = VT_BSTR;
    wReserved1 = 0;
    bstrVal = ::SysAllocStringLen(s.GetRawPtr(), s.Len());
    if (!bstrVal)
      throw kMemException;
    /* SysAllocStringLen probably appends a null-terminating character for NULL string.
       But it doesn't specified in MSDN.
       But we suppose that it works

    if (!s.GetRawPtr())
    {
      *bstrVal = 0;
    }
    */

    /* MSDN: Windows CE: SysAllocStringLen() : Passing invalid (and under some circumstances NULL)
                         pointers to this function causes  an unexpected termination of the application.
       Is it safe? Maybe we must chamnge the code for that case ? */
  }
  return *this;
}

CPropVariant& CPropVariant::operator=(const char *s)
{
  InternalClear();
  vt = VT_BSTR;
  wReserved1 = 0;
  bstrVal = AllocBstrFromAscii(s);
  if (!bstrVal)
  {
    throw kMemException;
    // vt = VT_ERROR;
    // scode = E_OUTOFMEMORY;
  }
  return *this;
}

CPropVariant& CPropVariant::operator=(bool bSrc) throw()
{
  if (vt != VT_BOOL)
  {
    InternalClear();
    vt = VT_BOOL;
  }
  boolVal = bSrc ? VARIANT_TRUE : VARIANT_FALSE;
  return *this;
}

BSTR CPropVariant::AllocBstr(unsigned numChars)
{
  if (vt != VT_EMPTY)
    InternalClear();
  vt = VT_BSTR;
  wReserved1 = 0;
  bstrVal = ::SysAllocStringLen(NULL, numChars);
  if (!bstrVal)
  {
    throw kMemException;
    // vt = VT_ERROR;
    // scode = E_OUTOFMEMORY;
  }
  return bstrVal;
}

#define SET_PROP_FUNC(type, id, dest) \
  CPropVariant& CPropVariant::operator=(type value) throw() \
  { if (vt != id) { InternalClear(); vt = id; } \
    dest = value; return *this; }

SET_PROP_FUNC(Byte, VT_UI1, bVal)
// SET_PROP_FUNC(Int16, VT_I2, iVal)
SET_PROP_FUNC(Int32, VT_I4, lVal)
SET_PROP_FUNC(UInt32, VT_UI4, ulVal)
SET_PROP_FUNC(UInt64, VT_UI8, uhVal.QuadPart)
SET_PROP_FUNC(Int64, VT_I8, hVal.QuadPart)
SET_PROP_FUNC(const FILETIME &, VT_FILETIME, filetime)

HRESULT PropVariant_Clear(PROPVARIANT *prop) throw()
{
  switch (prop->vt)
  {
    case VT_EMPTY:
    case VT_UI1:
    case VT_I1:
    case VT_I2:
    case VT_UI2:
    case VT_BOOL:
    case VT_I4:
    case VT_UI4:
    case VT_R4:
    case VT_INT:
    case VT_UINT:
    case VT_ERROR:
    case VT_FILETIME:
    case VT_UI8:
    case VT_R8:
    case VT_CY:
    case VT_DATE:
      prop->vt = VT_EMPTY;
      prop->wReserved1 = 0;
      prop->wReserved2 = 0;
      prop->wReserved3 = 0;
      prop->uhVal.QuadPart = 0;
      return S_OK;
  }
  return ::VariantClear((VARIANTARG *)prop);
  // return ::PropVariantClear(prop);
  // PropVariantClear can clear VT_BLOB.
}

HRESULT CPropVariant::Clear() throw()
{
  if (vt == VT_EMPTY)
    return S_OK;
  return PropVariant_Clear(this);
}

HRESULT CPropVariant::Copy(const PROPVARIANT* pSrc) throw()
{
  ::VariantClear((tagVARIANT *)this);
  switch (pSrc->vt)
  {
    case VT_UI1:
    case VT_I1:
    case VT_I2:
    case VT_UI2:
    case VT_BOOL:
    case VT_I4:
    case VT_UI4:
    case VT_R4:
    case VT_INT:
    case VT_UINT:
    case VT_ERROR:
    case VT_FILETIME:
    case VT_UI8:
    case VT_R8:
    case VT_CY:
    case VT_DATE:
      memmove((PROPVARIANT*)this, pSrc, sizeof(PROPVARIANT));
      return S_OK;
  }
  return ::VariantCopy((tagVARIANT *)this, (tagVARIANT *)const_cast<PROPVARIANT *>(pSrc));
}


HRESULT CPropVariant::Attach(PROPVARIANT *pSrc) throw()
{
  HRESULT hr = Clear();
  if (FAILED(hr))
    return hr;
  memcpy(this, pSrc, sizeof(PROPVARIANT));
  pSrc->vt = VT_EMPTY;
  return S_OK;
}

HRESULT CPropVariant::Detach(PROPVARIANT *pDest) throw()
{
  if (pDest->vt != VT_EMPTY)
  {
    HRESULT hr = PropVariant_Clear(pDest);
    if (FAILED(hr))
      return hr;
  }
  memcpy(pDest, this, sizeof(PROPVARIANT));
  vt = VT_EMPTY;
  return S_OK;
}

HRESULT CPropVariant::InternalClear() throw()
{
  if (vt == VT_EMPTY)
    return S_OK;
  HRESULT hr = Clear();
  if (FAILED(hr))
  {
    vt = VT_ERROR;
    scode = hr;
  }
  return hr;
}

void CPropVariant::InternalCopy(const PROPVARIANT *pSrc)
{
  HRESULT hr = Copy(pSrc);
  if (FAILED(hr))
  {
    if (hr == E_OUTOFMEMORY)
      throw kMemException;
    vt = VT_ERROR;
    scode = hr;
  }
}

int CPropVariant::Compare(const CPropVariant &a) throw()
{
  if (vt != a.vt)
    return MyCompare(vt, a.vt);
  switch (vt)
  {
    case VT_EMPTY: return 0;
    // case VT_I1: return MyCompare(cVal, a.cVal);
    case VT_UI1: return MyCompare(bVal, a.bVal);
    case VT_I2: return MyCompare(iVal, a.iVal);
    case VT_UI2: return MyCompare(uiVal, a.uiVal);
    case VT_I4: return MyCompare(lVal, a.lVal);
    case VT_UI4: return MyCompare(ulVal, a.ulVal);
    // case VT_UINT: return MyCompare(uintVal, a.uintVal);
    case VT_I8: return MyCompare(hVal.QuadPart, a.hVal.QuadPart);
    case VT_UI8: return MyCompare(uhVal.QuadPart, a.uhVal.QuadPart);
    case VT_BOOL: return -MyCompare(boolVal, a.boolVal);
    case VT_FILETIME: return ::CompareFileTime(&filetime, &a.filetime);
    case VT_BSTR: return 0; // Not implemented
    default: return 0;
  }
}

}}
