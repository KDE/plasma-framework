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
    data = controller.operationDescription("stop");
    print(controller.name());
    for ( var i in data ) {
        print(i + ' -> ' + data[i] );
    }

    controller.startOperationCall(controller.operationDescription("stop"));
    print("stopping");
}

plasmoid.setProgress = function(progress)
{
    operation = controller.operationDescription("seek");
    operation["seconds"] = progress;
    for ( var i in operation ) {
        print(i + ' -> ' + operation[i] );
    }

    controller.startOperationCall(operation);
    print("set progress to " + progress);
}

layout = new LinearLayout(plasmoid);
layout.setOrientation(Vertical);
label = new Label();
layout.addItem(label);

stop = new PushButton();
stop.text = "Stop";
layout.addItem(stop);

controller.associateWidget(stop, "stop");

stop["clicked()"].connect(plasmoid.stop);

progress = new Slider();
progress.orientation = Horizontal;
layout.addItem(progress);
controller.associateWidget(progress, "progress");

progress["valueChanged(int)"].connect(plasmoid.setProgress);

engine.connectSource(watchingPlayer, plasmoid, 500);
