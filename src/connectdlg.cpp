#include <QtSql>
#include <QTextStream>
#include <QFile>
#include <QMenu>
#include <QString>
#include <QFileDialog>



#include "connectdlg.h"

ConnectDlg::ConnectDlg(
               const QString& mode,
               const QString& host,
               int port,
               const QString& dbname,
               const QString& username ) 
          : QDialog() {
    setupUi(this);
    QStringList drivers = QSqlDatabase::drivers();
    drivers.removeAll("QMYSQL3");
    drivers.removeAll("QOCI8");
    drivers.removeAll("QODBC3");
    drivers.removeAll("QPSQL7");
    drivers.removeAll("QTDS7");
    cbMode->addItems(drivers);
    int i = cbMode->findText(mode, Qt::MatchFixedString);
    if ( i > -1 )
        cbMode->setCurrentIndex(i);
    
    if (!host.isEmpty()){
        chbHost->setCheckState(Qt::Checked);
        leHost->setText(host);
    }else{
        chbHost->setCheckState(Qt::Unchecked);
        leHost->clear();
    }
    if (port > 0){
        chbPort->setCheckState(Qt::Checked);
        sbPort->setValue(port);
    }else{
        chbPort->setCheckState(Qt::Unchecked);
        sbPort->setValue(0);
    }
    leDbname->setText(dbname);
    leUsername->setText(username);
    lePassword->clear();
	connect(bnSelectFile, SIGNAL(clicked()), this, SLOT(selectFile()));
	connect(buttonBox, SIGNAL(accepted()), this, SLOT(myAccept()));
	connect(cbMode, SIGNAL(currentIndexChanged(int)), this, SLOT(myChoisDriver()));
	myChoisDriver();

}

void ConnectDlg::myAccept()
{
	saveFile();
	QDialog::accept();
}
void ConnectDlg::selectFile()
{
	QString fileNameP = qApp->applicationDirPath() + QDir::separator() + "histori.txt";
	QFile file(fileNameP);
	bool sele = true;
	if (file.open(QFile::ReadOnly|QIODevice::Text)) {
		QMenu* menu = new QMenu();
		QTextStream stream(&file);
		QString line;
		QStringList sl;

		do {
			line = stream.readLine();
			if (!line.isEmpty()) {
				if (!sl.contains(line));
					menu->addAction(line);
			}
		} while (!line.isNull());
		file.close();
		QAction* open = menu->addAction("Open file");
		QAction* act = menu->exec(bnSelectFile->mapToGlobal(QPoint(0,0)));
		if (act) {
			if (open!=act) {
				leDbname->setText(act->text());
				sele = false;
			}
		}
	}
	if (sele) {
		QString fileName = QFileDialog::getOpenFileName(this);
		if (!fileName.isEmpty()){
			leDbname->setText(fileName);
			saveFile();
		}
	}

}
void ConnectDlg::myChoisDriver(){
	QString txtDriver  = cbMode->currentText();
	leUsername->setText("");
	lePassword->setText("");
	if (txtDriver == "QIBASE"){
		leUsername->setText("SYSDBA");
		lePassword->setText("masterkey");
	}

	//qDebug() << ":myChoisDriver(){";

}
void ConnectDlg::saveFile()
{
	QString fileNameP = qApp->applicationDirPath() + QDir::separator() + "histori.txt";
	QFile file(fileNameP);
	QString fileName = leDbname->text();
	bool exist = false;
	if (file.open(QFile::ReadOnly|QIODevice::Text)) {
		QTextStream stream(&file);
		QString line;
		do {
			line = stream.readLine();
			if (!line.isEmpty()) {
				if (fileName == line)
					exist = true;
			}
		} while (!line.isNull());
		file.close();
	}
	if (!exist){
		if (file.open(QFile::WriteOnly|QIODevice::Append|QIODevice::Text)) {
			QTextStream stream(&file);
			stream << fileName << "\n";
		}
	}
}
