#include "database.h"
#include<QStandardPaths>
#include "metadata.h"
#include <QFileInfo>
#include <QBuffer>
#include <QDebug>
#include <QSqlRecord>
#include <QApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <QFile>
#include <QHttpPart>
#include <QHttpMultiPart>
#include <QDir>
#include <QMap>
#include <QThread>
#include <QMutex>

// 定义静态成员
DataBase* DataBase::s_instance = nullptr;
QThreadStorage<QSqlDatabase> DataBase::threadDatabase;
QMutex DataBase::dbMutex(QMutex::Recursive); // 添加 QMutex 静态成员

DataBase* DataBase::instance()
{
        if (s_instance == nullptr) {
            s_instance = new DataBase();  // 创建唯一的实例
        }
    return s_instance;
}

DataBase::DataBase()
{
    // 主线程调用 createConnection() 初始化数据库（仅用于主线程）
    createConnection("metaData");
    // 主线程使用 getThreadDatabase() 自动创建专用连接
    db = getThreadDatabase();

    createFilePathTable();
    createPlayListNotExist();
    createHistoryListNotExist();
    createTagsTableNotExist();
    // 使用 getListCount() 时内部会调用 getThreadDatabase()
    // 注意：如果这些操作在其他线程中调用，都会使用各自的独立连接
    // 否则也用主线程的连接
    // countLocallist = getListCount("locallist");
    // countHistorylist = getListCount("historylist");
}
DataBase::~DataBase(){
}

// 如果需要重新统计，可以调用该接口
bool DataBase::reSetListCount(){
    // 示例中略去成员变量 countLocallist/countHistorylist
    // 具体实现根据需求自行处理
    return true;
}

bool DataBase::createConnection(const QString dataBase_Name)
{
    QMutexLocker locker(&dbMutex); // 加锁
    // 使用用户主目录路径替代 '~'
    QString buildPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(buildPath); // 确保目录存在

    // 仅在主线程中使用一个固定的连接名
    db = QSqlDatabase::addDatabase("QSQLITE", "conToMetaData");
    db.setDatabaseName(buildPath + "/" + dataBase_Name + ".db");

    // 打开数据库连接
    if (!db.open()) {
        qDebug() << "Error: Failed to connect database."<<buildPath << db.lastError();
        return false;
    }

    QSqlQuery pragmaQuery(db);

    // 1. 切换到 WAL 模式（写前日志）
    if (pragmaQuery.exec("PRAGMA journal_mode=WAL")) {
        qDebug() << "WAL mode enabled";
    } else {
        qDebug() << "Failed to set WAL mode:" << pragmaQuery.lastError().text();
    }

    // 2. 可选：设置同步级别为 NORMAL，减少磁盘 I/O 提升性能
    if (!pragmaQuery.exec("PRAGMA synchronous=NORMAL")) {
        qDebug() << "Failed to set synchronous mode:" << pragmaQuery.lastError().text();
    }

    return true;
}


int DataBase::getListCount(const QString &playListName) {
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QString sqlStatement = QString("SELECT COUNT(*) FROM %1;").arg(playListName);
    localDb.commit();
    if (!sql_query.exec(sqlStatement)) {
        qDebug() << "获取" << playListName << "总数失败：" << sql_query.lastError();
        return -1;
    }

    if (sql_query.next()) {
        return sql_query.value(0).toInt();
    }
    return 0;
}

bool DataBase::createFilePathTable()
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QString sqlStatement = "CREATE TABLE IF NOT EXISTS filepath_table ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                           "dirPath TEXT NOT NULL UNIQUE);";
    if (!sql_query.exec(sqlStatement)) {
        qDebug() << "创建文件路径表失败：" << sql_query.lastError();
        return false;
    }
    qDebug() << "成功创建 filepath_table";
    return true;
}

