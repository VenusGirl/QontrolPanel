#include "headsetcontrolmonitor.h"
#include "logmanager.h"
#include "usersettings.h"

HeadsetControlMonitor::HeadsetControlMonitor(QObject *parent)
    : QObject(parent)
    , m_fetchTimer(new QTimer(this))
    , m_isMonitoring(false)
    , m_fetchIntervalMs(30000)
    , m_hasSidetoneCapability(false)
    , m_hasLightsCapability(false)
    , m_deviceName("")
    , m_batteryStatus("BATTERY_UNAVAILABLE")
    , m_batteryLevel(-1)
    , m_anyDeviceFound(false)
{
    LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                    QString("HeadsetControlMonitor initialized - Library version: %1")
                                        .arg(QString::fromStdString(std::string(headsetcontrol::version()))));

    m_fetchTimer->setInterval(m_fetchIntervalMs);
    m_fetchTimer->setSingleShot(false);

    connect(m_fetchTimer, &QTimer::timeout, this, &HeadsetControlMonitor::fetchHeadsetInfo);
}

HeadsetControlMonitor::~HeadsetControlMonitor()
{
    LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                    "HeadsetControlMonitor destructor called");
    stopMonitoring();
}

void HeadsetControlMonitor::startMonitoring()
{
    if (m_isMonitoring) {
        LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                        "Headset monitoring already running, ignoring start request");
        return;
    }

    LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                    QString("Starting headset monitoring (fetch interval: %1ms)").arg(m_fetchIntervalMs));

    m_isMonitoring = true;
    m_fetchTimer->start();
    fetchHeadsetInfo();

    emit monitoringStateChanged(true);
}

void HeadsetControlMonitor::stopMonitoring()
{
    if (!m_isMonitoring) {
        return;
    }

    LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                    "Stopping headset monitoring");

    m_isMonitoring = false;
    m_fetchTimer->stop();

    m_cachedDevices.clear();
    m_headsets.clear();
    m_hasSidetoneCapability = false;
    m_hasLightsCapability = false;
    m_deviceName = "";
    m_batteryStatus = "";
    m_batteryLevel = 0;
    bool wasDeviceFound = m_anyDeviceFound;
    m_anyDeviceFound = false;

    emit capabilitiesChanged();
    emit deviceNameChanged();
    emit batteryStatusChanged();
    emit batteryLevelChanged();
    if (wasDeviceFound) {
        emit anyDeviceFoundChanged();
    }

    emit headsetDataUpdated(m_cachedDevices);
    emit monitoringStateChanged(false);

    LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                    "Headset monitoring stopped and data cleared");
}

bool HeadsetControlMonitor::isMonitoring() const
{
    return m_isMonitoring;
}

void HeadsetControlMonitor::setLights(bool enabled)
{
    if (!m_hasLightsCapability) {
        LogManager::instance()->sendWarn(LogManager::HeadsetControlManager,
                                         "Cannot set lights - device does not support lights capability");
        return;
    }

    if (m_headsets.empty()) {
        LogManager::instance()->sendWarn(LogManager::HeadsetControlManager,
                                         "Cannot set lights - no device connected");
        return;
    }

    LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                    QString("Setting headset lights: %1").arg(enabled ? "ON" : "OFF"));

    auto& headset = m_headsets[0];
    auto result = headset.setLights(enabled);

    if (!result) {
        LogManager::instance()->sendCritical(LogManager::HeadsetControlManager,
                                             QString("Failed to set lights: %1").arg(QString::fromStdString(result.error().fullMessage())));
    } else {
        LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                        "Lights set successfully");
    }
}

