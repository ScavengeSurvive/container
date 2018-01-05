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
#include <item>
#include <YSI\y_hooks>


/*==============================================================================

	Constant Definitions, Function Declarations and Documentation

==============================================================================*/


// Maximum amount of containers that can be created.
#if !defined MAX_CONTAINER
	#define MAX_CONTAINER (4096)
#endif

// Maximum string length for container names.
#if !defined MAX_CONTAINER_NAME
	#define MAX_CONTAINER_NAME (32)
#endif

// Maximum slot size amount for containers.
#if !defined MAX_CONTAINER_SLOTS
	#define MAX_CONTAINER_SLOTS (24)
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
due to MAX_CONTAINER limit being reached.
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

forward bool:IsValidContainer(containerid);
/*
# Description
Checks if a container ID is valid.
*/

forward GetContainerName(containerid, name[]);
/*
# Description
Returns a container's name.

# Returns
Zero on success, non-zero if error.
*/

forward SetContainerName(containerid, name[]);
/*
# Description
Updates a container's name

# Returns
Zero on success, non-zero if error.
*/

forward GetContainerSize(containerid, &size);
/*
# Description
Returns a container's item capacity.
*/

forward SetContainerSize(containerid, size);
/*
# Description
Sets a container's item capacity.

# Returns
Zero on success, non-zero if error.
*/

forward GetContainerItemCount(containerid, &count);
/*
# Description
Returns the amount of items inside a container.
*/

forward GetContainerSlotItem(containerid, slotid, &itemid);
/*
# Description
Returns the item stored in the specified slot.
*/

forward bool:IsContainerSlotUsed(containerid, slotid);
/*
# Description
Checks if a slot in a container is occupied by an item.
*/

forward bool:IsContainerFull(containerid);
/*
# Description
Checks if a container is full.
*/

forward bool:IsContainerEmpty(containerid);
/*
# Description
Checks if a container is empty.
*/

forward ContainerFitsItemType(containerid, ItemType:itemtype, &bool:result);
/*
# Description
Checks if an item type will fit into a container.
*/

forward GetContainerFreeSlots(containerid, &slots);
/*
# Description
Returns the number of free item slots in a container.
*/

forward GetItemContainer(itemid, &containerid);
/*
# Description
Returns the ID of the container that itemid is stored in.
*/

forward GetItemContainerSlot(itemid, &slot);
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

static enum E_CONTAINER_DATA {
	cnt_name[MAX_CONTAINER_NAME],
	cnt_size
}

static
	cnt_Data[MAX_CONTAINER][E_CONTAINER_DATA],
	cnt_Items[MAX_CONTAINER][MAX_CONTAINER_SLOTS],
	cnt_ItemContainer[MAX_ITEM] = {INVALID_CONTAINER_ID, ...},
	cnt_ItemContainerSlot[MAX_ITEM] = {-1, ...};

new
	Iterator:cnt_Index<MAX_CONTAINER>; // exported publically to facilitate iterating through containers


stock CreateContainer(name[], size) {
	new id = Iter_Free(cnt_Index);

	if(id == ITER_NONE) {
		err("container limit reached");
		return INVALID_CONTAINER_ID;
	}

	if(size > MAX_CONTAINER_SLOTS || size <= 0) {
		err("attempt to create a container with invalid size",
			_i("size", size));

		return INVALID_CONTAINER_ID;
	}

	cnt_Data[id][cnt_name][0] = EOS;
	strcat(cnt_Data[id][cnt_name], name, MAX_CONTAINER_NAME);
	cnt_Data[id][cnt_size] = size;

	for(new i; i < MAX_CONTAINER_SLOTS; i++) {
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

	return 0;
}

stock AddItemToContainer(containerid, itemid, playerid = INVALID_PLAYER_ID) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		err("attempt to add item to nonexistent container");
		return -1;
	}

	if(!IsValidItem(itemid)) {
		err("attempt to add nonexistent item to container");
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
			err("attempt to add an item to a container that it is already in");
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

	if(cnt_ItemContainer[itemid] != INVALID_CONTAINER_ID) {
		if(!RemoveItemFromContainer(cnt_ItemContainer[itemid], cnt_ItemContainerSlot[itemid])) {
			err("failed to remove item from the container it was already in");
			return -5;
		}
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

stock bool:IsValidContainer(containerid) {
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
	strcat(name, cnt_Data[containerid][cnt_name], MAX_CONTAINER_NAME);
	return 0;
}

stock SetContainerName(containerid, name[]) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return 1;
	}

	cnt_Data[containerid][cnt_name][0] = EOS;
	strcat(cnt_Data[containerid][cnt_name], name, MAX_CONTAINER_NAME);
	return 0;
}

