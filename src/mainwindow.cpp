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
#include <QDir>
#include <QStatusBar>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), isUntitled(true), isCompiling(false)
{
    setWindowTitle("Beach IDE");
    resize(1024, 768);

    // Create central widget and layout
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);
    QVBoxLayout *layout = new QVBoxLayout;
    QSplitter *splitter = new QSplitter(Qt::Vertical);
    
    // Setup editor with beach at night theme colors
    editor = new QTextEdit;
    setupEditor();
    splitter->addWidget(editor);

    // Setup compiler output with beach at night theme colors
    compilerOutput = new QTextEdit;
    compilerOutput->setReadOnly(true);
    compilerOutput->setFont(QFont("Courier", 12));
    compilerOutput->setStyleSheet(
        "QTextEdit {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                             stop:0 #1a2634, stop:0.3 #2a3d50,"    // Deep ocean gradient
        "                             stop:0.7 #3d4d5e, stop:1 #4a5d70);"   // Sandy ocean floor
        "  background-image: repeating-linear-gradient("                     // Wave pattern
        "    0deg,"
        "    rgba(210, 180, 140, 0.02),"  // Lighter sandy waves
        "    rgba(210, 180, 140, 0.02) 8px,"
        "    transparent 8px,"
        "    transparent 16px"
        "  );"
        "  color: #E2E8F0;"              // Soft white text
        "  border: 1px solid #d2b48c;"    // Sandy border
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  selection-background-color: #4a5d70;"  // Ocean highlight
        "}"
    );
    splitter->addWidget(compilerOutput);

    layout->addWidget(splitter);
    centralWidget->setLayout(layout);

    // Set the overall application style
    setStyleSheet(
        "QMainWindow {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                             stop:0 #1a2634, stop:0.4 #2a3d50,"  // Deep ocean gradient
        "                             stop:0.6 #3d4d5e, stop:1 #4a5d70);"  // Sandy ocean floor
        "}"
        "QSplitter::handle {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "                             stop:0 #d2b48c, stop:0.3 #deb887,"   // Sandy colors
        "                             stop:0.7 #d2b48c, stop:1 #deb887);"  // with variation
        "  height: 2px;"
        "}"
        "QMessageBox {"
        "  background-color: #1a2634;"  // Deep ocean
        "  color: #E2E8F0;"            // Soft white
        "}"
        "QMenuBar {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "                             stop:0 #1a2634, stop:0.4 #2a3d50,"
        "                             stop:0.6 #3d4d5e, stop:1 #4a5d70);"
        "  color: #E2E8F0;"
        "  border-bottom: 2px solid #d2b48c;"  // Sandy border
        "}"
        "QMenuBar::item:selected {"
        "  background: #4a5d70;"  // Ocean highlight
        "}"
        "QMenu {"
        "  background-color: #1a2634;"  // Deep ocean
        "  color: #E2E8F0;"
        "  border: 1px solid #d2b48c;"  // Sandy border
        "}"
        "QMenu::item:selected {"
        "  background-color: #4a5d70;"  // Ocean highlight
        "}"
        "QScrollBar:vertical {"
        "  background: #1a2634;"        // Deep ocean
        "  width: 12px;"
        "  margin: 0px;"
        "}"
        "QScrollBar::handle:vertical {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "                             stop:0 #d2b48c, stop:0.5 #deb887,"   // Sandy scrollbar
        "                             stop:1 #d2b48c);"
        "  min-height: 20px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
        "  height: 0px;"
        "}"
        "QScrollBar:horizontal {"
        "  background: #1a2634;"        // Deep ocean
        "  height: 12px;"
        "  margin: 0px;"
        "}"
        "QScrollBar::handle:horizontal {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "                             stop:0 #d2b48c, stop:0.5 #deb887,"   // Sandy scrollbar
        "                             stop:1 #d2b48c);"
        "  min-width: 20px;"
        "  border-radius: 6px;"
        "}"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
        "  width: 0px;"
        "}"
    );

    // Initialize process
    process = new QProcess(this);
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::processFinished);
    connect(process, &QProcess::errorOccurred, this, &MainWindow::processError);
    connect(process, &QProcess::readyReadStandardOutput, this, &MainWindow::readCompilerOutput);
    connect(process, &QProcess::readyReadStandardError, this, &MainWindow::readCompilerOutput);

    // Initialize completion widget
    completionWidget = new CompletionWidget(editor);

    createActions();
    createMenus();
}

