#ifndef QUEUE_H_
#define QUEUE_H_
#include <stdbool.h>
#include <stdint.h>

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

/**
 * @brief Create the queue.
 * @param queue (out) Pointer for the queue.
 * @param max_items (in) Max number of items this queue can hold.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_queue_create(Util_queue* queue, uint32_t max_items);

/**
 * @brief Add an event to the queue. Data is passed by reference not by copy.
 * @param queue (in) Pointer for the queue.
 * @param event_id (in) User defined event id.
 * @param data (in) Pointer for user defined data, can be NULL.
 * @param wait_ns (in) Wait time in us when queue is full.
 * @param event_id (in) User defined event id.
 * @param option (in) Queue options.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_queue_add(Util_queue* queue, uint32_t event_id, void* data, int64_t wait_us, Util_queue_option option);

/**
 * @brief Get an event from the queue. Data is passed by reference not by copy.
 * @param queue (in) Pointer for the queue.
 * @param event_id (out) Event id.
 * @param data (out) Pointer for data, can be NULL.
 * @param wait_ns (in) Wait time in us when queue is empty.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_queue_get(Util_queue* queue, uint32_t* event_id, void** data, int64_t wait_us);

/**
 * @brief Check if the specified event exist in the queue.
 * Always return false if queue is not initialized.
 * @param queue (in) Pointer for the queue.
 * @param event_id (in) Event id to check.
 * @note Thread safe
*/
bool Util_queue_check_event_exist(Util_queue* queue, uint32_t event_id);

/**
 * @brief Check how many spaces left in the queue.
 * Always return 0 if queue is not initialized.
 * @param queue (in) Pointer for the queue.
 * @note Thread safe
*/
uint32_t Util_queue_get_free_space(Util_queue* queue);

/**
 * @brief Delete the queue and if any, free all data in the queue.
 * Do nothing if queue is not initialized.
 * @param queue (in) Pointer for the queue.
 * @note Thread safe
*/
void Util_queue_delete(Util_queue* queue);

#endif //QUEUE_H_
