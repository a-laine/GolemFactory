#include "OpenSaveFileDialog.h"
#include "System.h"

#include <sstream>


#ifdef GF_OS_WINDOWS
#include <shobjidl.h>
#include <objbase.h>
//#include <knownfolders.h> // for KnownFolder APIs/datatypes/function headers
#include <propvarutil.h>  // for PROPVAR-related functions
//#include <propkey.h>      // for the Property key APIs/datatypes
//#include <propidl.h>      // for the Property System APIs
//#include <strsafe.h>      // for StringCchPrintfW
#include <shtypes.h>      // for COMDLG_FILTERSPEC

#pragma comment(lib,"Shlwapi.lib")
#pragma comment(lib,"Propsys.lib")

class CDialogEventHandler : public IFileDialogEvents, public IFileDialogControlEvents
{
public:
    // IUnknown methods
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv)
    {
        static const QITAB qit[] = {
            QITABENT(CDialogEventHandler, IFileDialogEvents),
            QITABENT(CDialogEventHandler, IFileDialogControlEvents),
            { 0 },
#pragma warning(suppress:4838)
        };
        return QISearch(this, qit, riid, ppv);
    }

    IFACEMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_cRef); }

    IFACEMETHODIMP_(ULONG) Release()
    {
        long cRef = InterlockedDecrement(&_cRef);
        if (!cRef)
            delete this;
        return cRef;
    }

    // IFileDialogEvents methods
    IFACEMETHODIMP OnFileOk(IFileDialog*) { return S_OK; };
    IFACEMETHODIMP OnFolderChange(IFileDialog*) { return S_OK; };
    IFACEMETHODIMP OnFolderChanging(IFileDialog*, IShellItem*) { return S_OK; };
    IFACEMETHODIMP OnHelp(IFileDialog*) { return S_OK; };
    IFACEMETHODIMP OnSelectionChange(IFileDialog*) { return S_OK; };
    IFACEMETHODIMP OnShareViolation(IFileDialog*, IShellItem*, FDE_SHAREVIOLATION_RESPONSE*) { return S_OK; };
    IFACEMETHODIMP OnTypeChange(IFileDialog* pfd)
    {
        IFileSaveDialog* pfsd;
        HRESULT hr = pfd->QueryInterface(&pfsd);
        if (SUCCEEDED(hr))
        {
            UINT uIndex;
            hr = pfsd->GetFileTypeIndex(&uIndex);
            if (SUCCEEDED(hr))
            {
                IPropertyDescriptionList* pdl = NULL;

                switch (uIndex)
                {
                    case 1:
                        hr = PSGetPropertyDescriptionListFromString(L"prop:System.Title", IID_PPV_ARGS(&pdl));
                        if (SUCCEEDED(hr))
                        {
                            hr = pfsd->SetCollectedProperties(pdl, FALSE);
                            pdl->Release();
                        }
                        break;

                    case 2:
                        hr = PSGetPropertyDescriptionListFromString(L"prop:System.Keywords", IID_PPV_ARGS(&pdl));
                        if (SUCCEEDED(hr))
                        {
                            hr = pfsd->SetCollectedProperties(pdl, FALSE);
                            pdl->Release();
                        }
                        break;

                    case 3:
                        hr = PSGetPropertyDescriptionListFromString(L"prop:System.Author", IID_PPV_ARGS(&pdl));
                        if (SUCCEEDED(hr))
                        {
                            hr = pfsd->SetCollectedProperties(pdl, TRUE);
                            pdl->Release();
                        }
                        break;
                }
            }
            pfsd->Release();
        }
        return hr;
    }
    IFACEMETHODIMP OnOverwrite(IFileDialog*, IShellItem*, FDE_OVERWRITE_RESPONSE*) { return S_OK; };

    // IFileDialogControlEvents methods
    IFACEMETHODIMP OnItemSelected(IFileDialogCustomize* pfdc, DWORD dwIDCtl, DWORD dwIDItem)
    {
        IFileDialog* pfd = NULL;
        HRESULT hr = pfdc->QueryInterface(&pfd);
        if (SUCCEEDED(hr))
        {
            if (dwIDCtl == 2)
            {
                switch (dwIDItem)
                {
                    case 1: hr = pfd->SetTitle(L"Longhorn Dialog"); break;
                    case 2: hr = pfd->SetTitle(L"Vista Dialog"); break;
                }
            }
            pfd->Release();
        }
        return hr;
    };
    IFACEMETHODIMP OnButtonClicked(IFileDialogCustomize*, DWORD) { return S_OK; };
    IFACEMETHODIMP OnCheckButtonToggled(IFileDialogCustomize*, DWORD, BOOL) { return S_OK; };
    IFACEMETHODIMP OnControlActivating(IFileDialogCustomize*, DWORD) { return S_OK; };

    CDialogEventHandler() : _cRef(1) { };
private:
    ~CDialogEventHandler() { };
    long _cRef;
};

#else
#error OS not supported
#endif









bool OpenSaveFileDialog::OpenFile(std::string& filename)
{
#ifdef GF_OS_WINDOWS
    // init
    filename = "";    
    IFileDialog* pfd = NULL;
    HRESULT hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pfd));
    if (!SUCCEEDED(hr))
        return false;

    // Create an event handling object, and hook it up to the dialog.
    IFileDialogEvents* pfde = NULL;
    CDialogEventHandler* pDialogEventHandler = new CDialogEventHandler();
    hr = pDialogEventHandler ? S_OK : E_OUTOFMEMORY;
    if (!SUCCEEDED(hr))
    {
        pfd->Release();
        return false;
    }

    hr = pDialogEventHandler->QueryInterface(IID_PPV_ARGS(&pfde));
    pDialogEventHandler->Release();
    if (!SUCCEEDED(hr))
    {
        pfde->Release();
        pfd->Release();
        return false;
    }

    // Hook up the event handler.
    DWORD dwCookie;
    hr = pfd->Advise(pfde, &dwCookie);
    if (!SUCCEEDED(hr))
    {
        pfd->Unadvise(dwCookie);
        pfde->Release();
        pfd->Release();
        return false;
    }

    // Before setting, always get the options first in order not to override existing options.
    DWORD dwFlags;
    hr = pfd->GetOptions(&dwFlags);
    hr = pfd->SetOptions(dwFlags | FOS_FORCEFILESYSTEM);
    //hr = pfd->SetFileTypes(ARRAYSIZE(c_rgSaveTypes), c_rgSaveTypes);
    hr = pfd->SetFileTypeIndex(1);
    hr = pfd->SetDefaultExtension(L"doc;docx");
    hr = pfd->Show(NULL);
    if (!SUCCEEDED(hr))
    {
        pfd->Unadvise(dwCookie);
        pfde->Release();
        pfd->Release();
        return false;
    }

    bool success = false;
    IShellItem* psiResult;
    hr = pfd->GetResult(&psiResult);
    if (SUCCEEDED(hr))
    {
        // We are just going to print out the 
        // name of the file for sample sake.
        PWSTR pszFilePath = NULL;
        hr = psiResult->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

        if (SUCCEEDED(hr))
        {
            std::stringstream ss;
            ss << pszFilePath;
            filename = ss.str();

            CoTaskMemFree(pszFilePath);
            success = true;
        }
        psiResult->Release();
    }

    pfd->Unadvise(dwCookie);
    pfde->Release();
    pfd->Release();
    return success;

#else
    return false;
#endif

}
