print(plasmoid.readConfig("Test"));
plasmoid.activeConfig = "secondary";
print(plasmoid.activeConfig);
print(plasmoid.readConfig("Test"));
plasmoid.writeConfig("Test", "You should see this!");
print(plasmoid.readConfig("Test"));

