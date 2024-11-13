#ifndef COMPLETIONWIDGET_H
#define COMPLETIONWIDGET_H

#include <QFrame>
#include <QTextEdit>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QString>

class CompletionWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CompletionWidget(QTextEdit *parent = nullptr);
    void showCompletion(const QString &completion);
    void hideCompletion();
    bool isVisible() const;
    QString currentCompletion() const;
    void setModel(const QString &model);
    QString currentModel() const;

    static const QString DEFAULT_MODEL;
    static const QStringList AVAILABLE_MODELS;

signals:
    void modelChanged(const QString &model);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void handleNetworkReply(QNetworkReply *reply);
    void requestCompletion();

private:
    void updatePosition();
    QString getContextAroundCursor();
    void setupStyle();
    QString createPrompt(const QString &context);

    QTextEdit *editor;
    QString completion;
    QNetworkAccessManager *networkManager;
    QTimer *completionTimer;
    QString model;
    static const int CONTEXT_CHARS = 500;  // Characters to consider before cursor
    static const int COMPLETION_DELAY = 500;  // Milliseconds to wait before requesting
};

#endif // COMPLETIONWIDGET_H
