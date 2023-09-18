#include "epoller.h"

/**
 * epoll_create(int size) -> create an instance of epoll in kernal
*/
Epoller::Epoller(int maxEvent) : 
    epollerFd(epoll_create(512)), events(maxEvent) {
    assert(epollerFd >= 0 && events.size() > 0);
}

Epoller::~Epoller() {
    close(epollerFd);
}


/**
 * int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event);
 * 
 * 向 epfd 对应的内核 epoll 实例添加、修改或删除对 fd 上事件 
 * event 的监听。op 可以为 EPOLL_CTL_ADD, EPOLL_CTL_MOD, 
 * EPOLL_CTL_DEL 分别对应的是添加新的事件，修改文件描述符上监听的
 * 事件类型，从实例上删除一个事件。如果 event 的 events 
 * 属性设置了 EPOLLET flag，那么监听该事件的方式是边缘触发
*/
bool Epoller::addFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollerFd, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::modFd(int fd, uint32_t events) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollerFd, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::delFd(int fd) {
    if(fd < 0) return false;
    epoll_event ev = {0};
    return 0 == epoll_ctl(epollerFd, EPOLL_CTL_DEL, fd, &ev);
}

int Epoller::wait(int timeoutMs) {
    return epoll_wait(epollerFd, &events[0], static_cast<int>(events.size()), timeoutMs);
}

int Epoller::getEventFd(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].events;
}