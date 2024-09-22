#include <iostream>
#include <stack>
#include <mutex>
#include <thread>
struct empty_stack: std::exception{
    const char * what () const throw();
};

template <typename T>
class threadsafe_stack{
private:
    std::stack<T> data;
    mutable std::mutex m;
public:
    threadsafe_stack(){}
    threadsafe_stack(const threadsafe_stack& other){
        std::lock_guard<std::mutex> lock (other.m);
    }
    threadsafe_stack & operator= (const threadsafe_stack &) =delete;
    void push(T new_value){
        std::lock_guard<std::mutex> lock(m);
        data.push(std::move(new_value));//1 可能会抛出异常，由于复制移动数据的时候会出现异常等，但是stack<>能够保证对应的安全
    }
    std::shared_ptr<T> pop(){
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();//2 可能会跑出empty_stack的异常，但是不会发生任何的数据改动，因此是安全的抛出行为
        std::shared_ptr<T> const res(
         std::make_shared<T>(std::move(data.top())));//3 类似于1，不会出现内存问题
        data.pop();//stack保证了安全
        return res;
    }
    void pop(T& value){
        std::lock_guard<std::mutex> lock(m);
        if (data.empty()) throw empty_stack();
        value = std::move(data.top());//5 拷贝赋值操作符或移动赋值操作符 ，可能能会抛出异常
        data.top();
    }
    bool empty() const{
        std::lock_guard<std::mutex> lock(m);
        return data.empty();
    }
};
//线程一旦为了获得锁而等待那么多线程就被迫串行化
int main (){
    std::cout<<"hello world"<<std::endl;
    return 0;
}