plasmoid.dataUpdate = function(a, b)
{
    for ( var i in b ) {
    print(i + ' -> ' + b[i] );
  }
    label.text = "Playing " + b.Title + " by " + b.Artist + ". time: " +
                 parseInt(b.Position/60) + ":" + (parseInt(b.Position)%60);
}

layout = new LinearLayout(plasmoid);
label = new Label();
layout.addItem(label);

plasmoid.dataEngine("nowplaying").connectSource("org.mpris.amarok", plasmoid, 500);