bool DataBase::createPlayListNotExist()
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QString sqlStatement;

    QString playListName = "locallist";
    sql_query.prepare("SELECT count(*) FROM sqlite_master WHERE type='table' AND name=:tableName;");
    sql_query.bindValue(":tableName", playListName);
    if (!sql_query.exec()) {
        qDebug() << "查找歌单失败：" << playListName << sql_query.lastError();
        return false;
    }
    if (sql_query.next()) {
        if (sql_query.value(0).toInt() == 0) {
            sqlStatement = QString("CREATE TABLE %1 (id INTEGER PRIMARY KEY, dirpath_id INTEGER NOT NULL, filePath TEXT NOT NULL, title TEXT, "
                                    "artist TEXT, album TEXT, duration TEXT, poster BLOB, FOREIGN KEY(dirpath_id) REFERENCES filePath_table(id));").arg(playListName);
            if (!sql_query.exec(sqlStatement)) {
                qDebug() << "歌单创建失败：" << playListName << sql_query.lastError();
                return false;
            }
        } else {
            qDebug() << "locallist 已经存在";
        }
    }
    qDebug() << "成功创建 locallist";
    return true;
}

bool DataBase::createHistoryListNotExist()
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QString sqlStatement;
    QString playListName = "historylist";
    sql_query.prepare("SELECT count(*) FROM sqlite_master WHERE type='table' AND name=:tableName;");
    sql_query.bindValue(":tableName", playListName);
    if (!sql_query.exec()) {
        qDebug() << "查找歌单失败：" << playListName << sql_query.lastError();
        return false;
    }
    if (sql_query.next()) {
        if (sql_query.value(0).toInt() == 0) {
            sqlStatement = QString("CREATE TABLE %1 (id INTEGER PRIMARY KEY, dirpath_id INTEGER NOT NULL, filePath TEXT NOT NULL, "
                                   "title TEXT, duration TEXT, FOREIGN KEY(dirpath_id) REFERENCES filepath_table(id));").arg(playListName);
            if (!sql_query.exec(sqlStatement)) {
                qDebug() << "歌单创建失败：" << playListName << sql_query.lastError();
                return false;
            }
        }
    }
    return true;
}

bool DataBase::saveMetaData(const QMap<QString, QString> &metaDataMap, const QPixmap &img, bool status)
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QString dirPath, filePath, title, artist, album, duration;
    QByteArray poster;
    QBuffer buffer(&poster);
    buffer.open(QIODevice::WriteOnly);
    img.save(&buffer, "PNG");

    for (auto it = metaDataMap.constBegin(); it != metaDataMap.constEnd(); ++it) {
        if      (it.key() == "dirPath")  { dirPath = it.value(); }
        else if (it.key() == "filePath") { filePath = it.value(); }
        else if (it.key() == "title")    { title = it.value(); }
        else if (it.key() == "artist")   { artist = it.value(); }
        else if (it.key() == "album")    { album = it.value(); }
        else if (it.key() == "duration") { duration = it.value(); }
    }
    if (dirPath.isNull() || filePath.isNull()) {
        return false;
    }

    if (!addFilePath(dirPath)) {
        return false;
    }

    sql_query.prepare("SELECT id FROM filepath_table WHERE dirPath = :dirPath;");
    sql_query.bindValue(":dirPath", dirPath);
    if (!sql_query.exec() || !sql_query.next()) {
        qDebug() << "获取文件夹路径 ID 失败：" << sql_query.lastError();
        return false;
    }
    int dirpathId = sql_query.value(0).toInt();

    QString sqlStatementDelete = "DELETE FROM locallist WHERE filePath = :filePath";
    sql_query.prepare(sqlStatementDelete);
    sql_query.bindValue(":filePath", filePath);
    if (!sql_query.exec()) {
        qDebug() << "无法清除相同源的歌曲元数据：" << filePath << sql_query.lastError();
        return false;
    }

    QString sqlStatementInsert = "INSERT INTO locallist (dirpath_id, filePath, title, artist, album, duration, poster) "
                                 "VALUES (:dirpath_id, :filePath, :title, :artist, :album, :duration, :poster);";
    sql_query.prepare(sqlStatementInsert);
    sql_query.bindValue(":dirpath_id", dirpathId);
    sql_query.bindValue(":filePath", filePath);
    sql_query.bindValue(":title", title.isEmpty() ? "未知歌名" : title);
    sql_query.bindValue(":artist", artist.isEmpty() ? "未知艺术家" : artist);
    sql_query.bindValue(":album", album.isEmpty() ? "未知专辑" : album);
    sql_query.bindValue(":duration", duration);
    sql_query.bindValue(":poster", poster);

    if (!sql_query.exec()) {
        qDebug() << "插入歌曲元数据失败：locallist" << sql_query.lastError();
        return false;
    }
    return true;
}

