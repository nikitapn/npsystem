#pragma once

#include <afxdisp.h>

namespace boost {
	namespace serialization {
		template<class Archive>
		inline void save(Archive & ar, const COleVariant &varSrc, const unsigned int /*file_version*/)
		{
			LPCVARIANT pSrc = (LPCVARIANT)varSrc;

			ar << pSrc->vt;

			// No support for VT_BYREF & VT_ARRAY
			if (pSrc->vt & VT_BYREF || pSrc->vt & VT_ARRAY)
				return;

			switch (pSrc->vt)
			{
			case VT_BOOL:
				ar << (WORD)V_BOOL(pSrc);
				break;
			case VT_I1:
				ar << pSrc->cVal;
				break;
			case VT_UI1:
				ar << pSrc->bVal;
				break;
			case VT_I2:
				ar << pSrc->iVal;
				break;
			case VT_UI2:
				ar << pSrc->uiVal;
				break;
			case VT_I4:
				ar << pSrc->lVal;
				break;
			case VT_UI4:
				ar << pSrc->ulVal;
				break;
			case VT_I8:
				ar << pSrc->llVal;
				break;
			case VT_UI8:
				ar << pSrc->ullVal;
				break;
			case VT_CY:
				ar << pSrc->cyVal.Lo;
				ar << pSrc->cyVal.Hi;
				break;
			case VT_R4:
				ar << pSrc->fltVal;
				break;
			case VT_R8:
				ar << pSrc->dblVal;
				break;
			case VT_DATE:
				ar << pSrc->date;
				break;
			case VT_BSTR:
			{
				ASSERT(FALSE);
		//		DWORD nLen = SysStringByteLen(pSrc->bstrVal);
		//		ar << nLen;
		//		if (nLen > 0)
		//			ar.Write(pSrc->bstrVal, nLen * sizeof(BYTE));
				break;
			}

			case VT_ERROR:
				ar << pSrc->scode;
				break;
			case VT_DISPATCH:
			case VT_UNKNOWN:
			{
				ASSERT(FALSE);
		/*		LPPERSISTSTREAM pPersistStream;
				CArchiveStream stm(&ar);

				// QI for IPersistStream or IPeristStreamInit
				SCODE sc = pSrc->punkVal->QueryInterface(
					IID_IPersistStream, (void**)&pPersistStream);
				if (FAILED(sc))
					sc = pSrc->punkVal->QueryInterface(
						IID_IPersistStreamInit, (void**)&pPersistStream);
				AfxCheckError(sc);

				TRY
				{
					// Get and archive the CLSID (GUID)
					CLSID clsid;
				AfxCheckError(pPersistStream->GetClassID(&clsid));
				ar << clsid.Data1;
				ar << clsid.Data2;
				ar << clsid.Data3;
				ar.Write(&clsid.Data4[0], sizeof clsid.Data4);

				// Always assume object is dirty
				AfxCheckError(pPersistStream->Save(&stm, TRUE));
				}
					CATCH_ALL(e)
				{
					pPersistStream->Release();
					THROW_LAST();
				}
				END_CATCH_ALL
					pPersistStream->Release();
				*/
				break;
			}
			case VT_EMPTY:
			case VT_NULL:
				break;
			default:
				ASSERT(FALSE);
				break;
			}
		}

