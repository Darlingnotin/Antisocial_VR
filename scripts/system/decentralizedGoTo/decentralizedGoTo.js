//  explore.js
//
//  Created by Darlingnotin in 2019.
//  Copyright 2019 Darling
//
//  App maintained in: https://github.com/kasenvr/Decentralized_GoTo_Experimental
//  App copied to: https://github.com/kasenvr/project-athena
//
//  Distributed under the ISC license.
//  See the accompanying file LICENSE or https://opensource.org/licenses/ISC

(function () {
    var defaultGoToJSON = "http://goto.darlingvr.net:8081/goto.json";
    
    var tablet = Tablet.getTablet("com.highfidelity.interface.tablet.system");
    Menu.menuItemEvent.connect(onMenuItemEvent);
    var AppUi = Script.require('appUi');
    var goToAddresses;
    var permission;
    Menu.addMenu("GoTo");
    Menu.addMenuItem("GoTo", "Subscribe to new GoTo provider");
    Menu.addMenu("GoTo > Unsubscribe from GoTo provider");
    var goToAddress = Settings.getValue("DarlingVRGoTo", "");
    for (var i = 0; i < goToAddress.length; i++) {
        Menu.addMenuItem("GoTo > Unsubscribe from GoTo provider", goToAddress[i]);
    }
    var ui;
    function startup() {
        goToAddress = Settings.getValue("DarlingVRGoTo", "");
        if (goToAddress == "") {
            var initialGoToList = Script.resolvePath(defaultGoToJSON);
            Menu.addMenuItem("GoTo > Unsubscribe from GoTo provider", initialGoToList);
            goToAddressNow = [
                initialGoToList
            ];
            Settings.setValue("DarlingVRGoTo", goToAddressNow);
        }

        var scriptDir = Script.resolvePath("");
        scriptDir = scriptDir.slice(0, scriptDir.lastIndexOf("/") + 1);

        ui = new AppUi({
            buttonName: "GoTo",
            home: Script.resolvePath("decentralizedGoTo.html"),
            graphicsDirectory: scriptDir
        });
    }

    function onWebEventReceived(event) {
        messageData = JSON.parse(event);
        if (messageData.action == "requestAddressList") {
            goToAddresses = Settings.getValue("DarlingVRGoTo", "");
            for (var i = 0; i < goToAddresses.length; i++) {

                try {
                    Script.require(goToAddresses[i] + "?" + Date.now());
                }
                catch (e) {
                    goToAddresses.remove(goToAddresses[i]);
                }
            }
            var children = goToAddresses
                .map(function (url) { return Script.require(url + '?' + Date.now()); })
                .reduce(function (result, someChildren) { return result.concat(someChildren); }, []);

            for (var z = 0; z < children.length; z++) {
                delete children[z].age;
                delete children[z].id;
                children[z]["Domain Name"] = children[z]["Domain Name"].replace(/</g, '&lt;').replace(/>/g, '&lt;');
                children[z].Owner = children[z].Owner.replace(/</g, '&lt;').replace(/>/g, '&lt;');
            }

            permission = Entities.canRez()

            var readyEvent = {
                "action": "addressList",
                "myAddress": children,
                "permission": permission
            };

            tablet.emitScriptEvent(JSON.stringify(readyEvent));

        } else if (messageData.action == "goToUrl") {
            Window.location = messageData.visit;
        } else if (messageData.action == "addLocation") {

            var locationBoxUserData = {
                owner: messageData.owner,
                domainName: messageData.domainName,
                port: messageData.Port,
                portForward: messageData.portForward,
                ipAddress: null,
                avatarCountRadius: null,
                customPath: null,
                grabbableKey: {
                    grabbable: false
                }
            };
            
            var locationBoxName = "Explore Marker (" + messageData.domainName + ")";

            locationboxID = Entities.addEntity({
                position: Vec3.sum(MyAvatar.position, Quat.getFront(MyAvatar.orientation)),
                userData: JSON.stringify(locationBoxUserData),
                serverScripts: messageData.script,
                color: { red: 255, green: 0, blue: 0 },
                type: "Box",
                name: locationBoxName,
                collisionless: true,
                grabbable: false
            });
        } else if (messageData.action == "retrievePortInformation") {
            var readyEvent = {
                "action": "retrievePortInformationResponse",
                "goToAddresses": goToAddresses
            };

            tablet.emitScriptEvent(JSON.stringify(readyEvent));
            
        }
    }

    function onMenuItemEvent(menuItem) {
        var menuItemList = JSON.stringify(Settings.getValue("DarlingVRGoTo", ""));
        var menuItemExists = menuItemList.indexOf(menuItem) !== -1;
        if (menuItem == "Subscribe to new GoTo provider") {
            goToAddress = Settings.getValue("DarlingVRGoTo", "");
            var arrayLength = goToAddress.length;
            var prom = Window.prompt("Enter URL to GoTo JSON.", "");
            if (prom) {
                goToAddress[arrayLength] = prom;
                Settings.setValue("DarlingVRGoTo", goToAddress);
                Menu.addMenuItem("GoTo > Unsubscribe from GoTo provider", prom);
            }
        } else if (menuItemExists) {
            goToAddresses = Settings.getValue("DarlingVRGoTo", "");
            Menu.removeMenuItem("GoTo > Unsubscribe from GoTo provider", menuItem);
            goToAddresses.remove(menuItem);
            Settings.setValue("DarlingVRGoTo", goToAddresses);
        }
    }

    Array.prototype.remove = function () {
        var what, a = arguments, L = a.length, ax;
        while (L && this.length) {
            what = a[--L];
            while ((ax = this.indexOf(what)) !== -1) {
                this.splice(ax, 1);
            }
        }
        return this;
    };

    tablet.webEventReceived.connect(onWebEventReceived);
    startup();

    Script.scriptEnding.connect(function () {
        Messages.unsubscribe("goTo");
        Menu.removeMenu("GoTo");
        tablet.webEventReceived.disconnect(onWebEventReceived);
        Menu.menuItemEvent.disconnect(onMenuItemEvent);
    });
}());
