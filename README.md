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

## 第4章 同步并发操作
- 等待事件

使用C++标准库提供的工具来等待事件本身。std::condition_variable的std::condition_variable_any，后者可以与任何互斥元一起工作，所以有额外代价的可能。
std::condition_variable可以调用notify_one()和notify_all()。然后std::condition_variable还可以wait(lk,[this]{return !data_queue.empty();}),这里的lk是unique_lock方便后面条件不满足的时候解锁，满足时开锁。
[清单4.1 使用std::condition_variable等待数据](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_01.cpp)

使用条件变量建立一个线程安全队列：[清单4.2 std::queue接口](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_02.cpp)、[清单4.4 从清单4.1中提取push()和wait_and_pop()](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_04.cpp)。

- 使用future来等待一次性事件

在一个线程不需要立刻得到结果的时候，你可以使用std::async来启动一个异步任务。std::async返回一个std::future对象，而不是给你一个std::thread对象让你在上面等待，std::future对象最终将持有函数的返回值，当你需要这个值时，只要在future上调用get(),线程就会阻塞知道future就绪，然后返回该值。
[清单4.6 使用std::future获取异步任务的返回值](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_06.cpp)

std::async允许你通过将额外的参数添加到调用中，来将附加参数传递给函数，这与std::thread是同样的方式。
[清单4.7 使用std::async来将参数传递给函数](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_07.cpp)

std::packaged_task<>将一个future绑定到一个函数或可调用对象上。当std::packaged_task<>对象被调用时，它就调用相关联的函数或可调用对象，并且让future就绪，将返回值作为关联数据存储。
[清单4.9 使用std::packaged_task在GUI线程上运行代码](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_09.cpp)

std::promise<T>提供一种设置值（类型T）方式，它可以在这之后通过相关联的std::future<T>对象进行读取。
[清单4.10 使用promise在单个线程中处理多个链接](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_10.cpp)，这个有点像select,或者poll。

同时，还要为future保存异常，以及使用share_future等待来自多个线程。

- 有时间限制的等待

1.基于时间段的超时。2.基于时间点的超时。
[清单4.11 等待一个具有超时的条件变量](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_11.cpp)

- 使用操作的同步来简化代码

解决同步问题的范式，函数式编程，其中每个任务产生的结果完全依赖于它的输入而不是外部环境，以及消息传递，ATM状态机，线程通信通过状态发送一部消息来实现的。
[清单4.13 使用future的并行快速排序](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_13.cpp)、
[清单4.15 ATM逻辑类的简单实现](https://github.com/xuyicpp/multi_threading/blob/master/chapter04/example4_15.cpp)。

## 第5章 C++内存模型和原子类型上操作
 
本章介绍了C++11内存模型的底层细节，以及在线程间提供同步基础的原子操作。这包括了由std::atomic<>类模板的特化提供的基本原子类型，由std::atomic<>主模板提供的泛型原子接口，在这些类型上的操作，以及各种内存顺序选项的复杂细节。
我们还看了屏障，以及它们如何通过原子类型上的操作配对，以强制顺序。最后，我们回到开头，看了看原子操作是如何用来在独立线程上的非原子操作之间强制顺序的。

在原子类型上的每一个操作均具有一个可选的内存顺序参数，它可以用来指定所需的内存顺序语义。
- 存储(store)操作，可以包括memory_order_relaxed、memory_order_release或memory_order_seq_cst顺序。
- 载入(load)操作，可以包括memory_order_relaxed、memory_order_consume、memory_order_acquire或memory_order_seq_cst顺序。
- 读-修改-写(read-modify-write)操作，可以包括memory_order_relaxed、memory_order_consume、memory_order_acquire、memory_order_release、memory_order_acq_rel或memory_order_seq_cst顺序。

所有操作的默认顺序为memory_order_seq_cst。

原子操作的内存顺序的三种模型：
- 顺序一致顺序(sequentially consistent):(memory_order_seq_cst):[清单5.4 顺序一致隐含着总体顺序](https://github.com/xuyicpp/multi_threading/blob/master/chapter05/example5_04.cpp)。
- 松散顺序(relaxed):(memory_order_relaxed):[清单5.6 多线程的松散操作](https://github.com/xuyicpp/multi_threading/blob/master/chapter05/example5_06.cpp)。
- 获取-释放顺序(acquire-release):(memory_order_consume、memory_order_acquire、memory_order_release和memory_order_acq_rel):[清单5.9 使用获取和释放顺序的传递性同步](https://github.com/xuyicpp/multi_threading/blob/master/chapter05/example5_09.cpp)、[清单5.10 使用std::memory_order_consume同步数据(原子载入操作指向某数据的指针)](https://github.com/xuyicpp/multi_threading/blob/master/chapter05/example5_10.cpp)

synchronizes-with(与同步):
- 在原子变量的载入和来自另一个线程的对该原子变量的载入之间，建立一个synchronizes-with关系，[清单5.11 使用原子操作从队列中读取值](https://github.com/xuyicpp/multi_threading/blob/master/chapter05/example5_11.cpp)
- 在一个线程中释放屏障，在另一个线程中获取屏障，从而实现synchronizes-with关系，[清单5.12 松散操作可以使用屏障来排序](https://github.com/xuyicpp/multi_threading/blob/master/chapter05/example5_12.cpp)

happens-before(发生于之前):传递性：如果A线程发生于B线程之前，并且B线程发生于C之前，则A线程间发生于C之前。
- [清单5.8 获取-释放操作可以在松散操作中施加顺序](https://github.com/xuyicpp/multi_threading/blob/master/chapter05/example5_08.cpp)
- [清单5.13 在非原子操作上强制顺序](https://github.com/xuyicpp/multi_threading/blob/master/chapter05/example5_13.cpp)