(function () {
    Menu.menuItemEvent.connect(onMenuItemEvent);
    Menu.addMenuItem("Settings", "Subscribe to new metaverse");
    Menu.addMenuItem("Settings", "Return to original metaverse");
    var MetaverseURLInSettings = Settings.getValue("metaverse/selectedMetaverseURL");
    if (MetaverseURLInSettings == "") {
        Settings.setValue("metaverse/selectedMetaverseURL", "http://metaverse.darlingvr.net:9400");
    }

    function onMenuItemEvent(menuItem) {
        if (menuItem == "Subscribe to new metaverse") {
            MetaverseURLInSettings = Settings.getValue("metaverse/selectedMetaverseURL");
            var metaverseUrl = Window.prompt("Enter metaverse URL", MetaverseURLInSettings);
            if (metaverseUrl) {
                Settings.setValue("metaverse/selectedMetaverseURL", metaverseUrl);
            }
        } else if (menuItem == "Return to original metaverse") {
            Settings.setValue("metaverse/selectedMetaverseURL", "http://metaverse.darlingvr.net:9400");
        }
    }

    Script.scriptEnding.connect(function () {
        Menu.removeMenuItem("Settings", "Subscribe to new metaverse");
        Menu.removeMenuItem("Settings", "Return to original metaverse");
    });
}());
