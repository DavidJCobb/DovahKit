#include "./cpuinfo.h"
#include <array>
#include <cstdint>
#ifdef _MSC_VER
	#include <intrin.h>
#elif defined(__GNUC__)
	#include <cpuid.h>
#endif

static void cpuid(std::array<int, 4>& dst, int what) {
	#ifdef _MSC_VER
		__cpuid(dst.data(), what);
	#elif defined(__GNUC__) || defined(__clang__)
		if (!__get_cpuid(1, dst[0], dst[1], dst[2], dst[3])) {
			dst = { 0 };
		}
	#endif
}

namespace lu {
	cpuinfo::cpuinfo() {
		std::array<int, 4> info;
		//
		// Check SSE - SSE4.2 and AVX:
		//
		cpuid(info, 1);
		this->extension_support.sse_1   = info[3] & (1 << 25);
		this->extension_support.sse_2   = info[3] & (1 << 26);
		this->extension_support.sse_3   = info[2] & 1;
		this->extension_support.ssse_3  = info[2] & (1 <<  9);
		this->extension_support.sse_4_1 = info[2] & (1 << 19);
		this->extension_support.sse_4_2 = info[2] & (1 << 20);
		this->extension_support.avx     = info[2] & (1 << 28);
		if (this->extension_support.avx) {
			bool osx = info[2] & (1 << 27);
			if (osx) {
				auto features = _xgetbv(0);
				this->extension_support.avx = (features & 0x6) == 0x6; // even with CPU support, AVX doesn't actually work unless the registers are saved by the OS on context switch
			}
		}
		//
		// Check SSE4a and SSE5:
		//
		cpuid(info, 0x80000000);
		int extended_id_count = info[0];
		if (extended_id_count > 0x80000000) {
			cpuid(info, 0x80000001);
			this->extension_support.sse_4a = info[2] & (1 <<  6);
			this->extension_support.sse_5  = info[2] & (1 << 11);
		}
	}
}