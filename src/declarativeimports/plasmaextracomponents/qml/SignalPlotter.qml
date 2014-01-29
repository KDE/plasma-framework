/*
 * Copyright 2013  Bhushan Shah <bhush94@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0

Canvas {

	id: graphCanvas

	property variant samples

	property bool showAxis: true
	property bool showHorizontalLines: true

	property bool smooth: false

	property int graphPadding: 10  //Replace with units later
	property int max: 0

	Timer {
        interval: 500
        running: true
        repeat: true
        onTriggered: {
			var r = Math.ceil (Math.random () * 100);
			addSample(r);
		}

    }

	QtObject {
		id: internal
		property int availableVSpace: height - (2*graphPadding)
		property int availableHSpace: width
		property int sampleCount: availableHSpace / graphPadding
	}

	function addSample(sample) {
		var temp = samples;
		if (temp == undefined) {
			temp = new Array();
		}
		temp.push(sample);
		samples = temp;
		//Assume first sample is 50% and adjust accordingly.
		if (max == 0) {
			max = sample*2;
		}
		if (sample > max) {
			max = sample;
		}
		requestPaint();
	}

	function drawGraph(context) {

		//print(internal.sampleCount);
		// Draw Axis
		if (showAxis) {
			context.beginPath();
			context.strokeStyle = "rgb(150,150,150)"; //TODO: use from theme
			context.lineWidth = 2;
			context.moveTo(0, height-graphPadding);
			//context.lineTo(graphPadding, height - graphPadding);
			context.lineTo(width, height - graphPadding);
			context.stroke();
			context.closePath();
		}

		// Draw horizontal lines
		if (showHorizontalLines) {
			context.beginPath();
			context.lineWidth = 1;
			context.strokeStyle = "rgb(150, 150, 150)" //TODO: use from theme
			for (var y = 0; y < height - graphPadding; y += height/10) {
				context.moveTo(0, y);
				context.lineTo(width, y);
			}
			context.stroke();
			context.closePath();
		}

		// Time to draw graph
		var graphSamples;
		graphSamples = samples;
		if (graphSamples == undefined)
			graphSamples = new Array();
		if (graphSamples.length != 0) {
			if(smooth && graphPadding > 5) {
			}
			else {

				var xPos = (graphSamples.length < internal.sampleCount) ? (internal.availableHSpace - (graphSamples.length * graphPadding)) : 0 - (2*graphPadding);
				context.beginPath();
				context.moveTo(xPos, height - graphPadding);
				var yPos0 = (internal.availableVSpace * (1 / 2));
				context.lineTo(xPos, yPos0);
				xPos += graphPadding;

				var loopInit = (graphSamples.length < internal.sampleCount) ? 0 : (graphSamples.length - internal.sampleCount) - 2;
				for(var i = loopInit; i < graphSamples.length ; i++){
					var yPer = (graphSamples[i] * 100) / max;
					//print(yPer);
					//print(internal.availableVSpace);
					var yPos = (internal.availableVSpace * (yPer / 100));
					context.lineTo(xPos, yPos);
					xPos += graphPadding;
				}
				context.lineTo(xPos, height-graphPadding);
				context.stroke();
				context.fillStyle = '#8ED6FF';
				context.fill();
			}
		} else {
			print("No samples yet");
		}

	}

	onPaint: {

		var ctx = getContext("2d");

		//TODO: from theme
		var grad = ctx.createLinearGradient(0, 0, 0, height);
		grad.addColorStop(0,   '#aaa');
		grad.addColorStop(1, '#fff');
		ctx.fillStyle = grad;
		ctx.fillRect(0, 0, width, height);

		drawGraph(ctx);

	}


}