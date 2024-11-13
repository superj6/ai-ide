#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QSplitter>
#include <QTemporaryFile>
#include <QTextStream>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isUntitled(true), isCompiling(false)
{
    setWindowTitle("C++ IDE");
    resize(800, 600);

    // Create central widget with splitter
    QWidget *centralWidget = new QWidget;
    QVBoxLayout *layout = new QVBoxLayout;
    QSplitter *splitter = new QSplitter(Qt::Vertical);
    
    // Setup editor
    editor = new QTextEdit;
    editor->setFontFamily("Courier");
    editor->setFontPointSize(12);
    splitter->addWidget(editor);

    // Setup compiler output
    compilerOutput = new QTextEdit;
    compilerOutput->setReadOnly(true);
    compilerOutput->setFontFamily("Courier");
    compilerOutput->setFontPointSize(10);
    splitter->addWidget(compilerOutput);

    layout->addWidget(splitter);
    centralWidget->setLayout(layout);
    setCentralWidget(centralWidget);

    // Initialize process
    process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::processFinished);
    connect(process, &QProcess::errorOccurred, this, &MainWindow::processError);
    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::readCompilerOutput);
    connect(process, &QProcess::readyReadStandardError, this, &MainWindow::readCompilerOutput);

    createActions();
    createMenus();
}

MainWindow::~MainWindow()
{
    if (process->state() == QProcess::Running) {
        process->terminate();
        process->waitForFinished();
    }
}

void MainWindow::createActions()
{
    QMenu *fileMenu = menuBar()->addMenu("&File");
    QAction *newAct = new QAction("&New", this);
    newAct->setShortcuts(QKeySequence::New);
    connect(newAct, SIGNAL(triggered()), this, SLOT(newFile()));
    fileMenu->addAction(newAct);

    QAction *openAct = new QAction("&Open...", this);
    openAct->setShortcuts(QKeySequence::Open);
    connect(openAct, SIGNAL(triggered()), this, SLOT(openFile()));
    fileMenu->addAction(openAct);

    QAction *saveAct = new QAction("&Save", this);
    saveAct->setShortcuts(QKeySequence::Save);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(saveFile()));
    fileMenu->addAction(saveAct);

    QAction *saveAsAct = new QAction("Save &As...", this);
    saveAsAct->setShortcuts(QKeySequence::SaveAs);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveFileAs()));
    fileMenu->addAction(saveAsAct);

    fileMenu->addSeparator();
    QAction *exitAct = new QAction("E&xit", this);
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));
    fileMenu->addAction(exitAct);

    QMenu *buildMenu = menuBar()->addMenu("&Build");
    QAction *runAct = new QAction("Compile and &Run", this);
    runAct->setShortcut(Qt::Key_F5);
    connect(runAct, SIGNAL(triggered()), this, SLOT(compileAndRun()));
    buildMenu->addAction(runAct);
}

void MainWindow::createMenus()
{
    // Menus are created in createActions()
}

void MainWindow::newFile()
{
    if (maybeSave()) {
        editor->clear();
        setCurrentFile(QString());
    }
}

void MainWindow::openFile()
{
    if (maybeSave()) {
        QString fileName = QFileDialog::getOpenFileName(this, "Open C++ Source File", "", "C++ Files (*.cpp *.h);;All Files (*)");
        if (!fileName.isEmpty()) {
            loadFile(fileName);
        }
    }
}

void MainWindow::saveFile()
{
    if (isUntitled) {
        saveFileAs();
    } else {
        saveFile(currentFile);
    }
}

void MainWindow::saveFileAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save As", "", "C++ Files (*.cpp *.h);;All Files (*)");
    if (!fileName.isEmpty()) {
        saveFile(fileName);
    }
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << editor->toPlainText();
        setCurrentFile(fileName);
        return true;
    }
    QMessageBox::warning(this, "Application", "Cannot write file " + fileName);
    return false;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        editor->setPlainText(in.readAll());
        setCurrentFile(fileName);
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    isUntitled = fileName.isEmpty();
    setWindowModified(false);
    
    QString shownName = currentFile;
    if (isUntitled) {
        shownName = "untitled.cpp";
    }
    setWindowTitle(QString("%1[*] - C++ IDE").arg(shownName));
}

bool MainWindow::maybeSave()
{
    if (editor->document()->isModified()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, "Application",
                                 "The document has been modified.\n"
                                 "Do you want to save your changes?",
                                 QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return saveFile(currentFile);
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::compileAndRun()
{
    if (isUntitled || editor->document()->isModified()) {
        QMessageBox::warning(this, "Compile", "Please save the file first");
        return;
    }

    compilerOutput->clear();
    QString program = "g++";
    QStringList arguments;
    arguments << currentFile << "-o" << currentFile + ".out";
    
    isCompiling = true;
    process->start(program, arguments);
}

void MainWindow::runCompiledProgram()
{
    QStringList args;
    process->start(currentFile + ".out", args);
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (isCompiling) {
        isCompiling = false;
        if (exitCode == 0 && exitStatus == QProcess::NormalExit) {
            compilerOutput->append("Compilation successful!\nRunning program...\n");
            runCompiledProgram();
        } else {
            compilerOutput->append("Compilation failed!");
        }
    }
}

void MainWindow::processError(QProcess::ProcessError error)
{
    QString errorString;
    switch (error) {
        case QProcess::FailedToStart:
            errorString = "Failed to start";
            break;
        case QProcess::Crashed:
            errorString = "Process crashed";
            break;
        default:
            errorString = "Unknown error";
    }
    compilerOutput->append("Error: " + errorString);
}

void MainWindow::readCompilerOutput()
{
    if (isCompiling) {
        compilerOutput->append(QString::fromLocal8Bit(process->readAllStandardError()));
    } else {
        QString output = QString::fromLocal8Bit(process->readAllStandardOutput());
        QString error = QString::fromLocal8Bit(process->readAllStandardError());
        if (!output.isEmpty()) compilerOutput->append(output);
        if (!error.isEmpty()) compilerOutput->append(error);
    }
}