// A library for executing scheduled actions based on time and interval settings.
// Author: Deyan Ginev, dginev@gmail.com
//

#include <stdlib.h>
#include "actions.h"

Action **executionArray;

static int indexOfAction(ActionsList *list, Action *a)
{
	for (int i = 0; i < (*list).availableActionsCount; i++)
	{
		if (a == &(*list).availableActions[i])
		{
			return i;
		}
	}

	return -1;
}

static Action *find(ActionsList *list, Action *item)
{
	Action *cur = (*list).first;

	while (cur != nullptr)
	{
		if (cur == item)
		{
			return cur;
		}
		cur = (*cur)._next;
	}
	return nullptr;
}

static bool canStart(Action *a, unsigned long time)
{

	if ((*a)._stopRequested)
	{
		return false;
	}

	if ((*a).state == MS_RUNNING)
	{
		return false;
	}

	if ((*a).ti > 0 && (*a).lst > 0 && (time - (*a).lst < (*a).ti))
	{
		return false;
	}

	if ((*a).canStart != nullptr)
	{
		return (*a).canStart(a);
	}

	return true;
}

static bool shouldStop(Action *a, unsigned long time)
{

	if ((*a)._stopRequested)
	{
		return true;
	}

	if ((*a).state == MS_RUNNING)
	{
		if ((*a).td > 0 && (*a).st > 0 && (time - (*a).st >= (*a).td))
		{
			return true;
		}
	}

	return false;
}

static Action *addListItem(ActionsList *list, Action *a)
{

	if (a != nullptr)
	{

		(*a)._next = nullptr;
		(*a)._prev = (*list).last;

		if ((*list).last != nullptr)
		{
			(*(*list).last)._next = a;
		}

		(*list).last = a;

		if ((*list).first == nullptr)
		{
			(*list).first = a;
		}

		(*list).count++;
		return a;
	}

	return nullptr;
}

static bool removeListItem(ActionsList *list, Action *item)
{

	Action *tr = find(list, item);

	if (tr == nullptr)
	{
		return false;
	}

	if ((*list).last == tr)
	{
		(*list).last = (*tr)._prev;
	}

	if ((*list).first == tr)
	{
		(*list).first = (*tr)._next;
	}

	Action *pr = (*tr)._prev;
	Action *nx = (*tr)._next;

	if (pr != nullptr)
	{
		(*pr)._next = nx;
	}

	if (nx != nullptr)
	{
		(*nx)._prev = pr;
	}

	(*tr)._prev = nullptr;
	(*tr)._next = nullptr;

	(*list).count--;
	return true;
}

void initActionsList(int actionsCount)
{
	executionArray = (Action **)calloc(actionsCount, sizeof(Action *));
}

bool requestStop(ActionsList *list, Action *a)
{
	Action *item = find(list, a);
	if (item != nullptr)
	{
		(*a)._stopRequested = true;
		return true;
	}

	return false;
}

bool descheduleAction(ActionsList *list, Action *a)
{
	Action *found = find(list, a);
	if (found == nullptr)
	{
		return false;
	}

	if (found != nullptr && (*found).state != MS_RUNNING)
	{
		(*found).state = MS_NON_ACTIVE;

		Action *child = (*found).child;
		while (child != nullptr)
		{
			(*child).state = MS_NON_ACTIVE;
			child = (*child).child;
		}

		removeListItem(list, found);

		return true;
	}

	return false;
}

bool scheduleAction(ActionsList *list, Action *a)
{
	Action *found = find(list, a);
	if (found != nullptr)
	{
		return false;
	}

	if ((*a).state != MS_NON_ACTIVE)
	{
		return false;
	}

	if (a != nullptr)
	{
		(*a).state = MS_SCHEDULED;
		Action *child = (*a).child;
		while (child != nullptr)
		{
			(*child).state = MS_CHILD_SCHEDULED;
			child = (*child).child;
		}
		addListItem(list, a);
		return true;
	}

	return false;
}

void doQueueActions(ActionsList *executionList, unsigned long time)
{

	// Create a list of Action pointers
	// for all actions currently to be processed
	// to avoid modifying the linked-list while
	// iterating over it
	int k = 0;
	Action *action = (*executionList).first;
	while (action != nullptr)
	{
		executionArray[k++] = action;
		action = (*action)._next;
	}

	int count = (*executionList).count;

	for (int i = 0; i < count; i++)
	{
		Action *action = executionArray[i];
		if (canStart(action, time))
		{
			(*action).start(action);
			(*action).state = MS_RUNNING;
			(*action).st = time;

			Action *child = (*action).child;
			while (child != nullptr)
			{
				(*child).start(child);
				(*child).state = MS_CHILD_RUNNING;
				child = (*child).child;
			}
		}
		else
		{

			if ((*action).state == MS_RUNNING)
			{
				if (time - (*action).st > (*action).to)
				{
					(*action).tick(action);
				}
			}

			if (shouldStop(action, time))
			{
				Action *child = (*action).child;
				while (child != nullptr)
				{
					(*child).stop(child);
					(*child).state = MS_CHILD_PENDING;
					child = (*child).child;
				}

				(*action).stop(action);
				(*action).lst = time;
				(*action).state = MS_PENDING;
				(*action)._clear = !(*action).frozen;
				(*action)._stopRequested = false;
			}
		}
	}

	// Iterate over the available actions and stop the
	// appropriate ones
	for (int i = 0; i < (*executionList).availableActionsCount; i++)
	{
		Action *a = &(*executionList).availableActions[i];
		if ((*a)._clear)
		{
			descheduleAction(executionList, a);
			(*a)._clear = false;
		}
	}
}
