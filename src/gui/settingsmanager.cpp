#include "settingsmanager.h"

#include <QFileInfo>
#include <QDir>
#include<QHotkey>
#include<QStandardPaths>

#include<QThread>
SettingsManager* SettingsManager::s_instance = nullptr;

SettingsManager* SettingsManager::instance() {
    if (!s_instance) {
        s_instance = new SettingsManager("config.ini");
    }
    return s_instance;
}

SettingsManager::SettingsManager(const QString &fileName)
{
    QString configFilePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/" + fileName;
    QFileInfo fileInfo(configFilePath);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    settings.reset(new QSettings(configFilePath, QSettings::IniFormat));

    // 初始化默认值
    if (!settings->contains("shortcut")) {
        saveSettings("localPaths", QStringList{}); // 写入空数组
        settings->setValue("shortcut", "");
        settings->sync();
    }

    loadSettings("localPaths", paths);
}
void SettingsManager::saveSettings(const QString &tag, const QStringList &paths)
{
    settings->beginWriteArray(tag);
    for (int i = 0; i < paths.size(); ++i) {
        settings->setArrayIndex(i);
        settings->setValue("value", paths.at(i));  // 用固定键 "value"
    }
    settings->endArray();
    settings->sync();
}


void SettingsManager::loadSettings(const QString &tag, QStringList &paths)
{
    paths.clear();
    int size = settings->beginReadArray(tag);
    for (int i = 0; i < size; ++i) {
        settings->setArrayIndex(i);
        paths.append(settings->value("value").toString());
    }
    settings->endArray();
}
void SettingsManager::saveSettingsShortcutsMap(const QMap<QString, QKeySequence> &shortcuts)
{    settings->beginGroup("Shortcuts");
    for (auto it = shortcuts.begin(); it != shortcuts.end(); ++it) {
        settings->setValue(it.key(), it.value().toString());
    }
    settings->endGroup();
    settings->sync();
}

void SettingsManager::loadSettingsShortcutsMap(QMap<QString, QKeySequence> &shortcuts)
{
    shortcuts.clear();
    settings->beginGroup("Shortcuts");
    QStringList keys = settings->childKeys();
    for (const QString &key : keys) {
        QString shortcutStr = settings->value(key).toString();
        shortcuts[key] = QKeySequence(shortcutStr);
    }
    settings->endGroup();
}

void SettingsManager::addNewPath(const QString &path)
{
    isAdding=1;
    paths.append(path);
    saveSettings("localPaths", paths);
    emit pathChange();
}
void SettingsManager::deletePath(const QString &path)
{
    while (isAdding != 0) {
           QThread::msleep(1);  // 每次等待 1 毫秒，避免 CPU 占用过高
       }
    paths.removeOne(path);
    saveSettings("localPaths", paths);  // 重新保存整个 paths
    emit pathChange();
}

// deleteSettings 不再需要对数组删除了，可以只保留对 group 的删除用法
void SettingsManager::deleteSettings(const QString &tag, const QString &key)
{
    settings->beginGroup(tag);
    settings->remove(key);
    settings->endGroup();
    settings->sync();
}
void SettingsManager::savePlayerStates(int volume, int speedState, int loopState)
{
    settings->beginGroup("PlayerStates");
    settings->setValue("volume", volume);
    settings->setValue("speedState", speedState);
    settings->setValue("loopState", loopState);
    settings->endGroup();
    settings->sync();
}

void SettingsManager::loadPlayerStates(int &volume, int &speedState, int &loopState)
{
    settings->beginGroup("PlayerStates");
    volume = settings->value("volume", 50).toInt();        // 默认音量 50
    speedState = settings->value("speedState", 1).toInt(); // 默认 1x
    loopState = settings->value("loopState", 0).toInt();   // 默认顺序播放
    settings->endGroup();
}
