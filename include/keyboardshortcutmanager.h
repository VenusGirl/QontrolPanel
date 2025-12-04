#pragma once
#include <QObject>
#include <QQmlEngine>
#include <Windows.h>

class KeyboardShortcutManager : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
    Q_PROPERTY(bool globalShortcutsEnabled READ globalShortcutsEnabled WRITE setGlobalShortcutsEnabled NOTIFY globalShortcutsEnabledChanged)
    Q_PROPERTY(bool globalShortcutsSuspended READ globalShortcutsSuspended WRITE setGlobalShortcutsSuspended NOTIFY globalShortcutsSuspendedChanged)
    Q_PROPERTY(int panelShortcutKey READ panelShortcutKey WRITE setPanelShortcutKey NOTIFY panelShortcutKeyChanged)
    Q_PROPERTY(int panelShortcutModifiers READ panelShortcutModifiers WRITE setPanelShortcutModifiers NOTIFY panelShortcutModifiersChanged)
    Q_PROPERTY(int chatMixShortcutKey READ chatMixShortcutKey WRITE setChatMixShortcutKey NOTIFY chatMixShortcutKeyChanged)
    Q_PROPERTY(int chatMixShortcutModifiers READ chatMixShortcutModifiers WRITE setChatMixShortcutModifiers NOTIFY chatMixShortcutModifiersChanged)
    Q_PROPERTY(int micMuteShortcutKey READ micMuteShortcutKey WRITE setMicMuteShortcutKey NOTIFY micMuteShortcutKeyChanged)
    Q_PROPERTY(int micMuteShortcutModifiers READ micMuteShortcutModifiers WRITE setMicMuteShortcutModifiers NOTIFY micMuteShortcutModifiersChanged)

public:
    static KeyboardShortcutManager* instance();
    static KeyboardShortcutManager* create(QQmlEngine *qmlEngine, QJSEngine *jsEngine);
    ~KeyboardShortcutManager() override;

    bool globalShortcutsEnabled() const;
    void setGlobalShortcutsEnabled(bool enabled);

    bool globalShortcutsSuspended() const;
    void setGlobalShortcutsSuspended(bool suspended);

    int panelShortcutKey() const;
    void setPanelShortcutKey(int key);

    int panelShortcutModifiers() const;
    void setPanelShortcutModifiers(int modifiers);

    int chatMixShortcutKey() const;
    void setChatMixShortcutKey(int key);

    int chatMixShortcutModifiers() const;
    void setChatMixShortcutModifiers(int modifiers);

    int micMuteShortcutKey() const;
    void setMicMuteShortcutKey(int key);

    int micMuteShortcutModifiers() const;
    void setMicMuteShortcutModifiers(int modifiers);

signals:
    void panelToggleRequested();
    void globalShortcutsEnabledChanged();
    void globalShortcutsSuspendedChanged();
    void panelShortcutKeyChanged();
    void panelShortcutModifiersChanged();
    void chatMixShortcutKeyChanged();
    void chatMixShortcutModifiersChanged();
    void chatMixEnabledChanged(bool enabled);
    void chatMixNotificationRequested(QString message);
    void chatMixToggleRequested();
    void micMuteShortcutKeyChanged();
    void micMuteShortcutModifiersChanged();
    void micMuteToggleRequested();
    void panelCloseRequested();

private:
    explicit KeyboardShortcutManager(QObject *parent = nullptr);
    static KeyboardShortcutManager* m_instance;
    static HHOOK keyboardHook;
    static LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

    int qtKeyToVirtualKey(int qtKey);

    bool m_globalShortcutsEnabled = false;
    bool m_globalShortcutsSuspended = false;
    int m_panelShortcutKey = Qt::Key_S;
    int m_panelShortcutModifiers = Qt::ControlModifier | Qt::ShiftModifier;
    int m_chatMixShortcutKey = Qt::Key_M;
    int m_chatMixShortcutModifiers = Qt::ControlModifier | Qt::ShiftModifier;
    int m_micMuteShortcutKey = Qt::Key_K;
    int m_micMuteShortcutModifiers = Qt::ControlModifier | Qt::ShiftModifier;

    void installKeyboardHook();
    void uninstallKeyboardHook();
    bool isModifierPressed(int qtModifier);
    void toggleChatMixFromShortcut(bool enabled);
    bool handleCustomShortcut(DWORD vkCode);
};
