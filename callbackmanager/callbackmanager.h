/*
 * callbackmanager.h
 *
 *  Created on: 16 jun. 2024
 *      Author: jancu
 */

#ifndef CALLBACKMANAGER_H_
#define CALLBACKMANAGER_H_

#include <functional>

template <typename R, typename... Args>
// restrict to arithmetic data types for return value, or void
#ifdef __GNUC__ // this requires a recent version of GCC.
#if __GNUC_PREREQ(10,0)
  requires std::is_void<R>::value || std::is_arithmetic_v<R>
#endif
#endif

class Callback {
public:
	Callback() : _callback(nullptr){}

	inline void set(std::function<R(Args... args)> callback) {
	    _callback = & callback;
	}

	inline void unset() {
	    _callback = nullptr;
	}

	/*
	 * R can either be an arithmetic type, or void
	 */
	inline R call(Args... args) {
		if constexpr (std::is_void<R>::value) {
			if (_callback == nullptr) {
				return;
			}
			(*_callback)(args...);
		}

		if constexpr (! std::is_void<R>::value) {
			if (_callback == nullptr) {
				return 0; // R can only be a arithmetic type. 0 should work as default.
			}
			return (*_callback)(args...);
		}
	}

	inline bool armed() {
		return (_callback != nullptr);		
	}

private:
	std::function<R(Args... args)> *_callback;
};


#endif /* CALLBACKMANAGER_H_ */
