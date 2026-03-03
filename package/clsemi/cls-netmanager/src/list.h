/*
 *  Copyright (c) 2021-2025, Clourney Semiconductor. All rights reserved.
 *
 *  This software and/or documentation is licensed by Clourney Semiconductor under limited terms and conditions.
 *  Reproduction and redistribution in binary or source form, with or without modification,
 *  for use solely in conjunction with a Clourney Semiconductor chipset, is permitted in condition which
 *  must retain the above copyright notice.
 *
 *  By using this software and/or documentation, you agree to the limited terms and conditions.
 */

#ifndef LIST_H
#define LIST_H

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#ifndef container_of
#define container_of(ptr, type, member) ({                      \
		const typeof(((type *)0)->member) * __mptr = (ptr);    \
		(type *)((char *)__mptr - offsetof(type, member)); })
#endif

struct dlist_head {
	struct dlist_head *next, *prev;
};


#define DLIST_HEAD_INIT(name) { &(name), &(name) }

#define DLIST_HEAD(name) struct dlist_head name = DLIST_HEAD_INIT(name)

/**
 * dlist_entry - get the struct for this entry
 * @ptr:        the &struct dlist_head pointer.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the dlist_head within the struct.
 */
#define dlist_entry(ptr, type, member) container_of(ptr, type, member)

/**
 * dlist_for_each_entry  -       iterate over list of given type
 * @pos:        the type * to use as a loop cursor.
 * @head:       the head for your list.
 * @member:     the name of the dlist_head within the struct.
 */
#define dlist_for_each_entry(pos, head, member)                          \
	for (pos = dlist_entry((head)->next, typeof(*pos), member);      \
		&pos->member != (head);    \
		pos = dlist_entry(pos->member.next, typeof(*pos), member))

/**
 * dlist_for_each_entry_safe - iterate over list of given type safe against removal of list entry
 * @pos:        the type * to use as a loop cursor.
 * @n:          another type * to use as temporary storage
 * @head:       the head for your list.
 * @member:     the name of the dlist_head within the struct.
 */
#define dlist_for_each_entry_safe(pos, n, head, member)                  \
	for (pos = dlist_entry((head)->next, typeof(*pos), member),      \
		n = dlist_entry(pos->member.next, typeof(*pos), member); \
		&pos->member != (head);                                    \
		pos = n, n = dlist_entry(n->member.next, typeof(*n), member))

/**
 * dlist_empty - tests whether a list is empty
 * @head: the list to test.
 */
static inline int dlist_empty(const struct dlist_head *head)
{
	return head->next == head;
}

/**
 * dlist_first_entry - get the first element from a list
 * @ptr:        the &struct dlist_head pointer.
 * @type:       the type of the struct this is embedded in.
 * @member:     the name of the dlist_head within the struct.
 */
#define dlist_first_entry(ptr, type, member) \
	(dlist_empty(ptr) ? NULL : dlist_entry((ptr)->next, type, member))

static inline void dlist_head_init(struct dlist_head *list)
{
	list->next = list;
	list->prev = list;
}

/*
 * Insert a new entry between two known consecutive entries.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __dlist_add(struct dlist_head *_new,
			      struct dlist_head *prev,
			      struct dlist_head *next)
{
	next->prev = _new;
	_new->next = next;
	_new->prev = prev;
	prev->next = _new;
}

/**
 * dlist_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
static inline void dlist_add_tail(struct dlist_head *_new,
				 struct dlist_head *head)
{
	__dlist_add(_new, head->prev, head);
}

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
static inline void __dlist_del(struct dlist_head *prev,
			      struct dlist_head *next)
{
	next->prev = prev;
	prev->next = next;
}

#define LIST_POISON1  ((void *) 0x00100100)
#define LIST_POISON2  ((void *) 0x00200200)
/**
 * dlist_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: dlist_empty() on entry does not return true after this, the entry is
 * in an undefined state.
 */
static inline void dlist_del(struct dlist_head *entry)
{
	__dlist_del(entry->prev, entry->next);
	entry->next = (struct dlist_head *) LIST_POISON1;
	entry->prev = (struct dlist_head *) LIST_POISON2;
}
#endif
