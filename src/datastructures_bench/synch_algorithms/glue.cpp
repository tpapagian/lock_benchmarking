#include "qd.hpp"

using locktype = qdlock;

extern "C" {
#include "cpplock.h"

AgnosticDXLock* cpplock_new() {
	AgnosticDXLock* x = (AgnosticDXLock*) std::malloc(sizeof(AgnosticDXLock) + sizeof(locktype)-1+1024);
	new (&x->lock) locktype;
	return x;
}

void cpplock_init(AgnosticDXLock* x) {
	locktype* l = reinterpret_cast<locktype*>(&x->lock);
	new (l) locktype;
}
void cpplock_free(AgnosticDXLock* x) {
	locktype* l = reinterpret_cast<locktype*>(&x->lock);
	l->~locktype();
	std::free(x);
}
	
void cpplock_delegate(AgnosticDXLock* x, void (*delgateFun)(int, int *), int data) {
	locktype* l = reinterpret_cast<locktype*>(&x->lock);
	l->delegate_n([](void (*fun)(int, int *), int d) {fun(d, nullptr);}, delgateFun, data);
}
int cpplock_delegate_and_wait(AgnosticDXLock* x, void (*delgateFun)(int, int *), int data) {
	locktype* l = reinterpret_cast<locktype*>(&x->lock);
	int resp;
	std::atomic<bool> flag(false);
	l->delegate_n([](void (*fun)(int, int *), int d , int* r, std::atomic<bool>* f) { fun(d, r);  f->store(true, std::memory_order_release);}, delgateFun, data, &resp, &flag);
	while(!flag.load(std::memory_order_acquire)) {
		qd::pause();
	}
	return resp;
}
void cpplock_lock(AgnosticDXLock* x) {
	locktype* l = reinterpret_cast<locktype*>(&x->lock);
	l->lock();
}
void cpplock_unlock(AgnosticDXLock* x) {
	locktype* l = reinterpret_cast<locktype*>(&x->lock);
	l->unlock();
}
//void cpplock_rlock(AgnosticDXLock* x) {
//	locktype* l = reinterpret_cast<locktype*>(&x->lock);
//	l->rlock();
//}
//void cpplock_runlock(AgnosticDXLock* x) {
//	locktype* l = reinterpret_cast<locktype*>(&x->lock);
//	l->runlock();
//}

} // extern "C"
