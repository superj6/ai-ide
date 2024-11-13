#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTextEdit>
#include <QProcess>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void newFile();
    void openFile();
    void saveFile();
    void saveFileAs();
    void compileAndRun();
    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void processError(QProcess::ProcessError error);
    void readCompilerOutput();

private:
    void createActions();
    void createMenus();
    void setupEditor();
    bool maybeSave();
    void loadFile(const QString &fileName);
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    void runCompiledProgram();

    QTextEdit *editor;
    QTextEdit *compilerOutput;
    QString currentFile;
    QProcess *process;
    bool isUntitled;
    bool isCompiling;
};

#endif