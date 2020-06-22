function onClick(entityID, event) {
    if (event.isSecondaryHeld && event.isPrimaryHeld) {
        var entity = Entities.getEntityProperties(entityID, ["renderWithZones"]);
        var renderWithZones = entity.renderWithZones;
        var renderWithZonesLength = entity.renderWithZones.length;
        var askForUuid = Window.prompt("Enter zone uuid", "");
        if (!askForUuid == "") {
            renderWithZones[renderWithZonesLength] = askForUuid;
            Entities.editEntity(entityID, {
                renderWithZones: renderWithZones
            });
        }
    }

};
Entities.clickDownOnEntity.connect(onClick);
Script.scriptEnding(function () {
    Entities.clickDownOnEntity.disconnect(onClick);
});