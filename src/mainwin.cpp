#include <QtGui>
#include <QSqlQuery>
#include <QRegExp>

#include "mainwin.h"
#include "connectdlg.h"


//#include "SQLParser/SQLParser.h"
#include "uSqlParser.h"


const QString gSpliterStyle =
"QSplitter::handle { "
"background :qlineargradient(spread:pad, x1:0.993909, y1:0.455, x2:0, y2:0.511, stop:0 rgba(172, 172, 172, 255), stop:1 rgba(255, 255, 255, 255));"
"width: 5px; "
"height: 5px; "
""
"border-radius: 2px;"
"}"
"QSplitterHandle::hover{}"
"QSplitter::handle:horizontal:hover {background:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(102, 102, 102, 255), stop:1 rgba(255, 255, 255, 255));}"
"QSplitter::handle:vertical:hover {background:qlineargradient(spread:pad, x1:0, y1:0, x2:1, y2:0, stop:0 rgba(102, 102, 102, 255), stop:1 rgba(255, 255, 255, 255));}"
;

MainWin::MainWin() : QMainWindow() {
    setupUi(this);
	QString vDate = QString("%1").arg(__DATE__);
	vDate = vDate.replace("  "," ");
	QStringList vDatesList = vDate.split(" ");
	if (vDatesList.size() == 3) {
		vDate = vDatesList.at(2) +"_"+ vDatesList.at(0)+ "_" + vDatesList.at(1);
	}


	m_versionText = QString("(%1-%2)").arg(vDate).arg(__TIME__); //
	splitter->setStyleSheet(gSpliterStyle);
	splitter_3->setStyleSheet(gSpliterStyle);
	splitter_2->setStyleSheet(gSpliterStyle);

	teQuery->setTabStopWidth(30);
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
	m_fldCntr = m_tableCnrt = 0;
	m_tableAddPKI = false;

//	// не прокатило...
	if (false) {

	m_editMenu = new QMenu(this);
	m_editMenu->addAction(actionComment);

	m_editBtn =   new QMenuToolButton;
	m_editBtn->setMenu(m_editMenu);
	m_editBtn->setDefaultAction(actionComment);
	toolBar->addWidget(m_editBtn);
	}

	// https://www.walletfox.com/course/customqtoolbutton.php - QToolButton with a drop-down menu that displays the last action
	//actionEdit->setPopupMode(QToolButton::MenuButtonPopup);
	updateCompleter();

	//actExport2File->setMenu();

	connect(this, SIGNAL(connectChanged(bool)),   this, SLOT(showStatus(bool)));
	connect(actionAddCvs, SIGNAL(triggered(bool)),   this, SLOT(addTextFile()));
	connect(actionhelp, SIGNAL(triggered(bool)),   this, SLOT(showHelp()));
	connect(actExport2File, SIGNAL(triggered(bool)),   this, SLOT(export2File()));
	connect(actionComment, SIGNAL(triggered(bool)),   this, SLOT(onFormatAction()));

	emit connectChanged(false);

	QTextCursor vCurs = teQuery->textCursor();
	teQuery->setTextCursor(vCurs);
	vCurs.setPosition(1, QTextCursor::MoveAnchor);


//    ui->setupUi(this);
//    ui->m_versionText->setText(vText);

}

void MainWin::openFile(QString psFile)
{
	if (!QFile::exists(psFile)) {
		return;
	}
	m_CollectoinFileName = psFile;
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
	} else if(ext.endsWith("sql")){
		isLoad = loadSqlFile(psFile);
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
	updateCompleter();

	// курсор в начало..
	QTextCursor vCurs = teQuery->textCursor();
	vCurs.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
	teQuery->setTextCursor(vCurs); // https://www.linux.org.ru/forum/development/15379945

	// trdm 2022-05-01 08:32:05
	teQuery->verticalScrollBar()->setValue(teQuery->verticalScrollBar()->minimum());

}


