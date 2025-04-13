#include "shortcutmanager.h"
#include <QApplication>
#include <QHotkey>

ShortcutManager* ShortcutManager::s_instance = nullptr;

ShortcutManager* ShortcutManager::instance() {
    if (!s_instance) {
        s_instance = new ShortcutManager(qApp);
    }
    return s_instance;
}

ShortcutManager::ShortcutManager(QObject *parent) : QObject(parent) {
    QStringList actionNames = {"播放|暂停", "切换循环模式", "下一曲", "上一曲", "音量加", "音量减"};

    // 为每个动作创建一个 QHotkey 对象（初始不设置快捷键）
    for (const QString &name : actionNames) {
        QHotkey *hotkey = new QHotkey(QKeySequence(), true, this);
        hotkeys[name] = hotkey;
    }


    // 连接 QHotkey 的 activated 信号到对应槽
    connect(hotkeys["播放|暂停"], &QHotkey::activated, this, &ShortcutManager::playTriggered);
    connect(hotkeys["切换循环模式"], &QHotkey::activated, this, &ShortcutManager::switchLoopTriggered);
    connect(hotkeys["下一曲"], &QHotkey::activated, this, &ShortcutManager::nextTriggered);
    connect(hotkeys["上一曲"], &QHotkey::activated, this, &ShortcutManager::previousTriggered);
    connect(hotkeys["音量加"], &QHotkey::activated, this, &ShortcutManager::volumeUpTriggered);
    connect(hotkeys["音量减"], &QHotkey::activated, this, &ShortcutManager::volumeDownTriggered);
}

void ShortcutManager::setupDefaultShortcuts() {
    QMap<QString, QKeySequence> defaults = {
        {"播放|暂停", QKeySequence("Ctrl+P")},
        {"切换循环模式", QKeySequence("Ctrl+Space")},
        {"下一曲", QKeySequence("Ctrl+Up")},
        {"上一曲", QKeySequence("Ctrl+Down")},
        {"音量加", QKeySequence("Ctrl+Left")},
        {"音量减", QKeySequence("Ctrl+Right")}
    };

    for (auto it = defaults.begin(); it != defaults.end(); ++it) {
        QHotkey *hotkey = hotkeys[it.key()];
        hotkey->setShortcut(it.value(), true);

        // 跨平台调试信息
        qDebug() << "Registering" << it.key()
                 << "Sequence:" << it.value().toString()
                 << "Registered:" << hotkey->isRegistered()
                 <<"short-key:"<< hotkey->shortcut();

        // 显示详细键值信息
       }
}
void ShortcutManager::updateShortcut(const QString &name, const QKeySequence &keySequence) {
    if (!hotkeys.contains(name)) return;

    QHotkey *hotkey = hotkeys[name];
    hotkey->setShortcut(keySequence, true); // 设置并注册新快捷键

    qDebug() << "Update shortcut -" << name
             << "Key:" << keySequence.toString()
             << "Status:" << (hotkey->isRegistered() ? "Success" : "Fail");
}