void MainWindow::setupEditor()
{
    QFont font("Courier", 12);
    editor->setFont(font);

    // Set up the editor widget style with beach at night colors and wave pattern
    editor->setStyleSheet(
        "QTextEdit {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1,"
        "                             stop:0 #1a2634, stop:0.3 #2a3d50,"    // Deep ocean gradient
        "                             stop:0.7 #3d4d5e, stop:1 #4a5d70);"   // Sandy ocean floor
        "  background-image: repeating-linear-gradient("                     // Wave pattern
        "    0deg,"
        "    rgba(210, 180, 140, 0.03),"  // Sandy color with low opacity
        "    rgba(210, 180, 140, 0.03) 10px,"
        "    transparent 10px,"
        "    transparent 20px"
        "  );"
        "  color: #E2E8F0;"              // Soft white text
        "  border: 1px solid #d2b48c;"    // Sandy border
        "  border-radius: 4px;"
        "  padding: 8px;"
        "  selection-background-color: #4a5d70;"  // Ocean highlight
        "}"
    );

    // Create syntax highlighter
    highlighter = new Highlighter(editor->document());

    // Connect document modification signal
    connect(editor->document(), &QTextDocument::contentsChanged,
            this, &MainWindow::documentWasModified);
}

void MainWindow::documentWasModified()
{
    setWindowModified(editor->document()->isModified());
}

bool MainWindow::saveFile(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::WriteOnly | QFile::Text)) {
        QTextStream out(&file);
        out << editor->toPlainText();
        file.close();
        
        setCurrentFile(fileName);
        editor->document()->setModified(false);
        statusBar()->showMessage(tr("File saved"), 2000);
        return true;
    }
    QMessageBox::warning(this, tr("Application"),
                        tr("Cannot write file %1:\n%2.")
                        .arg(QDir::toNativeSeparators(fileName),
                             file.errorString()));
    return false;
}

void MainWindow::loadFile(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream in(&file);
        editor->setPlainText(in.readAll());
        file.close();
        
        setCurrentFile(fileName);
        editor->document()->setModified(false);
        statusBar()->showMessage(tr("File loaded"), 2000);
    }
}

void MainWindow::setCurrentFile(const QString &fileName)
{
    currentFile = fileName;
    isUntitled = fileName.isEmpty();
    editor->document()->setModified(false);
    setWindowModified(false);
    
    QString shownName = currentFile;
    if (isUntitled) {
        shownName = "untitled.cpp";
    }
    setWindowTitle(QString("%1[*] - Beach IDE").arg(QFileInfo(shownName).fileName()));
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
    menuBar()->setStyleSheet(
        "QMenuBar {"
        "  background: qlineargradient(x1:0, y1:0, x2:1, y2:0,"
        "                             stop:0 #1a2634, stop:0.4 #2a3d50,"
        "                             stop:0.6 #3d4d5e, stop:1 #4a5d70);"
        "  color: #E2E8F0;"  // Soft white text
        "  padding: 4px;"
        "}"
        "QMenuBar::item:selected {"
        "  background: #4a5d70;"  // Slate highlight
        "}"
    );
    createModelMenu();
}

void MainWindow::createModelMenu()
{
    modelMenu = menuBar()->addMenu("AI Model");
    modelActionGroup = new QActionGroup(this);
    modelActionGroup->setExclusive(true);

    for (const QString &modelName : CompletionWidget::AVAILABLE_MODELS) {
        QAction *action = modelMenu->addAction(modelName);
        action->setCheckable(true);
        action->setData(modelName);
        modelActionGroup->addAction(action);
        
        if (modelName == completionWidget->currentModel()) {
            action->setChecked(true);
        }
    }

    connect(modelActionGroup, &QActionGroup::triggered, this, &MainWindow::setCompletionModel);
}

void MainWindow::setCompletionModel()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if (action) {
        QString modelName = action->data().toString();
        completionWidget->setModel(modelName);
    }
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