// 其它函数同样修改为使用 getThreadDatabase()，例如 saveHistoryData()、clearTable()、getUrlFromPlayList()、deleteByUrl()、queryByUrl() 等

bool DataBase::saveTagsFromJson(const QString &url, const QJsonArray &tagsArray)
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QJsonDocument doc(tagsArray);
    QString tagsJson = doc.toJson(QJsonDocument::Compact);
    sql_query.prepare("INSERT OR REPLACE INTO tags_table (filePath, tags) VALUES (:filePath, :tags);");
    sql_query.bindValue(":filePath", url);
    sql_query.bindValue(":tags", tagsJson);

    if (!sql_query.exec()) {
        qDebug() << "保存标签失败：" << sql_query.lastError();
        return false;
    }
    return true;
}

QJsonArray DataBase::getTagsArrayByUrl(const QString &filePath)
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    sql_query.prepare("SELECT tags FROM tags_table WHERE filePath = :filePath;");
    sql_query.bindValue(":filePath", filePath);
    if (!sql_query.exec() || !sql_query.next()) {
        return QJsonArray();
    }
    QJsonDocument doc = QJsonDocument::fromJson(sql_query.value(0).toString().toUtf8());
    return doc.array();
}

QJsonArray DataBase::toApi(const QString &filePath)
{

    qDebug()<<"trace5.5";
    if (checkUrlExistsInTags(filePath)) {

    qDebug()<<"read success "<<filePath;

        return getTagsArrayByUrl(filePath);
    }
    qDebug()<<"send request for "<<filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Unable to open file";
        return QJsonArray();
    }
    QFileInfo fileInfo(file.fileName());
    QString fileName = fileInfo.fileName();

    QHttpPart audioPart;
    audioPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                        QString("form-data; name=\"file\"; filename=\"%1\"").arg(fileName));
    audioPart.setHeader(QNetworkRequest::ContentTypeHeader, "audio/mp3");

    QByteArray fileData = file.readAll();
    file.close();
    audioPart.setBody(fileData);

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->append(audioPart);

    QUrl url("http://8.140.242.219:6001/api");
    QNetworkRequest request(url);
    loop = new QEventLoop();
    reply = manager.post(request, multiPart);
    QObject::connect(reply, &QNetworkReply::finished, loop, &QEventLoop::quit);

      loop->exec();

    if (reply->error() == QNetworkReply::NoError) {
        QByteArray response = reply->readAll();
        QJsonDocument responseDoc = QJsonDocument::fromJson(response);
        QJsonObject responseObject = responseDoc.object();
        if (responseObject.contains("tags") && responseObject["tags"].isArray()) {
            QJsonArray tagsArray = responseObject["tags"].toArray();
            saveTagsFromJson(filePath, tagsArray);

         qDebug()<<filePath<<"succeeful to Api";
            return tagsArray;
        }
    } else {
        qWarning() << "Ai api Request failed:" << reply->errorString();
    }
    reply->deleteLater();
        delete loop;
        reply = nullptr;
        loop = nullptr;

         qDebug()<<filePath<<"succeeful2 to Api";
    return QJsonArray();
}

QList<QPair<QString, qreal>> DataBase::parseTagsToOrderedList(const QJsonArray &tagsArray)
{
    QList<QPair<QString, qreal>> orderedList;
    for (const QJsonValue &tagValue : tagsArray) {
        if (!tagValue.isObject()) {
            qWarning() << "Invalid tag element type";
            continue;
        }
        QJsonObject tagObj = tagValue.toObject();
        if (!tagObj.contains("tag") || !tagObj.contains("avg_confidence")) {
            qWarning() << "Missing required tag fields";
            continue;
        }
        QString tagName = tagObj["tag"].toString();
        qreal confidence = tagObj["avg_confidence"].toDouble(0.0);
        orderedList.append(qMakePair(tagName, confidence));
    }
       return orderedList;
}

