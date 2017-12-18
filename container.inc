// built-in include guard removal
// just in case the user has a local dependency with the same file name
#if defined _inc_container
    #undef _inc_container
#endif
// custom include-guard to ensure we don't duplicate
#if defined _container_included
	#endinput
#endif
#define _container_included


#include <a_samp>
#include <logger>
#include <YSI\y_iterate>
#include <YSI\y_timers>
#include <item>

#include <YSI\y_hooks>

/*==============================================================================

	Constant Definitions, Function Declarations and Documentation

==============================================================================*/


// Maximum amount of containers that can be created.
#if !defined CNT_MAX
	#define CNT_MAX (4096)
#endif

// Maximum string length for container names.
#if !defined CNT_MAX_NAME
	#define CNT_MAX_NAME (32)
#endif

// Maximum slot size amount for containers.
#if !defined CNT_MAX_SLOTS
	#define CNT_MAX_SLOTS (24)
#endif


#define INVALID_CONTAINER_ID (-1)


// Functions


forward CreateContainer(name[], size);
/*
# Description
Creates a container to store items in with a static size.

# Parameters
- name: A name given to the container, for menus etc.
- size: The maximum capacity of items the container can store.

# Returns
ID of the newly created container or INVALID_CONTAINER_ID if the function failed
due to CNT_MAX limit being reached.
*/

forward DestroyContainer(containerid, destroyitems = true);
/*
# Description
Removes a container from memory and frees the ID.

# Parameters
- destroyitems: If true, items inside will also be deleted.
*/

forward AddItemToContainer(containerid, itemid, playerid = INVALID_PLAYER_ID);
/*
# Description
Adds an item to a container list, calls OnItemAddToContainer.

# Parameters
- playerid: If set, can tell other modules that a player triggered the event.

# Returns
Returns 0 on success. If item doesn't fit, returns a positive integer
representing required item slots, otherwise -1 if containerid invalid, -2 if
itemid invalid, -3 if item is already in container, -4 if OnItemAddToContainer
returned non-zero, -5 if item is inside another container, -6 if item is inside
a player inventory from the SIF/Inventory module.
*/

forward RemoveItemFromContainer(containerid, slotid, playerid = INVALID_PLAYER_ID, call = 1);
/*
# Description
Removes an item from a container slot.

# Parameters
- slotid: The slot to remove an item from (not the item ID).
- playerid: If set, can tell other modules that a player triggered the removal.
- call: If true, will call OnItemRemoveFromContainer
*/

forward IsValidContainer(containerid);
/*
# Description
Checks if a container ID is valid.
*/

forward GetContainerName(containerid, name[]);
/*
# Description
Returns a container's name.

# Returns
Boolean to indicate success or failure.
*/

forward GetContainerSize(containerid);
/*
# Description
Returns a container's item capacity.
*/

forward SetContainerSize(containerid, size);
/*
# Description
Sets a container's item capacity.

# Returns
Boolean to indicate success or failure.
*/

forward GetContainerItemCount(containerid);
/*
# Description
Returns the amount of items inside a container.
*/

forward GetContainerSlotItem(containerid, slotid);
/*
# Description
Returns the item stored in the specified slot.
*/

forward IsContainerSlotUsed(containerid, slotid);
/*
# Description
Checks if a slot in a container is occupied by an item.
*/

forward IsContainerFull(containerid);
/*
# Description
Checks if a container is full.
*/

forward IsContainerEmpty(containerid);
/*
# Description
Checks if a container is empty.
*/

forward WillItemTypeFitInContainer(containerid, ItemType:itemtype);
/*
# Description
Checks if an item type will fit into a container.
*/

forward GetContainerFreeSlots(containerid);
/*
# Description
Returns the number of free item slots in a container.
*/

forward GetItemContainer(itemid);
/*
# Description
Returns the ID of the container that itemid is stored in.
*/

forward GetItemContainerSlot(itemid);
/*
# Description
Returns the container slot that the item is stored in if inside a container.
*/


// Events


forward OnContainerCreate(containerid);
/*
# Called
Upon the creation of a container.
*/

forward OnContainerDestroy(containerid);
/*
# Called
Upon the destruction of a container.
*/

forward OnItemAddToContainer(containerid, itemid, playerid);
/*
# Called
When an item is added to a container. Note that the item won't actually be in the container list when this is called.

# Returns
1 to cancel the item being added.
*/

forward OnItemAddedToContainer(containerid, itemid, playerid);
/*
# Called
After an item has been added to a container.
*/

forward OnItemRemoveFromContainer(containerid, slotid, playerid);
/*
# Called
As an item is removed from a container. Note that the item will still be in the container list when this is called.

# Returns
1 to cancel the item being removed.
*/

forward OnItemRemovedFromContainer(containerid, slotid, playerid);
/*
# Called
After an item has been removed from a container.
*/


/*==============================================================================

	Setup

==============================================================================*/


enum E_CONTAINER_DATA {
	cnt_name[CNT_MAX_NAME],
	cnt_size
}


