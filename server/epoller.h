/**
 * @author peizhic
 * 
 * Utilize IO multiplexing (Epoll) and thread pool to 
 * implement a Reactor model.
*/

#ifndef EPOLLER_H
#define EPOLLER_H

#include <sys/epoll.h> // epoll_ctl()
#include <fcntl.h> // fcntl()
#include <unistd.h> // close()
#include <assert.h>
#include <vector>
#include <errno.h>

class Epoller {
public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    bool addFd(int fd, uint32_t events);
    bool modFd(int fd, uint32_t events);
    bool delFd(int fd);
    int wait(int timewait = -1); // return num of IO ready fds

    int getEventFd(size_t i) const;
    uint32_t getEvents(size_t i) const;

private:
    int epollerFd; // fd of the epoll
    std::vector<struct epoll_event> events; // stock fds(listenfd + connfds)
};

#endif