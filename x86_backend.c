#include "x86_backend.h"
#include <stdlib.h>

deferred_addr * defer_address(deferred_addr * old_head, uint32_t address, uint8_t *dest)
{
	deferred_addr * new_head = malloc(sizeof(deferred_addr));
	new_head->next = old_head;
	new_head->address = address & 0xFFFFFF;
	new_head->dest = dest;
	return new_head;
}

void remove_deferred_until(deferred_addr **head_ptr, deferred_addr * remove_to)
{
	for(deferred_addr *cur = *head_ptr; cur && cur != remove_to; cur = *head_ptr)
	{
		*head_ptr = cur->next;
		free(cur);
	}
}

void process_deferred(deferred_addr ** head_ptr, void * context, native_addr_func get_native)
{
	deferred_addr * cur = *head_ptr;
	deferred_addr **last_next = head_ptr;
	while(cur)
	{
		uint8_t * native = get_native(context, cur->address);//get_native_address(opts->native_code_map, cur->address);
		if (native) {
			int32_t disp = native - (cur->dest + 4);
			uint8_t * out = cur->dest;
			*(out++) = disp;
			disp >>= 8;
			*(out++) = disp;
			disp >>= 8;
			*(out++) = disp;
			disp >>= 8;
			*out = disp;
			*last_next = cur->next;
			free(cur);
			cur = *last_next;
		} else {
			last_next = &(cur->next);
			cur = cur->next;
		}
	}
}
