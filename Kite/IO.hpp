#ifndef KITE_IO_HPP_SKMD
#define KITE_IO_HPP_SKMD

namespace Kite {
    class IO {
    public:
        virtual ssize_t read(char *buf, int len) = 0;
        virtual ssize_t write(const char *buf, int len) = 0;
    };
};


#endif


