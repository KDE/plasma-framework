plasmoid.dataUpdate = function(a, b)
{
    label.text = "It is " + b.Time.toString() + " in " + a;
}

layout = new LinearLayout(plasmoid);
label = new Label();
layout.addItem(label);

print(dataEngine("time").query("UTC").Time.toString());
plasmoid.dataEngine("time").connectSource("UTC", plasmoid, 500);

