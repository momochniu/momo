#ifndef DELAY_TASK_MANAGER_H
#define DELAY_TASK_MANAGER_H

#include <map>
#include "hv/EventLoopThread.h"

namespace momo{

template<typename T, size_t DELAY_TIME_MS>
class DelayTaskManager {
public:
    DelayTaskManager(){
        m_loopThread.start();
    }

    ~DelayTaskManager(){
        m_loopThread.stop();
        m_loopThread.join();
    };
    
    void addUniqueTask(T key, hv::EventLoop::Functor fn){
        m_loopThread.loop()->queueInLoop([this, key, fn](){
            auto it = m_mapDelayTimer.find(key);
            if(it == m_mapDelayTimer.end()){
                auto timerID = m_loopThread.loop()->setTimeout(DELAY_TIME_MS, [this, key, fn](hv::TimerID){
                    fn();
                    auto it = m_mapDelayTimer.find(key);
                    if(it != m_mapDelayTimer.end()){
                        m_mapDelayTimer.erase(it);
                    }
                });
                m_mapDelayTimer.emplace(key, timerID);
            }else{
                m_loopThread.loop()->resetTimer(it->second, DELAY_TIME_MS);
            }
        });
    }
    
    void addNormalTask(hv::EventLoop::Functor fn){
        m_loopThread.loop()->setTimeout(DELAY_TIME_MS, [fn](hv::TimerID){
            fn();
        });
    }
    
    const hv::EventLoopPtr& loop(){
        return m_loopThread.loop();
    }
    
    DelayTaskManager(const DelayTaskManager&) = delete;
    DelayTaskManager& operator=(const DelayTaskManager&) = delete;
    DelayTaskManager(DelayTaskManager&&) = delete;
    DelayTaskManager& operator=(DelayTaskManager&&) = delete;
private:
    hv::EventLoopThread m_loopThread;
    std::map<T, hv::TimerID> m_mapDelayTimer;
};


}
#endif