bool DataBase::checkUrlExistsInTags(const QString &url)
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    sql_query.prepare("SELECT COUNT(filePath) FROM tags_table WHERE filePath = :url;");
    sql_query.bindValue(":url", url);
    if (!sql_query.exec()) {
        qDebug() << "检查URL存在性失败：" << sql_query.lastError();
        return false;
    }
    if (sql_query.next()) {
        int count = sql_query.value(0).toInt();
        return (count > 0);
    }
    return false;
}

bool DataBase::rewriteTagsWithList(const QString &url, const QList<QPair<QString, qreal>> &tagsList)
{
    if (url.isEmpty() || tagsList.isEmpty()) {
        qDebug() << "无效参数：URL 或标签数据为空";
        return false;
    }

    QJsonArray tagsArray;
    for (const auto &tagPair : tagsList) {
        if (tagPair.second < 0 || tagPair.second > 1) {
            qDebug() << "无效置信度值：" << tagPair.first << tagPair.second;
            return false;
        }
        QJsonObject tagObj;
        tagObj["tag"] = tagPair.first;
        tagObj["avg_confidence"] = static_cast<double>(tagPair.second) ;
        tagsArray.append(tagObj);
    }

    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery query(localDb);
    const QString sql = "INSERT OR REPLACE INTO tags_table (filePath, tags) VALUES (:url, :tags)";
    query.prepare(sql);
    QJsonDocument doc(tagsArray);
    query.bindValue(":url", url);
    query.bindValue(":tags", doc.toJson(QJsonDocument::Compact));

    if (!query.exec()) {
        qDebug() << "标签重写失败:" << query.lastError().text();
        return false;
    }
      return true;
}

QSqlDatabase DataBase::getThreadDatabase()
{
    // 如果当前线程已有连接，则直接返回
    if (threadDatabase.hasLocalData()) {
        return threadDatabase.localData();
    }
    // 生成唯一连接名
    QString connectionName = QString("conToMetaData_%1").arg((quintptr)QThread::currentThread());
    QSqlDatabase newDb = QSqlDatabase::addDatabase("QSQLITE", connectionName);
    QString buildPath =  QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    newDb.setDatabaseName(buildPath + "/metaData.db");
    if (!newDb.open()) {
        qDebug() << "Error: Failed to open database for thread"<<"in"<<buildPath << connectionName << newDb.lastError();
    }
    threadDatabase.setLocalData(newDb);
    return threadDatabase.localData();
}


bool DataBase::saveHistoryData(
                               const QList<HistoryMData> &history)
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase db = getThreadDatabase();
    QSqlQuery query(db);


    // 1) 确保表存在
    if (!createHistoryListNotExist()) {
        qDebug() << "确保 historylist 表存在失败";
        return false;
    }

    // 2) 清空指定目录的历史记录
    if (!query.exec("DELETE FROM historylist")) {
          qDebug() << "清空历史记录失败：" << query.lastError();
          return false;
      }
    // 4) 插入新的历史记录
    for (const HistoryMData &item : history) {
        QFileInfo fileInfo(item.url);
        QString dirPath = fileInfo.absolutePath();
    QSqlQuery q2(db);

    q2.prepare("SELECT id FROM filepath_table WHERE dirPath = :dirPath");
    q2.bindValue(":dirPath", dirPath);
    if (!q2.exec() || !q2.next()) {
        qDebug() << "获取 dirpath_id 失败：" << q2.lastError();
        return false;
    }
    int dirpath_id = q2.value(0).toInt();

        query.prepare(R"(
            INSERT INTO historylist
                (dirpath_id, filePath, title, duration)
            VALUES
                (:dirpath_id, :filePath, :title, :duration)
        )");
        query.bindValue(":dirpath_id", dirpath_id);
        query.bindValue(":filePath",    item.url);
        query.bindValue(":title",       item.title);
        // 如果 duration 本身是 int，也可以直接 bindValue(item.duration)
        query.bindValue(":duration",    QString::number(item.duration));

        if (!query.exec()) {
            qDebug() << "插入历史记录失败：" << query.lastError();
            return false;
        }
    }

    return true;
}
void DataBase::clearTable(const QString &playListName) {
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QString sqlStatement = QString("DELETE FROM %1").arg(playListName);

    if (!sql_query.exec(sqlStatement)) {
        qDebug() << "清空表失败：" << playListName << sql_query.lastError();
    }
}
 QStringList DataBase::getUrlFromPlayList(const QString &playListName)
 {
    QMutexLocker locker(&dbMutex); // 添加锁

     QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
     QString sqlStatement;

     QStringList urlList;

     sqlStatement = QString("SELECT filepath_table.dirPath || '/' || %1.filePath AS fullpath FROM %1 "
                            "INNER JOIN filepath_table ON %1.dirpath_id = filepath_table.id;").arg(playListName);
     if (!sql_query.exec(sqlStatement)) {
         qDebug() << "无法获取到歌单中所有歌曲的路径：" << playListName << sql_query.lastError();
         return urlList;
     }

     while(sql_query.next()) {
         QString fullpath = sql_query.value(0).toString();
         if (!fullpath.isNull()) {
             urlList.append(fullpath);
         }
     }
     return urlList;
 }


