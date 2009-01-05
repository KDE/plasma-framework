engine = dataEngine("nowplaying");
players = engine.sources;
watchingPlayer = players[0];
controller = service("nowplaying", watchingPlayer);

plasmoid.dataUpdate = function(a, b)
{
    label.text = "Playing " + b.Title + " by " + b.Artist + ". time: " +
                 parseInt(b.Position/60) + ":" + (parseInt(b.Position)%60);
}

plasmoid.stop = function()
{
    data = controler.operationDescription("stop");
    print(data+controller.name());
    for ( var i in data ) {
        print(i + ' -> ' + data[i] );
    }

    controller.startOperationCall(controller.operationDescription("stop"));
    print("stopping");
}

layout = new LinearLayout(plasmoid);
layout.orientation = Vertical;
label = new Label();
layout.addItem(label);

stop = new PushButton();
stop.text = "Stop";
layout.addItem(stop);

controller.associateWidget(stop, "stop");
print(controller.operationNames());

stop["clicked()"].connect(plasmoid.stop);

engine.connectSource(watchingPlayer, plasmoid, 500);
