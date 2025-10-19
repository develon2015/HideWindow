#include <windows.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <iostream>
#include <Functiondiscoverykeys_devpkey.h>

#include "PolicyConfig.h"

// 释放COM对象
#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = nullptr; } }

IAudioEndpointVolume *getVolume(int mic, LPWSTR device_name = nullptr, LPWSTR *device_id = nullptr);

// 设置默认音频设备
HRESULT SetDefaultAudioDevice(LPCWSTR deviceId) {
    IPolicyConfig* pPolicyConfig = nullptr;
    HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigClient), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pPolicyConfig));
    if (SUCCEEDED(hr)) {
        hr = pPolicyConfig->SetDefaultEndpoint(deviceId, eMultimedia);
        SAFE_RELEASE(pPolicyConfig);
    }
    return hr;
}

int switchDevice() {
    HRESULT hr = CoInitialize(nullptr); // 初始化COM
    if (FAILED(hr)) {
        std::cerr << "COM initialization failed: " << hr << std::endl;
        return -1;
    }

    // 创建IMMDeviceEnumerator实例
    IMMDeviceEnumerator* pEnumerator = nullptr;
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), nullptr, CLSCTX_ALL, IID_PPV_ARGS(&pEnumerator));
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator: " << hr << std::endl;
        CoUninitialize();
        return -1;
    }

    // 获取所有音频输出设备
    IMMDeviceCollection* pDeviceCollection = nullptr;
    hr = pEnumerator->EnumAudioEndpoints(eRender, DEVICE_STATE_ACTIVE, &pDeviceCollection);
    if (FAILED(hr)) {
        std::cerr << "Failed to enumerate audio devices: " << hr << std::endl;
        SAFE_RELEASE(pEnumerator);
        CoUninitialize();
        return -1;
    }

    // 获取设备数量
    UINT deviceCount = 0;
    pDeviceCollection->GetCount(&deviceCount);
    std::wcout << L"Found " << deviceCount << L" audio devices:" << std::endl;

    // 获取当前默认设备ID
    LPWSTR currentDeviceID = nullptr;
    IAudioEndpointVolume *currentDevice = getVolume(0, nullptr, &currentDeviceID);

    // 遍历并显示设备
    UINT next = 0;
    for (UINT i = 0; i < deviceCount; i++) {
        IMMDevice* pDevice = nullptr;
        hr = pDeviceCollection->Item(i, &pDevice);
        if (SUCCEEDED(hr)) {
            // 获取设备名称
            IPropertyStore* pProps = nullptr;
            pDevice->OpenPropertyStore(STGM_READ, &pProps);
            PROPVARIANT varName;
            PropVariantInit(&varName);
            pProps->GetValue(PKEY_Device_FriendlyName, &varName);

            // 获取设备ID
            LPWSTR deviceId = nullptr;
            pDevice->GetId(&deviceId);

            std::wcout << L"Device " << i << L": " << varName.pwszVal << L" - " << deviceId;
            if (wcscmp(deviceId, currentDeviceID) == 0) {
                std::wcout << L" -> Current Device" << std::endl;
                next = (i + 1) % deviceCount;
                break;
            }
            std::wcout << std::endl;

            // 清理
            CoTaskMemFree(deviceId);
            PropVariantClear(&varName);
            SAFE_RELEASE(pProps);
            SAFE_RELEASE(pDevice);
        }
    }

    // 将下一个设备作为默认设备
    {
        IMMDevice* pDevice = nullptr;
        hr = pDeviceCollection->Item(next, &pDevice);
        if (SUCCEEDED(hr)) {
            // 获取设备ID
            LPWSTR deviceId = nullptr;
            pDevice->GetId(&deviceId);

            hr = SetDefaultAudioDevice(deviceId);
            if (SUCCEEDED(hr)) {
                std::wcout << L"Set default device to: " << deviceId << std::endl;
            } else {
                std::cerr << "Failed to set default device: " << hr << std::endl;
            }

            // 清理
            CoTaskMemFree(deviceId);
            SAFE_RELEASE(pDevice);
        }
    }

    // 清理
    SAFE_RELEASE(pDeviceCollection);
    SAFE_RELEASE(pEnumerator);
    CoUninitialize();

    return 0;
}