bool DataBase::deleteByUrl(const QStringList &url, const QString &playListName) {
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QString sqlStatement;

    for(const QString &itUrl : url) {
        sqlStatement = QString("delete from %1 where filePath = :url").arg(playListName);
        sql_query.prepare(sqlStatement);
        sql_query.bindValue(":url", itUrl);
        if (!sql_query.exec()) {
            qDebug() << "删除歌曲元数据失败：" << playListName << sql_query.lastError();
            return false;
        }
    }
    return true;
}

bool DataBase::queryByUrl(const QString &url, const QString &playListName, QMap<QString,QString> &map) {
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QString sqlStatement = QString("SELECT * FROM %1 WHERE filePath = :url;").arg(playListName);

    sql_query.prepare(sqlStatement);
    sql_query.bindValue(":url", url);

    if (!sql_query.exec()) {
        qDebug() << "查询URL失败：" << url << sql_query.lastError();
        return false;
    }

    if (sql_query.next()) {
        QSqlRecord record = sql_query.record();
        for (int i = 0; i < record.count(); i++) {
            QString fieldName = record.fieldName(i);
            QVariant value = sql_query.value(i);

            // 只处理字符串类型且key在map中存在的字段
            if (value.type() == QVariant::String && map.contains(fieldName)) {
                map[fieldName] = value.toString();
            } else if (value.type() != QVariant::String && value.type() != QVariant::ByteArray) {
                qDebug() << "字段" << fieldName << "不是字符串类型";
            }
        }
        return true;
    }

    return false;
}

bool DataBase::addFilePath(const QString &dirPath)
{
    QSqlDatabase localDb = getThreadDatabase();

    QSqlQuery sql_query(localDb);
    sql_query.prepare("INSERT OR IGNORE INTO filepath_table (dirPath) VALUES (:dirPath);");
    sql_query.bindValue(":dirPath", dirPath);
    if (!sql_query.exec()) {
        qDebug() << "插入文件夹路径失败：" << sql_query.lastError();
        return false;
    }
    return true;
}

bool DataBase::deleteByDirPath(const QString &dirPath, const QString &playListName)
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);

    // 获取 dirPath 对应的 id
    sql_query.prepare("SELECT id FROM filepath_table WHERE dirPath = :dirPath;");
    sql_query.bindValue(":dirPath", dirPath);
    if (!sql_query.exec() || !sql_query.next()) {
        qDebug() << "获取文件夹路径 ID 失败：" << sql_query.lastError();
        return false;
    }
    int dirpathId = sql_query.value(0).toInt();

    // 删除与 dirpath_id 相关的所有记录
    QString sqlStatement = QString("DELETE FROM %1 WHERE dirpath_id = :dirpath_id;").arg(playListName);
    sql_query.prepare(sqlStatement);
    sql_query.bindValue(":dirpath_id", dirpathId);
    if (!sql_query.exec()) {
        qDebug() << "删除与 dirPath 相关的记录失败：" << playListName << sql_query.lastError();
        return false;
    }

    qDebug()<<"delete ok";
    return true;
}
/// 从第hint条记录开始获取到向下offset条记录