static
	cnt_Data[CNT_MAX][E_CONTAINER_DATA],
	cnt_Items[CNT_MAX][CNT_MAX_SLOTS],
	cnt_ItemContainer[ITM_MAX] = {INVALID_CONTAINER_ID, ...},
	cnt_ItemContainerSlot[ITM_MAX] = {-1, ...};

new
	Iterator:cnt_Index<CNT_MAX>; // exported publically to facilitate iterating through containers


/*==============================================================================

	Core Functions

==============================================================================*/


stock CreateContainer(name[], size) {
	new id = Iter_Free(cnt_Index);

	if(id == INVALID_ITERATOR_SLOT) {
		fatal("container limit reached");
		return INVALID_CONTAINER_ID;
	}

	if(size > CNT_MAX_SLOTS || size <= 0) {
		err("attempt to create a container with invalid size",
			_i("size", size));

		size = CNT_MAX_SLOTS;
	}

	strcpy(cnt_Data[id][cnt_name], name, CNT_MAX_NAME);
	cnt_Data[id][cnt_size] = size;

	for(new i; i < CNT_MAX_SLOTS; i++) {
		cnt_Items[id][i] = INVALID_ITEM_ID;
	}

	Iter_Add(cnt_Index, id);

	CallLocalFunction("OnContainerCreate", "d", id);

	return id;
}

stock DestroyContainer(containerid, destroyitems = true) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return 1;
	}

	CallLocalFunction("OnContainerDestroy", "d", containerid);

	if(destroyitems) {
		for(new i; i < cnt_Data[containerid][cnt_size]; i++) {
			if(cnt_Items[containerid][i] == INVALID_ITEM_ID) {
				break;
			}

			cnt_ItemContainer[cnt_Items[containerid][i]] = INVALID_CONTAINER_ID;
			DestroyItem(cnt_Items[containerid][i]);
			cnt_Items[containerid][i] = INVALID_ITEM_ID;
		}
	}

	cnt_Data[containerid][cnt_name][0]	= EOS;
	cnt_Data[containerid][cnt_size]		= 0;

	Iter_Remove(cnt_Index, containerid);

	return 1;
}

stock AddItemToContainer(containerid, itemid, playerid = INVALID_PLAYER_ID) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return -1;
	}

	if(!IsValidItem(itemid)) {
		return -2;
	}

	new
		slots,
		idx;

	while(idx < cnt_Data[containerid][cnt_size]) {
		if(!IsValidItem(cnt_Items[containerid][idx])) {
			break;
		}

		if(cnt_Items[containerid][idx] == itemid) {
			return -3;
		}

		slots += GetItemTypeSize(GetItemType(cnt_Items[containerid][idx]));
		idx++;
	}

	new itemsize = GetItemTypeSize(GetItemType(itemid));

	if((itemsize + slots) > cnt_Data[containerid][cnt_size]) {
		return (itemsize + slots) - cnt_Data[containerid][cnt_size];
	}

	if(CallLocalFunction("OnItemAddToContainer", "ddd", containerid, itemid, playerid)) {
		return -4;
	}

	if(cnt_ItemContainer[itemid] != INVALID_CONTAINER_ID)
	{
		if(!RemoveItemFromContainer(cnt_ItemContainer[itemid], cnt_ItemContainerSlot[itemid]))
			return -5;
	}

	cnt_Items[containerid][idx] = itemid;
	cnt_ItemContainer[itemid] = containerid;
	cnt_ItemContainerSlot[itemid] = idx;

	RemoveItemFromWorld(itemid);
	RemoveCurrentItem(GetItemHolder(itemid));

	CallLocalFunction("OnItemAddedToContainer", "ddd", containerid, itemid, playerid);
	
	return 0;
}

stock RemoveItemFromContainer(containerid, slotid, playerid = INVALID_PLAYER_ID, call = 1) {
	if(!(0 <= slotid < cnt_Data[containerid][cnt_size])) {
		return 1;
	}

	if(!IsValidItem(cnt_Items[containerid][slotid])) {
		if(cnt_Items[containerid][slotid] != INVALID_ITEM_ID) {
			err("found container slot with non-invalid item ID but no valid item present, cleaning up",
				_i("containerid", containerid),
				_i("itemid", cnt_Items[containerid][slotid]),
				_i("slotid", slotid));

			cnt_Items[containerid][slotid] = INVALID_ITEM_ID;

			if(slotid < (cnt_Data[containerid][cnt_size] - 1)) {
				for(new i = slotid; i < (cnt_Data[containerid][cnt_size] - 1); i++) {
					cnt_Items[containerid][i] = cnt_Items[containerid][i+1];

					if(cnt_Items[containerid][i] != INVALID_ITEM_ID) {
						cnt_ItemContainerSlot[cnt_Items[containerid][i]] = i;
					}
				}

				cnt_Items[containerid][(cnt_Data[containerid][cnt_size] - 1)] = INVALID_ITEM_ID;
			}
		}

		return 2;
	}

	if(call) {
		if(CallLocalFunction("OnItemRemoveFromContainer", "ddd", containerid, slotid, playerid)) {
			return 0;
		}
	}

	cnt_ItemContainer[cnt_Items[containerid][slotid]] = INVALID_CONTAINER_ID;
	cnt_ItemContainerSlot[cnt_Items[containerid][slotid]] = -1;
	cnt_Items[containerid][slotid] = INVALID_ITEM_ID;

	if(slotid < (cnt_Data[containerid][cnt_size] - 1)) {
		for(new i = slotid; i < (cnt_Data[containerid][cnt_size] - 1); i++) {
			cnt_Items[containerid][i] = cnt_Items[containerid][i+1];

			if(cnt_Items[containerid][i] != INVALID_ITEM_ID) {
				cnt_ItemContainerSlot[cnt_Items[containerid][i]] = i;
			}
		}

		cnt_Items[containerid][(cnt_Data[containerid][cnt_size] - 1)] = INVALID_ITEM_ID;
	}

	if(call) {
		CallLocalFunction("OnItemRemovedFromContainer", "ddd", containerid, slotid, playerid);
	}

	return 0;
}

