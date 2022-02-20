#include <QtGui>
#include <QSqlQuery>

#include "mainwin.h"
#include "connectdlg.h"
#include "sqlhighlighter.h"

MainWin::MainWin() : QMainWindow() {
    setupUi(this);
    SQLHighlighter *sqlhighlighter = new SQLHighlighter(teQuery->document());
    /*
    teQuery->setText(QString::fromUtf8(
        "create table employee \n"
         "id integer primary key, \n"
        "name varchar(50), \n"
        "department integer, \n"
        "salary number(14,2), \n"
        "married logical);\n"
        "\n"
        "create table department ( \n"
        "id integer primary key, \n"
        "name varchar(35));\n"
        "\n"
        "insert into department(id, name) values(1, 'Дирекция');\n"
        "insert into department(id, name) values(2, 'Бухгалтерия');\n"
        "insert into department(id, name) values(3, 'Отдел кадров');\n"
        "\n"
         "insert into employee(id, name, department, salary, married) \n"
               "values(1, 'Винни-Пух', 1, 1234.56, 1);\n"
        "\n"
        "insert into employee(id, name, department, salary, married) \n"
            "values(2, 'Пятачок', 3, 6543.21, 0);\n"
        "\n"
        "insert into employee(id, name, department, salary, married) \n"
        "    values(3, 'Ослик Иа', 2, 23.45, 0);\n"
        "\n"
        "select * from employee;\n"));
    */
    connected = false;
    dbname = "";
	dbfilename = "";
    host = "";
    port = 0;
    username = "";
    password = "";
    tables.clear();
    listmodel = new QStringListModel(tables);
    listmodel->setStringList(tables);
    listTables->setModel(listmodel);
    listTables->setEditTriggers(QAbstractItemView::NoEditTriggers);
	completer = 0;
	updateCompleter();

	connect(this, SIGNAL(connectChanged(bool)),   this, SLOT(showStatus(bool)));
	connect(actionAddCvs, SIGNAL(triggered(bool)),   this, SLOT(addTextFile()));
	connect(actionhelp, SIGNAL(triggered(bool)),   this, SLOT(showHelp()));
	emit connectChanged(false);
}

void MainWin::openFile(QString psFile)
{
	if (!QFile::exists(psFile)) {
		return;
	}
	// E:\Projects\__My\unNStudio\DB\SiteMaker\uoMeta.db
	// E:\Документы\Литература\_English\__СловарныйЗАпас\Add\__Statistic.txt
	QFileInfo vFInfo(psFile);
	QString vStrFF = "";
	QFile vVinFl(psFile);
	bool vIsSqlite = false;
	if (vVinFl.open( QIODevice::ReadOnly)) {
		if (vFInfo.size() > 50) {
			vStrFF = vVinFl.read(50);
			vIsSqlite = vStrFF.startsWith("SQLite format");
		}
	}
//	QString ext = vFInfo.completeSuffix();  // ext = "tar.gz"
	QString ext = vFInfo.suffix();  ext = ext.toLower();
	QString vNameF = vFInfo.baseName();
	bool isLoad;
	if (ext.endsWith("txt") ) {
		isLoad = loadTextInMemory(psFile, vNameF);
	} else if( ext.endsWith("csv")){
		isLoad = loadCSVTextInMemory(psFile, vNameF);
	} else if(ext.endsWith("csvdfl") || ext.endsWith("csvfl")){
		isLoad = loadCsvdfl(psFile,1);
	} else if(ext.endsWith("txtfl") || ext.endsWith("txtdfl")){
		isLoad = loadCsvdfl(psFile,0);
	} else if(ext.endsWith("db") || ext.endsWith("sqlite") || vIsSqlite){
		if (connected) {
			db.close();
			connected = false;
		}
		db = QSqlDatabase::addDatabase("QSQLITE");
		dbname = psFile;
		if (!dbname.isEmpty()){
			db.setDatabaseName(dbname);
			dbfilename = dbname;
		}
		connected = db.open();
		emit connectChanged(connected);
		if (!connected) {
			vStrFF = QString("Connect Error! (%1)").arg(psFile);
			QMessageBox::critical(this, vStrFF,  db.lastError().text(),    QMessageBox::Ok );
		}
	}
	setWindowTitle("Sql Query " + psFile);
}


