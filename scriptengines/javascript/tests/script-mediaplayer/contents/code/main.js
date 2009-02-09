
layout = new LinearLayout(plasmoid);
video = new VideoWidget();
layout.addItem(video);
video.file = startupArguments[0];
video.play();


