#ifndef REQUEST_H
#define REQUEST_H

class Request {
public:
  int id; // Идентификатор запроса
  Request(int id);
  ~Request(); 
};

#endif // REQUEST_H