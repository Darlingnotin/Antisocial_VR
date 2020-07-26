(function () {
    Menu.menuItemEvent.connect(onMenuItemEvent);
    Menu.addMenu("Metaverse");
    Menu.addMenuItem("Metaverse", "Subscribe to new metaverse");

    function onMenuItemEvent(menuItem) {
        if (menuItem == "Subscribe to new metaverse") {
            var metaverseUrl = Window.prompt("Enter metaverse URL", "http://metaverse.darlingvr.club:9400");
            if (metaverseUrl) {
                Settings.setValue("metaverse/selectedMetaverseURL", metaverseUrl);
            }
        }
    }

    Script.scriptEnding.connect(function () {
        Menu.removeMenu("Metaverse");
    });
}());
