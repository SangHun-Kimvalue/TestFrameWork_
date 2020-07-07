#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

template <typename T>
class SharedQueue
{
public:
	SharedQueue(int max_size);
	~SharedQueue();

	T& pop();

	typename std::deque<T>::iterator begin();
	typename std::deque<T>::iterator end();

	bool push_back(const T& item);
	bool push_back(T&& item);

	int size();
	int max_size();
	bool empty();
	bool clear();
	bool full();
	void shrink_to_fit();

	typedef typename std::deque<T>::iterator iterator;

private:
	T& front();
	void pop_front();

private:
	std::deque<T> queue_;
	std::mutex mutex_;
	std::condition_variable cond_;

private:
	int max_size_;
};

template <typename T>
SharedQueue<T>::SharedQueue(int max_size) { max_size_ = max_size; }

template <typename T>
SharedQueue<T>::~SharedQueue() {}

template <typename T>
T& SharedQueue<T>::front()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty())
	{
		cond_.wait(mlock);
	}

	return queue_.front();
}

template <typename T>
void SharedQueue<T>::pop_front()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty())
	{
		cond_.wait(mlock);
	}
	queue_.pop_front();
}

template <typename T>
T& SharedQueue<T>::pop()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty())
	{
		cond_.wait(mlock);
	}
	T& item = queue_.front();

	queue_.pop_front();

	return item;
}

template <typename T>
bool SharedQueue<T>::push_back(const T& item)
{
	bool ret = false;
	std::unique_lock<std::mutex> mlock(mutex_);
	if ((int)queue_.size() < max_size_)
	{
		queue_.push_back(item);
		ret = true;
	}
	mlock.unlock();     // unlock before notificiation to minimize mutex con
	cond_.notify_one(); // notify one waiting thread

	return ret;
}

template <typename T>
bool SharedQueue<T>::push_back(T&& item)
{
	bool ret = false;
	std::unique_lock<std::mutex> mlock(mutex_);
	if ((int)queue_.size() < max_size_)
	{
		queue_.push_back(std::move(item));
		ret = true;
	}
	mlock.unlock();     // unlock before notificiation to minimize mutex con
	cond_.notify_one(); // notify one waiting thread

	return ret;
}

template <typename T>
int SharedQueue<T>::size()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	int size = (int)queue_.size();
	mlock.unlock();
	return size;
}

template <typename T>
int SharedQueue<T>::max_size()
{
	return max_size_;
}

template <typename T>
bool SharedQueue<T>::empty()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	bool size = queue_.empty();
	mlock.unlock();
	return size;
}

template <typename T>
bool SharedQueue<T>::clear()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	bool size = queue_.empty();
	queue_.empty();
	mlock.unlock();
	return size;
}

template <typename T>
bool SharedQueue<T>::full()
{
	bool ret = false;
	std::unique_lock<std::mutex> mlock(mutex_);
	if ((int)queue_.size() == max_size_)
	{
		ret = true;
	}
	mlock.unlock();
	return ret;
}

template <typename T>
typename std::deque<T>::iterator SharedQueue<T>::begin()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty())
	{
		cond_.wait(mlock);
	}

	return queue_.begin();
}

template <typename T>
typename std::deque<T>::iterator SharedQueue<T>::end()
{
	std::unique_lock<std::mutex> mlock(mutex_);
	while (queue_.empty())
	{
		cond_.wait(mlock);
	}

	return queue_.end();
}

template <typename T>
void SharedQueue<T>::shrink_to_fit()
{
	queue_.shrink_to_fit();
}