bool MainWin::query_exec(QString psQText, bool psTahQ, bool psToExport)
{
	if (psQText.isEmpty()) {
		return false;
	}
	QString vQText = psQText;

	vQText = vQText.trimmed();
	query = new QSqlQuery(db);
	QRegExp re("create\\s+view",Qt::CaseInsensitive); 	//QRegExp re("CREATE\\s+VIEW",Qt::CaseInsensitive);
	bool vIsCr = false;
	if (re.indexIn(psQText) != -1) {
		vIsCr = true;
	}

	if ( query->exec(psQText) ){
		if (query->isSelect() /*|| query->is*/) ///\todo 2022-03-25 11:07:47 - нужны обновления таблиц на CRTEATE VIEW
			if (!psToExport) {
				showResult();
			}
		if (!psTahQ) {
			QString vTmp = tr("<b>Query succeeded: </b>")	+ vQText;
			message( vTmp );	//		teLog->append( vTmp );			//teLog->append( tr("<br /><b>Query succeeded: </b>")	+ vQText );

		}
		sb->showMessage(tr("Query executed successfully"), 3000);
	}else{
		sb->showMessage(tr("Query error"), 3000);
		// <span style=" color:#ff0000;">features</span>
		QString vErrorMsg = QString::fromUtf8("<b>Query: </b> '%1' has error:<span style="" color:#ff0000;""> '%2' </span>").arg(vQText).arg(query->lastError().text());
		message(vErrorMsg); //		teLog->append( vErrorMsg );
		//QMessageBox::critical(	this, tr("Query Error"), 	query->lastError().text(),		QMessageBox::Ok );
		return false;
	}
	if (vIsCr) {
		getTables();
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



bool MainWin::loadCSVTextInMemory(QString psFile, QString psFName, QString psLineSep)
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
	m_fldCntr = 0; // сброим для таблицы
	QString vFName = psFName;
	vFName = getNormalSqlField(vFName,0);
	if (vFName != psFName) {
		message(QString("<b>Translate table name:</b> '%1' >> '%2'").arg(psFName).arg(vFName));
	}

	QString vLineSep = ",", vLineTQ = "\"", vLineMArcSett = "--settings:",vText = "",vText0 = "";
	if (!psLineSep.isEmpty()) {
		vLineSep = psLineSep;
	}
	QString vKeyWordsDMLstr = "select,insert,update,alter";
	QStringList vKeyWordsDML; vKeyWordsDML = vKeyWordsDMLstr.split(",");
	QString vQuery;
	QString line, lineLC;
	int vRowNumber = 0;
	while (!vFile.atEnd()) {
		line = vFile.readLine(); // read wavelength line and store it
		line = line.trimmed();
		if (line.isEmpty() || line.length() < 3) {// может быть \n или \r\n
			continue;
		}
		if (line.startsWith("--!")) {
			continue;
			//!\doc

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
				int vFounds = qMax(lineLC.indexOf("select"),lineLC.indexOf("insert"));
				vFounds = qMax(vFounds,lineLC.indexOf("title"));
				vFounds = qMax(vFounds,lineLC.indexOf("update"));
				vFounds = qMax(vFounds,lineLC.indexOf("alter"));
				if (vFounds != -1) {
					vQueries.append(line);
				}
				//line = line.mid(2);
			}
			continue;
		} //line.startsWith("--")
		bool vQueIsCreate = false;
		if (vCaptions.size() == 0) {
			// trdm 2022-02-24 12:06:35
			if (line.indexOf(vLineSep) == -1) {
				if (line.indexOf(";") != -1) {
					vLineSep = ";";
				}
			}
		}
		vFieldsData = line.split(vLineSep);
		vQuery = "INSERT INTO " + vFName + " VALUES(";
		if (vCaptions.size() == 0) {
			vCaptions = line.split(vLineSep);
			vQuery = "CREATE TABLE " + vFName + " (";
			if (m_tableAddPKI) {
				vQuery.append("id INTEGER PRIMARY KEY, "); //				vQuery.append("id INTEGER PRIMARY KEY AUTOINCREMENT, ");
				//( employee_id INTEGER PRIMARY KEY AUTOINCREMENT,
			}
			vQueIsCreate = true;
		}
		if (!vQueIsCreate) {
			vRowNumber++;
		}
		QString vTypeField = "TEXT", vTypeField_t;
		for (int var = 1; var <= vCaptions.size(); ++var) {
			vText = "";
			if (vQueIsCreate) {
				m_fldCntr++;
				vTypeField = "TEXT";
				vText = vCaptions.at(var-1);
				vText0 = vText.trimmed();
				vText = vText0;
				if (vText.startsWith("\"") && vText.endsWith("\"")) {
					vText = vText.mid(1,vText.length() -2); // Убираем "" с начали и с конца.
				}
				vText = getNormalSqlField(vText);
				vTypeField_t = extractType(vText);
				if (!vTypeField_t.isEmpty()) {
					vTypeField = vTypeField_t;
				}
				if (vText != vText0) {
					message(QString("Translate field name: '%1' << '%2'").arg(vText).arg(vText0));
				}

				if (!m_addWordCompleter.contains(vText)) {
					m_addWordCompleter.append(vText);
				}
				if (vQueIsCreate) {
					vText.append(" ");
					vText.append(vTypeField);
				}
				if (var != vCaptions.size()) {
					vText.append(", ");
				}
			} else {
				if (vFieldsData.size() >= var) {
					vText = vFieldsData.at(var-1);
					vText = vText.trimmed();
				}
				if (vText.startsWith("\"") && vText.endsWith("\"")) {
					vText = vText.mid(1,vText.length() -2); // Убираем "" с начали и с конца.
				}
				vText.append("'").prepend("'");
				if (m_tableAddPKI && var == 1) {
					vText.prepend(QString::number(vRowNumber)+",");
				}
				if (var != vCaptions.size()) {
					vText.append(", ");
				}
			}
			vQuery.append(vText);
		}
		vQuery.append(")");
		// qDebug() << vQuery;

		if ( query_exec(vQuery,true) ){
			if (vQueIsCreate) {
				getTables();
				message(vQuery);
			}
		}
	}
	for (int var = 0; var < vQueries.size(); ++var) {
		vQuery = vQueries.at(var);
		int vPos = vQuery.indexOf("SELECT")+vQuery.indexOf("ALTER")+vQuery.indexOf("UPDATE");
//		-- ALTER TABLE __wordSize ADD size_n numeric;
//		-- UPDATE __wordSize SET size_n = CAST(size AS INTEGER)

		if (vPos != -1) {
			teQuery->append( vQuery );
		} else {
			vQuery.append("<br/>");
			message( vQuery ); //				teLog->append( vQuery );
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
	QString vLineSep = ",";
	vFilePath = vFilePath.replace("/","\\");
	int vPos = 0;
	m_fldCntr = m_tableCnrt = 0;
	vFilePath.append("\\");
	m_tableAddPKI = false;
	while (!vFile.atEnd()) {
		line = vFile.readLine(); // read wavelength line and store it
		line = line.trimmed();
		if (line.isEmpty() || line.length() < 3) {// может быть \n или \r\n
			continue;
		}
		if (line.isEmpty()) {
			continue;
		}
		if (line.startsWith("--")){
			vPos = line.indexOf("separator");
			if (vPos != -1) {
				vPos = line.indexOf("=");
				if (vPos != -1) {
					vLineSep = line.mid(vPos+1);
					if (vLineSep.length() >=3) { // уберем кавычки
						vLineSep = vLineSep.mid(1,vLineSep.length()-2);
					}
				}
			} else {
				loadParseCommentLine(line);
			}
			continue;
		}
		vFileName = line;
		if (line.indexOf("/") == -1 && line.indexOf("\\") == -1) {
			vFileName = vFilePath + line;
		}
		if (QFile::exists(vFileName)) {
			QFileInfo vFileInfo2(vFileName);
			QString ext = vFileInfo2.suffix();  ext = ext.toLower();

			if(ext.endsWith("sql")){
				loadSqlFile(vFileName);
			} else {
				m_tableCnrt++;
				if (psAsCSV == 1) {
					loadCSVTextInMemory(vFileName,vFileInfo2.baseName(),vLineSep);
				} else {
					loadTextInMemory(vFileName,vFileInfo2.baseName());
				}
			}

		}
		//loadTextInMemory
	}
	vFile.close();
	updateCompleter();
	return true;
}

bool MainWin::loadSqlFile(QString psFile)
{
	if (!QFile::exists(psFile)) {
		return false;
	}
	QFile vFile(psFile);
	if (!vFile.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QString line;
	QFileInfo vFileInfo(psFile);
	QString vQuery = "";
	int vPos = 0;
	while (!vFile.atEnd()) {
		line = vFile.readLine(); // read wavelength line and store it
		line = line.trimmed();
		if (line.isEmpty() || line.length() < 3) {// может быть \n или \r\n
			continue;
		}
		if (line.startsWith("--")){
			continue;
		}
		vQuery = line;
		query_exec(vQuery);
	}
	vFile.close();
	updateCompleter();
	return true;

}

bool MainWin::loadParseCommentLine(QString psText)
{
	//--set_pka=1
	if (!psText.startsWith("--")) {
		return false;
	}
	QString vText = psText, vText2 = "", vText3 = "";
	vText = vText.mid(2);
	vText = vText.trimmed();
	if (vText.length() < 3) {
		return false;
	}
	QStringList li = vText.split("=");
	if (li.size() < 2 ) {
		return false;
	}
	vText = li.at(0); vText = vText.trimmed();
	vText2 = li.at(1); vText2 = vText2.trimmed();

	vText3 = getNormalSqlField(vText);
	if (vText != vText3) {
		return false;
	}
	vText = vText.toLower();
	bool vText2Bool = false;
	vText3 = vText2;
	vText3 = vText3.toLower();
	if (vText2 == "1" || vText3 == "y" || vText3 == "yes" /*||*/) {
		vText2Bool = true;
	}
	if (vText == "set_pka") {
		m_tableAddPKI = vText2Bool;
	}
	return true;
}


bool MainWin::addTextFile()
{
	QString vDir = qApp->applicationDirPath();
	if (false) {

	}
	QString fileName = QFileDialog::getOpenFileName(this,"Add cvs..",vDir);
	if (!fileName.isEmpty()){
		openFile(fileName);
	}
	return true;
}

void MainWin::onFormatAction()
{
	//qDebug() << "actionComment";
	QString vQuery = teQuery->toPlainText();
	uSqlParser p(vQuery);
	QTextCursor vCurs = teQuery->textCursor();
	teQuery->setTextCursor(vCurs);
	int vPos = vCurs.position(); //	int vBn = vCurs.blockNumber();
	QString vBlockText = vCurs.block().text();
	vBlockText = vBlockText.trimmed();
	// Блок просто ремит или очищает коммент в строке..
	if (!vBlockText.startsWith("--")) {
		//http://www.prog.org.ru/topic_32957_0.html
		vCurs.setPosition(vCurs.block().position(), QTextCursor::MoveAnchor);
		vCurs.setPosition(vCurs.block().position(), QTextCursor::KeepAnchor);
		vCurs.insertText("--");
	} else {
		vPos = vBlockText.indexOf("--");
		vCurs.setPosition(vCurs.block().position(), QTextCursor::MoveAnchor);
		vCurs.setPosition(vCurs.block().position()+vPos+2, QTextCursor::KeepAnchor);
		vCurs.insertText("");
	}
	if (0) {
		qDebug() << "vCurs.position()"<< vCurs.position() << "vCurs.blockNumber()" << vCurs.blockNumber() << "vCurs.block().text()" << vCurs.block().text();
		qDebug() << "vCurs.block().position()" << vCurs.block().position() << "vBlockText" << vBlockText;
	}

//	hsql::SQLParserResult result;
//	hsql::SQLParser::parse(vQuery.toStdString(), &result);
//            src/SQLParser/SQLParser.cpp \


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

QString MainWin::getAliveQuery()
{
	QString vText = teQuery->toPlainText();
	QStringList li = vText.split("\n");
	vText = "";
	foreach (QString var, li) {
		var = var.trimmed();
		if (!var.startsWith("--")) {
			vText.append(var).append("\n");
		}
	}
	vText = vText.trimmed();
	return vText;
}

// trdm 2022-03-16 12:02:56
void MainWin::message(QString psMsg)
{
	//QString vErrorMsg = QString::fromUtf8("%1 <br />").arg(psMsg);
	QString vErrorMsg = QString::fromUtf8("%1 ").arg(psMsg);
	teLog->append( vErrorMsg );
	teLog->verticalScrollBar()->setValue(teLog->verticalScrollBar()->maximum());
	//teLog->scr

}

// trdm 2022-03-16 11:35:51 в csv иногда попадаются кирилические поля
QString MainWin::getNormalSqlField(QString psInput, int psIsField)
{
	QString vLarChars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_";
	QString vRetVal = "";
	QChar vHr;
	bool va = false;
	for (int var = 0; var < psInput.length(); ++var) {
		vHr = psInput.at(var);
		if (!va && vRetVal.length() > 0) {
			vLarChars.append("0123456789");
			va = true;
		}
		if (vLarChars.indexOf(vHr) != -1) {
			vRetVal.append(vHr);
		}
	}
	QString vPrep = "table_";
	QString vApendCntr = QString("%1").arg(m_tableCnrt);
	if (psIsField == 1) {
		vPrep = "field_";
		vApendCntr = QString("%1").arg(m_fldCntr);
	}
	if (vRetVal.length() == 0) {
		vRetVal = vPrep;
		vRetVal.append(vApendCntr);
	} else {
		vHr = vRetVal.at(0);
		if (!(vHr.isLetter() || vHr == '_')) {
			vRetVal = vPrep;
			vRetVal.append(vApendCntr);
		}
	}
	return vRetVal;

}

QString MainWin::extractType(QString &psField)
{
	QString vTypeRv = "", vFieldName = psField;
	int vPos = psField.indexOf("_");
	if (vPos != -1) {
		// field_astype_TYPE
		QStringList vList = psField.split("_");

		QStringList vListTypeLC = QString("INTEGER").split(",");
		int vSz = vList.size();
		if (vSz >= 3 ) {
			QString vTypes_ = "INTEGER,INT,NUMERIC";
			vTypes_ = vTypes_.toLower();
			QStringList vListTypeLC = vTypes_.split(",");

			QString vMarker = "", vType = "";
			vMarker = vList.at(vSz-2); vMarker = vMarker.toLower();
			vType = vList.at(vSz-1); vType = vType.toLower();
			if (vMarker == "astype" && vListTypeLC.indexOf(vType) != 0) {
				vSz = 1+1+6+vType.length();
				vFieldName = vFieldName.mid(0, vFieldName.length() - (vSz));
				psField = vFieldName;
				vTypeRv = vType;
			}


		}
	}
	return vTypeRv;
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
			QMessageBox::critical(&dlg, tr("Connect Error"),db.lastError().text(), QMessageBox::Ok );
    }
}

void MainWin::on_actDisconnect_triggered(){
    if (connected)
        db.close();

    connected = false;
    connectChanged(connected);
	dbfilename = "";
	dbname = "";
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
		// trdm 2022-03-25 12:06:13 - Добавил вьювы и систем таблы...
		tables = db.tables(QSql::AllTables);		// tables = db.tables();
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
	bool vEnabled = connected && (teQuery->toPlainText().trimmed().length() > 0);
	actRun->setEnabled(vEnabled);
	actExport2File->setEnabled(vEnabled);
    if (state)
		if (!host.isEmpty()) {
			label->setText(tr("Connected: host=%1 dbname=%2 dbfilename=%3 version=%4")	.arg(host) .arg(dbname).arg(dbfilename).arg(m_versionText) ); //dbfilename = dbname;
		} else {
			label->setText(tr("Connected: dbname=%1 dbfilename=%2 version=%3")	.arg(dbname).arg(dbfilename).arg(m_versionText) ); //dbfilename = dbname;
		}
    else
		label->setText(tr("Disconnected version=%3").arg(m_versionText));

    getTables();
}

void MainWin::on_teQuery_textChanged(){
	bool vEnabled =  connected && (teQuery->toPlainText().trimmed().length() > 0);

	actExport2File->setEnabled(vEnabled);
	actRun->setEnabled(vEnabled);
}

void MainWin::on_actRun_triggered(){
	QString vText = getAliveQuery();
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
	//teLog->clear();
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
	"	Запросы SQLite: <a href=\"https://oracleplsql.ru/sql-query-types-sqlite.html\">https://oracleplsql.ru/sql-query-types-sqlite.html</a>.<br/>"
	""));
}

void MainWin::export2File()
{
	QString vFaleName = "output.csv";
	QString vDir = qApp->applicationDirPath(); vDir.append("\\").append(vFaleName);
	vFaleName = QFileDialog::getSaveFileName(this,QString::fromUtf8("Выберите файл для импорта"),vDir,QString::fromUtf8("*.csv"));
	qDebug() << vFaleName;
	if (vFaleName.isEmpty()) {
		return;
	}
	QString vText = getAliveQuery();
	if (!query_exec(vText, true,true)) {
		return;
	}
	QString vData;

	QFile data(vFaleName);
	if (data.open(QFile::WriteOnly)) {
		QTextStream outTxt(&data);
		bool firstLine=true;
		while (query->next()) {
			const QSqlRecord recrd= query->record();
			if(firstLine){
				for(int i=0;i<recrd.count();++i)
					outTxt << recrd.field(i).name() << ','; //Headers
			}
			firstLine=false;
			outTxt << "\r\n";
			for(int i=0;i<recrd.count();++i) {
				vData = recrd.value(i).toString();
				if (i==0) {
						//qDebug() << "vData" << vData;
					}
					outTxt << vData << ',';
			}
		}
		data.close();
	}
}

void MainWin::on_listTables_doubleClicked(const QModelIndex& index){
    if (index.isValid()){
		QString vTName = listTables->model()->data(index).toString();
		/// \todo 2022-03-22 12:09:35

		qDebug() << vTName;
		showTable(vTName);
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
		QMessageBox::critical(this, tr("Query Error"), model->lastError().text(), QMessageBox::Ok );
    }
    tvResult->setModel(model);
    tvResult->setEditTriggers( QAbstractItemView::DoubleClicked 
                             | QAbstractItemView::EditKeyPressed );
    tvResult->resizeRowsToContents();

	///\todo 2022-04-11 08:49:01 собратьсписок полей...
//	QString vQ_0 = QString("SELECT * FROM %1 LImit 1").arg(name);
//	QSqlQuery query_0 = new QSqlQuery(db);

	QString vQ = QString("<b>Query: </b>SELECT * FROM %1").arg(name);
	message(vQ);

}

