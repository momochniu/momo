#ifndef CONCURRENCY_UTILS
#define CONCURRENCY_UTILS

namespace momo{

template<typename T, typename U, size_t POOL_SIZE>
std::vector<std::vector<T>> convert_mt(const std::vector<U> &src, function<std::vector<T> (const U &u)> fn){
    static_assert(POOL_SIZE >= 2 && POOL_SIZE <= 8, "POOL_SIZE should between 2 and 8.");
    int taskCount = src.size();
    std::vector<std::vector<T>> dst;
    dst.resize(taskCount);  //notice: resize
    hv::EventLoopThreadPool pool(POOL_SIZE);
    pool.start();
    CountDownLatch latch(POOL_SIZE);
    atomic_int incTaskIndex = 0;
    for(int i = 0; i < POOL_SIZE; ++i){
        pool.loop(i)->queueInLoop([&dst, &src, fn](){
            while(true){
                int taskIndex = incTaskIndex.fetch_add(1);
                if(taskIndex >= taskCount){
                    break;
                }
                vector<T> tmp = fn(src[taskIndex]);
                dst[taskIndex].swap(tmp);
            }
            latch.countdown();
        });
    }
    latch.wait();
    return std::move(dst);
}

}
#endif