// cnt_size
stock GetContainerSize(containerid, &size) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return 1;
	}

	size =cnt_Data[containerid][cnt_size];
	return 0;
}

stock SetContainerSize(containerid, size) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return 1;
	}

	cnt_Data[containerid][cnt_size] = size;
	return 0;
}

stock GetContainerItemCount(containerid, &count) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return 1;
	}

	count = 0;
	while(count < cnt_Data[containerid][cnt_size]) {
		if(!IsValidItem(cnt_Items[containerid][count])) {
			break;
		}

		count++;
	}

	return 0;
}

// cnt_Items
stock GetContainerSlotItem(containerid, slotid, &itemid) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return 1;
	}

	if(!(0 <= slotid < MAX_CONTAINER_SLOTS)) {
		return 2;
	}

	itemid = cnt_Items[containerid][slotid];

	return 0;
}

stock bool:IsContainerSlotUsed(containerid, slotid) {
	if(!(0 <= slotid < MAX_CONTAINER_SLOTS)) {
		return false;
	}

	if(!IsValidItem(cnt_Items[containerid][slotid])) {
		return false;
	}

	return true;
}

stock bool:IsContainerFull(containerid) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return false;
	}

	new slots, ret;
	ret = GetContainerFreeSlots(containerid, slots);
	if(ret != 0) {
		err("GetContainerFreeSlots failed",
			_i("ret", ret));
		return false;
	}

	return slots == 0;
}

stock bool:IsContainerEmpty(containerid) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return false;
	}

	return !IsValidItem(cnt_Items[containerid][0]);
}

stock ContainerFitsItemType(containerid, ItemType:itemtype, &bool:result) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return 1;
	}

	if(!IsValidItemType(itemtype)) {
		return 2;
	}

	new
		slots,
		idx;

	while(idx < cnt_Data[containerid][cnt_size]) {
		if(!IsValidItem(cnt_Items[containerid][idx])) {
			break;
		}

		slots += GetItemTypeSize(GetItemType(cnt_Items[containerid][idx]));
		idx++;
	}

	if(slots + GetItemTypeSize(itemtype) > cnt_Data[containerid][cnt_size]) {
		result = false;
	} else {
		result = true;
	}

	return 0;
}

stock GetContainerFreeSlots(containerid, &slots) {
	if(!Iter_Contains(cnt_Index, containerid)) {
		return 1;
	}

	new
		idx;

	while(idx < cnt_Data[containerid][cnt_size]) {
		if(!IsValidItem(cnt_Items[containerid][idx])) {
			break;
		}

		slots += GetItemTypeSize(GetItemType(cnt_Items[containerid][idx]));
		idx++;
	}

	slots = cnt_Data[containerid][cnt_size] - slots;
	return 0;
}

stock GetItemContainer(itemid, &containerid) {
	if(!IsValidItem(itemid)) {
		return 1;
	}

	containerid = cnt_ItemContainer[itemid];
	return 0;
}

stock GetItemContainerSlot(itemid, &slot) {
	if(!IsValidItem(itemid)) {
		return 1;
	}

	slot = cnt_ItemContainerSlot[itemid];
	return 0;
}


/*==============================================================================

	Internal Functions and Hooks

==============================================================================*/


hook OnItemDestroy(itemid) {
	if(cnt_ItemContainer[itemid] != INVALID_CONTAINER_ID) {
		RemoveItemFromContainer(cnt_ItemContainer[itemid], cnt_ItemContainerSlot[itemid]);
	}
}

hook OnItemCreateInWorld(itemid) {
	if(cnt_ItemContainer[itemid] != INVALID_CONTAINER_ID) {
		RemoveItemFromContainer(cnt_ItemContainer[itemid], cnt_ItemContainerSlot[itemid]);
	}
}
