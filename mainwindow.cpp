#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDropEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QSettings>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton_Add, SIGNAL(clicked()), this, SLOT(addFile()));
    connect(ui->pushButton_Remove, SIGNAL(clicked()), this, SLOT(removeFile()));
    connect(ui->pushButton_Up, SIGNAL(clicked()), this, SLOT(moveUp()));
    connect(ui->pushButton_Down, SIGNAL(clicked()), this, SLOT(moveDown()));
    connect(ui->pushButton_Clear, SIGNAL(clicked()), this, SLOT(clearFile()));
    connect(ui->pushButton_Build, SIGNAL(clicked()), this, SLOT(makeSample()));
    connect(ui->listWidget, SIGNAL(currentRowChanged(int)), this, SLOT(changeFile()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addFile(void)
{
    QSettings setting("WaPENTools", "MakeSample");
    QString dir = setting.value("readdir", "").toString();

    QStringList filenames = QFileDialog::getOpenFileNames(this, nullptr, nullptr, "PENファイル (*.pen)");
    if(filenames.length() == 0) return;
    dir = QFileInfo(filenames[0]).absolutePath();
    setting.setValue("readdir", dir);
    for(QString filename : filenames)
            ui->listWidget->addItem(filename);
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasFormat("text/uri-list"))
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent* event)
{
    if(event->mimeData()->hasUrls())
    {
        QList<QUrl> urls = event->mimeData()->urls();
        for(QUrl url: urls)
            ui->listWidget->addItem(url.toLocalFile());
        changeFile();
    }
}

void MainWindow::removeFile(void)
{
    int index = ui->listWidget->currentRow();
    if(index >= 0)
    {
        delete ui->listWidget->takeItem(index);
        changeFile();
    }
}

void MainWindow::moveUp(void)
{
    int index = ui->listWidget->currentRow();
    if(index > 0)
    {
        ui->listWidget->insertItem(index - 1, ui->listWidget->takeItem(index));
        ui->listWidget->setCurrentRow(index - 1);
    }
}

void MainWindow::moveDown(void)
{
    int index = ui->listWidget->currentRow();
    if(index < ui->listWidget->count() - 1)
    {
        ui->listWidget->insertItem(index + 1, ui->listWidget->takeItem(index));
        ui->listWidget->setCurrentRow(index + 1);
    }
}

void MainWindow::changeFile(void)
{
    int index = ui->listWidget->currentRow();
    if(index >= 0)
    {
        QString filename = ui->listWidget->item(index)->text();
        QFile file(filename);
        if(file.open(QIODevice::ReadOnly))
        {
            QTextStream in(&file);
            QString text = in.readAll();
            file.close();
            ui->textEdit->setText(text);
        }
    }
    else ui->textEdit->clear();
}

void MainWindow::clearFile(void)
{
    ui->listWidget->clear();
    changeFile();
}

void MainWindow::makeSample(void)
{
    QSettings setting("WaPENTools", "MakeSample");
    QString dir = setting.value("writedir", "").toString();
    dir = QFileDialog::getExistingDirectory(this, "保存するディレクトリ");
    if(dir.length() == 0) return;
    setting.setValue("writedir", dir);
    QString outfilename = dir + QDir::separator() + "sample.js";
    QFile outfile(outfilename);
    if(outfile.open(QIODevice::WriteOnly))
    {
        QTextStream ostream(&outfile);
        ostream.setCodec("UTF-8");
        ostream << "\"use strict\";\nvar sample=[" << endl;
        for(int i = 0; i < ui->listWidget->count(); i++)
        {
            QString infilename = ui->listWidget->item(i)->text();
            QFile infile(infilename);
            if(infile.open(QIODevice::ReadOnly))
            {
                QTextStream istream(&infile);
                QString line;
                while(istream.readLineInto(&line))
                    ostream << "\"" + line.replace('"',"\"") << "\\n\"+" << endl;
            }
            ostream << "\"\"," << endl;
            infile.close();
        }
        ostream << "];\n";
        outfile.close();
        QMessageBox msgbox(this);
        msgbox.setWindowTitle("MakeSample");
        msgbox.setText(outfilename + "を作成しました");
        msgbox.exec();
    }
}
