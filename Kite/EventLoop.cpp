#include "EventLoop.hpp"
#include <stropts.h>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>

using namespace Kite;

Evented::Evented(std::weak_ptr<EventLoop> ev)
: p_Ev(ev)
{
}

Evented::~Evented()
{
    auto ev = p_Ev.lock();
    auto it = ev->p_evs.begin();
    while (it != ev->p_evs.end()) {
        if (it->second == this)
            ev->p_evs.erase(it++);
        else
            it++;
    }
}

void Evented::evAdd(int fd)
{
    auto ev = p_Ev.lock();
    ev->p_evs.insert(std::make_pair(fd, this));
}

void Evented::evRemove(int fd)
{
    auto ev = p_Ev.lock();
    auto it = ev->p_evs.begin();
    while (it != ev->p_evs.end()) {
        if (it->second == this && it->first == fd)
            ev->p_evs.erase(it++);
        else
            it++;
    }
}


EventLoop::EventLoop()
{
    // wake up pipe to interrupt poll()
    pipe(p_intp);
    fcntl(p_intp[0], F_SETFL, O_NONBLOCK);
}

EventLoop::~EventLoop()
{
    close (p_intp[0]);
    close (p_intp[1]);
}

int EventLoop::exec()
{
    p_running = true;
    while (p_running) {
        int    pollnum = p_evs.size() + 1;
        struct pollfd fds[pollnum];

        fds[0].fd = p_intp[0];
        fds[0].events = POLLIN;

        int i = 1;
        auto it = p_evs.begin();
        while (it != p_evs.end()) {
            fds[i].fd = it->first;
            fds[i].events = POLLIN;
            i++;
            it++;
        }

        int ret =  poll(fds, pollnum, -1);

        i = 1;
        it = p_evs.begin();
        while (it != p_evs.end()) {
            if (fds[i].revents & POLLIN) {
                auto ev = it->second;
                ev->onActivated(fds[i].fd);
            }
            i++;
            it++;
        }

        while (!p_lq.empty()) {
            auto fn = p_lq.front();
            p_lq.pop();
            fn();
        }
    }
    return p_exitCode;
}

void EventLoop::later(std::function<void()> fn)
{
    p_lq.push(fn);
}

void EventLoop::exit(int e)
{
    later([this, e] () {
            p_exitCode = e;
            p_running = false;
    });
    write(p_intp[1], "\n", 1);
}
