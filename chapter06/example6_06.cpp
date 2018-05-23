//使用细粒度锁的线程安全队列
template <typename T>
class threadsafe_queue
{
private:
	struct node
	{
		std::shared_ptr<T> data;
		std::unique_ptr<node> next;
	};

	std::mutex head_mutex;
	std::unique_ptr<node> head;
	std::mutex tail_mutex;
	node* tail;

	node* get_tail()
	{
		std::lock_guard<std::mutex> tail_lock(tail_mutex);
		return tail;
	}

	std::unique_ptr<node> pop_head()
	{
		std::lock_guard<std::mutex> head_lock(head_mutex);

		if(head.get()==get_tail())
		{
			return nullptr;
		}
		std::unique_ptr<node> old_head=std::move(head);
		head=std::move(old_head->next);
		return old_head;
	}
	
public:
	threadsafe_queue();
};