bool MainWin::query_exec(QString psQText, bool psTahQ)
{
	if (psQText.isEmpty()) {
		return false;
	}
	query = new QSqlQuery(db);

	if ( query->exec(psQText) ){
		if (query->isSelect())
			showResult();
		if (!psTahQ) {
			teLog->append( tr("<p><b>Query succeeded:</b><br />")	+ psQText );
		}
		sb->showMessage(tr("Query executed successfully"), 3000);
	}else{
		sb->showMessage(tr("Query error"), 3000);
		QMessageBox::critical(
		this, tr("Query Error"),
		query->lastError().text(),
		QMessageBox::Ok );
		return false;
	}
	return true;
}

bool MainWin::createDBMemory(QString psFile)
{
	if (dbname != ":memory:") {

		db = QSqlDatabase::addDatabase("QSQLITE");
		dbname = ":memory:"; 		dbfilename = psFile;
		if (!dbname.isEmpty()){
			db.setDatabaseName(dbname);
		}
		connected = db.open();
		emit connectChanged(connected);
		if (!connected) {
			QString vStrFF = QString("Connect Error! (%1)").arg(psFile);
			QMessageBox::critical(this, vStrFF,  db.lastError().text(),    QMessageBox::Ok );
			return false;
		}
	}
	return true;
}



