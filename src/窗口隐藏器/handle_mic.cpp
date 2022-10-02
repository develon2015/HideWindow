#include "include.h"
#include <mmdeviceapi.h>
#include <endpointvolume.h>

static HMENU mainmenu = NULL;
static UINT item = 0;
static HHOOK hhook = NULL;
static IAudioEndpointVolume *microphone = NULL;

/**
 * 0 扬声器
 * 1 麦克风
 * @see https://github.com/fcannizzaro/win-audio
 */
IAudioEndpointVolume *getVolume(int mic)
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
            if (p->vkCode == 'T' && (p->flags & 0b00100000)) { // 长按Alt+F12解锁
                switchMute();
                return TRUE;
            }
		}
	}
	// hook procedure must pass the message *Always*
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

static void setMsg(LPSTR msg) {
    // MENUITEMINFO info = {0};
    // info.cbSize = sizeof info;
    // info.fMask = MIIM_STRING;
    // info.dwTypeData = NULL;
    // GetMenuItemInfo(mainmenu, 201, FALSE, &info);
    // printf("cch -> %d.\n", info.cch);
    // info.dwTypeData = (LPSTR)malloc(info.cch); // 获取菜单项内容需要自行分配内存
    // GetMenuItemInfo(mainmenu, item, FALSE, &info);
    // printf("item content -> %s.\n", info.dwTypeData);
    MENUITEMINFO info = {0};
    info.cbSize = sizeof info;
    info.fMask = MIIM_STRING;
    char buf[1024] = { 0 };
    if (msg) {
        sprintf(buf, "麦克风（%s）", msg);
        printf("麦克风（%s）\n", msg);
    } else {
        sprintf(buf, "麦克风");
    }
    info.dwTypeData = buf;
    SetMenuItemInfo(mainmenu, item, FALSE, &info);
}

static void updateMicStatus(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
    if (pNotify->bMuted) {
        return setMsg("已静音");
    }
    char buf[24] = { 0 };
    sprintf(buf, "音量: %.0lf", pNotify->fMasterVolume * 100);
    setMsg(buf);
}

static void updateMicStatusByIAudioEndpointVolume(IAudioEndpointVolume *microphone)
{
    BOOL muted = FALSE;
    microphone->GetMute(&muted);
    if (muted) {
        return setMsg("已静音");
    }
    char buf[24] = { 0 };
    float volume = 0;
    microphone->GetMasterVolumeLevelScalar(&volume);
    sprintf(buf, "音量: %.0lf", volume * 100);
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

static auto event = CAudioEndpointVolumeCallback();

static void install()
{
    microphone = getVolume(1);
    if (!microphone) {
        printf("microphone 无法访问\n");
        return;
    }
    microphone->RegisterControlChangeNotify(&event);
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
    microphone->UnregisterControlChangeNotify(&event);
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