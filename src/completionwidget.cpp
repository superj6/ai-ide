#include "completionwidget.h"
#include <QPainter>
#include <QTextBlock>
#include <QKeyEvent>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QTextStream>
#include <QDir>

const QString CompletionWidget::DEFAULT_MODEL = "gpt-4";
const QStringList CompletionWidget::AVAILABLE_MODELS = {"gpt-4", "gpt-3.5-turbo"};

CompletionWidget::CompletionWidget(QTextEdit *parent)
    : QFrame(parent), editor(parent), model(DEFAULT_MODEL)
{
    setFrameStyle(QFrame::Box | QFrame::Plain);
    setLineWidth(1);
    
    // Set up network manager
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &CompletionWidget::handleNetworkReply);

    // Set up completion timer
    completionTimer = new QTimer(this);
    completionTimer->setSingleShot(true);
    connect(completionTimer, &QTimer::timeout,
            this, &CompletionWidget::requestCompletion);

    // Install event filter on editor
    if (editor) {
        editor->installEventFilter(this);
    }

    // Hide initially
    hide();
    setupStyle();
}

void CompletionWidget::setupStyle()
{
    setStyleSheet(R"(
        CompletionWidget {
            background: rgba(255, 255, 255, 240);
            border: 1px solid #81d4fa;
            border-radius: 4px;
            padding: 4px;
            color: #666666;
            font-family: Courier;
        }
    )");
}

void CompletionWidget::showCompletion(const QString &text)
{
    completion = text;
    if (!completion.isEmpty()) {
        updatePosition();
        show();
        raise();
    }
}

void CompletionWidget::hideCompletion()
{
    hide();
    completion.clear();
}

bool CompletionWidget::isVisible() const
{
    return QFrame::isVisible();
}

QString CompletionWidget::currentCompletion() const
{
    return completion;
}

void CompletionWidget::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);
    if (!completion.isEmpty()) {
        QPainter painter(this);
        painter.setFont(editor->font());
        painter.setPen(Qt::gray);
        painter.drawText(rect().adjusted(5, 2, -5, -2), completion);
    }
}

void CompletionWidget::updatePosition()
{
    if (!editor) return;

    QTextCursor cursor = editor->textCursor();
    QRect cursorRect = editor->cursorRect(cursor);
    QPoint pos = editor->mapToGlobal(cursorRect.bottomRight());
    
    // Calculate size based on completion text
    QFontMetrics fm(editor->font());
    int width = fm.horizontalAdvance(completion) + 20;
    int height = fm.height() + 10;

    // Adjust position to stay within editor bounds
    QRect screenRect = editor->rect();
    if (pos.x() + width > screenRect.right()) {
        pos.setX(screenRect.right() - width);
    }
    
    setGeometry(editor->mapFromGlobal(pos).x(), cursorRect.bottom() + 5, width, height);
}

QString CompletionWidget::getContextAroundCursor()
{
    if (!editor) return QString();

    QTextCursor cursor = editor->textCursor();
    QTextDocument *doc = editor->document();
    
    // Get text before cursor
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, CONTEXT_CHARS);
    int start = qMax(0, cursor.position());
    cursor.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, CONTEXT_CHARS);
    
    return doc->toPlainText().mid(start, CONTEXT_CHARS);
}

QString CompletionWidget::createPrompt(const QString &context)
{
    return QString(
        "You are an expert C++ code completion assistant. Analyze the context and provide a completion that:\n"
        "1. Matches the coding style in the context\n"
        "2. Uses modern C++ features when appropriate\n"
        "3. Considers variable names and types from the context\n"
        "4. Completes the current statement or block\n"
        "5. Is concise and follows best practices\n\n"
        "Provide ONLY the completion code, no explanations. Context:\n\n%1").arg(context);
}

