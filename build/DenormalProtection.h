#pragma once

#include <xmmintrin.h>
#include <pmmintrin.h>

class DenormalHandler {
public:
	DenormalHandler() {
		// save daw flag
		original_flags = _mm_getcsr();
		// set ftx and denormal flag
		_mm_setcsr(original_flags | 0x8040);
	}
	~DenormalHandler() {
		// restore daw flag when out of scope
		_mm_setcsr(original_flags);
	}
private:
	unsigned int original_flags;
};