		template<class Archive>
		inline void load(Archive & ar, COleVariant &varSrc, const unsigned int /*file_version*/)
		{
			LPVARIANT pSrc = &varSrc;

			// Free up current data if necessary
			if (pSrc->vt != VT_EMPTY)
				VariantClear(pSrc);

			ar >> pSrc->vt;

			// No support for VT_BYREF & VT_ARRAY
			if (pSrc->vt & VT_BYREF || pSrc->vt & VT_ARRAY)
				return;

			switch (pSrc->vt)
			{
			case VT_BOOL:
				ar >> (WORD&)V_BOOL(pSrc);
				break;
			case VT_I1:
				ar >> pSrc->cVal;
				break;
			case VT_UI1:
				ar >> pSrc->bVal;
				break;
			case VT_I2:
				ar >> pSrc->iVal;
				break;
			case VT_UI2:
				ar >> pSrc->uiVal;
				break;
			case VT_I4:
				ar >> pSrc->lVal;
				break;
			case VT_UI4:
				ar >> pSrc->ulVal;
				break;
			case VT_I8:
				ar >> pSrc->llVal;
				break;
			case VT_UI8:
				ar >> pSrc->ullVal;
				break;
			case VT_CY:
				ar >> pSrc->cyVal.Lo;
				ar >> pSrc->cyVal.Hi;
				break;
			case VT_R4:
				ar >> pSrc->fltVal;
				break;
			case VT_R8:
				ar >> pSrc->dblVal;
				break;
			case VT_DATE:
				ar >> pSrc->date;
				break;
			case VT_BSTR:
			{
				ASSERT(FALSE);
		/*		DWORD nLen;
				ar >> nLen;
				if (nLen > 0)
				{
					pSrc->bstrVal = SysAllocStringByteLen(NULL, nLen);
					if (pSrc->bstrVal == NULL)
						AfxThrowMemoryException();
					ar.EnsureRead(pSrc->bstrVal, nLen * sizeof(BYTE));
				} else
					pSrc->bstrVal = NULL;
					*/
				break;
			}
			case VT_ERROR:
				ar >> pSrc->scode;
				break;
			case VT_DISPATCH:
			case VT_UNKNOWN:
			{
				ASSERT(FALSE);
			/*	LPPERSISTSTREAM pPersistStream = NULL;
				CArchiveStream stm(&ar);

				// Retrieve the CLSID (GUID) and create an instance
				CLSID clsid;
				ar >> clsid.Data1;
				ar >> clsid.Data2;
				ar >> clsid.Data3;
				ar.EnsureRead(&clsid.Data4[0], sizeof clsid.Data4);

				// Create the object
				SCODE sc = CoCreateInstance(clsid, NULL, CLSCTX_ALL | CLSCTX_REMOTE_SERVER,
					pSrc->vt == VT_UNKNOWN ? IID_IUnknown : IID_IDispatch,
					(void**)&pSrc->punkVal);
				if (sc == E_INVALIDARG)
				{
					// may not support CLSCTX_REMOTE_SERVER, so try without
					sc = CoCreateInstance(clsid, NULL,
						CLSCTX_ALL & ~CLSCTX_REMOTE_SERVER,
						pSrc->vt == VT_UNKNOWN ? IID_IUnknown : IID_IDispatch,
						(void**)&pSrc->punkVal);
				}
				AfxCheckError(sc);

				TRY
				{
					// QI for IPersistStream or IPeristStreamInit
					sc = pSrc->punkVal->QueryInterface(
						IID_IPersistStream, (void**)&pPersistStream);
				if (FAILED(sc))
					sc = pSrc->punkVal->QueryInterface(
						IID_IPersistStreamInit, (void**)&pPersistStream);
				AfxCheckError(sc);

				// Always assumes object is dirty
				AfxCheckError(pPersistStream->Load(&stm));
				}
					CATCH_ALL(e)
				{
					// Clean up
					if (pPersistStream != NULL)
						pPersistStream->Release();

					pSrc->punkVal->Release();
					THROW_LAST();
				}
				END_CATCH_ALL

				pPersistStream->Release();*/
				break;
			}
			case VT_EMPTY:
			case VT_NULL:
				break;
			default:
				ASSERT(FALSE);
				break;
			}
		}

		template<class Archive>
		inline void serialize(Archive & ar, COleVariant &v, const unsigned int file_version)
		{
			boost::serialization::split_free(ar, v, file_version);
		}

	} // namespace serialization
} // namespace boost