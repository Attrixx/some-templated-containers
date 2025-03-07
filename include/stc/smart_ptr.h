#pragma once

#include <type_traits>
#include <cassert>

namespace stc {
	template<typename T>
	struct control_block
	{
		long ref_count;
		long weak_count;
		T* ptr;

		control_block(T* p)
			: ref_count(1), weak_count(0), ptr(p) {
		}

		virtual void reset()
		{
			delete ptr;
		}
	};

	template <typename T>
	struct control_block_ex : public control_block<T>
	{
		T object;

		template<typename... Args>
		control_block_ex(Args&&... args)
			: control_block<T>(&object), object(std::forward<Args>(args)...) {}

		void reset() override
		{
			object.~T();
			this->ptr = nullptr;
		}
	};

	template <typename T>
	class shared_ptr
	{
	private:
		template <typename> friend class weak_ptr;
		control_block<T>* control;

	public:
		explicit shared_ptr(control_block<T>* ctrl = nullptr)
			: control(ctrl) {
		}

		shared_ptr(T* raw_ptr)
			: control(new control_block<T>(raw_ptr))
		{
		}

		shared_ptr(const shared_ptr& other)
			: control(other.control)
		{
			if (control) ++control->ref_count;
		}

		shared_ptr(shared_ptr&& other) noexcept
			: control(other.control)
		{
			other.control = nullptr;
		}

		shared_ptr(const class weak_ptr<T>& weak)
			: control(weak.control)
		{
			if (control && control->ref_count > 0)
			{
				++control->ref_count;
			}
			else
			{
				control = nullptr;
			}
		}

		shared_ptr& operator=(const shared_ptr& other)
		{
			if (this != &other)
			{
				reset();
				control = other.control;
				if (control) ++control->ref_count;
			}
			return *this;
		}

		shared_ptr& operator=(shared_ptr&& other) noexcept
		{
			if (this != &other)
			{
				reset();
				control = other.control;
				other.control = nullptr;
			}
			return *this;
		}

		~shared_ptr() { reset(); }

		void reset()
		{
			if (control)
			{
				if (--control->ref_count == 0)
				{
					control->reset();

					if (control->weak_count == 0)
					{
						delete control;
					}
				}
				control = nullptr;
			}
		}

		T* get() const
		{
			return control ? control->ptr : nullptr;
		}

		T& operator*() const
		{
			assert(control && control->ptr);
			return *control->ptr;
		}

		T* operator->() const
		{
			assert(control && control->ptr);
			return control->ptr;
		}

		int use_count() const
		{
			return control ? control->ref_count : 0;
		}

		explicit operator bool() const
		{
			return control && control->ptr;
		}
	};

	template<typename T>
	class weak_ptr
	{
	private:
		template <typename> friend class shared_ptr;
		control_block<T>* control;

	public:
		weak_ptr() : control(nullptr) {}

		weak_ptr(const shared_ptr<T>& shared)
			: control(shared.control)
		{
			if (control) ++control->weak_count;
		}

		weak_ptr(const weak_ptr& other)
			: control(other.control)
		{
			if (control) ++control->weak_count;
		}

		weak_ptr(weak_ptr&& other) noexcept
			: control(other.control)
		{
			other.control = nullptr;
		}

		weak_ptr& operator=(const weak_ptr& other)
		{
			if (this != &other)
			{
				reset();
				control = other.control;
				if (control) ++control->weak_count;
			}
			return *this;
		}

		weak_ptr& operator=(weak_ptr&& other) noexcept
		{
			if (this != &other)
			{
				reset();
				control = other.control;
				other.control = nullptr;
			}
			return *this;
		}

		~weak_ptr() { reset(); }

		void reset()
		{
			if (control)
			{
				if (--control->weak_count == 0 && control->ref_count == 0)
				{
					delete control;
				}
				control = nullptr;
			}
		}

		int use_count() const
		{
			return control ? control->ref_count : 0;
		}

		bool expired() const
		{
			return !control || control->ref_count == 0;
		}

		shared_ptr<T> lock() const {
			return shared_ptr<T>(*this);
		}
	};

	template<typename T, typename... Args>
	shared_ptr<T> make_shared(Args&&... args)
	{
		return shared_ptr<T>(new control_block_ex<T>(std::forward<Args>(args)...));
	}

	template<typename T> using mofa_shared_ptr = shared_ptr<T>;
	template<typename T> using mofa_weak_ptr = weak_ptr<T>;
}