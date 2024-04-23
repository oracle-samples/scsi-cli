// SPDX-License-Identifier: UPL-1.0
/*
 * Copyright (c) 2024, Oracle and/or its affiliates.
 *
 * The Universal Permissive License (UPL), Version 1.0
 *
 * Subject to the condition set forth below, permission is hereby granted to any
 * person obtaining a copy of this software, associated documentation and/or data
 * (collectively the "Software"), free of charge and under any and all copyright
 * rights in the Software, and any and all patent rights owned or freely
 * licensable by each licensor hereunder covering either (i) the unmodified
 * Software as contributed to or provided by such licensor, or (ii) the Larger
 * Works (as defined below), to deal in both
 *
 * (a) the Software, and
 * (b) any piece of software and/or hardware listed in the
 * lrgrwrks.txt file if one is included with the Software (each a "Larger
 * Work" to which the Software is contributed by such licensors),
 *
 * without restriction, including without limitation the rights to copy, create
 * derivative works of, display, perform, and distribute the Software and make,
 * use, sell, offer for sale, import, export, have made, and have sold the
 * Software and the Larger Work(s), and to sublicense the foregoing rights on
 * either these or other terms.
 *
 * This license is subject to the following condition:
 * The above copyright notice and either this complete permission notice or at
 * a minimum a reference to the UPL must be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef _UTILS_H
#define _UTILS_H

#include <stdlib.h>
#include <stddef.h>

#define unlikely __glibc_unlikely

#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))

#ifdef __packed
#else /* __packed */
#define __packed __attribute__((__packed__))
#endif /* __packed */

#define NUM_ENTRIES(x) (int)(sizeof(x) / sizeof(x[0]))

typedef __signed__ char __s8;
typedef unsigned char __u8;

typedef __signed__ short __s16;
typedef unsigned short __u16;

typedef __signed__ int __s32;
typedef unsigned int __u32;

#ifdef __GNUC__
__extension__ typedef __signed__ long long __s64;
__extension__ typedef unsigned long long __u64;
#else
typedef __signed__ long long __s64;
typedef unsigned long long __u64;
#endif

#define u8	__u8
#define u16	__u16
#define u32	__u32
#define u64	__u64

/* complex for_each that checkpatch has issues with but is correct and
 * used in multiple include files in the kernel
 */
#define for_each_dir(entry, subdir)			\
	while ((entry = readdir(subdir)) != NULL)	\
		if (strcmp(entry->d_name, ".") &&	\
		    strcmp(entry->d_name, ".."))

/* simple linked list functions */

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }

#define LIST_HEAD(name) \
	struct list_head name = LIST_HEAD_INIT(name)

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->next = (ptr); (ptr)->prev = (ptr); \
} while (0)

/*
 * Insert a new entry between tow consecutive node
 */
static inline void __list_add(struct list_head *entry,
			      struct list_head *prev,
			      struct list_head *next)
{
	next->prev = entry;
	entry->next = next;
	entry->prev = prev;
	prev->next = entry;
}

/**
 * list add - add a new entry
 * @entry : new entry to be added
 * @list : List head to add it after
 *
 * Insert a new entry after the specified head
 */
static inline void list_add(struct list_head *entry, struct list_head *list)
{
	__list_add(entry, list, list->next);
}

static inline void list_add_tail(struct list_head *entry,
				 struct list_head *list)
{
	__list_add(entry, list->prev, list);
}

/**
 * Delete a list entry by making prev/next entries
 * point to each other
 */
static inline void list_del(struct list_head *entry)
{
	entry->next->prev = entry->prev;
	entry->prev->next = entry->next;
}

static inline int list_empty(const struct list_head *list)
{
	return list->next == list;
}

#define offset_of(type, member) ((size_t) &((type *)0)->member)

#define container_of(ptr, type, member) ({				   \
	 const typeof(((type *)0)->member) (*__mptr) = (ptr);		   \
		 (type *)((char *) __mptr - offset_of(type, member));	   \
	})

#define list_entry(entry, type, member)					\
	container_of(entry, type, member)

#define list_first_entry(ptr, type, member)				\
	list_entry((ptr)->next, type, member)

#define list_for_each(entry, list)					\
	for (entry = (list)->next; entry != (list); entry = entry->next)

#define list_for_each_safe(entry, tmp, list)				\
	for (entry = (list)->next, tmp = entry->next; entry != (list);	\
	     entry = tmp, tmp = entry->next)

#define list_for_each_entry(entry, list, member)			   \
	for (entry = list_entry((list)->next, typeof(*entry), member);     \
	     &entry->member != (list);					   \
	     entry = list_entry(entry->member.next, typeof(*entry), member))

#define list_for_each_entry_safe(entry, tmp, list, member)		   \
	for (entry = list_entry((list)->next, typeof(*entry), member),     \
	     tmp = list_entry(entry->member.next, typeof(*entry), member); \
	     &entry->member != (list);					   \
	     entry = tmp,						   \
	     tmp = list_entry(tmp->member.next, typeof(*tmp), member))

#define print_debug(fmt, args...)\
	do {				\
		if (debug) {			\
			printf("%s (Line: %d) " fmt "\n", __func__, __LINE__, ##args);	\
			fflush(stdout);	\
		} \
	} while (0)

#define print_trace_enter()\
	do {				\
		if (trace) {		\
			printf("%s (Line: %d): Enter \n", __func__, __LINE__);	\
			fflush(stdout);	\
		}	\
	} while (0)

#define print_info(fmt, args...)\
	do {				\
		printf(fmt "\n",  ##args);	\
		fflush(stdout);	\
	} while (0)

#define print_err(fmt, args...)\
	do {			\
		fprintf(stderr, "Error: " fmt "\n" , ##args); \
		fflush(stderr); \
	} while (0)

#define UNUSED(x) ((void) x)

#define min(x, y) ((x < y) ? x : y)

#define __round_mask(x, y) ((__typeof__(x))((y) - 1))
#define round_up(x, y) ((((x) - 1) | __round_mask(x, y)) + 1)

#define valid_delim	", "
#define colon_delim	":"
#endif
