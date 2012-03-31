function placeEditBubble(mouse) {
    // Find the root item, then map our cursor position to it
    // in order to check if the edit bubble could end up off-screen
    var rootItem = parent;
    while (rootItem.parent) {
        rootItem = rootItem.parent;
    }
    var distanceToTop = mouseEventListener.mapToItem(rootItem, mouse.x, mouse.y);
    //print( "   distanceToTop: " + distanceToTop.x);
    if (distanceToTop.x < editBubble.width/2) {
        // hitting the left edge
        //editBubble.x = mouse.x

    } else {
        editBubble.x = mouse.x-(editBubble.width/2)
    }
    if (distanceToTop.y > editBubble.height) {
        editBubble.y = mouse.y-editBubble.height-8
    } else {
        //editBubble.y = mouse.y-(editBubble.width/2)
    }

}
