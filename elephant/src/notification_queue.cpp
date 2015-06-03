#include <Poco/Notification.h>
#include <Poco/NotificationQueue.h>
#include <Poco/ThreadPool.h>
#include <Poco/Runnable.h>
#include <Poco/AutoPtr.h>
#include <iostream>

using namespace std;
using namespace Poco;

class WorkNotification: public Notification
{
public:
  WorkNotification (int data) : _data(data) {};

  int data()
  {
    return _data;
  }

private:
  int _data;
};

class Worker : public Runnable
{
public:
  Worker (NotificationQueue& queue) : _queue(queue) {};

  void run()
  {
    AutoPtr<Notification> pNf(_queue.waitDequeueNotification());
    while (pNf) {
      WorkNotification * pWorkNf = dynamic_cast<WorkNotification*>(pNf.get());
      if (pWorkNf) {
        cout << "data: " << pWorkNf->data() << endl;
      }
      pNf = _queue.waitDequeueNotification();
    }
  }

private:
  NotificationQueue& _queue;
};

int main(int argc, char *argv[])
{
  NotificationQueue queue;
  Worker worker1(queue);
  Worker worker2(queue);

  ThreadPool::defaultPool().start(worker1);
  ThreadPool::defaultPool().start(worker2);

  for (int i = 0; i < 100; ++i) {
    queue.enqueueNotification(new WorkNotification(i));
  }

  while(!queue.empty()) {
    Poco::Thread::sleep(100);
  }

  queue.wakeUpAll();
  ThreadPool::defaultPool().joinAll();

  return 0;
}
