
layout = new LinearLayout(plasmoid);
video = new VideoWidget();
video.usedControls = video.DefaultControls;
layout.addItem(video);
video.url = startupArguments[0];
video.play();
