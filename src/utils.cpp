#include "utils.h"
#include <QProcess>
#include <QStyleHints>
#include <QGuiApplication>
#include <QScreen>
#include <QFile>
#include <Windows.h>

Utils* Utils::m_instance = nullptr;

Utils::Utils(QObject* parent)
    : QObject(parent)
{
    m_instance = this;

    QFile successFile(":/sounds/outcome-success.wav");
    if (successFile.open(QIODevice::ReadOnly)) {
        m_successSound = successFile.readAll();
        successFile.close();
    }

    QFile failureFile(":/sounds/outcome-failure.wav");
    if (failureFile.open(QIODevice::ReadOnly)) {
        m_failureSound = failureFile.readAll();
        failureFile.close();
    }
}

Utils::~Utils()
{
    if (m_instance == this) {
        m_instance = nullptr;
    }
}

Utils* Utils::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)

    if (!m_instance) {
        m_instance = new Utils();
    }
    return m_instance;
}

Utils* Utils::instance()
{
    return m_instance;
}

void Utils::openLegacySoundSettings() {
    QProcess::startDetached("control", QStringList() << "mmsys.cpl");
}

void Utils::openModernSoundSettings() {
    QProcess::startDetached("explorer", QStringList() << "ms-settings:sound");
}

int Utils::getAvailableDesktopWidth() const
{
    if (QGuiApplication::primaryScreen()) {
        return QGuiApplication::primaryScreen()->availableGeometry().width();
    }
    return 1920;
}

int Utils::getAvailableDesktopHeight() const
{
    if (QGuiApplication::primaryScreen()) {
        return QGuiApplication::primaryScreen()->availableGeometry().height();
    }
    return 1080;
}

void Utils::playFeedbackSound()
{
    PlaySound(TEXT("SystemDefault"), NULL, SND_ALIAS | SND_ASYNC);
}

void Utils::playNotificationSound(bool enabled)
{
    const QByteArray& soundData = enabled ? m_successSound : m_failureSound;

    if (soundData.isEmpty()) {
        return;
    }

    PlaySoundA(reinterpret_cast<LPCSTR>(soundData.constData()),
               NULL,
               SND_MEMORY | SND_ASYNC);
}

void Utils::setStyle(int style) {
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

