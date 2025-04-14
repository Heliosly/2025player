///目前主要用数据库存音频等元信息
#ifndef DATABASE_H
#define DATABASE_H
#include"metadata.h"
#include <QStringList>
#include <QMap>
#include <QPixmap>
#include <QList>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include<QJsonArray>
#include <QSqlError>
#include <QThreadStorage>
class DataBase : public QObject
{

    Q_OBJECT
public:
        static DataBase* instance();


        DataBase(const DataBase&) = delete;
        void operator=(const DataBase&) = delete;

    QSqlDatabase db;

public:
    bool rewriteTagsWithList(const QString &url, const QList<QPair<QString, qreal>> &tagsList);
bool checkUrlExistsInTags(const QString &url);
QList<QPair<QString, double>> parseTagsToOrderedList(const QJsonArray& tagsArray);
QJsonArray getTagsArrayByUrl(const QString &url);
bool saveTagsFromJson(const QString &url, const QJsonArray &tagsArray);
    void clearTable(const QString &playListName);
    bool createConnection(QString dataBase_Name);
    bool createTagsTableNotExist() ;
    MetaData getMetaDataByUrl(const QString &url);
    int getListCount(const QString &playListName);


    bool createPlayListNotExist();
    bool createHistoryListNotExist();
    bool createFilePathTable();
    bool createMediaUrlTable();
    bool addFilePath(const QString &dirpath);

    bool addMediaUrl(const QString &dir, const QString &mediaUrl);

    bool reSetListCount();
    bool deleteFromHistoryListByUrl(const QString &url);

    bool saveMetaData(const QMap<QString, QString> &metaDataMap,  const QPixmap &img, bool status);

    bool deleteByUrl(const QStringList &urllist,const QString&playListName);
    bool saveHistoryData(const QList<HistoryMData> &history);
    bool queryByUrl(const QString &url, const QString &playListName, QMap<QString,QString> &map);
    QStringList getUrlFromPlayList(const QString &playListName);
    bool deleteByDirPath(const QString &dirpath, const QString &playListName);
    QList<MetaData>getDataFromLocallistwithHint(int hint, int offset);

QJsonArray toApi(const QString &filePath);
// 在每个线程真正结束前，调用下面这个清理函数
void cleanupThreadDatabase();
private:
static QThreadStorage<QSqlDatabase> threadDatabase;
    // 返回当前线程的数据库连接，如果不存在则新建
    QSqlDatabase getThreadDatabase();
    DataBase();
    static DataBase *s_instance;
};

#endif // DATABASE_H
