#pragma once

template <class T>
class scoped_ptr
{
public:
	scoped_ptr()
		: ptr_(NULL)
	{
	}
	scoped_ptr(T* t)
		: ptr_(NULL)
	{
		reset(t)
	}
	~scoped_ptr()
	{
		Clear();
	}
	void reset(T* t = NULL)
	{
		Clear();
		ptr_ = t;
	}
	void Clear()
	{
		if (ptr_)
			delete ptr_;
		ptr_ = NULL;
	}
	T* operator->() const
	{
		return ptr_;
	}
	T* get() const
	{
		return ptr_;
	}

private:
	T* ptr_;
};


template <class T>
class scoped_refptr
{
public:
	scoped_refptr()
		: p_(NULL)
	{
	}
	scoped_refptr(T* t)
	{
		p_ = t;
		if (p_)
			p_->AddRef();
	}
	~scoped_refptr()
	{
		Clear();
	}
	scoped_refptr(const scoped_refptr<T>& r) : p_(r.p_) {
		if (p_)
			p_->AddRef();
	}
	void reset(T* t = NULL)
	{
		Clear();
		p_ = t;
		if (p_)
			p_->AddRef();
	}
	void Clear()
	{
		if (p_)
			p_->Release();
	}
	T* operator->() const
	{
		return p_;
	}
	T* get() const
	{
		return p_;
	}


private:
	T* p_;
};