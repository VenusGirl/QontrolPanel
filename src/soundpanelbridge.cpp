#include "soundpanelbridge.h"
#include "shortcutmanager.h"
#include <QBuffer>
#include <QPixmap>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QStyleHints>
#include <Windows.h>
#include <mmsystem.h>
#include <wtsapi32.h>

#pragma comment(lib, "powrprof.lib")
#pragma comment(lib, "wtsapi32.lib")
#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "user32.lib")

SoundPanelBridge* SoundPanelBridge::m_instance = nullptr;

SoundPanelBridge::SoundPanelBridge(QObject* parent)
    : QObject(parent)
{
    m_instance = this;
}

SoundPanelBridge::~SoundPanelBridge()
{
    if (m_instance == this) {
        m_instance = nullptr;
    }
}

SoundPanelBridge* SoundPanelBridge::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    if (!m_instance) {
        m_instance = new SoundPanelBridge();
    }
    return m_instance;
}

SoundPanelBridge* SoundPanelBridge::instance()
{
    return m_instance;
}

bool SoundPanelBridge::getShortcutState()
{
    return ShortcutManager::isShortcutPresent("QontrolPanel.lnk");
}

void SoundPanelBridge::setStartupShortcut(bool enabled)
{
    ShortcutManager::manageShortcut(enabled, "QontrolPanel.lnk");
}

void SoundPanelBridge::toggleChatMixFromShortcut(bool enabled)
{
    emit chatMixEnabledChanged(enabled);
}

void SoundPanelBridge::suspendGlobalShortcuts()
{
    m_globalShortcutsSuspended = true;
}

void SoundPanelBridge::resumeGlobalShortcuts()
{
    m_globalShortcutsSuspended = false;
}

bool SoundPanelBridge::areGlobalShortcutsSuspended() const
{
    return m_globalShortcutsSuspended;
}

void SoundPanelBridge::requestChatMixNotification(QString message) {
    emit chatMixNotificationRequested(message);
}

void SoundPanelBridge::openLegacySoundSettings() {
    QProcess::startDetached("control", QStringList() << "mmsys.cpl");
}

void SoundPanelBridge::openModernSoundSettings() {
    QProcess::startDetached("explorer", QStringList() << "ms-settings:sound");
}

int SoundPanelBridge::getAvailableDesktopWidth() const
{
    if (QGuiApplication::primaryScreen()) {
        return QGuiApplication::primaryScreen()->availableGeometry().width();
    }
    return 1920;
}

int SoundPanelBridge::getAvailableDesktopHeight() const
{
    if (QGuiApplication::primaryScreen()) {
        return QGuiApplication::primaryScreen()->availableGeometry().height();
    }
    return 1080;
}

void SoundPanelBridge::playFeedbackSound()
{
    PlaySound(TEXT("SystemDefault"), NULL, SND_ALIAS | SND_ASYNC);
}

void SoundPanelBridge::setStyle(int style) {
    switch (style) {
    case 0:
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Unknown);
        break;
    case 1:
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Dark);
        break;
    case 2:
        QGuiApplication::styleHints()->setColorScheme(Qt::ColorScheme::Light);
        break;
    }
}

