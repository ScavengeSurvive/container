#include "container.inc"

#define RUN_TESTS
#include <YSI\y_testing>


new ItemType:item_Medkit;

main() {
	item_Medkit = DefineItemType("Medkit", "Medkit", 11736, 1, 0.0, 0.0, 0.0, 0.004, 0.197999, 0.038000, 0.021000,  79.700012, 0.000000, 90.899978, .maxhitpoints = 1);

	// RemoveItemFromContainer(containerid, slotid, playerid = INVALID_PLAYER_ID, call = 1);
	// IsValidContainer(containerid);
	// GetContainerName(containerid, name[]);
	// GetContainerSize(containerid);
	// SetContainerSize(containerid, size);
	// GetContainerItemCount(containerid);
	// GetContainerSlotItem(containerid, slotid);
	// IsContainerSlotUsed(containerid, slotid);
	// IsContainerFull(containerid);
	// IsContainerEmpty(containerid);
	// WillItemTypeFitInContainer(containerid, ItemType:itemtype);
	// GetContainerFreeSlots(containerid);
	// GetItemContainer(itemid);
	// GetItemContainerSlot(itemid);
}

Test:CreateDestroy() {
	new containerid = CreateContainer("test", 5);
	ASSERT(DestroyContainer(containerid, true) == 0);
}

Test:AddItem() {
	new containerid = CreateContainer("test", 5);
	new itemid = CreateItem(item_Medkit);
	ASSERT(AddItemToContainer(containerid, itemid) == 0);
}

public OnContainerCreate(containerid) {
	//
}

public OnContainerDestroy(containerid) {
	//
}

public OnItemAddToContainer(containerid, itemid, playerid) {
	//
}

public OnItemAddedToContainer(containerid, itemid, playerid) {
	//
}

public OnItemRemoveFromContainer(containerid, slotid, playerid) {
	//
}

public OnItemRemovedFromContainer(containerid, slotid, playerid) {
	//
}

