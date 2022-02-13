#include <QtGui>
#include <QtSql>
#include "mainwin.h"

int main(int argc, char *argv[]) {

    QApplication app(argc, argv);
    QTextCodec *codec = QTextCodec::codecForName("CP1251"); 
    QTextCodec::setCodecForTr(codec);
    QTextCodec::setCodecForCStrings(codec);

    MainWin mw;
    mw.show();
	if (argc > 1) {
		mw.openFile(argv[1]);
	}

    return app.exec();
}
