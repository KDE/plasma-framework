/*  This file is part of the KDE project
    Copyright (C) 2007 John Tapsell <tapsell@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/


#include <QtTest>
#include <QtCore>

#include <klocale.h>
#include <qtest_kde.h>
#include <kdebug.h>

#include "processcore/processes.h"
#include "processcore/process.h"

#include "processtest.h"

void testProcess::testProcesses() {
	KSysGuard::Processes *processController = KSysGuard::Processes::getInstance();
	processController->updateAllProcesses();
	QList<KSysGuard::Process *> processes = processController->getAllProcesses();
	QSet<long> pids;
	foreach( KSysGuard::Process *process, processes) {
		if(process->pid == 0) continue;
		QVERIFY(process->pid > 0);
		QVERIFY(!process->name.isEmpty());

		//test all the pids are unique
		QVERIFY(!pids.contains(process->pid));
		pids.insert(process->pid);
	}
	processController->updateAllProcesses();
	QList<KSysGuard::Process *> processes2 = processController->getAllProcesses();
	foreach( KSysGuard::Process *process, processes2) {
		if(process->pid == 0) continue;
		QVERIFY(process->pid > 0);
		QVERIFY(!process->name.isEmpty());

		//test all the pids are unique
		if(!pids.contains(process->pid)) {
			kDebug() << process->pid << " not found. " << process->name;
		}
		pids.remove(process->pid);
	}

	QVERIFY(processes2.size() == processes.size());
	QCOMPARE(processes, processes2); //Make sure calling it twice gives the same results.  The difference in time is so small that it really shouldn't have changed
}


unsigned long testProcess::countNumChildren(KSysGuard::Process *p) {
	unsigned long total = p->children.size();
	for(int i =0; i < p->children.size(); i++) {
		total += countNumChildren(p->children[i]);
	}
	return total;
}

void testProcess::testProcessesTreeStructure() {
	KSysGuard::Processes *processController = KSysGuard::Processes::getInstance();
	processController->updateAllProcesses();
	QList<KSysGuard::Process *> processes = processController->getAllProcesses();
	
	foreach( KSysGuard::Process *process, processes) {
		QCOMPARE(countNumChildren(process), process->numChildren);

                for(int i =0; i < process->children.size(); i++) {
			QVERIFY(process->children[i]->parent);
			QCOMPARE(process->children[i]->parent, process);
		}
	}

}

void testProcess::testProcessesModification() {
	//We will modify the tree, then re-call getProcesses and make sure that it fixed everything we modified
	KSysGuard::Processes *processController = KSysGuard::Processes::getInstance();
	processController->updateAllProcesses();
	KSysGuard::Process *initProcess = processController->getProcess(1);

	if(!initProcess || initProcess->numChildren < 3)
		return;

	QVERIFY(initProcess);
	QVERIFY(initProcess->children[0]);
	QVERIFY(initProcess->children[1]);
	kDebug() << initProcess->numChildren;
	initProcess->children[0]->parent = initProcess->children[1];
	initProcess->children[1]->children.append(initProcess->children[0]);
	initProcess->children[1]->numChildren++;
	initProcess->numChildren--;
	initProcess->children.removeAt(0);
}

void testProcess::testTime() {
	//See how long it takes to get proccess information	
	KSysGuard::Processes *processController = KSysGuard::Processes::getInstance();
	QTime t;
	t.start();
	for(int i =0; i < 100; i++) 
	  processController->updateAllProcesses();
	kDebug() << "Time elapsed: "<< t.elapsed() <<" ms, so " << t.elapsed()/100 << "ms" <<  endl;
	QVERIFY(t.elapsed()/100 < 300); //It should take less than about 100ms.  Anything longer than 300ms even on a slow system really needs to be optimised
}

QTEST_KDEMAIN_CORE(testProcess)

#include "processtest.moc"

