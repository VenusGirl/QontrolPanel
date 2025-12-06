#include "keyboardshortcutmanager.h"
#include "usersettings.h"

HHOOK KeyboardShortcutManager::keyboardHook = NULL;

KeyboardShortcutManager* KeyboardShortcutManager::m_instance = nullptr;

KeyboardShortcutManager* KeyboardShortcutManager::instance()
{
    if (!m_instance) {
        m_instance = new KeyboardShortcutManager();
    }
    return m_instance;
}

KeyboardShortcutManager* KeyboardShortcutManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

KeyboardShortcutManager::KeyboardShortcutManager(QObject *parent)
    : QObject(parent)
{
    if (UserSettings::instance()->globalShortcutsEnabled()) {
        installKeyboardHook();
    }
}

KeyboardShortcutManager::~KeyboardShortcutManager()
{
    uninstallKeyboardHook();

    if (m_instance == this) {
        m_instance = nullptr;
    }
}

void KeyboardShortcutManager::manageKeyboardHook(bool enabled)
{
    if (enabled) {
        installKeyboardHook();
    } else {
        uninstallKeyboardHook();
    }
}

bool KeyboardShortcutManager::globalShortcutsSuspended() const
{
    return m_globalShortcutsSuspended;
}

void KeyboardShortcutManager::setGlobalShortcutsSuspended(bool suspended)
{
    if (m_globalShortcutsSuspended == suspended)
        return;

    m_globalShortcutsSuspended = suspended;
    emit globalShortcutsSuspendedChanged();
}

void KeyboardShortcutManager::toggleChatMixFromShortcut(bool enabled)
{
    emit chatMixEnabledChanged(enabled);
}

void KeyboardShortcutManager::installKeyboardHook()
{
    if (keyboardHook == NULL) {
        keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, NULL, 0);
    }
}

void KeyboardShortcutManager::uninstallKeyboardHook()
{
    if (keyboardHook != NULL) {
        UnhookWindowsHookEx(keyboardHook);
        keyboardHook = NULL;
    }
}

bool KeyboardShortcutManager::isModifierPressed(int qtModifier)
{
    bool result = true;
    if (qtModifier & Qt::ControlModifier) {
        result &= (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
    }
    if (qtModifier & Qt::ShiftModifier) {
        result &= (GetAsyncKeyState(VK_SHIFT) & 0x8000) != 0;
    }
    if (qtModifier & Qt::AltModifier) {
        result &= (GetAsyncKeyState(VK_MENU) & 0x8000) != 0;
    }
    return result;
}

int KeyboardShortcutManager::qtKeyToVirtualKey(int qtKey)
{
    switch (qtKey) {
    case Qt::Key_A: return 0x41;
    case Qt::Key_B: return 0x42;
    case Qt::Key_C: return 0x43;
    case Qt::Key_D: return 0x44;
    case Qt::Key_E: return 0x45;
    case Qt::Key_F: return 0x46;
    case Qt::Key_G: return 0x47;
    case Qt::Key_H: return 0x48;
    case Qt::Key_I: return 0x49;
    case Qt::Key_J: return 0x4A;
    case Qt::Key_K: return 0x4B;
    case Qt::Key_L: return 0x4C;
    case Qt::Key_M: return 0x4D;
    case Qt::Key_N: return 0x4E;
    case Qt::Key_O: return 0x4F;
    case Qt::Key_P: return 0x50;
    case Qt::Key_Q: return 0x51;
    case Qt::Key_R: return 0x52;
    case Qt::Key_S: return 0x53;
    case Qt::Key_T: return 0x54;
    case Qt::Key_U: return 0x55;
    case Qt::Key_V: return 0x56;
    case Qt::Key_W: return 0x57;
    case Qt::Key_X: return 0x58;
    case Qt::Key_Y: return 0x59;
    case Qt::Key_Z: return 0x5A;
    case Qt::Key_F1: return VK_F1;
    case Qt::Key_F2: return VK_F2;
    case Qt::Key_F3: return VK_F3;
    case Qt::Key_F4: return VK_F4;
    case Qt::Key_F5: return VK_F5;
    case Qt::Key_F6: return VK_F6;
    case Qt::Key_F7: return VK_F7;
    case Qt::Key_F8: return VK_F8;
    case Qt::Key_F9: return VK_F9;
    case Qt::Key_F10: return VK_F10;
    case Qt::Key_F11: return VK_F11;
    case Qt::Key_F12: return VK_F12;
    default: return 0;
    }
}

LRESULT CALLBACK KeyboardShortcutManager::KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        PKBDLLHOOKSTRUCT pKeyboard = reinterpret_cast<PKBDLLHOOKSTRUCT>(lParam);

        if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
            if (UserSettings::instance()->globalShortcutsEnabled()) {
                if (m_instance->handleCustomShortcut(pKeyboard->vkCode)) {
                    return 1;
                }
            }
        }
    }

    return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

bool KeyboardShortcutManager::handleCustomShortcut(DWORD vkCode)
{
    if (m_globalShortcutsSuspended) {
        return false;
    }

    if (vkCode == VK_LWIN || vkCode == VK_RWIN) {
        emit panelCloseRequested();
        return false;
    }

    int panelKey = qtKeyToVirtualKey(UserSettings::instance()->panelShortcutKey());
    if (vkCode == panelKey && isModifierPressed(UserSettings::instance()->panelShortcutModifiers())) {
        emit panelToggleRequested();
        return true;
    }

    int chatMixKey = qtKeyToVirtualKey(UserSettings::instance()->chatMixShortcutKey());
    if (vkCode == chatMixKey && isModifierPressed(UserSettings::instance()->chatMixShortcutModifiers())) {
        emit chatMixToggleRequested();
        return true;
    }

    int micMuteKey = qtKeyToVirtualKey(UserSettings::instance()->micMuteShortcutKey());
    if (vkCode == micMuteKey && isModifierPressed(UserSettings::instance()->micMuteShortcutModifiers())) {
        emit micMuteToggleRequested();
        return true;
    }

    return false;
}
