#include "logmanager.h"
#include "usersettings.h"
#include <QDebug>
#include <QDateTime>

LogManager* LogManager::m_instance = nullptr;

LogManager::LogManager(QObject *parent)
    : QObject(parent)
{
    m_audioManagerLogging = UserSettings::instance()->audioManagerLogging();
    m_mediaSessionManagerLogging = UserSettings::instance()->mediaSessionManagerLogging();
    m_monitorManagerLogging = UserSettings::instance()->monitorManagerLogging();
    m_soundPanelBridgeLogging = UserSettings::instance()->soundPanelBridgeLogging();
    m_updaterLogging = UserSettings::instance()->updaterLogging();
    m_shortcutManagerLogging = UserSettings::instance()->shortcutManagerLogging();
    m_coreLogging = UserSettings::instance()->coreLogging();
    m_localServerLogging = UserSettings::instance()->localServerLogging();
    m_uiLogging = UserSettings::instance()->uiLogging();
    m_powerManagerLogging = UserSettings::instance()->powerManagerLogging();
    m_headsetControlManagerLogging = UserSettings::instance()->headsetControlManagerLogging();
    m_windowFocusManagerLogging = UserSettings::instance()->windowFocusManagerLogging();
}

LogManager* LogManager::create(QQmlEngine *qmlEngine, QJSEngine *jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

LogManager* LogManager::instance()
{
    if (!m_instance) {
        m_instance = new LogManager();
    }
    return m_instance;
}

void LogManager::sendLog(Sender sender, const QString &content)
{
    if (isLoggingEnabled(sender)) {
        emitLog(sender, Info, content);
    }
}

void LogManager::sendWarn(Sender sender, const QString &content)
{
    if (isLoggingEnabled(sender)) {
        emitLog(sender, Warning, content);
    }
}

void LogManager::sendCritical(Sender sender, const QString &content)
{
    if (isLoggingEnabled(sender)) {
        emitLog(sender, Critical, content);
    }
}

bool LogManager::isLoggingEnabled(Sender sender) const
{
    switch (sender) {
    case AudioManager:
        return m_audioManagerLogging;
    case MediaSessionManager:
        return m_mediaSessionManagerLogging;
    case MonitorManager:
        return m_monitorManagerLogging;
    case SoundPanelBridge:
        return m_soundPanelBridgeLogging;
    case Updater:
        return m_updaterLogging;
    case ShortcutManager:
        return m_shortcutManagerLogging;
    case Core:
        return m_coreLogging;
    case LocalServer:
        return m_localServerLogging;
    case Ui:
        return m_uiLogging;
    case PowerManager:
        return m_powerManagerLogging;
    case HeadsetControlManager:
        return m_headsetControlManagerLogging;
    case WindowFocusManager:
        return m_windowFocusManagerLogging;
    default:
        return true;
    }
}

void LogManager::setAudioManagerLogging(bool enabled)
{
    m_audioManagerLogging = enabled;
}

void LogManager::setMediaSessionManagerLogging(bool enabled)
{
    m_mediaSessionManagerLogging = enabled;
}

void LogManager::setMonitorManagerLogging(bool enabled)
{
    m_monitorManagerLogging = enabled;
}

void LogManager::setSoundPanelBridgeLogging(bool enabled)
{
    m_soundPanelBridgeLogging = enabled;
}

void LogManager::setUpdaterLogging(bool enabled)
{
    m_updaterLogging = enabled;
}

void LogManager::setShortcutManagerLogging(bool enabled)
{
    m_shortcutManagerLogging = enabled;
}

void LogManager::setCoreLogging(bool enabled)
{
    m_coreLogging = enabled;
}

void LogManager::setLocalServerLogging(bool enabled)
{
    m_localServerLogging = enabled;
}

void LogManager::setUiLogging(bool enabled)
{
    m_uiLogging = enabled;
}

void LogManager::setPowerManagerLogging(bool enabled)
{
    m_powerManagerLogging = enabled;
}

void LogManager::setHeadsetControlManagerLogging(bool enabled)
{
    m_headsetControlManagerLogging = enabled;
}

void LogManager::setWindowFocusManagerLogging(bool enabled)
{
    m_windowFocusManagerLogging = enabled;
}

QString LogManager::senderToString(Sender sender) const
{
    switch (sender) {
    case AudioManager:
        return "AudioManager";
    case MediaSessionManager:
        return "MediaSessionManager";
    case MonitorManager:
        return "MonitorManager";
    case SoundPanelBridge:
        return "SoundPanelBridge";
    case Updater:
        return "Updater";
    case ShortcutManager:
        return "ShortcutManager";
    case Core:
        return "Core";
    case LocalServer:
        return "LocalServer";
    case Ui:
        return "Ui";
    case PowerManager:
        return "PowerManager";
    case HeadsetControlManager:
        return "HeadsetControlManager";
    case WindowFocusManager:
        return "WindowFocusManager";
    default:
        return "Unknown";
    }
}

QString LogManager::formatMessage(Sender sender, LogType type, const QString &content) const
{
    QString typeStr;
    switch (type) {
    case Info:
        typeStr = "INFO";
        break;
    case Warning:
        typeStr = "WARN";
        break;
    case Critical:
        typeStr = "CRITICAL";
        break;
    }

    // Get current timestamp (hours:minutes:seconds only)
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");

    // Return plain text format (no colors) for QML
    return QString("[%1] %2 [%3] - %4")
        .arg(timestamp, senderToString(sender), typeStr, content);
}

void LogManager::emitLog(Sender sender, LogType type, const QString &content)
{
    QString typeStr;
    QString colorCode;

    switch (type) {
    case Info:
        typeStr = "INFO";
        colorCode = "\033[32m";
        break;
    case Warning:
        typeStr = "WARN";
        colorCode = "\033[33m";
        break;
    case Critical:
        typeStr = "CRITICAL";
        colorCode = "\033[31m";
        break;
    }

    const QString resetCode = "\033[0m";
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");

    QString consoleMessage = QString("[%1] %2 [%3%4%5] - %6")
                                 .arg(timestamp, senderToString(sender), colorCode, typeStr, resetCode, content);

    QString plainMessage = QString("[%1] %2 [%3] - %4")
                               .arg(timestamp, senderToString(sender), typeStr, content);

    qDebug().noquote() << consoleMessage;

    if (m_qmlReady) {
        // QML is ready, emit immediately
        emit logReceived(plainMessage, type);
    } else {
        // Buffer the log for later
        m_bufferedLogs.append(qMakePair(plainMessage, type));
    }
}

void LogManager::setQmlReady()
{
    if (m_qmlReady) return;

    m_qmlReady = true;

    // Convert buffered logs to QVariantList and emit
    QVariantList bufferedLogs;
    for (const auto& log : m_bufferedLogs) {
        QVariantMap logEntry;
        logEntry["message"] = log.first;
        logEntry["type"] = static_cast<int>(log.second);
        bufferedLogs.append(logEntry);
    }

    if (!bufferedLogs.isEmpty()) {
        emit bufferedLogsReady(bufferedLogs);
    }

    m_bufferedLogs.clear();
}
