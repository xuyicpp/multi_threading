# multi_threading
《C++并发编程实战》的读书笔记，供以后工作中查阅。
## 第一章
- 何谓并发和多线程

并发：单个系统里同时执行多个独立的活动。

多线程：每个线程相互独立运行，且每个线程可以运行不同的指令序列。但进程中所有线程都共享相同的地址空间，并且从所有的线程中访问到大部分数据。

- 为什么要在应用程序中使用并发和多线程

关注点分离（DVD程序逻辑分离）和性能（加快程序运行速度）

- 一个简单的C++多线程程序是怎么样的

[清单1.1 一个简单的Hello,Cuncurrent World程序](https://github.com/xuyicpp/multi_threading/blob/master/chapter01/example1_1.cpp)

## 第二章
- 启动线程，以及让各种代码在新线程上运行的方法

多线程在分离detach的时候，离开局部函数后，会在后台持续运行，直到程序结束。如果仍然需要访问局部函数的变量（就会造成悬空引用的错误）。
[清单2.1 当线程仍然访问局部变量时返回的函数](https://github.com/xuyicpp/multi_threading/blob/master/chapter02/example2_1.cpp)
解决上述错误的一个常见的方式，使函数自包含，并且把数据复制到该线程中而不是共享数据。

std::thread是支持移动的，如同std::unique_ptr是可移动的，而非可复制的。以下是两个转移thread控制权的例子
[清单2.5 从函数中返回std::thread,控制权从函数中转移出](https://github.com/xuyicpp/multi_threading/blob/master/chapter02/example2_5.cpp)、[清单2.6 scoped_thread和示例用法,一旦所有权转移到该对象其他线程就不就可以动它了，保证退出一个作用域线程完成](https://github.com/xuyicpp/multi_threading/blob/master/chapter02/example2_6.cpp)
- 等待线程完成并让它自动运行

在当前线程的执行到达f末尾时，局部对象会按照构造函数的逆序被销毁，因此，thread_guard对象g首先被销毁。所以使用thread_guard类可以保证std::thread对象被销毁前，在thread_guard析构函数中调用join。
[清单2.3 使用RAII等待线程完成](https://github.com/xuyicpp/multi_threading/blob/master/chapter02/example2_3.cpp)

- 唯一地标识线程

线程标识符是std::thread::id类型的
1.通过与之相关联的std::thread对象中调用get_id()。
2.当前线程的标识符可以调用std::this_thread::get_id()获得。

## 第三章

- 线程间共享数据的问题

所有线程间共享数据的问题，都是修改数据导致的（竞争条件）。如果所有的共享数据都是只读的，就没问题，因为一个线程所读取的数据不受另一个线程是否正在读取相同的数据而影响。

避免有问题的竞争条件
1.用保护机制封装你的数据结构，以确保只有实际执行修改的线程能够在不变量损坏的地方看到中间数据。
2.修改数据结构的设计及其不变量，从而令修改作为一系列不可分割的变更来完成，每个修改均保留其不变量。者通常被称为无锁编程，且难以尽善尽美。

- 用互斥元保护数据
在[清单3.1 用互斥元保护列表](https://github.com/xuyicpp/multi_threading/blob/master/chapter03/example3_1.cpp)中，有一个全局变量，它被相应的std::mutex的全局实例保护。在add_to_list()以及list_contains()中对std::lock_guard<std::mutex>的使用意味着这些函数中的访问是互斥的list_contains()将无法再add_to_list()进行修改的半途看到该表。

注意：一个迷路的指针或引用，所有的保护都将白费。在[清单3.2 意外地传出对受保护数据的引用](https://github.com/xuyicpp/multi_threading/blob/master/chapter03/example3_2.cpp)展示了这一个错误的做法。

发现接口中固有的竞争条件，这是一个粒度锁定的问题，就是说锁定从语句上升到接口了，书中用一个stack类做了一个扩展，详见[清单3.5 一个线程安全栈的详细类定义](https://github.com/xuyicpp/multi_threading/blob/master/chapter03/example3_5.cpp)

死锁：问题和解决方案:为了避免死锁，常见的建议是始终使用相同的顺序锁定者两个互斥元。
std::lock函数可以同时锁定两个或更多的互斥元，而没有死锁的风险。
常见的思路：
- 避免嵌套锁
- 在持有锁时，避免调用用户提供的代码
- 以固定顺序获取锁
这里有几个简单的事例：[清单3.7 使用锁层次来避免死锁](https://github.com/xuyicpp/multi_threading/blob/master/chapter03/example3_7.cpp)、[清单3.9 用std::unique_lock灵活锁定](https://github.com/xuyicpp/multi_threading/blob/master/chapter03/example3_9.cpp)

锁定在恰当的粒度
特别的，在持有锁时，不要做任何耗时的活动，比如文件的I/O。
一般情况下，只应该以执行要求的操作所需的最小可能时间而去持有锁。这也意味着耗时的操作，比如获取获取另一个锁（即便你知道它不会死锁）或是等待I/O完成，都不应该在持有锁的时候去做，除非绝对必要。
在[清单3.10 在比较运算符中每次锁定一个互斥元](https://github.com/xuyicpp/multi_threading/blob/master/chapter03/example3_10.cpp)虽然减少了持有锁的时间，但是也暴露在竞争条件中去了。

- 用于保护共享数据的替代工具
二次检测锁定模式，注意这个和单例模式中的饱汉模式不一样，它后面有对数据的使用
```
void undefined_behaviour_with_double_checked_locking()
{
	if(!resource_ptr)
	{
		std::lock_guard<std::mutex> lk(resource_mutex);
		if(!resource_ptr)
		{
			resoutce_ptr.reset(new some_resource);
		}
	}
	resource_ptr->do_something();
}
```
它有可能产生恶劣的竞争条件，因为在锁外部的读取与锁内部由另一线程完成的写入不同步。这就因此创建了一个竞争条件，不仅涵盖了指针本身，还涵盖了指向的对象。

C++标准库提供了std::once_flag和std::call_once来处理这种情况。使用std::call_once比显示使用互斥元通常会由更低的开销，特别是初始化已经完成的时候，应优先使用。[清单3.12 使用std::call_once的线程安全的类成员延迟初始化](https://github.com/xuyicpp/multi_threading/blob/master/chapter03/example3_12.cpp)

保护很少更新的数据结构：例如DNS缓存，使用读写互斥元：单个“写”线程独占访问或共享，由多个“读”线程并发访问。
[清单3.13 使用boost::share_mutex保护数据结构](https://github.com/xuyicpp/multi_threading/blob/master/chapter03/example3_13.cpp)