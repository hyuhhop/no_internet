#include "unit.h"



void disable_network(bool disable) {
    HRESULT hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        throw std::runtime_error("COM初始化失败");
    }

    hres = CoInitializeSecurity(
        NULL, -1, NULL, NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE, NULL
    );
    if (FAILED(hres)) {
        CoUninitialize();
        throw std::runtime_error("COM安全初始化失败");
    }

    IWbemLocator* pLoc = NULL;
    hres = CoCreateInstance(
        CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
        IID_IWbemLocator, (LPVOID*)&pLoc
    );
    if (FAILED(hres)) {
        CoUninitialize();
        throw std::runtime_error("创建IWbemLocator失败");
    }

    IWbemServices* pSvc = NULL;
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"),
        NULL, NULL, 0, NULL, 0, 0, &pSvc
    );
    if (FAILED(hres)) {
        pLoc->Release();
        CoUninitialize();
        throw std::runtime_error("连接WMI服务失败");
    }

    hres = CoSetProxyBlanket(
        pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
        RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL, EOAC_NONE
    );
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        throw std::runtime_error("设置WMI代理安全失败");
    }

    IEnumWbemClassObject* pEnumerator = NULL;
    std::string method = disable ? "Disable" : "Enable";
    std::string query = disable
        ? "SELECT * FROM Win32_NetworkAdapter WHERE NetEnabled = TRUE"
        : "SELECT * FROM Win32_NetworkAdapter WHERE NetEnabled = FALSE";
    hres = pSvc->ExecQuery(
        bstr_t("WQL"),
        bstr_t(query.c_str()),
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL, &pEnumerator
    );
    if (FAILED(hres)) {
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        throw std::runtime_error("WMI查询失败");
    }

    IWbemClassObject* pAdapter = NULL;
    ULONG uReturn = 0;
    int count = 0;
    while (pEnumerator) {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pAdapter, &uReturn);
        if (0 == uReturn) break;

        VARIANT vtPath, vtName;
        hr = pAdapter->Get(L"__PATH", 0, &vtPath, 0, 0);
        pAdapter->Get(L"Name", 0, &vtName, 0, 0);

        if (SUCCEEDED(hr)) {
            IWbemClassObject* pOutParams = NULL;
            hres = pSvc->ExecMethod(
                vtPath.bstrVal,
                bstr_t(method.c_str()),
                0, NULL, NULL, &pOutParams, NULL
            );
            if (SUCCEEDED(hres)) {
                count++;
                if (pOutParams) pOutParams->Release();
            }
        }
        VariantClear(&vtPath);
        VariantClear(&vtName);
        pAdapter->Release();
    }
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();
}