stock IsValidContainer(containerid) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return false;
	}

	return true;
}

// cnt_name
stock GetContainerName(containerid, name[]) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return 1;
	}

	name[0] = EOS;
	strcat(name, cnt_Data[containerid][cnt_name], CNT_MAX_NAME);

	return 0;
}

// cnt_size
stock GetContainerSize(containerid)
{

	if(!Iter_Contains(cnt_Index, containerid))
		return -1;

	return cnt_Data[containerid][cnt_size];
}
stock SetContainerSize(containerid, size)
{

	if(!Iter_Contains(cnt_Index, containerid))
		return 0;

	cnt_Data[containerid][cnt_size] = size;

	return 1;
}

stock GetContainerItemCount(containerid)
{

	if(!Iter_Contains(cnt_Index, containerid))
		return 0;

	new size;

	while(size < cnt_Data[containerid][cnt_size])
	{
		if(!IsValidItem(cnt_Items[containerid][size]))
			break;

		size++;
	}

	return size;
}

// cnt_Items
stock GetContainerSlotItem(containerid, slotid)
{

	if(!Iter_Contains(cnt_Index, containerid))
		return INVALID_ITEM_ID;

	if(!(0 <= slotid < CNT_MAX_SLOTS))
		return INVALID_ITEM_ID;

	return cnt_Items[containerid][slotid];
}

stock IsContainerSlotUsed(containerid, slotid)
{

	if(!(0 <= slotid < CNT_MAX_SLOTS))
		return -1;

	if(!IsValidItem(cnt_Items[containerid][slotid]))
		return 0;

	return 1;
}

stock IsContainerFull(containerid)
{

	if(!Iter_Contains(cnt_Index, containerid))
		return 0;

	return GetContainerFreeSlots(containerid) == 0;
}

stock IsContainerEmpty(containerid)
{

	if(!Iter_Contains(cnt_Index, containerid))
		return 0;

	return !IsValidItem(cnt_Items[containerid][0]);
}

stock WillItemTypeFitInContainer(containerid, ItemType:itemtype)
{

	if(!Iter_Contains(cnt_Index, containerid))
		return 0;

	new
		slots,
		idx;

	while(idx < cnt_Data[containerid][cnt_size])
	{
		if(!IsValidItem(cnt_Items[containerid][idx]))
			break;

		slots += GetItemTypeSize(GetItemType(cnt_Items[containerid][idx]));
		idx++;
	}

	if(slots + GetItemTypeSize(itemtype) > cnt_Data[containerid][cnt_size])
		return 0;

	return 1;
}

stock GetContainerFreeSlots(containerid)
{

	if(!Iter_Contains(cnt_Index, containerid))
		return 0;

	new
		slots,
		idx;

	while(idx < cnt_Data[containerid][cnt_size])
	{
		if(!IsValidItem(cnt_Items[containerid][idx]))
			break;

		slots += GetItemTypeSize(GetItemType(cnt_Items[containerid][idx]));
		idx++;
	}

	return cnt_Data[containerid][cnt_size] - slots;
}

stock GetItemContainer(itemid)
{

	if(!IsValidItem(itemid))
		return INVALID_CONTAINER_ID;

	return cnt_ItemContainer[itemid];
}

stock GetItemContainerSlot(itemid)
{

	if(!IsValidItem(itemid))
		return INVALID_CONTAINER_ID;

	return cnt_ItemContainerSlot[itemid];
}


/*==============================================================================

	Internal Functions and Hooks

==============================================================================*/


hook OnItemDestroy(itemid)
{
	if(cnt_ItemContainer[itemid] != INVALID_CONTAINER_ID)
	{
		RemoveItemFromContainer(cnt_ItemContainer[itemid], cnt_ItemContainerSlot[itemid]);
	}
}

hook OnItemCreateInWorld(itemid)
{
	if(cnt_ItemContainer[itemid] != INVALID_CONTAINER_ID)
	{
		RemoveItemFromContainer(cnt_ItemContainer[itemid], cnt_ItemContainerSlot[itemid]);
	}
}
