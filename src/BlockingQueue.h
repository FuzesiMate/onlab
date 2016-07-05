/*
 * BlockingQueue.h
 *
 *  Created on: 2016. ápr. 5.
 *      Author: Máté
 */

#ifndef BLOCKINGQUEUE_H_
#define BLOCKINGQUEUE_H_

#include <mutex>
#include <condition_variable>
#include <chrono>
#include <deque>

template<class T>
class BlockingQueue {
public:

	/**
	 * Adds an element to the queue.
	 */
	void push(T const& value);

	/**
	 * Retrieves the next element from the queue without pop
	 */
	T back();

	/**
	 * Retrieves the next element from the queue, or waits for one.
	 */
	T pop();

	/**
	 * Retrieves the next element from the queue, or waits for one for timeout milliseconds.
	 *
	 * @timeout - Time to wait in milliseconds.
	 */
	bool tryPop(long timeout, T& value);

	/**
	 * Returns true if the queue is empty
	 */
	bool empty();

	/**
	 * Returns the size of the queue
	 */
	size_t size();


private:
	mutex mutex_;
	condition_variable condition_;
	std::deque<T> queue_;
};

template<class T>
inline void BlockingQueue<T>::push(T const& value) {
	{
		std::unique_lock<mutex> lock(mutex_);
		queue_.push_front(value);
	}
	condition_.notify_one();
}

template <class T>
inline bool BlockingQueue<T>::empty(){
	std::unique_lock<mutex> lock(mutex_);
	return queue_.empty();
}

template <class T>
inline size_t BlockingQueue<T>::size(){
	std::unique_lock<mutex> lock(mutex_);
	return queue_.size();
}

template<class T>
inline T BlockingQueue<T>::back(){
	std::unique_lock<mutex> lock(mutex_);
	condition_.wait(lock, [&]{return !queue_.empty();});
	T rc(std::move(queue_.back()));
	return rc;
}

template<class T>
inline T BlockingQueue<T>::pop() {
	std::unique_lock<mutex> lock(mutex_);
	condition_.wait(lock, [&]{return !queue_.empty();});
	T rc(std::move(queue_.back()));
	queue_.pop_back();
	return rc;
}

template<class T>
inline bool BlockingQueue<T>::tryPop(long timeout, T& value) {
	std::unique_lock<mutex> lock(mutex_);
	if(!condition_.wait_for(lock, std::chrono::milliseconds(timeout), [&]{return !queue_.empty();}))
		return false;

	value = queue_.back();
	queue_.pop_back();

	return true;
}




#endif /* BLOCKINGQUEUE_H_ */