void CompletionWidget::requestCompletion()
{
    QString context = getContextAroundCursor();
    if (context.isEmpty()) return;

    // Read API key from .env file
    QString apiKey;
    QString envPath = QDir::currentPath() + "/.env";
    QFile envFile(envPath);
    
    qDebug() << "Looking for .env file at:" << envPath;
    
    if (envFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&envFile);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (line.startsWith("OPENAI_API_KEY=")) {
                apiKey = line.mid(15).trimmed();  // Skip "OPENAI_API_KEY=" (15 characters)
                // Remove any quotes if present
                if (apiKey.startsWith('"') && apiKey.endsWith('"')) {
                    apiKey = apiKey.mid(1, apiKey.length() - 2);
                }
                if (apiKey.startsWith("'") && apiKey.endsWith("'")) {
                    apiKey = apiKey.mid(1, apiKey.length() - 2);
                }
                qDebug() << "Found API key, length:" << apiKey.length();
                break;
            }
        }
        envFile.close();
    } else {
        qDebug() << "Failed to open .env file:" << envFile.errorString();
    }

    if (apiKey.isEmpty()) {
        qDebug() << "No API key found in .env file";
        return;
    }

    // Prepare the request
    QNetworkRequest request(QUrl("https://api.openai.com/v1/chat/completions"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    // Prepare the chat completion request
    QJsonObject message;
    message["role"] = "user";
    message["content"] = createPrompt(context);

    QJsonArray messages;
    messages.append(message);

    QJsonObject json;
    json["model"] = model;
    json["messages"] = messages;
    json["temperature"] = 0.3;
    json["max_tokens"] = 50;
    json["stop"] = QJsonArray{";", "}", "{"};

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();
    qDebug() << "Sending request to OpenAI API...";
    qDebug() << "Request URL:" << request.url().toString();
    qDebug() << "Request data:" << QString::fromUtf8(data);
    
    networkManager->post(request, data);
}

void CompletionWidget::handleNetworkReply(QNetworkReply *reply)
{
    QString response = reply->readAll();
    qDebug() << "Response from OpenAI API:";
    qDebug() << "Status code:" << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "Response:" << response;

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(response.toUtf8());
        QJsonObject obj = doc.object();
        
        if (obj.contains("choices") && obj["choices"].isArray()) {
            QString suggestion = obj["choices"].toArray().first()
                                  .toObject()["message"].toObject()["content"].toString();
            if (!suggestion.isEmpty()) {
                showCompletion(suggestion.trimmed());
            }
        }
    } else {
        qDebug() << "Network error:" << reply->errorString();
        qDebug() << "Error details:" << response;
    }
    reply->deleteLater();
}

void CompletionWidget::setModel(const QString &newModel)
{
    if (AVAILABLE_MODELS.contains(newModel) && model != newModel) {
        model = newModel;
        emit modelChanged(model);
    }
}

QString CompletionWidget::currentModel() const
{
    return model;
}

bool CompletionWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == editor) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
            
            if (isVisible()) {
                if (keyEvent->key() == Qt::Key_Tab) {
                    // Accept completion
                    if (!completion.isEmpty()) {
                        QTextCursor cursor = editor->textCursor();
                        cursor.insertText(completion);
                        hideCompletion();
                    }
                    return true;
                } else if (keyEvent->key() == Qt::Key_Escape) {
                    // Cancel completion
                    hideCompletion();
                    return true;
                } else {
                    // Hide completion on any other key press
                    hideCompletion();
                }
            }
            
            // Reset and restart completion timer on relevant keys
            completionTimer->stop();
            if (keyEvent->key() == Qt::Key_Space || 
                keyEvent->key() == Qt::Key_Return || 
                keyEvent->key() == Qt::Key_Period ||
                keyEvent->key() == Qt::Key_Greater ||
                keyEvent->key() == Qt::Key_Colon) {
                completionTimer->start(COMPLETION_DELAY);
            }
        }
    }
    return QFrame::eventFilter(obj, event);
}
