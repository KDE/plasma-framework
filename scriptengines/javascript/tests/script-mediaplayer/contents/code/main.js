
layout = new LinearLayout(plasmoid);
video = new VideoWidget();
layout.addItem(video);
video.url = startupArguments[0];
video.play();


