#pragma once
#include <QObject>
#include <QQmlEngine>
#include <Windows.h>

class KeyboardShortcutManager : public QObject
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

    Q_INVOKABLE void manageKeyboardHook(bool enabled);

signals:
    void panelToggleRequested();
    void globalShortcutsSuspendedChanged();
    void chatMixEnabledChanged(bool enabled);
    void chatMixNotificationRequested(QString message);
    void chatMixToggleRequested();
    void micMuteToggleRequested();
    void panelCloseRequested();

private:
    explicit KeyboardShortcutManager(QObject *parent = nullptr);
    static KeyboardShortcutManager* m_instance;
    static HHOOK keyboardHook;
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    int qtKeyToVirtualKey(int qtKey);
    bool m_globalShortcutsSuspended = false;

    void installKeyboardHook();
    void uninstallKeyboardHook();
    bool isModifierPressed(int qtModifier);
    void toggleChatMixFromShortcut(bool enabled);
    bool handleCustomShortcut(DWORD vkCode);
};
