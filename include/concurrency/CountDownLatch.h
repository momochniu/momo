#ifndef COUNTDOWN_LATCH
#define COUNTDOWN_LATCH

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <string>

namespace momo{

class CountDownLatch{
public:
    CountDownLatch(int count)
        :m_count(count)
    {
        if(count < 0){
            throw std::string("parameter error!");
        }
    }
    ~CountDownLatch(){
        std::lock_guard<std::mutex> lk(m_mtx);
        if(m_count > 0){
            std::abort();
        }
    }
    CountDownLatch(const CountDownLatch &) = delete;
    CountDownLatch& operator=(const CountDownLatch &) = delete;
    CountDownLatch(CountDownLatch &&) = delete;
    CountDownLatch& operator=(CountDownLatch &&) = delete;

    int getCount(){
        return m_count;   
    }

    void wait(){
        if(m_count > 0){
            std::unique_lock<std::mutex> lk(m_mtx);
            m_cv.wait(lk, [this]{
                return m_count <= 0;
            });
            if(m_count < 0){
                throw std::string("Latch Broken");
            }
        }
    }

    void countdown(){
        if(m_count > 0){
            std::lock_guard<std::mutex> lk(m_mtx);
            if(m_count > 0){
                --m_count;
                if(m_count == 0){
                    m_cv.notify_all();
                }
            }
        }
    }
private:
    std::atomic_int m_count;
    std::mutex m_mtx;
    std::condition_variable m_cv;
};

}

#endif
