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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Canvas {

	id: graphCanvas

	property variant samples

	property bool showAxis: true
	property bool showHorizontalLines: true

	property bool smooth: true

	property bool drawBackGround: false
	property color backGroundColor: theme.buttonBackgroundColor
	property color plotColor: theme.highlightColor

	property int graphPadding: 10  //Replace with units later
	property int max: 0

	QtObject {
		id: internal
		property int availableVSpace: height - (2*graphPadding)
		property int availableHSpace: width
		property int sampleCount: availableHSpace / graphPadding
	}

	PlasmaComponents.Label {
		id: sampleLabel
		anchors.horizontalCenter: graphCanvas.horizontalCenter
	}

	function addSample(sample) {
		var temp = samples;
		if (temp == undefined) {
			temp = new Array();
		}
		temp.push(sample);
		samples = temp;
		requestPaint();
	}

	function getYPos(sample) {
		var yPer = (sample * 100) / max;
		var yPos = (internal.availableVSpace * ((100-yPer) / 100));
		return yPos;
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
			for (var y = 0; y < height - graphPadding; y += (height-graphPadding)/10) {
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

			var xPos = (graphSamples.length < internal.sampleCount) ? (internal.availableHSpace - (graphSamples.length * graphPadding)) : 0 - (2*graphPadding);
			if(smooth && graphPadding > 5) {

				context.beginPath();
				context.moveTo(xPos, height - graphPadding);
				var loopInit = (graphSamples.length < internal.sampleCount) ? 0 : (graphSamples.length - internal.sampleCount) - 3;
				var yPos0 = (getYPos(graphSamples[loopInit]));
				context.lineTo(xPos, yPos0);
				xPos += graphPadding;

				for (var i = loopInit; i < graphSamples.length; i ++)
				{
					var xc = ((xPos * 2) + graphPadding ) / 2;
					var yc = (getYPos(graphSamples[i]) + getYPos(graphSamples[i + 1])) / 2;
					context.quadraticCurveTo(xPos, getYPos(graphSamples[i]), xc, yc);
					xPos += graphPadding;
				}
				context.quadraticCurveTo(width+graphPadding, height-graphPadding, width - graphPadding,height -graphPadding);
				context.stroke();
				context.fillStyle = plotColor;
				context.fill();
			}
			else {
				context.beginPath();
				context.moveTo(xPos, height - graphPadding);
				var loopInit = (graphSamples.length < internal.sampleCount) ? 0 : (graphSamples.length - internal.sampleCount) - 2;
				var yPos0 = (getYPos(graphSamples[loopInit]));
				context.lineTo(xPos, yPos0);
				xPos += graphPadding;
				for(var i = loopInit; i < graphSamples.length ; i++){
					context.lineTo(xPos, getYPos(graphSamples[i]));
					xPos += graphPadding;
				}
				context.lineTo(xPos, height-graphPadding);
				context.stroke();
				context.fillStyle = plotColor;
				context.fill();
			}
			sampleLabel.text = graphSamples[graphSamples.length-3]
		} else {
			print("No samples yet");
		}

	}

	onPaint: {

		var ctx = getContext("2d");

		ctx.clearRect(0, 0, width, height)

		if (drawBackGround) {
			var gradient = ctx.createLinearGradient(0, 0, 0, height);
			gradient.addColorStop(0, backGroundColor);
			ctx.fillStyle = gradient;
			ctx.fillRect(0, 0, width, height);
		}

		drawGraph(ctx);

	}


}