#include "panelengine.h"
#include "mediasessionmanager.h"
#include "LanguageBridge.h"
#include <QMenu>
#include <QApplication>
#include <QScreen>
#include <QRect>
#include <QWindow>
#include <QQmlContext>
#include <QTimer>
#include <QFontMetrics>
#include <Windows.h>
#include <QProcess>

HHOOK PanelEngine::mouseHook = NULL;
PanelEngine* PanelEngine::instance = nullptr;
static bool validMouseDown = false;

PanelEngine::PanelEngine(QWidget *parent)
    : QWidget(parent)
    , engine(nullptr)
    , panelWindow(nullptr)
    , settings("Odizinne", "QontrolPanel")
    , localServer(nullptr)
{
    bool enableMediaSessionManager = settings.value("enableMediaSessionManager", true).toBool();
    if (enableMediaSessionManager) {
        MediaSessionManager::initialize();
    }

    instance = this;
    initializeQMLEngine();
    setupLocalServer();

    if (LanguageBridge::instance()) {
        connect(LanguageBridge::instance(), &LanguageBridge::languageChanged,
                this, &PanelEngine::onLanguageChanged);
    }

    LanguageBridge::instance()->changeApplicationLanguage(settings.value("languageIndex", 0).toInt());
}

PanelEngine::~PanelEngine()
{
    MediaSessionManager::cleanup();
    uninstallGlobalMouseHook();
    destroyQMLEngine();
    cleanupLocalServer();
    instance = nullptr;
}

void PanelEngine::initializeQMLEngine()
{
    if (engine) {
        return;
    }

    engine = new QQmlApplicationEngine(this);
    engine->loadFromModule("Odizinne.QontrolPanel", "Main");

    if (!engine->rootObjects().isEmpty()) {
        panelWindow = qobject_cast<QWindow*>(engine->rootObjects().constFirst());
        if (panelWindow) {
            connect(panelWindow, &QWindow::visibleChanged,
                    this, &PanelEngine::onPanelVisibilityChanged);
        }
    }
}

void PanelEngine::onPanelVisibilityChanged(bool visible)
{
    isPanelVisible = visible;

    if (visible) {
        installGlobalMouseHook();
    } else {
        uninstallGlobalMouseHook();
    }
}

void PanelEngine::destroyQMLEngine()
{
    if (engine) {
        engine->deleteLater();
        engine = nullptr;
    }
    panelWindow = nullptr;
}

void PanelEngine::installGlobalMouseHook()
{
    if (mouseHook == NULL) {
        mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseProc, NULL, 0);
    }
}

void PanelEngine::uninstallGlobalMouseHook()
{
    if (mouseHook != NULL) {
        UnhookWindowsHookEx(mouseHook);
        mouseHook = NULL;
    }
}

LRESULT CALLBACK PanelEngine::MouseProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (nCode == HC_ACTION) {
        if (wParam == WM_LBUTTONDOWN || wParam == WM_RBUTTONDOWN) {
            QPoint cursorPos = QCursor::pos();
            QRect soundPanelRect;

            if (instance->panelWindow && instance->isPanelVisible) {
                soundPanelRect = instance->panelWindow->geometry();
            } else {
                soundPanelRect = QRect();
            }

            validMouseDown = !soundPanelRect.contains(cursorPos);
        }
        else if ((wParam == WM_LBUTTONUP || wParam == WM_RBUTTONUP) && validMouseDown) {
            QPoint cursorPos = QCursor::pos();
            QRect soundPanelRect;

            if (instance->panelWindow && instance->isPanelVisible) {
                soundPanelRect = instance->panelWindow->geometry();
            } else {
                soundPanelRect = QRect();
            }

            validMouseDown = false;

            if (!soundPanelRect.contains(cursorPos)) {
                QMetaObject::invokeMethod(instance->panelWindow, "hidePanel");
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void PanelEngine::onLanguageChanged()
{
    if (engine) {
        engine->retranslate();
    }
}

void PanelEngine::setupLocalServer()
{
    localServer = new QLocalServer(this);
    QLocalServer::removeServer("QontrolPanel");

    if (!localServer->listen("QontrolPanel")) {
        qWarning() << "Failed to create local server:" << localServer->errorString();
        return;
    }

    connect(localServer, &QLocalServer::newConnection,
            this, &PanelEngine::onNewConnection);
}

void PanelEngine::cleanupLocalServer()
{
    if (localServer) {
        localServer->close();
        QLocalServer::removeServer("QontrolPanel");
        delete localServer;
        localServer = nullptr;
    }
}

void PanelEngine::onNewConnection()
{
    QLocalSocket* clientSocket = localServer->nextPendingConnection();
    if (!clientSocket) {
        return;
    }

    connect(clientSocket, &QLocalSocket::readyRead, this, [this, clientSocket]() {
        QByteArray data = clientSocket->readAll();
        QString message = QString::fromUtf8(data);

        if (message == "show_panel") {
            if (panelWindow) {
                QMetaObject::invokeMethod(panelWindow, "showPanel");
            }
        }

        clientSocket->disconnectFromServer();
    });

    connect(clientSocket, &QLocalSocket::disconnected,
            this, &PanelEngine::onClientDisconnected);
}

void PanelEngine::onClientDisconnected()
{
    QLocalSocket* clientSocket = qobject_cast<QLocalSocket*>(sender());
    if (clientSocket) {
        clientSocket->deleteLater();
    }
}
