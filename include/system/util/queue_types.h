#if !defined(DEF_QUEUE_TYPES_H)
#define DEF_QUEUE_TYPES_H
#include <stdbool.h>
#include <stdint.h>
#include "3ds/synchronization.h"

typedef enum
{
	QUEUE_OPTION_NONE					= 0,		//Default.
	QUEUE_OPTION_DO_NOT_ADD_IF_EXIST	= (1 << 1), //Do not add the event if the same event id exist.
	QUEUE_OPTION_SEND_TO_FRONT			= (1 << 2), //Send an event to the front of the queue, use it for high priority event.
} Util_queue_option;

typedef struct
{
	bool deleting;					//Whether this queue is being deleted.
	void** data;					//Data list.
	uint32_t* event_id;				//Event id list.
	uint32_t max_items;				//Queue capacity.
	uint32_t next_index;			//Next free index.
	uint32_t reference_count;		//Reference count for this queue.
	LightEvent receive_wait_event;	//If timeout is not 0, this is used to wait for new message from this queue.
	LightEvent send_wait_event;		//If timeout is not 0, this is used to wait for available space to send data to this queue.
} Util_queue;

#endif //!defined(DEF_QUEUE_TYPES_H)
