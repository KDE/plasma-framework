// because we put the following line in the metadata.desktop file, we have access
// to the HTTP extension in this Plasmoid.
//
// X-Plasma-RequiredExtensions=http
//
// More documentation can be found here:
//
// http://techbase.kde.org/Development/Tutorials/Plasma/JavaScript/API#Extensions

output = new TextEdit
output.readOnly = true

layout = new LinearLayout
layout.orientation = QtVertical
layout.addItem(output)

// in case our request for HTTP urls in the metadata.desktop was rejected (e.g. due
// to security restrictions) we won't have a plasmoid.get, so let's check for it
// before using it!
if (plasmoid.getUrl) {
    var getJob = plasmoid.getUrl("http://dot.kde.org/rss.xml");
    function recv(job, data)
    {
        if (job == getJob) {
            print("we have our job")
            if (data.length) {
                output.append(data.toUtf8())
            }
        }
    }

    function fini(job)
    {
        if (job == getJob) {
            print("our job is finished")
        } else {
            print("some other job is finished?")
        }
    }

    getJob.data.connect(recv)
    getJob.finished.connect(fini)
} else {
    output.text = i18n("HTTP access denied!")
}
