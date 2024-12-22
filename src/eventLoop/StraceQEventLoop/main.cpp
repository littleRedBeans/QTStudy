#include <QCoreApplication>
#include <QEventLoop>
#include <QObject>
#include <QThread>
#include <QTimer>
#include "Logger.h"
#include "Worker1.h"
#include <unistd.h>
int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    printf("main threadID: %d\n", getpid());

    //简化日志类
    Logger logger;

    //创建worker子线程
    QThread *worker1Thread = new QThread;
    worker1Thread->setObjectName("worker1");
    Worker1 worker1;
    worker1.moveToThread(worker1Thread);
    QObject::connect(&worker1, &Worker1::sigLogText, &logger, &Logger::onLogText);

    //启动子线程
    worker1Thread->start();

    //睡眠一会，方便调用strace跟踪子线程
    QThread::sleep(40);

    //在子线程调用worker1.logText
    QTimer::singleShot(0, &worker1, [&]() { worker1.logText("send some data."); });

    //开启事件循环
    QEventLoop loop;
    loop.exec();
    return 0;
}