bool MainWin::loadCSVTextInMemory(QString psFile, QString psFName)
{
	dbfilename = psFile;
	if (!createDBMemory(psFile)) {
		return false;
	}

	QStringList vQueries;
	QStringList vCaptions;
	QStringList vFieldsData;
	QStringList vTempList;
	QFile vFile(psFile);
	if (!vFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	query = new QSqlQuery(db);


	QString vLineSep = ",", vLineTQ = "\"", vLineMArcSett = "--settings:",vText = "";
	QString vQuery;
	QString line, lineLC;
	while (!vFile.atEnd()) {
		line = vFile.readLine(); // read wavelength line and store it
		line = line.trimmed();
		if (line.isEmpty() || line.length() < 3) {// может быть \n или \r\n
			continue;
		}
		if (line.startsWith("--")) {
			lineLC = line;
			lineLC = lineLC.toLower();
			if (line.startsWith(vLineMArcSett)) {
				line = line.mid(vLineMArcSett.length());
				line = line.toLower();
				line = line.trimmed();
				vFieldsData = line.split("\t");
				foreach (vText, vFieldsData) {
					if (!vText.isEmpty()) {
						vTempList = vText.split("=");
						if (vTempList.size() == 2) {
							if (vTempList.at(0) == "separator") {
								vLineSep = vTempList.at(1);
								if (vLineSep.length() > 1) {
									vLineSep = vLineSep.mid(1,vLineSep.length() - 2);
								}
							} else if (vTempList.at(0) == "textquote") {
								vLineTQ = vTempList.at(1);
								if (vLineTQ.length() > 1) {
									vLineTQ = vLineTQ.mid(1,vLineTQ.length() - 2);
								}
							}
						}
					}
				}
				continue;
			} else {
				if (lineLC.indexOf("select") != -1) {
					vQueries.append(line);
				}
				//line = line.mid(2);
			}
			continue;
		} //line.startsWith("--")
		bool vQueIsCreate = false;
		vFieldsData = line.split(vLineSep);
		vQuery = "INSERT INTO " + psFName + " VALUES(";
		if (vCaptions.size() == 0) {
			vCaptions = line.split(vLineSep);
			vQuery = "CREATE TABLE " + psFName + " (";
			vQueIsCreate = true;
		}
		for (int var = 1; var <= vCaptions.size(); ++var) {
			vText = "";
			if (vQueIsCreate) {
				vText = vCaptions.at(var-1);
				vText = vText.trimmed();
				if (!m_addWordCompleter.contains(vText)) {
					m_addWordCompleter.append(vText);
				}
				if (vQueIsCreate) {
					vText.append(" TEXT");
				}
				if (var != vCaptions.size()) {
					vText.append(", ");
				}
			} else {
				if (vFieldsData.size() >= var) {
					vText = vFieldsData.at(var-1);
					vText = vText.trimmed();
				}
				vText.append("'").prepend("'");
				if (var != vCaptions.size()) {
					vText.append(", ");
				}
			}
			vQuery.append(vText);
		}
		vQuery.append(")");
		qDebug() << vQuery;

		if ( query_exec(vQuery,true) ){
			if (vQueIsCreate) {
				getTables();
			}
		}
	}
	for (int var = 0; var < vQueries.size(); ++var) {
		vQuery = vQueries.at(var);
		int vPos = vQuery.indexOf("SELECT");
		if (vPos != -1) {
			teQuery->append( vQuery );
		} else {
			vQuery.append("<br/>");
			teLog->append( vQuery );
		}
	}

	vFile.close();
	return true;
}

bool MainWin::loadTextInMemory(QString psFile, QString psFName)
{
	dbfilename = psFile;
	if (!createDBMemory(psFile)) {
		return false;
	}
	QFile vFile(psFile);
	if (!vFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	query = new QSqlQuery(db);
	QString vQuery;
	vQuery = "CREATE TABLE " + psFName + " (row_id INTEGER, row TEXT)";
	if ( query_exec(vQuery,true) ){
		getTables();
	}

	vQuery = "INSERT INTO " + psFName + "(row_id, row) VALUES(?,?)";
	QSqlQuery* query2 = new QSqlQuery(db);
	query2->prepare(vQuery);

	QString line, lineLC;
	qint64 vCounter = 0;
	while (!vFile.atEnd()) {
		line = vFile.readLine(); // read wavelength line and store it
		line = line.trimmed();
		if (line.isEmpty()) {// может быть \n или \r\n
			continue;
		}
		vCounter++;
		query2->bindValue(0,vCounter);
		query2->bindValue(1,line);
		query2->exec();

	}
	vFile.close();
	return true;
}
// trdm 2022-02-10 09:51:05
bool MainWin::loadCsvdfl(QString psFile, int psAsCSV )
{
	if (!QFile::exists(psFile)) {
		return false;
	}
	QFile vFile(psFile);
	if (!vFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QString line;
	QFileInfo vFileInfo(psFile);
	QString vFileName, vFilePath = vFileInfo.dir().path();
	vFilePath = vFilePath.replace("/","\\");
	vFilePath.append("\\");
	while (!vFile.atEnd()) {
		line = vFile.readLine(); // read wavelength line and store it
		line = line.trimmed();
		if (line.isEmpty() || line.length() < 3) {// может быть \n или \r\n
			continue;
		}
		if (line.startsWith("--") || line.isEmpty()) {
			continue;
		}
		vFileName = line;
		if (line.indexOf("/") == -1 && line.indexOf("\\") == -1) {
			vFileName = vFilePath + line;
		}
		if (QFile::exists(vFileName)) {
			QFileInfo vFileInfo2(vFileName);
			if (psAsCSV == 1) {
				loadCSVTextInMemory(vFileName,vFileInfo2.baseName());
			} else {
				loadTextInMemory(vFileName,vFileInfo2.baseName());
			}
		}
		//loadTextInMemory
	}
	vFile.close();
	updateCompleter();
	return true;
}


bool MainWin::addTextFile()
{
	QString fileName = QFileDialog::getOpenFileName(this);
	if (!fileName.isEmpty()){
//		leDbname->setText(fileName);
//		saveFile();
	}
	return true;
}

QStringList MainWin::listFromFile(const QString &fileName)
{
	QStringList words;
	QFile file(fileName);
	if (!file.open(QFile::ReadOnly))
		return words;

#ifndef QT_NO_CURSOR
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
#endif

	while (!file.atEnd()) {
		QByteArray line = file.readLine();
		if (!line.isEmpty())
			words << line.trimmed();
	}
	words << QString::fromUtf8("data_time");
	words << QString::fromUtf8("comments");
	words.sort();
	QString vStr = "";
	if (m_addWordCompleter.size() != 0) {
		for (int var = 0; var < m_addWordCompleter.size(); ++var) {
			vStr = m_addWordCompleter.at(var).trimmed();
			if (!words.contains(vStr)) {
				words << vStr;
			}
		}
	}
	if (tables.size() != 0) {
		for (int var = 0; var < tables.size(); ++var) {
			vStr = tables.at(var).trimmed();
			if (!words.contains(vStr)) {
				words << vStr;
			}
		}
	}
	words.sort();

#ifndef QT_NO_CURSOR
	QApplication::restoreOverrideCursor();
#endif
//	foreach (QString var, words) {
//		qDebug() << "words: "<< var;
//	}
//	qDebug() << "words.count()"<< words.count();
	return words;
}


QAbstractItemModel *MainWin::modelFromFile(const QString& fileName)
{
	QStringList words = listFromFile(fileName);
	return new QStringListModel(words, completer);
}


void MainWin::updateCompleter(){
	if (!completer) {
		delete completer;
	}
	QStringList words = listFromFile(":/resources/wordlist.txt");
	completer = new MyCompleter(words,teQuery);
//	completer->setModel(modelFromFile(":/resources/wordlist.txt"));
	completer->setModelSorting(QCompleter::CaseInsensitivelySortedModel);
	completer->setCaseSensitivity(Qt::CaseInsensitive);
	//completer->setF
	completer->setWrapAround(true/*false*/);
	teQuery->setCompleter(completer);

}

void MainWin::on_actExit_triggered(){
    close();
}

void MainWin::on_actConnect_triggered(){
	ConnectDlg dlg(dbmode, host, port, dbname, username);
    if ( dlg.exec() ) {
        dbmode = dlg.cbMode->currentText();
        db = QSqlDatabase::addDatabase(dbmode);
        if (dlg.chbHost->isChecked()) {
            host = dlg.leHost->text();
            db.setHostName(host);
        }
        //else            host = "127.0.0.1";
        
        int port;
        if (dlg.chbPort->isChecked()){
            port = dlg.sbPort->value();
            db.setPort(port);
        }else
            port = 0;
        
        dbname = dlg.leDbname->text().trimmed();
        if (!dbname.isEmpty()){
            db.setDatabaseName(dbname);
        }
		dbfilename = dbname;
        
        username = dlg.leUsername->text().trimmed();
        password = dlg.lePassword->text();
        if (!username.isEmpty()){
            db.setUserName(username);
            db.setPassword(password);
        }
        connected = db.open();
        emit connectChanged(connected);
        if (!connected)
            QMessageBox::critical(
                &dlg, tr("Connect Error"),
                db.lastError().text(),
                QMessageBox::Ok );
    }
}

void MainWin::on_actDisconnect_triggered(){
    if (connected)
        db.close();

    connected = false;
    connectChanged(connected);
}

void MainWin::on_actNew_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this,tr("Save File"),"newdatabase.db",tr("db files (*.db *.sqlite *.*)"));
	if (!fileName.isEmpty()){
		QSqlDatabase dbt;
		dbt = QSqlDatabase::addDatabase("QSQLITE","tempnewdb");
		dbt.setDatabaseName(fileName);
		if (dbt.open())
		{
			dbt.exec("PRAGMA encoding = \"UTF-8\"");
			dbt.close();
		}		
	}    
}

void MainWin::closeEvent(QCloseEvent *event){
    db.close();
    emit connectChanged(false);
       event->accept();
       return;
    if ( QMessageBox::question(
            this, tr("Exit application"),
            tr("Confirm to exit"),
            QMessageBox::Yes | QMessageBox::No,
            QMessageBox::Yes)
          == QMessageBox::Yes ) {
        if (connected){
            db.close();
            emit connectChanged(false);
        }
        event->accept();
    }else
        event->ignore();
}

void MainWin::getTables(){
    if (connected){
        tables = db.tables();
    }else{
        tables.clear();
    }
    listmodel->setStringList(tables);
    listTables->setModel(listmodel);
    listTables->update();
}

void MainWin::showStatus(bool state){
    connected = state;
    actConnect->setEnabled(!connected);
    actDisconnect->setEnabled(connected);
	//actionAddCvs->setEnabled(connected);
    actRun->setEnabled( connected &&
                       (teQuery->toPlainText().trimmed().length() > 0));
    if (state)
		label->setText(tr("Connected: host=%1 dbname=%2 dbfilename=%3")
		.arg(host) .arg(dbname).arg(dbfilename) ); //dbfilename = dbname;
    else
        label->setText(tr("Disconnected"));

    getTables();
}

void MainWin::on_teQuery_textChanged(){
    actRun->setEnabled( connected &&
          (teQuery->toPlainText().trimmed().length() > 0));
}

void MainWin::on_actRun_triggered(){
	QString vText = teQuery->toPlainText();
	QStringList li = vText.split("\n");
	vText = "";
	foreach (QString var, li) {
		var = var.trimmed();
		if (!var.startsWith("--")) {
			vText.append(var).append("\n");
		}
	}
	query_exec(vText);
}

void MainWin::showResult(){
    if (query->isActive()){
        QSqlQueryModel *model = new QSqlQueryModel(tvResult);
        model->setQuery(*query);
        tvResult->setModel(model);
        tvResult->resizeRowsToContents();
	}
}

void MainWin::showHelp()
{
	teLog->clear();
	teLog->append(QString::fromUtf8("<h2>Параметры командной строки.</h2> "
	"	<h3>sqlquery.exe [файл] </h3>"
	"	[файл] <br/>"
	"	*.txt - превращается в таблицу 1 типа: столбцы \"row_id\", \"text\", где \"row_id\" - номер строки с 1 до N, в \"text\" - содержимое строки.<br/>"
	"	*.txtfl | *.txtdfl - файлы со списками путей к файлам, которые будет интерпретироваться как *.txt и грузиться в таблицы соответственно их именам.<br/>"
	" <br/>"
	"	*.csv - создает таблицу с заголовками из строки с заголовком. А дальше добавляет в нее найденные данные.<br/>"
	"	*.csvfl | *.csvdfl - файлы со списками путей к файлам, которые будет интерпретироваться как *.csv и грузиться в таблицы соответственно их именам.<br/>"
	" <br/>"
	"	строки с префиксом \"--\" считаются комментарием и пропускаются.<br/>"
	"	Так-же пропускаются пустые строки.<br/>"
	""));
}

void MainWin::on_listTables_doubleClicked(const QModelIndex& index){
    if (index.isValid()){
        showTable(listTables->model()->data(index).toString());
    }
}

void MainWin::showTable(const QString& name){
    QSqlTableModel *model = new QSqlTableModel(tvResult);
    model->setEditStrategy(QSqlTableModel::OnRowChange);
    model->setTable(name);
    model->setSort(0, Qt::AscendingOrder);
    model->select();
	QString ss; // = model->selectStatement();
    if (model->lastError().type() != QSqlError::NoError){
        sb->showMessage(tr("Query error"), 3000);
        QMessageBox::critical(
                this, tr("Query Error"),
                model->lastError().text(),
                QMessageBox::Ok );
    }
    tvResult->setModel(model);
    tvResult->setEditTriggers( QAbstractItemView::DoubleClicked 
                             | QAbstractItemView::EditKeyPressed );
    tvResult->resizeRowsToContents();
}

