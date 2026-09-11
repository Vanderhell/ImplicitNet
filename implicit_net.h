#ifndef IMPLICIT_NET_H
#define IMPLICIT_NET_H

#include <stdint.h>

#if defined(_WIN32) && defined(IMPLICIT_NET_BUILDING_LIBRARY)
#  define IMPLICIT_NET_API __declspec(dllexport)
#elif defined(_WIN32) && !defined(IMPLICIT_NET_STATIC)
#  define IMPLICIT_NET_API __declspec(dllimport)
#else
#  define IMPLICIT_NET_API
#endif

#ifdef __cplusplus
extern "C" {
#endif

/** Opaque 64-bit state used to select the network configuration. */
typedef uint64_t in_state;

/** Initialise @p s with @p seed. N must be a power of two in [4, 65536]. */
IMPLICIT_NET_API int in_init(in_state *s, uint32_t n, uint64_t seed);

/** Map a node identifier through the state-derived bijection. */
IMPLICIT_NET_API uint32_t in_map(uint32_t x, uint32_t n, in_state s);

/** Invert an identifier produced by in_map. */
IMPLICIT_NET_API uint32_t in_unmap(uint32_t y, uint32_t n, in_state s);

/** Return the neighbor in dimension [0, log2(N)); input errors return UINT32_MAX. */
IMPLICIT_NET_API uint32_t in_neighbor(uint32_t x, uint32_t direction, uint32_t n, in_state s);

/** Take the deterministic greedy routing step from current toward target. */
IMPLICIT_NET_API uint32_t in_next(uint32_t current, uint32_t target, uint32_t n, in_state s);

/** Reversibly apply an event to the state. Returns zero for invalid arguments. */
IMPLICIT_NET_API int in_step(in_state *s, uint32_t n, uint64_t event);

/** Reverse in_step for the same event. */
IMPLICIT_NET_API int in_prev(in_state *s, uint32_t n, uint64_t event);

#ifdef __cplusplus
}
#endif

#endif
