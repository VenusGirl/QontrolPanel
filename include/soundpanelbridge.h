#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QtQml/qqmlregistration.h>
#include <QSettings>
#include <QScreen>
#include <QGuiApplication>
#include <QTranslator>
#include <QTimer>

class SoundPanelBridge : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

public:
    explicit SoundPanelBridge(QObject* parent = nullptr);
    ~SoundPanelBridge() override;

    static SoundPanelBridge* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);
    static SoundPanelBridge* instance();

    Q_INVOKABLE void toggleChatMixFromShortcut(bool enabled);
    Q_INVOKABLE void suspendGlobalShortcuts();
    Q_INVOKABLE void resumeGlobalShortcuts();
    bool areGlobalShortcutsSuspended() const;
    void requestChatMixNotification(QString message);

    Q_INVOKABLE void openLegacySoundSettings();
    Q_INVOKABLE void openModernSoundSettings();
    Q_INVOKABLE int getAvailableDesktopWidth() const;
    Q_INVOKABLE int getAvailableDesktopHeight() const;
    Q_INVOKABLE void playFeedbackSound();
    Q_INVOKABLE void setStyle(int style);

signals:
    void chatMixEnabledChanged(bool enabled);
    void chatMixNotificationRequested(QString message);

private:
    static SoundPanelBridge* m_instance;
    bool m_globalShortcutsSuspended = false;
};
