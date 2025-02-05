// ActionsList.h
#ifndef _ACTIONSLIST_h
#define _ACTIONSLIST_h

typedef enum ActionState
{
	MS_NON_ACTIVE = 0,
	MS_SCHEDULED = 1,
	MS_PENDING = 2,
	MS_RUNNING = 3,
	MS_CHILD_RUNNING = 4,
	MS_CHILD_PENDING = 5,
	MS_CHILD_SCHEDULED = 6
};

typedef struct Action
{
	/**
	 * A pointer to the next action in the list.
	 */
	Action *_next = nullptr;

	/**
	 * A pointer to the previous action in the list.
	 */
	Action *_prev = nullptr;

	/**
	 * A flag that indicates that the action should be removed from the executable list at
	 * the end of the execution iteration.
	 */
	bool _clear;

	/**
	 * A flag that indicates that a stop has been requested for the particular action.
	 */
	bool _stopRequested = false;

	/**
	 * If set, once scheduled the action will never be descheduled.
	 */
	bool frozen = false;

	/**
	 * A descriptive name of the action
	 */
	char *name;

	/**
	 * The desired interval, in milliseconds, between two consecutive ticks.
	 */
	unsigned long ti = 0;

	/**
	 * The amount of time after start, in milliseconds, in which ticks will be sent before stopping the action.
	 * If set to '0' ticks will be sent forever.
	 */
	unsigned long td = 1000;

	/**
	 * The last point in time since application start at which the action was stopped.
	 */
	unsigned long lst = 0;

	/**
	 * The time, in milliseconds, for which the first tick of the action will be sent after start.
	 */
	unsigned long to = 0;

	/**
	 * The last point in time since application start at which the action was started.
	 */
	unsigned long st = 0; // last start time

	/**
	 * The current state of the action.
	 */
	ActionState state = MS_NON_ACTIVE;

	/**
	 * A pointer to a function which will be called when a tick is sent.
	 */
	void (*tick)(Action *);

	/**
	 * A pointer to a function which will be called when starting the action but before sending a tick.
	 */
	void (*start)(Action *);

	/**
	 * A pointer to a function which will be called before stopping the action but after sending the last tick.
	 */
	void (*stop)(Action *);

	/**
	 * A pointer to a function which will be called when the system decides that an action is eligible for starting.
	 * This function can be used to override the system decision.
	 */
	bool (*canStart)(Action *);

	/**
	 * A pointer to an object representing the context of the action.
	 */
	void *context;

	/**
	 * A pointer to another action which will be treated as a depended action to this one.
	 * A child action is activated after the master action has been activated.
	 * A child action is deactivated before the master action is deactivated.
	 * Child actions are not part of the execution queue.
	 */
	Action *child = nullptr;
};

typedef struct ActionsList
{
	unsigned int count = 0;
	unsigned int availableActionsCount = 0;
	Action *first = nullptr;
	Action *last = nullptr;
	Action *availableActions = nullptr;
};

/**
 * Call to request stopping the provided action, part of the passed list.
 */
bool requestStop(ActionsList *list, Action *a);

/**
 * Call to deschedule the provided action, part of the passed list.
 */
bool descheduleAction(ActionsList *list, Action *a);

/**
 * Call to schedule the provided action, part of the passed list.
 */
bool scheduleAction(ActionsList *list, Action *a);

/**
 * Call to perform actions scheduling.
 */
void doQueueActions(ActionsList *executionList, unsigned long time);

/**
 * Call to initialize the system.
 */
void initActionsList(int actionsCount);

#endif