QList<MetaData> DataBase::getDataFromLocallistwithHint(int hint, int offset) {
        QList<MetaData> resultList;
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);

    QString sqlStatement = QString(
        "SELECT filePath, title, artist, album, duration, poster "  // 增加 poster 字段
        "FROM locallist "
        "LIMIT :offset OFFSET :hint;"
    );

    sql_query.prepare(sqlStatement);
    sql_query.bindValue(":hint", hint);
    sql_query.bindValue(":offset", offset);

    // 执行查询
    if (!sql_query.exec()) {
        qDebug() << "从 locallist 获取数据失败：" << sql_query.lastError();
        return resultList;
    }

    // 解析查询结果
    if(!localDb.isValid()){
        qDebug()<<"unvia";
    }
    while (sql_query.next()) {
        QString filePath = sql_query.value("filePath").toString();
        QString title = sql_query.value("title").toString();
        QString artist = sql_query.value("artist").toString();
        QString album = sql_query.value("album").toString();
        int duration = sql_query.value("duration").toInt();

        // 读取 poster 的 BLOB 数据并转换为 QPixmap
        QByteArray posterData = sql_query.value("poster").toByteArray();
        QPixmap poster;
        if (!posterData.isEmpty()) {
            if (!poster.loadFromData(posterData)) {
                qDebug() << "加载 poster 失败";
            }
        }

        // 创建 MetaData 对象并添加到结果列表
        MetaData metaData(filePath, title, artist, album, duration, poster);
        resultList.append(metaData);
    }

    return resultList;
}
bool DataBase::createTagsTableNotExist() {
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    QSqlQuery sql_query(localDb);
    QString sqlStatement = "CREATE TABLE IF NOT EXISTS tags_table ("
                          "filePath TEXT PRIMARY KEY, "
                          "tags TEXT);"; // tags字段存储JSON格式的标签数据
    if (!sql_query.exec(sqlStatement)) {
        qDebug() << "创建标签表失败：" << sql_query.lastError();
        return false;
    }
    return true;
}

MetaData DataBase::getMetaDataByUrl(const QString &url)
{
    QMutexLocker locker(&dbMutex); // 添加锁

    QSqlDatabase localDb = getThreadDatabase();
    MetaData metaData;
    QSqlQuery sql_query(localDb);

    // SQL 查询语句，根据 URL 查找对应的元数据
    sql_query.prepare("SELECT title, artist, album, duration, poster FROM locallist WHERE filePath = :filePath;");
    sql_query.bindValue(":filePath", url);

    if (!sql_query.exec()) {
        qDebug() << "查询歌曲元数据失败：" << sql_query.lastError();
        return metaData;  // 返回一个默认构造的 MetaData 对象
    }

    if (sql_query.next()) {
        // 从查询结果中提取数据
        metaData.url = url;
        metaData.title = sql_query.value("title").toString();
        metaData.artist = sql_query.value("artist").toString();
        metaData.album = sql_query.value("album").toString();
        metaData.duration = sql_query.value("duration").toInt();

        // 获取封面图片数据并转换为 QPixmap
        QByteArray poster = sql_query.value("poster").toByteArray();
        QPixmap pixmap;
        pixmap.loadFromData(poster);
        metaData.covpix = pixmap;  // 设置封面图
    }

    return metaData;
}

//// 在每个线程真正结束前，调用下面这个清理函数
//void DataBase::cleanupThreadDatabase() {
//    if (threadDatabase.hasLocalData()) {
//        QSqlDatabase db = threadDatabase.localData();
//        QString connName = db.connectionName();

//        db.close();
//        // 从全局连接池中移除
//        QSqlDatabase::removeDatabase(connName);
//    }
//}
void DataBase::cleanupThreadDatabase() {
    if (threadDatabase.hasLocalData()) {
        QSqlDatabase db = threadDatabase.localData();

    QMutexLocker locker(&dbMutex); // 添加锁
        QString connName = db.connectionName();

        if (db.isOpen()) {
            db.close();  // 关闭连接
            // 确保连接已经完全关闭
            while (db.isOpen()) {
                QThread::msleep(10);  // 等待连接关闭
            }
        }

        QSqlDatabase::removeDatabase(connName);  // 移除数据库连接
    }
}

void DataBase::abortRequest(){
        if (reply &&reply->isRunning()) {
           reply->abort();
       }
       if (loop && loop->isRunning()) {
           loop->quit();
       }

}
