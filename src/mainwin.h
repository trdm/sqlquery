#include "ui_sqlquerywin.h"
#include "textedit.h"
class QCompleter;
class TextEdit;
class QAbstractItemModel;


class MainWin : public QMainWindow, public Ui_SqlQueryMainWin {
    Q_OBJECT
public:
    MainWin();
	void openFile(QString psFile);
	bool loadCSVTextInMemory(QString psFile, QString psFName);
	bool loadTextInMemory(QString psFile, QString psFName);
	bool loadCsvdfl(QString psFile, int psAsCSV = 1);
	bool createDBMemory(QString psFile);
	bool query_exec(QString psQText, bool psTahQ = false);


signals:
    void connectChanged(bool state);
private slots:
    void on_actExit_triggered();
    void on_actConnect_triggered();
    void on_actDisconnect_triggered();
    void on_actNew_triggered();
    void getTables();
    void showStatus(bool state);
    void on_teQuery_textChanged();
    void on_actRun_triggered();
    void showResult();
	void showHelp();
    void showTable(const QString& name);
    void on_listTables_doubleClicked(const QModelIndex& index);
	bool addTextFile();
protected:
    virtual void closeEvent(QCloseEvent *event);
    QAbstractItemModel *modelFromFile(const QString& fileName);
	QStringList listFromFile(const QString& fileName);
    QSqlDatabase db;
    QSqlQuery *query;
    bool connected;
    QStringList tables;
	QStringListModel *listmodel;
	QStringList m_addWordCompleter;
	void updateCompleter();

	QString dbmode;
    QString host;
    int port;
	QString dbname;
	QString dbfilename;
	QString username;
    QString password;
	MyCompleter *completer;

};
