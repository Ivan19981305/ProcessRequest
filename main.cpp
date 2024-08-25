#include <windows.h>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>

class Request {
  // ... реализовать
};

Request* GetRequest() throw() {
  // ... реализовать
}

void ProcessRequest(Request* request) throw() {
  // ... реализовать
}

const int NumberOfThreads = 2;

std::queue<Request*> taskQueue; // Очередь задач
std::mutex queueMutex; // Мьютекс для синхронизации доступа к очереди
std::condition_variable cv; // Условная переменная для сигнализации потокам
bool stopRequested = false; // Флаг, указывающий на запрос остановки

void workerThread() {
  while (true) {
    Request* request = nullptr;
    {
      std::unique_lock<std::mutex> lock(queueMutex); // Блокируем мьютекс
      cv.wait(lock, [&] { return !taskQueue.empty() || stopRequested; }); // Ждем задачу или сигнал остановки

      if (stopRequested && taskQueue.empty()) {
        break; // Выходим из потока, если запрошена остановка и очередь пуста
      }

      request = taskQueue.front();
      taskQueue.pop();
    }
    
    if (request) {
      ProcessRequest(request); 
      delete request; // Удаляем запрос после обработки
    }
  }
}

int main() {
  // Создаем рабочие потоки
  std::vector<std::thread> workers;
  for (int i = 0; i < NumberOfThreads; ++i) {
    workers.emplace_back(workerThread);
  }

  // Получаем запросы
  Request* request;
  while (request = GetRequest()) { 
    {
      std::lock_guard<std::mutex> lock(queueMutex); // Блокируем мьютекс
      taskQueue.push(request); // Добавляем задачу в очередь
    }
    cv.notify_one(); // Сигнализируем одному из рабочих потоков
  }

  // Сигнализируем рабочим потокам об остановке
  {
    std::lock_guard<std::mutex> lock(queueMutex); // Блокируем мьютекс
    stopRequested = true; // Устанавливаем флаг остановки
  }
  cv.notify_all(); // Будим все рабочие потоки

  // Ждем завершения рабочих потоков
  for (auto& worker : workers) {
    worker.join();
  }

  return 0;
}