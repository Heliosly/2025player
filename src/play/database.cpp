#include "database.h"
#include "metadata.h"
#include <QFileInfo>
#include<QBuffer>
#include<QDebug>
#include<QSqlRecord>
#include<QApplication>
DataBase* DataBase::s_instance = nullptr;
DataBase* DataBase::instance()
{
    if (s_instance == nullptr) {
        s_instance = new DataBase();  // 创建唯一的实例
    }
    return s_instance;
}
DataBase::DataBase()
{
    createConnection("metaData"); //建立数据库连接接口
    db = QSqlDatabase::database("conToMetaData");//通过接口连接数据库
    createFilePathTable();
    createPlayListNotExist();
    createHistoryListNotExist();
    countLocallist=getListCount("locallist");
    countHistorylist=getListCount("historylist");
    
}
bool DataBase::reSetListCount(){
 countLocallist=getListCount("locallist");
    countHistorylist=getListCount("historylist");
}
 bool DataBase::createConnection(QString dataBase_Name)
{

     QString buildPath = QCoreApplication::applicationDirPath() + "/build";
     QDir().mkpath(buildPath); // 确保目录存在


    db = QSqlDatabase::addDatabase("QSQLITE", "conToMetaData");
    db.setDatabaseName(buildPath + "/" + dataBase_Name + ".db");
    //判断是否连接成功
    if (!db.open()) { //未连接到数据库
        qDebug() << "Error: Failed to connect database." << db.lastError();
        return false;
    }
    else { //连接到数据库
        return true;
    }
}
int DataBase::getListCount(const QString &playListName) {
    QSqlQuery sql_query(db);
    QString sqlStatement = QString("SELECT COUNT(*) FROM %1;").arg(playListName);

    if (!sql_query.exec(sqlStatement)) {
        qDebug() << "获取" << playListName << "总数失败：" << sql_query.lastError();
        return -1; // 返回 -1 表示查询失败
    }

    if (sql_query.next()) {
        return sql_query.value(0).toInt();
    }

    return 0; // 如果没有记录，返回 0
}
bool DataBase::createFilePathTable()
{
    QSqlQuery sql_query(db);
    QString sqlStatement = "CREATE TABLE IF NOT EXISTS filepath_table ("
                           "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                           "dirpath TEXT NOT NULL UNIQUE);";
    if (!sql_query.exec(sqlStatement)) {
        qDebug() << "创建文件路径表失败：" << sql_query.lastError();
        return false;
    }
    qDebug()<<"成功创建filepath table";
    return true;
}
 bool DataBase::createPlayListNotExist()
 {
     QSqlQuery sql_query(db);
     QString sqlStatement;

     QString playListName = "locallist";
     //查询数据库中是否存在该歌单
     sql_query.prepare("select count(*) from sqlite_master where type='table' and name=:tableName;");
     sql_query.bindValue(":tableName",playListName);
     if (!sql_query.exec()) {
         qDebug() << "查找歌单失败：" << playListName << sql_query.lastError();
         return false;
     }

     if (sql_query.next()) {
         if (sql_query.value(0).toInt() == 0) {//如果不存在该歌单，则创建
             /*歌单表属性
              * 1.  id
              * 2.  dirpath_id (外键)
              * 3.  filepath
              * 4.  title
              * 5.  artist
              * 6.  album
              * 7.  duration
              * 8.  poster
              */
             sqlStatement = QString("create table %1 (id INTEGER PRIMARY KEY, dirpath_id INTEGER NOT NULL, filepath TEXT NOT NULL, title text,"
                                    "artist text, album text,"
                                    "duration text, poster BLOB, FOREIGN KEY(dirpath_id) REFERENCES filepath_table(id)"
                                    ");").arg(playListName);
             if (!sql_query.exec(sqlStatement)) {
                 qDebug() << "歌单创建失败：" << playListName << sql_query.lastError();
                 return false;
             }
         }else{
             qDebug()<<"locallist已经存在";
         }
     }
     else{
         qDebug()<<"?";
     }
 qDebug()<<"成功创建 locallist；wtable";
     return true;
 }
 bool DataBase::createHistoryListNotExist()
 {
     QSqlQuery sql_query(db);
     QString sqlStatement;

     QString playListName = "historylist";
     // 查询数据库中是否存在该歌单
     sql_query.prepare("select count(*) from sqlite_master where type='table' and name=:tableName;");
     sql_query.bindValue(":tableName", playListName);
     if (!sql_query.exec()) {
         qDebug() << "查找歌单失败：" << playListName << sql_query.lastError();
         return false;
     }

     if (sql_query.next()) {
         if (sql_query.value(0).toInt() == 0) { // 如果不存在该歌单，则创建
             /* 歌单表属性
              * 1.  id
              * 2.  dirpath_id (外键)
              * 3.  url
              * 4.  title
              * 5.  duration
              */
             sqlStatement = QString("CREATE TABLE %1 (id INTEGER PRIMARY KEY, dirpath_id INTEGER NOT NULL, url TEXT NOT NULL, "
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
     QSqlQuery sql_query(db);
     QString sqlStatement;

     QString dirpath;
     QString filepath;
     QString title;
     QString artist;
     QString album;

     QString duration;

     QByteArray poster;
     QBuffer buffer(&poster);
     buffer.open(QIODevice::WriteOnly);

     img.save(&buffer, "PNG");


     for (QMap<QString,QString>::const_iterator it = metaDataMap.constBegin(); it != metaDataMap.constEnd(); it++) {
         if      (it.key() == "dirpath")      { dirpath = it.value(); }
         else if (it.key() == "filepath")     { filepath = it.value(); }
         else if (it.key() == "title")       { title = it.value();}
         else if (it.key() == "artist")      { artist = it.value();}
         else if (it.key() == "album")       { album = it.value();}
         else if (it.key() == "duration")    { duration = it.value();}

     }
     if (dirpath.isNull() || filepath.isNull()) {
         return false;
     }

     // 插入 dirpath 到 filepath_table
     if (!addFilePath(dirpath)) {
         return false;
     }

     // 获取 dirpath 对应的 id
     sql_query.prepare("SELECT id FROM filepath_table WHERE dirpath = :dirpath;");
     sql_query.bindValue(":dirpath", dirpath);
     if (!sql_query.exec() || !sql_query.next()) {
         qDebug() << "获取文件夹路径 ID 失败：" << sql_query.lastError();
         return false;
     }
     int dirpathId = sql_query.value(0).toInt();

     // 删除歌单中具有相同 filepath 的歌曲元数据
     sqlStatement = QString("DELETE FROM locallist WHERE filepath = :filepath");
     sql_query.prepare(sqlStatement);
     sql_query.bindValue(":filepath", filepath);
     if (!sql_query.exec()) {
         qDebug() << "无法清除相同源的歌曲元数据：" << filepath << sql_query.lastError();
         return false;
     }

     sqlStatement = QString("insert into locallist (dirpath_id, filepath, title, artist, album, duration, poster)"
                            "values(:dirpath_id, :filepath, :title, :artist, :album, :duration, :poster);");
     sql_query.prepare(sqlStatement);
     sql_query.bindValue(":dirpath_id", dirpathId);
     sql_query.bindValue(":filepath", filepath);
     sql_query.bindValue(":title",title.isEmpty() ? "未知歌名" : title);
     sql_query.bindValue(":artist",artist.isEmpty() ? "未知艺术家" : artist);
     sql_query.bindValue(":album",album.isEmpty() ? "未知专辑" : album);
     sql_query.bindValue(":duration",duration);
     sql_query.bindValue(":poster",poster);


     if (!sql_query.exec()) {
         qDebug() << "插入歌曲元数据失败：locallist"  << sql_query.lastError();
         return false;
     }

     return true;
 }

bool DataBase::saveHistoryData(const QList<HistoryMData> &history) {
    QSqlQuery sql_query(db);


    // Clear existing history table
    if (!sql_query.exec("DELETE FROM historylist")) {
        qDebug() << "清空历史记录失败：" << sql_query.lastError();
        return false;
    }

    // Insert new history records
    for (const HistoryMData &item : history) {
        QString sqlStatement = "INSERT INTO historylist (url, title, duration) "
                             "VALUES (:url, :title, :duration)";
                             
        sql_query.prepare(sqlStatement);
        sql_query.bindValue(":url", item.url);
        sql_query.bindValue(":title", item.title);
        sql_query.bindValue(":duration", QString::number(item.duration));

        if (!sql_query.exec()) {
            qDebug() << "插入历史记录失败：" << sql_query.lastError();
            return false;
        }
    }

    return true;
}
void DataBase::clearTable(const QString &playListName) {
    QSqlQuery sql_query(db);
    QString sqlStatement = QString("DELETE FROM %1").arg(playListName);
    
    if (!sql_query.exec(sqlStatement)) {
        qDebug() << "清空表失败：" << playListName << sql_query.lastError();
    }
}
 QStringList DataBase::getUrlFromPlayList(const QString &playListName)
 {
     QSqlQuery sql_query(db);
     QString sqlStatement;

     QStringList urlList;

     sqlStatement = QString("SELECT filepath_table.dirpath || '/' || %1.filepath AS fullpath FROM %1 "
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
    QSqlQuery sql_query(db);
    QString sqlStatement;

    for(const QString &itUrl : url) {
        sqlStatement = QString("delete from %1 where url = :url").arg(playListName);
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
    QSqlQuery sql_query(db);
    QString sqlStatement = QString("SELECT * FROM %1 WHERE url = :url;").arg(playListName);
    
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

bool DataBase::addFilePath(const QString &dirpath)
{
    QSqlQuery sql_query(db);
    sql_query.prepare("INSERT OR IGNORE INTO filepath_table (dirpath) VALUES (:dirpath);");
    sql_query.bindValue(":dirpath", dirpath);
    if (!sql_query.exec()) {
        qDebug() << "插入文件夹路径失败：" << sql_query.lastError();
        return false;
    }
    return true;
}

bool DataBase::deleteByDirPath(const QString &dirpath, const QString &playListName)
{
    QSqlQuery sql_query(db);

    // 获取 dirpath 对应的 id
    sql_query.prepare("SELECT id FROM filepath_table WHERE dirpath = :dirpath;");
    sql_query.bindValue(":dirpath", dirpath);
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
        qDebug() << "删除与 dirpath 相关的记录失败：" << playListName << sql_query.lastError();
        return false;
    }

    qDebug()<<"delete ok";
    return true;
}
/// 从第hint条记录开始获取到向下offset条记录

QList<MetaData> DataBase::getDataFromLocallistwithHint(int hint, int offset) {
    QList<MetaData> resultList;
    QSqlQuery sql_query(db);

    // 构建 SQL 查询语句
    QString sqlStatement = QString(
        "SELECT filepath, title, artist, album, duration "
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
    while (sql_query.next()) {
        QString filepath = sql_query.value("filepath").toString();
        QString title = sql_query.value("title").toString();
        QString artist = sql_query.value("artist").toString();
        QString album = sql_query.value("album").toString();
        int duration = sql_query.value("duration").toInt();

        // 创建 MetaData 对象并添加到结果列表
        MetaData metaData(filepath, title, artist, album, duration, QPixmap());
        resultList.append(metaData);
    }

    return resultList;
}
