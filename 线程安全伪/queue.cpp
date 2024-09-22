#include <iostream>
#include <queue>
#include <mutex>
#include <thread>
#include <condition_variable>

template<typename T>
class threadsafe_queue{
private:
    mutable std::mutex mtx;
    std::queue<T> data_queue;
    std::condition_variable data_cond;
public:
    threadsafe_queue(){}
    void push(T new_value){
        std::lock_guard<std::mutex> lk(mtx);
        data_queue.push(std::move(new_value));
        data_cond.notify_one();
    }
    void wait_and_pop(T & value){
        std::lock_guard<std::mutex> lk(mtx);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        value=std::move(data_queue.front());
        data_queue.pop();
    }
    std::shared_ptr<T> wait_and_pop(){
        std::lock_guard<std::mutex> lk(mtx);
        data_cond.wait(lk,[this]{return !data_queue.empty();});
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }
    bool try_pop(T& value){
        std::lock_guard<std::mutex> lk(mtx);
        if (!data_queue.empty())
            return false;
        value=std::move(data_queue.front());
        data_queue.pop();
        return true;
    }
    std::shared_ptr<T> try_pop(){
        std::lock_guard<std::mutex> lk(mtx);
        if (data_queue.empty())
            return std::shared_ptr<T>();
        std::shared_ptr<T> res(std::make_shared<T>(std::move(data_queue.front())));
        data_queue.pop();
        return res;
    }
    bool empty() const{
        std::lock_guard<std::mutex> lk(mtx);
        return data_queue;
    }
};


int main(){  
    std::cout<<"hello world"<<std::endl;
    return 0;
}