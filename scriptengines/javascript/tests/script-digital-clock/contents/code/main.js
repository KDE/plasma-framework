layout = new QGraphicsLinearLayout(Horizontal, plasmoid);

label = new Label();

plasmoid.dataEngine("time").connectSource("UTC", plasmoid, 500);
print(dataEngine("time").query("UTC").toString());

layout.addItem(label);
label.text = "test clock";
plasmoid.setLayout(layout);


plasmoid.dataUpdated = function(a, b)
{
    print("This should print time");
}
