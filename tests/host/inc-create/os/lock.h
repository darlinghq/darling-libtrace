#pragma once
// Host stand-in for libplatform's unfair lock: a spin lock, enough for the threaded test.
typedef struct { unsigned _os_unfair_lock_opaque; } os_unfair_lock;
#define OS_UNFAIR_LOCK_INIT ((os_unfair_lock){0})
static inline void os_unfair_lock_lock(os_unfair_lock *l) {
	while (__atomic_exchange_n(&l->_os_unfair_lock_opaque, 1, __ATOMIC_ACQUIRE)) {}
}
static inline void os_unfair_lock_unlock(os_unfair_lock *l) {
	__atomic_store_n(&l->_os_unfair_lock_opaque, 0, __ATOMIC_RELEASE);
}
