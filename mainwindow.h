#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QDomDocument>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void GetEventsURLs(QString date, QStringList& eventURLs);   //yyyy-mm-dd
    QString GetEventInfos(QString eventUrl);
    void ExtractMonthInfos();

private:
    Ui::MainWindow *ui;
    QNetworkAccessManager _manager;
    QString _year;
    QString _month;      // "2019-10/"
    QStringList _eventsURL;

};

#endif // MAINWINDOW_H
