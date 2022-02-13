#include "ui_connectdlg.h"

class ConnectDlg : public QDialog, public Ui_ConnectDlg {
    Q_OBJECT
public:
    ConnectDlg(const QString& mode = "",
               const QString& host = "",
               int port = 0,
               const QString& dbname = "",
               const QString& username = "" );
private slots:
	void selectFile();
	void saveFile();
	void myAccept();
	void myChoisDriver();

};
