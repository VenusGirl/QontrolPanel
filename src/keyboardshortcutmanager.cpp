#include "keyboardshortcutmanager.h"
#include "usersettings.h"
#include <QCoreApplication>
#include <QWindow>

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
    // Get a window handle - we'll use a message-only window
    m_hwnd = CreateWindowEx(0, L"STATIC", L"QontrolPanelHotkeys",
                            0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);

    // Install the native event filter
    QCoreApplication::instance()->installNativeEventFilter(this);

    if (UserSettings::instance()->globalShortcutsEnabled()) {
        registerHotkeys();
    }
}

KeyboardShortcutManager::~KeyboardShortcutManager()
{
    unregisterHotkeys();

    // Remove the native event filter
    QCoreApplication::instance()->removeNativeEventFilter(this);

    // Destroy the message-only window
    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    if (m_instance == this) {
        m_instance = nullptr;
    }
}

void KeyboardShortcutManager::manageGlobalShortcuts(bool enabled)
{
    if (enabled) {
        registerHotkeys();
    } else {
        unregisterHotkeys();
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

void KeyboardShortcutManager::registerHotkeys()
{
    if (!m_hwnd) return;

    // Unregister any existing hotkeys first
    unregisterHotkeys();

    // Register panel toggle hotkey
    UINT panelMods = convertQtModifiers(UserSettings::instance()->panelShortcutModifiers());
    UINT panelKey = qtKeyToVirtualKey(UserSettings::instance()->panelShortcutKey());
    if (panelKey != 0 && RegisterHotKey(m_hwnd, HOTKEY_PANEL_TOGGLE, panelMods, panelKey)) {
        m_registeredHotkeys[HOTKEY_PANEL_TOGGLE] = true;
    }

    // Register ChatMix toggle hotkey
    UINT chatMixMods = convertQtModifiers(UserSettings::instance()->chatMixShortcutModifiers());
    UINT chatMixKey = qtKeyToVirtualKey(UserSettings::instance()->chatMixShortcutKey());
    if (chatMixKey != 0 && RegisterHotKey(m_hwnd, HOTKEY_CHATMIX_TOGGLE, chatMixMods, chatMixKey)) {
        m_registeredHotkeys[HOTKEY_CHATMIX_TOGGLE] = true;
    }

    // Register mic mute hotkey
    UINT micMuteMods = convertQtModifiers(UserSettings::instance()->micMuteShortcutModifiers());
    UINT micMuteKey = qtKeyToVirtualKey(UserSettings::instance()->micMuteShortcutKey());
    if (micMuteKey != 0 && RegisterHotKey(m_hwnd, HOTKEY_MIC_MUTE, micMuteMods, micMuteKey)) {
        m_registeredHotkeys[HOTKEY_MIC_MUTE] = true;
    }
}

void KeyboardShortcutManager::unregisterHotkeys()
{
    if (!m_hwnd) return;

    for (auto it = m_registeredHotkeys.begin(); it != m_registeredHotkeys.end(); ++it) {
        UnregisterHotKey(m_hwnd, it.key());
    }
    m_registeredHotkeys.clear();
}

void KeyboardShortcutManager::updateHotkey(HotkeyId id, int qtKey, int qtMods)
{
    if (!m_hwnd) return;

    // Unregister the old hotkey
    if (m_registeredHotkeys.contains(id)) {
        UnregisterHotKey(m_hwnd, id);
        m_registeredHotkeys.remove(id);
    }

    // Register the new hotkey
    UINT mods = convertQtModifiers(qtMods);
    UINT key = qtKeyToVirtualKey(qtKey);
    if (key != 0 && RegisterHotKey(m_hwnd, id, mods, key)) {
        m_registeredHotkeys[id] = true;
    }
}

UINT KeyboardShortcutManager::convertQtModifiers(int qtMods)
{
    UINT winMods = 0;
    if (qtMods & Qt::ControlModifier) winMods |= MOD_CONTROL;
    if (qtMods & Qt::ShiftModifier) winMods |= MOD_SHIFT;
    if (qtMods & Qt::AltModifier) winMods |= MOD_ALT;
    return winMods;
}

bool KeyboardShortcutManager::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    Q_UNUSED(result)

    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        MSG* msg = static_cast<MSG*>(message);

        if (msg->message == WM_HOTKEY) {
            // Check if shortcuts are suspended
            if (m_globalShortcutsSuspended) {
                return false;
            }

            // Handle the hotkey based on its ID
            switch (msg->wParam) {
                case HOTKEY_PANEL_TOGGLE:
                    emit panelToggleRequested();
                    return true;
                case HOTKEY_CHATMIX_TOGGLE:
                    emit chatMixToggleRequested();
                    return true;
                case HOTKEY_MIC_MUTE:
                    emit micMuteToggleRequested();
                    return true;
            }
        }
    }

    return false;
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
