plasmoid.dataUpdate = function(a, b)
{
    label.text = "Playing " + b.Title + " by " + b.Artist + ". time: " +
                 parseInt(b.Position/60) + ":" + (parseInt(b.Position)%60);
}

plasmoid.stop = function()
{
    print("stopping");
}

layout = new LinearLayout(plasmoid);
layout.orientation = Vertical;
label = new Label();
layout.addItem(label);

stop = new PushButton();
stop.text = "Stop";
layout.addItem(stop);

stop["clicked()"].connect(plasmoid.stop);

plasmoid.dataEngine("nowplaying").connectSource("org.mpris.amarok", plasmoid, 500);