void HeadsetControlMonitor::setSidetone(int value)
{
    if (!m_hasSidetoneCapability) {
        LogManager::instance()->sendWarn(LogManager::HeadsetControlManager,
                                         "Cannot set sidetone - device does not support sidetone capability");
        return;
    }

    if (m_headsets.empty()) {
        LogManager::instance()->sendWarn(LogManager::HeadsetControlManager,
                                         "Cannot set sidetone - no device connected");
        return;
    }

    value = qBound(0, value, 128);

    LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                    QString("Setting headset sidetone to %1").arg(value));

    auto& headset = m_headsets[0];
    auto result = headset.setSidetone(static_cast<uint8_t>(value));

    if (!result) {
        LogManager::instance()->sendCritical(LogManager::HeadsetControlManager,
                                             QString("Failed to set sidetone: %1").arg(QString::fromStdString(result.error().fullMessage())));
    } else {
        LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                        QString("Sidetone set to %1 successfully").arg(value));
    }
}

void HeadsetControlMonitor::fetchHeadsetInfo()
{
    if (!m_isMonitoring) {
        return;
    }

    try {
        // Discover headsets
        m_headsets = headsetcontrol::discover();

        if (m_headsets.empty()) {
            LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                            "No headset devices found");

            m_cachedDevices.clear();
            m_hasSidetoneCapability = false;
            m_hasLightsCapability = false;
            m_deviceName = "";
            m_batteryStatus = "";
            m_batteryLevel = 0;
            bool wasDeviceFound = m_anyDeviceFound;
            m_anyDeviceFound = false;

            emit capabilitiesChanged();
            emit deviceNameChanged();
            emit batteryStatusChanged();
            emit batteryLevelChanged();
            if (wasDeviceFound) {
                emit anyDeviceFoundChanged();
            }
            emit headsetDataUpdated(m_cachedDevices);
            return;
        }

        LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                        QString("Found %1 headset device(s)").arg(m_headsets.size()));

        updateDeviceCache();

    } catch (const std::exception& e) {
        LogManager::instance()->sendCritical(LogManager::HeadsetControlManager,
                                             QString("Exception during headset discovery: %1").arg(e.what()));

        emit headsetDataUpdated(m_cachedDevices);
    }
}

void HeadsetControlMonitor::updateDeviceCache()
{
    QList<HeadsetControlDevice> newDevices;

    for (auto& headset : m_headsets) {
        HeadsetControlDevice device;

        device.deviceName = QString::fromStdString(std::string(headset.name()));
        device.vendorId = QString("0x%1").arg(headset.vendorId(), 4, 16, QChar('0')).toUpper();
        device.productId = QString("0x%1").arg(headset.productId(), 4, 16, QChar('0')).toUpper();
        device.vendor = device.vendorId;  // Could be enhanced with vendor name lookup
        device.product = device.productId;

        device.capabilities = getCapabilityList(headset);

        // Query battery if supported
        if (headset.supports(CAP_BATTERY_STATUS)) {
            auto batteryResult = headset.getBattery();
            if (batteryResult) {
                device.batteryLevel = batteryResult->level_percent;
                device.batteryStatus = batteryStatusToString(batteryResult->status);

                LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                                QString("Device %1: Battery %2 at %3%")
                                                    .arg(device.deviceName, device.batteryStatus).arg(device.batteryLevel));
            } else {
                device.batteryLevel = -1;
                device.batteryStatus = "BATTERY_UNAVAILABLE";
                LogManager::instance()->sendWarn(LogManager::HeadsetControlManager,
                                                 QString("Failed to read battery: %1")
                                                     .arg(QString::fromStdString(batteryResult.error().fullMessage())));
            }
        } else {
            device.batteryStatus = "BATTERY_UNAVAILABLE";
            device.batteryLevel = -1;
        }

        LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                        QString("Found headset device: %1 with %2 capabilities")
                                            .arg(device.deviceName).arg(device.capabilities.size()));

        newDevices.append(device);
    }

    m_cachedDevices = newDevices;
    updateCapabilities();

    // Update bridge battery info from the first device
    if (!m_cachedDevices.isEmpty()) {
        const HeadsetControlDevice& primaryDevice = m_cachedDevices.first();
        if (primaryDevice.batteryStatus != m_batteryStatus) {
            m_batteryStatus = primaryDevice.batteryStatus;
            emit batteryStatusChanged();
        }
        if (primaryDevice.batteryLevel != m_batteryLevel) {
            m_batteryLevel = primaryDevice.batteryLevel;
            emit batteryLevelChanged();
        }
    }

    emit headsetDataUpdated(m_cachedDevices);
}

