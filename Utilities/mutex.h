#pragma once

#include "types.h"
#include "Atomic.h"


typedef struct lock {
	void *Ptr;
} LOCK, *PLOCK;

// Shared mutex.
class shared_mutex final
{
	LOCK m_lock = { 0 };

public:
	constexpr shared_mutex() = default;

	bool try_lock_shared();
	void lock_shared();
	void unlock_shared();
	bool try_lock();
	void lock();
	void unlock();
	void lock_upgrade();
	bool is_lockable();
#if 0
	bool try_lock_upgrade();

	void lock_upgrade()
	{
		if (!m_value.compare_and_swap_test(c_one - c_min, 0))
		{
			imp_lock_upgrade();
		}
	}

	bool try_lock_degrade();

	void lock_degrade()
	{
		if (!m_value.compare_and_swap_test(0, c_one - c_min))
		{
			imp_lock_degrade();
		}
	}

	bool is_reading() const
	{
		return (m_value.load() % c_one) != 0;
	}

	bool is_lockable() const
	{
		return m_value.load() >= c_min;
	}
#endif
};

// Simplified shared (reader) lock implementation.
class reader_lock final
{
	shared_mutex& m_mutex;
	bool m_upgraded = false;

	void lock()
	{
		m_upgraded ? m_mutex.lock() : m_mutex.lock_shared();
	}

	void unlock()
	{
		m_upgraded ? m_mutex.unlock() : m_mutex.unlock_shared();
	}

	friend class cond_variable;

public:
	reader_lock(const reader_lock&) = delete;

	explicit reader_lock(shared_mutex& mutex)
		: m_mutex(mutex)
	{
		lock();
	}

	// One-way lock upgrade
	void upgrade()
	{
		if (!m_upgraded)
		{
			m_mutex.lock_upgrade();
			m_upgraded = true;
		}
	}

	~reader_lock()
	{
		unlock();
	}
};

// Simplified exclusive (writer) lock implementation.
class writer_lock final
{
	shared_mutex& m_mutex;

	void lock()
	{
		m_mutex.lock();
	}

	void unlock()
	{
		m_mutex.unlock();
	}

	friend class cond_variable;

public:
	writer_lock(const writer_lock&) = delete;

	explicit writer_lock(shared_mutex& mutex)
		: m_mutex(mutex)
	{
		lock();
	}

	~writer_lock()
	{
		unlock();
	}
};

// Safe reader lock. Can be recursive above other safe locks (reader or writer).
class safe_reader_lock final
{
	shared_mutex& m_mutex;
	bool m_is_owned;

	void lock()
	{
		m_mutex.lock_shared();
	}

	void unlock()
	{
		m_mutex.unlock_shared();
	}

	friend class cond_variable;

public:
	safe_reader_lock(const safe_reader_lock&) = delete;

	explicit safe_reader_lock(shared_mutex& mutex);

	~safe_reader_lock();
};

// Safe writer lock. Can be recursive above other safe locks. Performs upgrade and degrade operations above existing reader lock if necessary.
class safe_writer_lock final
{
	shared_mutex& m_mutex;
	bool m_is_owned;
	bool m_is_upgraded;

	void lock()
	{
		m_mutex.lock();
	}

	void unlock()
	{
		m_mutex.unlock();
	}

	friend class cond_variable;

public:
	safe_writer_lock(const safe_writer_lock&) = delete;

	explicit safe_writer_lock(shared_mutex& mutex);

	~safe_writer_lock();
};
