engine = dataEngine("nowplaying");
players = engine.sources;
watchingPlayer = players[0];
controller = service("nowplaying", watchingPlayer);

plasmoid.dataUpdate = function(a, b)
{
    label.text = "Playing " + b.Title + " by " + b.Artist + ". time: " +
                 Math.floor(b.Position/60) + ":" + (parseInt(b.Position)%60);
    progress.value = 100*b.Position/b.Length;
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
layout.setOrientation(QtVertical);
label = new Label();
layout.addItem(label);

stop = new PushButton();
stop.text = "Stop";
layout.addItem(stop);

controller.associateWidget(stop, "stop");

stop["clicked()"].connect(plasmoid.stop);

progress = new Slider();
progress.orientation = QtHorizontal;
layout.addItem(progress);
controller.associateWidget(progress, "progress");

progress["sliderMoved(int)"].connect(plasmoid.setProgress);

engine.connectSource(watchingPlayer, plasmoid, 500);
