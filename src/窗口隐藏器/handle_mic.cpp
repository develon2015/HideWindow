#include "include.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>

static HMENU mainmenu = NULL;
static UINT item = 0;
static HHOOK hhook = NULL;
static IAudioEndpointVolume *microphone = NULL;

/**
 * 0 扬声器
 * 1 麦克风
 * @see https://github.com/fcannizzaro/win-audio
 */
IAudioEndpointVolume *getVolume(int mic, LPWSTR device_name = nullptr, LPWSTR *device_id = nullptr)
{
    HRESULT hr;
    IMMDeviceEnumerator *enumerator = NULL;
    IAudioEndpointVolume *volume = NULL;
    IMMDevice *defaultDevice = NULL;
    CoInitialize(NULL);
    hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&enumerator);
    hr = enumerator->GetDefaultAudioEndpoint(mic ? eCapture : eRender, eConsole, &defaultDevice);
    if (hr != 0)
    {
        return volume;
    }

    // 获取设备属性
    if (device_name) {
        IPropertyStore* pPropStore = NULL;
        hr = defaultDevice->OpenPropertyStore(STGM_READ, &pPropStore);
        if (SUCCEEDED(hr)) {
            PROPVARIANT varName;
            pPropStore->GetValue(PKEY_Device_DeviceDesc, &varName);
            wsprintfW(device_name, L"%s", varName.pwszVal);
        }
    }

    // 获取设备ID
    if (device_id) {
        defaultDevice->GetId(device_id);
    }

    hr = defaultDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, (LPVOID *)&volume);
    enumerator->Release();
    defaultDevice->Release();
    CoUninitialize();
    return volume;
}

static void switchMute() {
    BOOL muted = FALSE;
    microphone->GetMute(&muted);
    if (muted) {
        microphone->SetMute(FALSE, NULL);
    } else {
        microphone->SetMute(TRUE, NULL);
    }
}

static void setupVolume(float step) {
    float volume = 0;
    microphone->GetMasterVolumeLevelScalar(&volume);
    volume += step / 100;
    microphone->SetMasterVolumeLevelScalar(volume, NULL);
    BOOL muted = FALSE;
    microphone->GetMute(&muted);
    if (!muted && volume < 0.01) {
        microphone->SetMute(TRUE, NULL);
    } else if (muted && volume > 0.01 && step > 0) {
        microphone->SetMute(FALSE, NULL);
    }
}

