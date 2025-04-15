/// 播放器的抽象
#ifndef MUSICPLAYER_H
#define MUSICPLAYER_H
#include "database.h"
#include "metadata.h"
#include"videoplayer.h"
#include <QWidget>
#include <QVector>


class HistoryList :public QObject
{

    Q_OBJECT
private:
    const int MAX_HISTORY = 100; // 限制历史记录数量

public:

    QList<HistoryMData> history;
    void addToHistory(const HistoryMData &item)
    {
        // 检查是否存在相同项
        auto it = std::find(history.begin(), history.end(), item);
        if (it != history.end())
        {
            int index = std::distance(history.begin(), it);
            history.erase(it);
            emit historyListRemove(index);
        }
        history.prepend(item);

        while (history.size() > MAX_HISTORY)
        {
            history.removeLast();
            emit historyListRemove(history.size()-1);
        }
    }
    void sendHistoryRemove();
    signals:
    void historyListRemove(int index);
};

class MusicPlayer : public QObject
{
    Q_OBJECT
public:
    MusicPlayer();
    ~MusicPlayer();

    bool enable=0;
    QtAV::AVPlayer*player;
    HistoryList history;
    QString filePath;

    void initMusicByDir(const QString &mediaPath);

    void addPathToDB(const QString &filePath);

    static MusicPlayer &instance()
    {
        static MusicPlayer player;
        return player;
    }
    //nh
    void play();
    void pause();

    void stop();
    QtAV::AVPlayer::State state();
    void setVolume(int volume);
    void setPosition(qint64 position);
    qint64 duration();
    void setSpeed(qreal speed);

//nh

//下方所有load函数都仅仅是和数据库交互，而不直接和musictable交互
    void readHistoryList();

    void initConnect();
    // void readMusicList( const QString &playListName);
private:

    const QString locallist = "locallist";
    const QString historylist = "historylist";
    QStringList musicExtensions = {"*.mp3", "*.wav", "*.flac", "*.aac"};

    void loadLocalMusic(const QString &url, const QString &playListName);
    bool isUrlInDatabase(DataBase *db, const QString &url, const QString &playListName);

    void insertItemToMusicTableFromNewDir(const QString &url, const QString &dir);

signals:
    void mediaSetted(const QString &dir);
    ///用于addpath的信号
    void mediaListAdd();
    void mediaListSub(const QString &dir);
signals:
    void musicToMusictable(const MetaData & music);
signals:
    void mediaStatusChanged( QtAV::MediaStatus status);

signals:
    void stateChanged( QtAV::AVPlayer::State newState);


signals:
    void historyListChange(HistoryMData& item);
  public slots:
    void play(const QString& url);

    /// 负责向数据库上传路径并发送信号给musictable使其更新页面;
    void installPath(const QString &filePath);
    /// 负责向数据库卸载路径并发送信号给musictable使其更新页面;
    void uninstallPath(const QString &filePath);

    void onMediaChange(QtAV::MediaStatus state);


    void onAppAboutToQuit();
    void onStateChanged(QtAV::AVPlayer::State state);
//    void onMediaStatusChanged(QtAV::MediaStatus status);
};
#endif // MUSICPLAYER_H
