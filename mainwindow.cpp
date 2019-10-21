#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("VilleMorteExport");
    this->setWindowIcon(QIcon(":/icon/favicon.png"));

    QObject::connect(ui->pushButton, &QPushButton::clicked, this, &MainWindow::ExtractMonthInfos);

    QDate nextMonth(QDate::currentDate().year(), QDate::currentDate().month() != 12 ? QDate::currentDate().month() + 1 : 1, QDate::currentDate().day());
    ui->dateEdit->setDate(nextMonth);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ExtractMonthInfos()
{
    QString monthString = ui->dateEdit->sectionText(QDateTimeEdit::MonthSection);
    QString yearString = ui->dateEdit->sectionText(QDateTimeEdit::YearSection);
    QStringList eventsURLs;

    for(int day = 1; day <= 31; day++)
    {
        eventsURLs.clear();
        QString dayString = QString::number(day).rightJustified(2,'0');
        QString dateString = yearString + '-' + monthString + '-' + dayString;
        GetEventsURLs(dateString, eventsURLs);

        for(int eventURLIndex = 0 ; eventURLIndex < eventsURLs.size() ; eventURLIndex++)
        {
            QString currentEventInfos = dayString + '/' + monthString + ' ' + GetEventInfos(eventsURLs.at(eventURLIndex));
            ui->textEdit->append(currentEventInfos);
        }
    }
}

void MainWindow::GetEventsURLs(QString date, QStringList& eventURLs)
{
    QString dateUrl = "http://villemorte.fr/agenda/" + date + "/";    

    QNetworkAccessManager manager;
    QNetworkReply *response = manager.get(QNetworkRequest(QUrl(dateUrl)));
    QEventLoop event;
    connect(response,SIGNAL(finished()),&event,SLOT(quit()));
    event.exec();
    QString html = response->readAll();

    QString tribeEventsBegin = "<div class=\"tribe-events-list-event-description tribe-events-content description entry-summary\"";
    QString tribeEventsEnd = "</div>";
    int tribeEventBeginIndex = html.indexOf(tribeEventsBegin);
    int tribeEventEndIndex = html.indexOf(tribeEventsEnd, tribeEventBeginIndex);

    //on parcourt le html pour récuperer les liens vers les evenements de la date
    while(tribeEventBeginIndex != -1)
    {
        QString htmlTableContent = html.mid(tribeEventBeginIndex, tribeEventEndIndex - tribeEventBeginIndex + tribeEventsEnd.length());
        QDomDocument dom;
        QString errorMsg;
        int errorLine;
        int errorColumn;
        bool isOk = dom.setContent(htmlTableContent, &errorMsg, &errorLine, &errorColumn);
        if(!isOk)
            qDebug() << "failed to setContent: " << errorMsg << " " << errorLine << " " << errorColumn;

        //lien vers les details
        QDomNodeList aNodes = dom.elementsByTagName("a");
        for(int i=0;i<aNodes.count();i++)
        {
            QDomNode currentNode = aNodes.at(i);
            QDomNamedNodeMap map = currentNode.attributes();
            if(map.namedItem("class").nodeValue() == "tribe-events-read-more")
            {
                eventURLs.append(map.namedItem("href").nodeValue());
                break;
            }
        }

        tribeEventBeginIndex = html.indexOf(tribeEventsBegin, tribeEventEndIndex);
        tribeEventEndIndex = html.indexOf(tribeEventsEnd, tribeEventBeginIndex);
    }

}

QString questionify(QString info){
    if(info.isEmpty())
        return "<b>?</b>";
    else {
        return info;
    }
}

QString MainWindow::GetEventInfos(QString eventUrl)
{        
    QString eventTitle;
    QString eventLocation;
    QString eventPrice;

    QNetworkAccessManager manager;
    QNetworkReply *response = manager.get(QNetworkRequest(QUrl(eventUrl)));
    QEventLoop event;
    connect(response,SIGNAL(finished()),&event,SLOT(quit()));
    event.exec();
    QString html = QString::fromUtf8(response->readAll()); // Source should be stored here

    //on récupère une sous-partie du html pour réduire la taille du QDomDocument
    QString tableBegin = "<main ";
    QString tableEnd = "</main>";
    int tableBeginIndex = html.indexOf(tableBegin);
    int tableEndIndex = html.indexOf(tableEnd, tableBeginIndex);
    QString htmlTableContent = html.mid(tableBeginIndex, tableEndIndex - tableBeginIndex + tableEnd.size());
    htmlTableContent = htmlTableContent.replace("&rsquo;","'");

    QDomDocument tableDom;
    tableDom.setContent(htmlTableContent);

    //titre
    QDomNodeList h1Nodes = tableDom.elementsByTagName("h1");
    for(int i=0;i<h1Nodes.count();i++)
    {
        QDomNode currentNode = h1Nodes.at(i);
        QDomNamedNodeMap map = currentNode.attributes();
        if(map.namedItem("class").nodeValue() == "tribe-events-single-event-title")
        {
            eventTitle = currentNode.firstChild().nodeValue();
            break;
        }
    }

    //lieu et prix
    QDomNodeList ddNodes = tableDom.elementsByTagName("dd");
    for(int i=0;i<ddNodes.count();i++)
    {
        QDomNode currentNode = ddNodes.at(i);
        QDomNamedNodeMap map = currentNode.attributes();
        if(map.namedItem("class").nodeValue() == "author fn org")
        {
            eventLocation = currentNode.firstChild().nodeValue();
        }

        if(map.namedItem("class").nodeValue() == "tribe-events-event-cost")
        {
            eventPrice = currentNode.firstChild().nodeValue();
        }

    }

    eventTitle = eventTitle.trimmed();
    eventLocation = eventLocation.trimmed();
    eventPrice = eventPrice.trimmed();

    return eventTitle + QString(" @ ")+ questionify(eventLocation) + " - " + questionify(eventPrice);
}
