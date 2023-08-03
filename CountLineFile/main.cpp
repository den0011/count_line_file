#include <QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QThreadPool>
#include <QtConcurrent/QtConcurrent>
#include <QFuture>
#include <QFutureWatcher>



qint64 findLineStart(QFile& file, qint64 pos)
{
    // Перемещаем позицию в начало текущей строки
    QTextStream stream(&file);
    stream.seek(pos);
    stream.readLine();
    return stream.pos();
}

qint64 findLineEnd(QFile& file, qint64 pos)
{
    // Перемещаем позицию в конец текущей строки
    QTextStream stream(&file);
    stream.seek(pos);
    while (!stream.atEnd() && stream.read(1) != "\n")
    {
        // Пропускаем символы, пока не найдем символ новой строки
    }
    return stream.pos();
}

int countLinesInFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug() << "Failed to open the file: " << filePath;
        return -1; // Return -1 to indicate an error.
    }

    int numThreads = QThreadPool::globalInstance()->maxThreadCount();  // Получаем количество доступных потоков
    qint64 fileSize = file.size();
    qint64 chunkSize = fileSize / numThreads;
    QVector<QFuture<int>> futures;
    QVector<QFutureWatcher<int>*> watchers;     // Используем указатель на QFutureWatche (QFutureWatcher нельзя копировать)

    // Обрабатываем каждый чанк в отдельном потоке
    for (int i = 0; i < numThreads; ++i)
    {
        qint64 start = i * chunkSize;
        qint64 end = (i == numThreads - 1) ? fileSize : start + chunkSize;


        // Регулируем начальную и конечную позиции, чтобы включить первую и последнюю строку каждого фрагмента

        if (i > 0)
            start = findLineStart(file, start);
        if (i < numThreads - 1)
            end = findLineEnd(file, end);

        QFuture<int> future = QtConcurrent::run([filePath, start, end]() {
            QFile file(filePath);
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
                return 0;

            QTextStream stream(&file);
            stream.seek(start);

            int lineCount = 0;
            while (stream.pos() < end && !stream.atEnd())
            {
                stream.readLine();
                lineCount++;
            }

            return lineCount;
        });

        futures.append(future);
        QFutureWatcher<int>* watcher = new QFutureWatcher<int>();
        watchers.append(watcher);
        watcher->setFuture(future);
    }


    // Ждем завершения всех потоков
    for (const auto& watcher : watchers)
        watcher->waitForFinished();

    // Суммируем результаты всех потоков
    int totalLines = 0;
    for (const auto& future : futures)
        totalLines += future.result();


    // Удаляем объекты QFutureWatcher, чтобы избежать утечек памяти
    for (const auto& watcher : watchers)
    {
        watcher->deleteLater();
    }

    file.close();
    return totalLines;
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    // Устанавливаем обработчик вывода для qDebug, чтобы вывод шел в консоль
    qInstallMessageHandler([](QtMsgType type, const QMessageLogContext& context, const QString& msg) {
        Q_UNUSED(context)

        QByteArray localMsg = msg.toLocal8Bit();
        switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s\n", localMsg.constData());
            break;
        case QtInfoMsg:
            fprintf(stderr, "Info: %s\n", localMsg.constData());
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s\n", localMsg.constData());
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s\n", localMsg.constData());
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s\n", localMsg.constData());
            abort();
        }
    });

    QString filePath = "D:/1.log";

    int numLines = countLinesInFile(filePath);

    if (numLines >= 0)
    {
        qDebug() << "Number of lines in the file: " << numLines;
    }
    else
    {
        qDebug() << "Error occurred while reading the file.";
    }


    return a.exec();
}
