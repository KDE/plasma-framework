#include <QString>
#include <QProcess>
#include <klocale.h>

#include "lsof.h"

struct KLsofWidgetPrivate {
	qlonglong pid;
	QProcess *process;
};

KLsofWidget::KLsofWidget(QWidget *parent) : QTreeWidget(parent), d(new KLsofWidgetPrivate)
{
	d->pid = -1;
	setColumnCount(3);
	setUniformRowHeights(true);
	setRootIsDecorated(false);
	setItemsExpandable(false);
	setSortingEnabled(true);
	setAllColumnsShowFocus(true);
	setHeaderLabels(QStringList() << i18nc("Short for File Descriptor", "FD") << i18n("Type") << i18n("Object"));
	d->process = new QProcess(this);
	connect(d->process, SIGNAL(finished ( int, QProcess::ExitStatus)), this, SLOT(finished(int, QProcess::ExitStatus)));
}

KLsofWidget::~KLsofWidget()
{
        delete d;
}

qlonglong KLsofWidget::pid() const
{
	return d->pid;
}

void KLsofWidget::setPid(qlonglong pid) {
	d->pid = pid;
	update();
}

bool KLsofWidget::update()
{
	clear();
	QStringList args;
	d->process->waitForFinished();
	args << "-Fftn";
	if(d->pid > 0)
		args << ("-p" + QString::number(d->pid));
	d->process->start("lsof", args);
	return true;
}

void KLsofWidget::finished ( int exitCode, QProcess::ExitStatus exitStatus ) 
{
	char buf[1024];
	QTreeWidgetItem *process = NULL;
	while(true) {
		qint64 lineLength = d->process->readLine(buf, sizeof(buf));

		if(lineLength <= 0)
			break;
		if(buf[lineLength-1] == '\n')
			lineLength--;

		switch(buf[0]) {
			/* Process related stuff */
			case 'f':
				process = new QTreeWidgetItem(this);
				process->setText(0,QString::fromUtf8(buf+1, lineLength - 1));
				break;
			case 't':
				if(process)
					process->setText(1,QString::fromUtf8(buf+1, lineLength - 1));
				break;

			case 'n':
				if(process)
					process->setText(2,QString::fromUtf8(buf+1, lineLength - 1));
				break;
			default:
				break;
		}
	}
}

#include "lsof.moc"