static LRESULT CALLBACK HookProcedure(int nCode, WPARAM wParam, LPARAM lParam)
{
	KBDLLHOOKSTRUCT *p = (KBDLLHOOKSTRUCT *)lParam;
	// Do the wParam and lParam parameters contain information about a keyboard message.
	if (nCode == HC_ACTION)
	{
		// Messsage data is ready for pickup
		if (wParam == WM_SYSKEYDOWN || wParam == WM_KEYDOWN)
		// if (wParam == WM_SYSKEYUP || wParam == WM_KEYUP)
		{
            if (p->vkCode == 'T' && (p->flags & 0b00100000)) { // ALT+T快捷键静音
                switchMute();
                return TRUE;
            }
            if (p->vkCode == VK_UP && (p->flags & 0b00100000)) { // ALT+UP调节音量
                setupVolume(1);
                return TRUE;
            }
            if (p->vkCode == VK_DOWN && (p->flags & 0b00100000)) { // ALT+DOWN调节音量
                setupVolume(-1);
                return TRUE;
            }
		}
	}
	// hook procedure must pass the message *Always*
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static void setMsg(LPCWSTR msg) {
    // MENUITEMINFO info = {0};
    // info.cbSize = sizeof info;
    // info.fMask = MIIM_STRING;
    // info.dwTypeData = NULL;
    // GetMenuItemInfo(mainmenu, 201, FALSE, &info);
    // printf("cch -> %d.\n", info.cch);
    // info.dwTypeData = (LPSTR)malloc(info.cch); // 获取菜单项内容需要自行分配内存
    // GetMenuItemInfo(mainmenu, item, FALSE, &info);
    // printf("item content -> %s.\n", info.dwTypeData);
    MENUITEMINFOW info = {0};
    info.cbSize = sizeof info;
    info.fMask = MIIM_STRING;
    WCHAR buf[1024] = { 0 };
    if (msg) {
        // 这里，为什么wsprintfW不支持%Ts，只能小写为%ts
        wsprintfW(buf, L"麦克风（%ts）", msg);
        toast(buf);
    } else {
        wsprintfW(buf, L"麦克风");
    }
    info.dwTypeData = buf;
    SetMenuItemInfoW(mainmenu, item, FALSE, &info);
}

static void updateMicStatus(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    if (pNotify->bMuted) {
        return setMsg(L"已静音");
    }
    WCHAR buf[24] = { 0 };
    char buf_float[24] = { 0 };
    sprintf(buf_float, "%.0f", pNotify->fMasterVolume * 100);
    wsprintfW(buf, L"音量: %hs", buf_float);
    setMsg(buf);
}

static void updateMicStatusByIAudioEndpointVolume(IAudioEndpointVolume *microphone)
{
    BOOL muted = FALSE;
    microphone->GetMute(&muted);
    if (muted) {
        return setMsg(L"已静音");
    }
    WCHAR buf[24] = { 0 };
    float volume = 0;
    microphone->GetMasterVolumeLevelScalar(&volume);
    // wsprintf 不支持浮点数格式
    char buf_float[24] = { 0 };
    sprintf(buf_float, "%.0f", volume * 100);
    wsprintfW(buf, L"音量: %hs", buf_float);
    setMsg(buf);
}

/**
 * 音量监听
 * @see https://learn.microsoft.com/en-us/windows/win32/coreaudio/endpoint-volume-controls
 */
class CAudioEndpointVolumeCallback : public IAudioEndpointVolumeCallback
{
    LONG _cRef;

public:
    CAudioEndpointVolumeCallback() : _cRef(1)
    {
    }
    ~CAudioEndpointVolumeCallback()
    {
    }
    ULONG STDMETHODCALLTYPE AddRef()
    {
        return InterlockedIncrement(&_cRef);
    }

    ULONG STDMETHODCALLTYPE Release()
    {
        ULONG ulRef = InterlockedDecrement(&_cRef);
        if (0 == ulRef)
        {
            delete this;
        }
        return ulRef;
    }

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
    {
        if (IID_IUnknown == riid)
        {
            AddRef();
            *ppvInterface = (IUnknown *)this;
        }
        else if (__uuidof(IAudioEndpointVolumeCallback) == riid)
        {
            AddRef();
            *ppvInterface = (IAudioEndpointVolumeCallback *)this;
        }
        else
        {
            *ppvInterface = NULL;
            return E_NOINTERFACE;
        }
        return S_OK;
    }

    // Callback method for endpoint-volume-change notifications.

    HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
    {
        if (pNotify == NULL)
        {
            return E_INVALIDARG;
        }
        updateMicStatus(pNotify);
        return S_OK;
    }
};

static IAudioEndpointVolumeCallback *event = NULL;

static void install()
{
    microphone = getVolume(1);
    if (!microphone) {
        printf("microphone 无法访问\n");
        return;
    }
    event = new CAudioEndpointVolumeCallback();
    microphone->RegisterControlChangeNotify(event);
    updateMicStatusByIAudioEndpointVolume(microphone);
    hhook = SetWindowsHookExW(
        WH_KEYBOARD_LL,        // low-level keyboard input events
        HookProcedure,         // pointer to the hook procedure
        GetModuleHandle(NULL), // A handle to the DLL containing the hook procedure
        NULL                   // desktop apps, if this parameter is zero
    );
}

static void uninstall()
{
    UnhookWindowsHookEx(hhook);
    microphone->UnregisterControlChangeNotify(event);
    setMsg(NULL);
}

static void check() {
    MENUITEMINFO info = { 0 };
    info.cbSize = sizeof(info);
    info.fMask = MIIM_STATE;
    GetMenuItemInfo(mainmenu, item, FALSE, &info);
    if (info.fState == MFS_CHECKED)
    {
        uninstall();
        info.fState = MFS_UNCHECKED;
        SetMenuItemInfo(mainmenu, item, FALSE, &info);
    }
    else
    {
        install();
        init_float_window();
        info.fState = MFS_CHECKED;
        SetMenuItemInfo(mainmenu, item, FALSE, &info);
    }
    return;
}

void handle_mic(HMENU _mainmenu, UINT _item) {
    mainmenu = _mainmenu;
    item = _item;
    check();
}