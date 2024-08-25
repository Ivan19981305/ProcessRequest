#include <windows.h>
#include <queue>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <iostream> 
#include <chrono> 
#include <random> 

class Request {
public:
  int id; // Идентификатор запроса
  Request(int id) : id(id) {} 
};

Request* GetRequest() throw() {
  static int idCounter = 0; // Счетчик для генерации ID запросов

  // Имитация получения запроса из какого-либо источника
  // (например, чтение из файла, получение по сети)
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(1, 10); // Генерируем случайное число от 1 до 10

  // С вероятностью 1/10 возвращаем nullptr (для завершения)
  if (distrib(gen) == 1) {
    return nullptr;
  } 

  // Иначе создаем новый запрос и возвращаем указатель на него
  return new Request(idCounter++); 
}

void ProcessRequest(Request* request) throw() {
  // Имитация обработки запроса (например, сложные вычисления)
  std::cout << "Обрабатывается запрос с ID: " << request->id << std::endl;
  std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Имитация длительной операции
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