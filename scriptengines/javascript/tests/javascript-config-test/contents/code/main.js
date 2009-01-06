print(plasmoid.readConfig("Test"));
plasmoid.activeConfig = "secondary";
print(plasmoid.activeConfig);
print(plasmoid.readConfig("Test"));