void HeadsetControlMonitor::updateCapabilities()
{
    bool newSidetoneCapability = false;
    bool newLightsCapability = false;
    QString newDeviceName = "";
    bool newAnyDeviceFound = !m_cachedDevices.isEmpty();
    bool wasDeviceFound = m_anyDeviceFound;

    if (!m_headsets.empty()) {
        const auto& headset = m_headsets[0];
        newDeviceName = QString::fromStdString(std::string(headset.name()));

        newSidetoneCapability = headset.supports(CAP_SIDETONE);
        newLightsCapability = headset.supports(CAP_LIGHTS);

        LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                        QString("Device capabilities - Sidetone: %1, Lights: %2")
                                            .arg(newSidetoneCapability ? "YES" : "NO", newLightsCapability ? "YES" : "NO"));
    }

    if (newSidetoneCapability != m_hasSidetoneCapability ||
        newLightsCapability != m_hasLightsCapability) {
        m_hasSidetoneCapability = newSidetoneCapability;
        m_hasLightsCapability = newLightsCapability;
        emit capabilitiesChanged();
    }

    if (newDeviceName != m_deviceName) {
        m_deviceName = newDeviceName;
        emit deviceNameChanged();
    }

    if (newAnyDeviceFound != m_anyDeviceFound) {
        m_anyDeviceFound = newAnyDeviceFound;
        emit anyDeviceFoundChanged();

        if (!wasDeviceFound && newAnyDeviceFound) {
            LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                            "New headset device detected, applying saved settings");

            if (newLightsCapability) {
                bool lightsEnabled = UserSettings::instance()->headsetcontrolLights();
                LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                                QString("Applying saved lights setting: %1").arg(lightsEnabled ? "ON" : "OFF"));
                setLights(lightsEnabled);
            }
            if (newSidetoneCapability) {
                int sidetoneValue = UserSettings::instance()->headsetcontrolSidetone();
                LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                                QString("Applying saved sidetone setting: %1").arg(sidetoneValue));
                setSidetone(sidetoneValue);
            }
        }
    }
}

QString HeadsetControlMonitor::batteryStatusToString(battery_status status) const
{
    switch (status) {
    case BATTERY_AVAILABLE:
        return "BATTERY_AVAILABLE";
    case BATTERY_CHARGING:
        return "BATTERY_CHARGING";
    case BATTERY_UNAVAILABLE:
        return "BATTERY_UNAVAILABLE";
    case BATTERY_HIDERROR:
        return "BATTERY_HIDERROR";
    case BATTERY_TIMEOUT:
        return "BATTERY_TIMEOUT";
    default:
        return "BATTERY_UNAVAILABLE";
    }
}

QStringList HeadsetControlMonitor::getCapabilityList(const headsetcontrol::Headset& headset) const
{
    QStringList capabilities;

    // Check all known capabilities
    for (int cap = 0; cap < NUM_CAPABILITIES; ++cap) {
        if (headset.supports(static_cast<enum capabilities>(cap))) {
            const char* capName = capability_to_enum_string(static_cast<enum capabilities>(cap));
            capabilities << QString::fromUtf8(capName);
        }
    }

    return capabilities;
}

void HeadsetControlMonitor::setFetchInterval(int intervalMs)
{
    m_fetchIntervalMs = intervalMs;
    m_fetchTimer->setInterval(m_fetchIntervalMs);

    LogManager::instance()->sendLog(LogManager::HeadsetControlManager,
                                    QString("Fetch interval updated to %1ms").arg(m_fetchIntervalMs));
}
