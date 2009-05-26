plasmoid.configChanged = function()
{
    plasmoid.activeConfig = "main";
    print("Configuration changed: " + plasmoid.readConfig("Test"));
}

print(plasmoid.readConfig("Test"));
plasmoid.activeConfig = "secondary";
print(plasmoid.activeConfig);
print(plasmoid.readConfig("Test"));
plasmoid.writeConfig("Test", "A different text!");
print(plasmoid.readConfig("Test"));

