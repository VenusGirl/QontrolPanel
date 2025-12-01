#pragma once

#include <QObject>
#include <QQmlEngine>

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

    Q_INVOKABLE void openLegacySoundSettings();
    Q_INVOKABLE void openModernSoundSettings();
    Q_INVOKABLE int getAvailableDesktopWidth() const;
    Q_INVOKABLE int getAvailableDesktopHeight() const;
    Q_INVOKABLE void playFeedbackSound();
    Q_INVOKABLE void setStyle(int style);

private:
    static SoundPanelBridge* m_instance;
};
