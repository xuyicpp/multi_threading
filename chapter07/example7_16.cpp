//从使用引用计数tail的无锁队列中将结点出队列
template <typename T>
class lock_free_queue
{
private:
	struct node
	{
		void release_ref();
	};
public:
	lock_free_queue();
	~lock_free_queue();
	
};