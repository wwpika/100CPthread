# 100CPthread
c++100行代码实现线程池

std::packaged_task  包装一个可调用的对象，并且允许异步获取该可调用对象产生的结果;该对象包含两个
基本元素，被包装的任务和共享状态，其中，共享状态用于保存任务的返回值

join 阻塞，等待线程执行完毕

detach 将线程放到后台，不过放到后台你就没法控制它了

std::future 通常由某个 Provider 创建，你可以把 Provider 想象成一个异步任务的提供者

std::async 函数，本文后面会介绍 std::async() 函数

std::promise::get_future，get_future 为 promise 类的成员函数

std::packaged_task::get_future，此时 get_future为 packaged_task 的成员函数

std::result_of 作用是可以在编译的时候推导出一个函数调用表达式的返回值类型

emplace_back 向容器内添加数据

std::function 它是函数的容器

std::future::get() 该方法有阻塞的作用，只要异步线程中任务没有完成，使用该方法后，主线程会一直等待
