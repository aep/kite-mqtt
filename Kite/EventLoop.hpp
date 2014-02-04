#ifndef KITE_EVENTLOOP_HPP_KNMS
#define KITE_EVENTLOOP_HPP_KNMS

#include <map>
#include <memory>
#include <functional>
#include <queue>

namespace Kite  {

    class EventLoop;
    class Evented {
    public:
        Evented(std::weak_ptr<EventLoop> ev);
        virtual ~Evented();

    protected:
        void evAdd(int);
        void evRemove(int);

        std::shared_ptr<EventLoop> ev() const { return p_Ev.lock();}
    private:
        friend class EventLoop;
        virtual void onActivated(int) = 0;
        std::weak_ptr<EventLoop> p_Ev;
    };

    class EventLoop {
    public:
        EventLoop();
        ~EventLoop();
        int exec();
        void later(std::function<void()> fn);
        void exit(int e);
    private:
        friend class Evented;
        std::map <int, Evented*> p_evs;
        bool p_running;
        int  p_exitCode;
        int  p_intp[2];
        std::queue<std::function<void()> > p_lq;
    };
}

#endif
