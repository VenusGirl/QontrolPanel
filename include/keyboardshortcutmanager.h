#pragma once
#include <QObject>
#include <QQmlEngine>
#include <QAbstractNativeEventFilter>
#include <QMap>
#include <Windows.h>

class KeyboardShortcutManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool globalShortcutsSuspended READ globalShortcutsSuspended WRITE setGlobalShortcutsSuspended NOTIFY globalShortcutsSuspendedChanged)

public:
    static KeyboardShortcutManager* instance();
    static KeyboardShortcutManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    ~KeyboardShortcutManager() override;

    bool globalShortcutsSuspended() const;
    void setGlobalShortcutsSuspended(bool suspended);

    Q_INVOKABLE void manageGlobalShortcuts(bool enabled);

    // Native event filter to handle WM_HOTKEY messages
    bool nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result) override;

signals:
    void panelToggleRequested();
    void globalShortcutsSuspendedChanged();
    void chatMixEnabledChanged(bool enabled);
    void chatMixNotificationRequested(QString message);
    void chatMixToggleRequested();
    void micMuteToggleRequested();

private:
    explicit KeyboardShortcutManager(QObject *parent = nullptr);
    static KeyboardShortcutManager* m_instance;

    enum HotkeyId {
        HOTKEY_PANEL_TOGGLE = 1,
        HOTKEY_CHATMIX_TOGGLE = 2,
        HOTKEY_MIC_MUTE = 3
    };

    int qtKeyToVirtualKey(int qtKey);
    UINT convertQtModifiers(int qtMods);
    bool m_globalShortcutsSuspended = false;

    HWND m_hwnd = nullptr;
    QMap<int, bool> m_registeredHotkeys;

    void registerHotkeys();
    void unregisterHotkeys();
    void updateHotkey(HotkeyId id, int qtKey, int qtMods);
    void toggleChatMixFromShortcut(bool enabled);
};
