#pragma once

#include <QMainWindow>
#include <QQmlApplicationEngine>
#include <QWindow>
#include <Windows.h>
#include <QLocalServer>
#include <QLocalSocket>

class PanelEngine : public QWidget
{
    Q_OBJECT

public:
    PanelEngine(QWidget *parent = nullptr);
    ~PanelEngine();
    static PanelEngine* instance;

private slots:
    void onLanguageChanged();
    void onNewConnection();
    void onClientDisconnected();
    void onPanelVisibilityChanged(bool visible);

private:
    QQmlApplicationEngine* engine;
    QWindow* panelWindow;

    void initializeQMLEngine();
    void destroyQMLEngine();
    void showPanel();
    void hidePanel();

    static LRESULT CALLBACK MouseProc(int nCode, WPARAM wParam, LPARAM lParam);
    static HHOOK mouseHook;

    void installGlobalMouseHook();
    void uninstallGlobalMouseHook();

    QLocalServer* localServer;
    void setupLocalServer();
    void cleanupLocalServer();
    bool isPanelVisible = false;
};
