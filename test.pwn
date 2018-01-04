#include "container.inc"

#define RUN_TESTS
#include <YSI\y_testing>


Test:CreateDestroy() {
	new containerid = CreateContainer("test", 5);
	ASSERT(IsValidContainer(containerid));
	ASSERT(DestroyContainer(containerid, true) == 0);
	ASSERT(!IsValidContainer(containerid));
}

Test:ContainerAttributes() {
	new containerid = CreateContainer("test", 5);

	new name[MAX_CONTAINER_NAME];
	ASSERT(GetContainerName(containerid, name) == 0);
	ASSERT(!strcmp(name, "test"));

	ASSERT(SetContainerName(containerid, "rename") == 0);

	ASSERT(GetContainerName(containerid, name) == 0);
	ASSERT(!strcmp(name, "rename"));

	new size;
	ASSERT(GetContainerSize(containerid, size) == 0);
	ASSERT(size == 5);

	ASSERT(SetContainerSize(containerid, 10) == 0);

	ASSERT(GetContainerSize(containerid, size) == 0);
	ASSERT(size == 10);

	new count;
	ASSERT(GetContainerItemCount(containerid, count) == 0);
	ASSERT(count == 0);
}

Test:ItemOperations() {
	new ItemType:itemType = DefineItemType("Medkit", "Medkit", 11736, 1, 0.0, 0.0, 0.0, 0.004, 0.197999, 0.038000, 0.021000,  79.700012, 0.000000, 90.899978);
	new containerid = CreateContainer("test", 5);
	new slot0 = CreateItem(itemType);

	new count;
	ASSERT(GetContainerItemCount(containerid, count) == 0);
	ASSERT(count == 0);

	ASSERT(IsContainerEmpty(containerid));

	ASSERT(AddItemToContainer(containerid, slot0) == 0); // slot 0

	ASSERT(GetContainerItemCount(containerid, count) == 0);
	ASSERT(count == 1);

	new slot1 = CreateItem(itemType);
	ASSERT(AddItemToContainer(containerid, slot1) == 0); // slot 1
	new slot2 = CreateItem(itemType);
	ASSERT(AddItemToContainer(containerid, slot2) == 0); // slot 2
	new slot3 = CreateItem(itemType);
	ASSERT(AddItemToContainer(containerid, slot3) == 0); // slot 3

	new bool:fits;
	ASSERT(ContainerFitsItemType(containerid, itemType, fits) == 0);
	ASSERT(fits == true);

	ASSERT(!IsContainerFull(containerid));

	new slot4 = CreateItem(itemType);
	ASSERT(AddItemToContainer(containerid, slot4) == 0); // slot 4 (maximum)

	ASSERT(ContainerFitsItemType(containerid, itemType, fits) == 0);
	ASSERT(fits == false);

	ASSERT(IsContainerFull(containerid));

	new noSlot = CreateItem(itemType);
	ASSERT(AddItemToContainer(containerid, noSlot) == 1); // does not fit, returns amount of required slots as positive integer

	// make sure all the slots contain the items we expect
	new slotItem;

	ASSERT(GetContainerSlotItem(containerid, 0, slotItem) == 0);
	ASSERT(slotItem == slot0);

	ASSERT(GetContainerSlotItem(containerid, 1, slotItem) == 0);
	ASSERT(slotItem == slot1);

	ASSERT(GetContainerSlotItem(containerid, 2, slotItem) == 0);
	ASSERT(slotItem == slot2);

	ASSERT(GetContainerSlotItem(containerid, 3, slotItem) == 0);
	ASSERT(slotItem == slot3);

	ASSERT(GetContainerSlotItem(containerid, 4, slotItem) == 0);
	ASSERT(slotItem == slot4);

	// removing item from slot 0 should shift every other item down a slot
	ASSERT(RemoveItemFromContainer(containerid, 0) == 0);

	// make sure all the items moved down a slot
	ASSERT(GetContainerSlotItem(containerid, 0, slotItem) == 0);
	ASSERT(slotItem == slot1);

	ASSERT(GetContainerSlotItem(containerid, 1, slotItem) == 0);
	ASSERT(slotItem == slot2);

	ASSERT(GetContainerSlotItem(containerid, 2, slotItem) == 0);
	ASSERT(slotItem == slot3);

	ASSERT(GetContainerSlotItem(containerid, 3, slotItem) == 0);
	ASSERT(slotItem == slot4);

	ASSERT(GetContainerSlotItem(containerid, 4, slotItem) == 0);
	ASSERT(slotItem == INVALID_ITEM_ID);

	ASSERT(IsContainerSlotUsed(containerid, 0));
	ASSERT(!IsContainerSlotUsed(containerid, 4));

	new gotContainer;
	ASSERT(GetItemContainer(slot4, gotContainer) == 0);
	ASSERT(gotContainer == containerid);

	new gotSlot;
	ASSERT(GetItemContainerSlot(slot4, gotSlot) == 0);
	ASSERT(gotSlot == 3);
}

main() {
	//
}

// GetItemContainerSlot(itemid, &slot);


public OnContainerCreate(containerid) {
	log("OnContainerCreate",
		_i("containerid", containerid));
}

public OnContainerDestroy(containerid) {
	log("OnContainerDestroy",
		_i("containerid", containerid));
}

public OnItemAddToContainer(containerid, itemid, playerid) {
	log("OnItemAddToContainer",
		_i("containerid", containerid),
		_i("itemid", itemid),
		_i("playerid", playerid));
}

public OnItemAddedToContainer(containerid, itemid, playerid) {
	log("OnItemAddedToContainer",
		_i("containerid", containerid),
		_i("itemid", itemid),
		_i("playerid", playerid));
}

public OnItemRemoveFromContainer(containerid, slotid, playerid) {
	log("OnItemRemoveFromContainer",
		_i("containerid", containerid),
		_i("slotid", slotid),
		_i("playerid", playerid));
}

public OnItemRemovedFromContainer(containerid, slotid, playerid) {
	log("OnItemRemovedFromContainer",
		_i("containerid", containerid),
		_i("slotid", slotid),
		_i("playerid", playerid));
}

